#include "DarkLotus/Entities/Fighter.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

Fighter::Fighter(int playerIndex, const std::string& name, Color themeColor, Vector3 startPos)
    : m_playerIndex(playerIndex), m_name(name), m_themeColor(themeColor), m_position(startPos) {
}

Fighter::~Fighter() {
    if (m_hasBlenderModel) {
        ::UnloadModel(m_model);
    }
}

void Fighter::Init() {
    m_health = m_maxHealth;
    m_recoverableHealth = m_maxHealth;
    m_heatGauge = 100.0f;
    m_heatActive = false;
    m_stance = StanceState::STAND;
    m_actionState = ActionState::IDLE;
    m_velocity = Vector3{0.0f, 0.0f, 0.0f};
    m_moveFrame = 0;
    m_stunFrames = 0;
    m_hitboxEmitted = false;
    m_yaw = (m_playerIndex == 0) ? 90.0f : -90.0f;
}

void Fighter::ResetRound(Vector3 spawnPos) {
    m_position = spawnPos;
    m_velocity = Vector3{0.0f, 0.0f, 0.0f};
    m_health = m_maxHealth;
    m_recoverableHealth = m_maxHealth;
    m_heatGauge = 100.0f;
    m_heatActive = false;
    m_stance = StanceState::STAND;
    m_actionState = ActionState::IDLE;
    m_moveFrame = 0;
    m_stunFrames = 0;
    m_hitboxEmitted = false;
    m_yaw = (m_playerIndex == 0) ? 90.0f : -90.0f;
}

bool Fighter::LoadBlenderModel(const std::string& glbFilePath) {
    if (::FileExists(glbFilePath.c_str())) {
        if (m_hasBlenderModel) {
            ::UnloadModel(m_model);
        }
        m_model = ::LoadModel(glbFilePath.c_str());
        m_hasBlenderModel = true;
        Logger::Info("Fighter", m_name + " successfully loaded Blender 3D model: " + glbFilePath);
        return true;
    }
    Logger::Info("Fighter", "Blender model not found at '" + glbFilePath + "'. Using procedural 3D mannequin rig.");
    m_hasBlenderModel = false;
    return false;
}

bool Fighter::IsBlocking() const {
    // If standing or crouching in block posture
    return (m_actionState == ActionState::BLOCK_STUN) ||
           (m_actionState == ActionState::WALK_BACK);
}

void Fighter::ExecuteMove(const Hitbox3D& move) {
    if (m_actionState == ActionState::HIT_STUN ||
        m_actionState == ActionState::BLOCK_STUN ||
        m_actionState == ActionState::JUGGLED ||
        m_actionState == ActionState::KNOCKED_DOWN) {
        return; // In hitstun / air, cannot attack
    }

    m_currentAttack = move;
    m_actionState = ActionState::ATTACKING;
    m_moveFrame = 0;
    m_hitboxEmitted = false;
}

void Fighter::UpdateAttackFrames() {
    if (m_actionState != ActionState::ATTACKING) return;

    m_moveFrame++;
    int totalFrames = m_currentAttack.startupFrames + m_currentAttack.activeFrames + m_currentAttack.recoveryFrames;

    // Visual punch/kick extension curve
    int peakFrame = m_currentAttack.startupFrames + (m_currentAttack.activeFrames / 2);
    if (m_moveFrame <= peakFrame) {
        m_limbExtension = static_cast<float>(m_moveFrame) / std::max(1, peakFrame);
    } else {
        m_limbExtension = 1.0f - static_cast<float>(m_moveFrame - peakFrame) / std::max(1, totalFrames - peakFrame);
    }

    if (m_moveFrame >= totalFrames) {
        // Move recovery completed
        m_actionState = ActionState::IDLE;
        m_currentAttack.active = false;
        m_limbExtension = 0.0f;
        m_moveFrame = 0;
    }
}

const Hitbox3D* Fighter::GetActiveHitbox() const {
    if (m_actionState == ActionState::ATTACKING) {
        int activeStart = m_currentAttack.startupFrames;
        int activeEnd = m_currentAttack.startupFrames + m_currentAttack.activeFrames;
        if (m_moveFrame >= activeStart && m_moveFrame <= activeEnd) {
            return &m_currentAttack;
        }
    }
    return nullptr;
}

std::vector<BoundingBox> Fighter::GetHurtboxes() const {
    std::vector<BoundingBox> boxes;
    float crouchOffset = (m_stance == StanceState::CROUCH) ? -0.4f : 0.0f;

    // Head hurtbox
    boxes.push_back({
        Vector3{m_position.x - 0.25f, m_position.y + 1.4f + crouchOffset, m_position.z - 0.25f},
        Vector3{m_position.x + 0.25f, m_position.y + 1.85f + crouchOffset, m_position.z + 0.25f}
    });

    // Torso hurtbox
    boxes.push_back({
        Vector3{m_position.x - 0.35f, m_position.y + 0.8f + crouchOffset, m_position.z - 0.35f},
        Vector3{m_position.x + 0.35f, m_position.y + 1.4f + crouchOffset, m_position.z + 0.35f}
    });

    // Legs hurtbox
    boxes.push_back({
        Vector3{m_position.x - 0.35f, m_position.y, m_position.z - 0.35f},
        Vector3{m_position.x + 0.35f, m_position.y + 0.8f + crouchOffset, m_position.z + 0.35f}
    });

    return boxes;
}

void Fighter::UpdateAI(Fighter* opponent, unsigned int& outInputBits) {
    if (!opponent) return;

    outInputBits = BTN_NONE;
    float dist = Vector3Distance(m_position, opponent->GetPosition());

    // Basic AI decision tree
    if (dist > 3.0f) {
        // Close distance
        outInputBits |= (m_position.x < opponent->GetPosition().x) ? BTN_RIGHT : BTN_LEFT;
    } else if (dist < 1.4f && opponent->IsAttacking()) {
        // Defend or sidestep
        int roll = ::GetRandomValue(0, 100);
        if (roll < 45) {
            outInputBits |= (m_position.x < opponent->GetPosition().x) ? BTN_LEFT : BTN_RIGHT; // Guard
        } else if (roll < 70) {
            outInputBits |= BTN_UP; // Sidestep
        } else {
            outInputBits |= BTN_DOWN; // Crouch duck
        }
    } else if (dist <= 2.2f && !IsAttacking()) {
        // Attack probability
        int roll = ::GetRandomValue(0, 100);
        if (roll < 30) {
            outInputBits |= BTN_LP; // 1 Jab
        } else if (roll < 55) {
            outInputBits |= BTN_RP; // 2 Cross
        } else if (roll < 75) {
            outInputBits |= BTN_LK; // 3 Kick
        } else if (roll < 90) {
            outInputBits |= BTN_RK; // 4 Roundhouse
        } else if (IsInRage()) {
            outInputBits |= BTN_HEAT_RAGE; // Rage Art
        }
    }
}

void Fighter::Update(float deltaTime, int /*currentFrame*/, unsigned int inputBits, Fighter* opponent) {
    m_animTime += deltaTime;

    if (m_isAI) {
        UpdateAI(opponent, inputBits);
    }

    // Orient facing opponent (Tekken 3D facing lock)
    if (opponent && m_actionState != ActionState::KNOCKED_DOWN) {
        Vector3 toOpponent = Vector3Subtract(opponent->GetPosition(), m_position);
        float angleRad = std::atan2(toOpponent.z, toOpponent.x);
        m_yaw = -angleRad * RAD2DEG;
    }

    // Hitstun / Blockstun decay
    if (m_stunFrames > 0) {
        m_stunFrames--;
        if (m_stunFrames <= 0) {
            if (m_actionState == ActionState::HIT_STUN || m_actionState == ActionState::BLOCK_STUN) {
                m_actionState = ActionState::IDLE;
            }
        }
    }

    // Grounded knockdown recovery
    if (m_actionState == ActionState::KNOCKED_DOWN) {
        if (m_stunFrames <= 0) {
            m_actionState = ActionState::IDLE;
            m_position.y = 0.0f;
        }
    }

    // Update attack animation frame progression
    UpdateAttackFrames();

    // Movement & Inputs (only when free to act)
    if (m_actionState == ActionState::IDLE || m_actionState == ActionState::WALK_FORWARD ||
        m_actionState == ActionState::WALK_BACK || m_actionState == ActionState::CROUCH_IDLE) {

        float moveSpeed = 3.6f;
        float sidestepSpeed = 4.2f;
        bool isFacingRight = (opponent && opponent->GetPosition().x > m_position.x);

        // Movement Direction
        if (inputBits & BTN_UP) {
            // Sidestep Up (Z-)
            m_velocity.z = -sidestepSpeed;
            m_actionState = ActionState::SIDESTEP_LEFT;
            m_stance = StanceState::SIDESTEP_LEFT;
        } else if (inputBits & BTN_DOWN) {
            if ((inputBits & BTN_LEFT) || (inputBits & BTN_RIGHT)) {
                // Sidestep Down (Z+)
                m_velocity.z = sidestepSpeed;
                m_actionState = ActionState::SIDESTEP_RIGHT;
                m_stance = StanceState::SIDESTEP_RIGHT;
            } else {
                // Crouch duck
                m_stance = StanceState::CROUCH;
                m_actionState = ActionState::CROUCH_IDLE;
                m_velocity.x = 0.0f;
                m_velocity.z = 0.0f;
            }
        } else {
            m_stance = StanceState::STAND;
            m_velocity.z = 0.0f;

            if (inputBits & BTN_LEFT) {
                m_velocity.x = -moveSpeed;
                m_actionState = isFacingRight ? ActionState::WALK_BACK : ActionState::WALK_FORWARD;
            } else if (inputBits & BTN_RIGHT) {
                m_velocity.x = moveSpeed;
                m_actionState = isFacingRight ? ActionState::WALK_FORWARD : ActionState::WALK_BACK;
            } else {
                m_velocity.x = 0.0f;
                m_actionState = ActionState::IDLE;
            }
        }

        // ==========================================
        // TEKKEN ATTACK COMMANDS
        // ==========================================
        if (inputBits & BTN_HEAT_RAGE) {
            // Rage Art (Super move when HP <= 25%)
            Hitbox3D rageMove{};
            rageMove.moveName = "RAGE ART: LOTUS CRUSH";
            rageMove.offset = Vector3{0.9f, 1.2f, 0.0f};
            rageMove.radius = 0.55f;
            rageMove.damage = IsInRage() ? 3200 : 1400; // Extra power in Rage
            rageMove.height = AttackHeight::MID;
            rageMove.startupFrames = 20;
            rageMove.activeFrames = 6;
            rageMove.recoveryFrames = 40;
            rageMove.hitAdvantage = 15;
            rageMove.blockAdvantage = -18; // Very unsafe on block
            rageMove.knockback = Vector3{isFacingRight ? 8.0f : -8.0f, 6.0f, 0.0f};
            rageMove.isLauncher = true;
            rageMove.causesSplat = true;
            ExecuteMove(rageMove);
        } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_RP)) {
            // Uppercut Launcher (Mid launcher for Tekken juggle combos!)
            Hitbox3D uppercut{};
            uppercut.moveName = "WIND GOD UPPERCUT";
            uppercut.offset = Vector3{0.7f, 1.3f, 0.0f};
            uppercut.radius = 0.45f;
            uppercut.damage = 1600;
            uppercut.height = AttackHeight::MID;
            uppercut.startupFrames = 15;
            uppercut.activeFrames = 5;
            uppercut.recoveryFrames = 22;
            uppercut.hitAdvantage = 25;
            uppercut.blockAdvantage = -10;
            uppercut.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 9.5f, 0.0f}; // Huge vertical launch!
            uppercut.isLauncher = true;
            uppercut.causesSplat = true;
            ExecuteMove(uppercut);
        } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_LK)) {
            // Low Sweep (Ducks under highs, hits low!)
            Hitbox3D lowSweep{};
            lowSweep.moveName = "DRAGON LOW SWEEP";
            lowSweep.offset = Vector3{0.8f, 0.25f, 0.0f};
            lowSweep.radius = 0.40f;
            lowSweep.damage = 1100;
            lowSweep.height = AttackHeight::LOW;
            lowSweep.startupFrames = 16;
            lowSweep.activeFrames = 4;
            lowSweep.recoveryFrames = 20;
            lowSweep.hitAdvantage = 12;
            lowSweep.blockAdvantage = -13;
            lowSweep.knockback = Vector3{isFacingRight ? 4.5f : -4.5f, 1.5f, 0.0f};
            lowSweep.isLauncher = false;
            lowSweep.causesSplat = false;
            ExecuteMove(lowSweep);
        } else if (inputBits & BTN_LP) {
            // 1: Left Punch (High jab, fast 10 frames)
            Hitbox3D jab{};
            jab.moveName = "1: FLASH JAB";
            jab.offset = Vector3{0.75f, 1.45f, 0.0f};
            jab.radius = 0.35f;
            jab.damage = 600;
            jab.height = AttackHeight::HIGH;
            jab.startupFrames = 10;
            jab.activeFrames = 3;
            jab.recoveryFrames = 11;
            jab.hitAdvantage = 8;
            jab.blockAdvantage = 1; // +1 frame advantage on block!
            jab.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
            jab.isLauncher = false;
            jab.causesSplat = false;
            ExecuteMove(jab);
        } else if (inputBits & BTN_RP) {
            // 2: Right Punch (Mid straight punch, 12 frames)
            Hitbox3D cross{};
            cross.moveName = "2: STRAIGHT CROSS";
            cross.offset = Vector3{0.85f, 1.35f, 0.0f};
            cross.radius = 0.38f;
            cross.damage = 950;
            cross.height = AttackHeight::MID;
            cross.startupFrames = 12;
            cross.activeFrames = 4;
            cross.recoveryFrames = 14;
            cross.hitAdvantage = 6;
            cross.blockAdvantage = -3;
            cross.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 0.0f, 0.0f};
            cross.isLauncher = false;
            cross.causesSplat = false;
            ExecuteMove(cross);
        } else if (inputBits & BTN_LK) {
            // 3: Left Kick (Mid front kick, 14 frames)
            Hitbox3D midKick{};
            midKick.moveName = "3: TORSO KICK";
            midKick.offset = Vector3{0.95f, 1.15f, 0.0f};
            midKick.radius = 0.40f;
            midKick.damage = 1200;
            midKick.height = AttackHeight::MID;
            midKick.startupFrames = 14;
            midKick.activeFrames = 4;
            midKick.recoveryFrames = 18;
            midKick.hitAdvantage = 7;
            midKick.blockAdvantage = -5;
            midKick.knockback = Vector3{isFacingRight ? 4.0f : -4.0f, 0.0f, 0.0f};
            midKick.isLauncher = false;
            midKick.causesSplat = true;
            ExecuteMove(midKick);
        } else if (inputBits & BTN_RK) {
            // 4: Right Kick (High roundhouse, 16 frames, counter-hit launcher)
            Hitbox3D roundhouse{};
            roundhouse.moveName = "4: HIGH ROUNDHOUSE";
            roundhouse.offset = Vector3{0.9f, 1.55f, 0.0f};
            roundhouse.radius = 0.42f;
            roundhouse.damage = 1450;
            roundhouse.height = AttackHeight::HIGH;
            roundhouse.startupFrames = 16;
            roundhouse.activeFrames = 5;
            roundhouse.recoveryFrames = 22;
            roundhouse.hitAdvantage = 14;
            roundhouse.blockAdvantage = -8;
            roundhouse.knockback = Vector3{isFacingRight ? 6.0f : -6.0f, 4.0f, 0.0f};
            roundhouse.isLauncher = true; // Launches on hit!
            roundhouse.causesSplat = true;
            ExecuteMove(roundhouse);
        }
    }

    UpdatePhysics(deltaTime);
}

void Fighter::UpdatePhysics(float deltaTime) {
    // Air juggle / Jump gravity
    if (m_position.y > 0.0f || m_velocity.y != 0.0f) {
        float gravity = 24.0f;
        m_velocity.y -= gravity * deltaTime;
        m_position.y += m_velocity.y * deltaTime;

        if (m_position.y <= 0.0f) {
            m_position.y = 0.0f;
            m_velocity.y = 0.0f;

            if (m_actionState == ActionState::JUGGLED) {
                // Ground bounce / Knockdown
                m_actionState = ActionState::KNOCKED_DOWN;
                m_stunFrames = 30; // 30 frames to get up
                m_velocity.x = 0.0f;
                m_velocity.z = 0.0f;
            }
        }
    }

    // Lateral displacement
    m_position.x += m_velocity.x * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    // Decay pushback speed horizontally
    m_velocity.x = Lerp(m_velocity.x, 0.0f, 10.0f * deltaTime);
    m_velocity.z = Lerp(m_velocity.z, 0.0f, 12.0f * deltaTime);
}

bool Fighter::ReceiveHit(const Hitbox3D& attack, Fighter* attacker) {
    int incomingDmg = attack.damage;

    // Apply damage
    m_health = std::max(0, m_health - incomingDmg);
    // Recoverable health decays slower (Tekken red bar)
    m_recoverableHealth = std::max(m_health, m_recoverableHealth - (incomingDmg / 2));

    // Stun
    m_stunFrames = std::max(12, attack.recoveryFrames / 2);
    m_actionState = ActionState::HIT_STUN;

    // Pushback velocity
    if (attacker) {
        bool pushRight = (attacker->GetPosition().x < m_position.x);
        m_velocity.x = pushRight ? std::abs(attack.knockback.x) : -std::abs(attack.knockback.x);
    }

    if (attack.isLauncher || attack.knockback.y > 1.0f) {
        TriggerJuggleLaunch(attack.knockback);
    }

    return (m_health <= 0);
}

void Fighter::TriggerJuggleLaunch(const Vector3& launchImpulse) {
    m_actionState = ActionState::JUGGLED;
    m_velocity.y = launchImpulse.y;
    m_velocity.x = launchImpulse.x * 0.8f;
}

void Fighter::TriggerWallSplat() {
    m_actionState = ActionState::WALL_SPLATTED;
    m_velocity = Vector3{0.0f, 0.0f, 0.0f};
    m_stunFrames = 35; // Vulnerable to Tekken wall combo!
}

void Fighter::DrawProcedural3DMannequin() {
    float crouchY = (m_stance == StanceState::CROUCH) ? -0.4f : 0.0f;
    float breathe = std::sin(m_animTime * 4.0f) * 0.03f;
    Vector3 root = Vector3{m_position.x, m_position.y + crouchY, m_position.z};

    // Direction vector from yaw
    float rad = -m_yaw * DEG2RAD;
    Vector3 fwd = {std::cos(rad), 0.0f, std::sin(rad)};
    Vector3 rgt = {-std::sin(rad), 0.0f, std::cos(rad)};

    Color jointColor = Color{45, 50, 60, 255};
    Color skinColor = m_themeColor;

    // In Rage? Red pulsing aura!
    if (IsInRage()) {
        float auraPulse = (std::sin(m_animTime * 12.0f) + 1.0f) * 0.5f;
        skinColor = ColorLerp(m_themeColor, Color{239, 68, 68, 255}, 0.5f + auraPulse * 0.4f);
    }

    // 1. Pelvis / Hips
    Vector3 pelvis = Vector3Add(root, Vector3{0.0f, 0.95f + breathe, 0.0f});
    ::DrawSphere(pelvis, 0.22f, jointColor);

    // 2. Torso / Chest
    Vector3 chest = Vector3Add(pelvis, Vector3{0.0f, 0.45f, 0.0f});
    ::DrawCube(chest, 0.42f, 0.45f, 0.26f, skinColor);
    ::DrawCubeWires(chest, 0.42f, 0.45f, 0.26f, jointColor);

    // 3. Head & Karate Headband / Visor
    Vector3 head = Vector3Add(chest, Vector3{0.0f, 0.42f, 0.0f});
    ::DrawSphere(head, 0.20f, skinColor);
    // Eye visor indicating direction
    Vector3 eyePos = Vector3Add(head, Vector3Scale(fwd, 0.16f));
    eyePos.y += 0.04f;
    ::DrawCube(eyePos, 0.12f, 0.06f, 0.26f, Color{250, 204, 21, 255});

    // 4. Arms & Fists (with dynamic attack reach)
    float punchReach = (m_actionState == ActionState::ATTACKING) ? m_limbExtension * 0.75f : 0.0f;
    Vector3 lShoulder = Vector3Add(chest, Vector3Add(Vector3Scale(rgt, -0.28f), Vector3{0.0f, 0.15f, 0.0f}));
    Vector3 rShoulder = Vector3Add(chest, Vector3Add(Vector3Scale(rgt, 0.28f), Vector3{0.0f, 0.15f, 0.0f}));

    Vector3 lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.45f + punchReach), Vector3{0.0f, -0.05f, 0.0f}));
    Vector3 rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.35f + (punchReach * 0.5f)), Vector3{0.0f, -0.10f, 0.0f}));

    // Draw Arm Cylinders & Fists
    ::DrawCylinderEx(lShoulder, lFist, 0.08f, 0.07f, 6, jointColor);
    ::DrawSphere(lFist, 0.12f, skinColor);

    ::DrawCylinderEx(rShoulder, rFist, 0.08f, 0.07f, 6, jointColor);
    ::DrawSphere(rFist, 0.12f, skinColor);

    // 5. Legs & Feet
    Vector3 lHip = Vector3Add(pelvis, Vector3Scale(rgt, -0.16f));
    Vector3 rHip = Vector3Add(pelvis, Vector3Scale(rgt, 0.16f));

    Vector3 lFoot = Vector3Add(root, Vector3Add(Vector3Scale(rgt, -0.22f), Vector3Scale(fwd, 0.15f)));
    Vector3 rFoot = Vector3Add(root, Vector3Add(Vector3Scale(rgt, 0.22f), Vector3Scale(fwd, -0.15f)));

    ::DrawCylinderEx(lHip, lFoot, 0.10f, 0.08f, 6, jointColor);
    ::DrawCube(lFoot, 0.14f, 0.10f, 0.26f, skinColor);

    ::DrawCylinderEx(rHip, rFoot, 0.10f, 0.08f, 6, jointColor);
    ::DrawCube(rFoot, 0.14f, 0.10f, 0.26f, skinColor);

    // Shadow on the arena floor
    ::DrawCircle3D(Vector3{m_position.x, 0.02f, m_position.z}, 0.55f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, Color{0, 0, 0, 90});
}

void Fighter::Draw3D(bool showHitboxes) {
    if (m_hasBlenderModel) {
        // Draw Blender 3D Model with rotation & position
        Vector3 rotAxis = {0.0f, 1.0f, 0.0f};
        ::DrawModelEx(m_model, m_position, rotAxis, m_yaw, Vector3{1.0f, 1.0f, 1.0f}, WHITE);
    } else {
        DrawProcedural3DMannequin();
    }

    // Debug 3D Hitbox and Hurtbox visualization
    if (showHitboxes) {
        // Green Hurtboxes
        auto hurts = GetHurtboxes();
        for (const auto& box : hurts) {
            ::DrawBoundingBox(box, Color{34, 197, 94, 255});
        }

        // Red Hitbox
        const Hitbox3D* atk = GetActiveHitbox();
        if (atk) {
            float rad = -m_yaw * DEG2RAD;
            Vector3 fwd = {std::cos(rad), 0.0f, std::sin(rad)};
            Vector3 worldHitPos = Vector3Add(m_position, Vector3{
                fwd.x * atk->offset.x,
                atk->offset.y,
                fwd.z * atk->offset.x
            });
            ::DrawSphereWires(worldHitPos, atk->radius, 8, 8, Color{239, 68, 68, 255});
        }
    }
}

} // namespace DarkLotus
