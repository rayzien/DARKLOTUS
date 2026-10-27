#include "DarkLotus/Entities/Fighter.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

Fighter::Fighter(int playerIndex, int characterId, const std::string& name, Color themeColor, Vector3 startPos)
    : m_playerIndex(playerIndex), m_characterId(characterId), m_name(name), m_themeColor(themeColor), m_position(startPos) {
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
    if (::IsWindowReady() && ::FileExists(glbFilePath.c_str())) {
        if (m_hasBlenderModel) {
            ::UnloadModel(m_model);
        }
        m_model = ::LoadModel(glbFilePath.c_str());
        m_hasBlenderModel = true;
        Logger::Info("Fighter", m_name + " successfully loaded Blender 3D model: " + glbFilePath);
        return true;
    }
    if (!::IsWindowReady()) {
        Logger::Info("Fighter", "OpenGL window context not ready. Deferring 3D model upload.");
    } else {
        Logger::Info("Fighter", "Blender model not found at '" + glbFilePath + "'. Using specialized 3D mannequin rig.");
    }
    m_hasBlenderModel = false;
    return false;
}

bool Fighter::IsBlocking() const {
    return (m_actionState == ActionState::BLOCK_STUN) ||
           (m_actionState == ActionState::WALK_BACK);
}

void Fighter::ExecuteMove(const Hitbox3D& move) {
    if (m_actionState == ActionState::HIT_STUN ||
        m_actionState == ActionState::BLOCK_STUN ||
        m_actionState == ActionState::JUGGLED ||
        m_actionState == ActionState::KNOCKED_DOWN) {
        return;
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

    int peakFrame = m_currentAttack.startupFrames + (m_currentAttack.activeFrames / 2);
    if (m_moveFrame <= peakFrame) {
        m_limbExtension = static_cast<float>(m_moveFrame) / std::max(1, peakFrame);
    } else {
        m_limbExtension = 1.0f - static_cast<float>(m_moveFrame - peakFrame) / std::max(1, totalFrames - peakFrame);
    }

    if (m_moveFrame >= totalFrames) {
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

    if (dist > 3.2f) {
        outInputBits |= (m_position.x < opponent->GetPosition().x) ? BTN_RIGHT : BTN_LEFT;
    } else if (dist < 1.4f && opponent->IsAttacking()) {
        int roll = ::GetRandomValue(0, 100);
        if (roll < 45) {
            outInputBits |= (m_position.x < opponent->GetPosition().x) ? BTN_LEFT : BTN_RIGHT;
        } else if (roll < 70) {
            outInputBits |= BTN_UP;
        } else {
            outInputBits |= BTN_DOWN;
        }
    } else if (dist <= 2.2f && !IsAttacking()) {
        int roll = ::GetRandomValue(0, 100);
        if (IsInRage() && roll < 20) {
            outInputBits |= BTN_HEAT_RAGE;
        } else if (roll < 20) {
            outInputBits |= (BTN_LP | BTN_RP); // 1+2 signature move
        } else if (roll < 35) {
            outInputBits |= (BTN_LK | BTN_RK); // 3+4 signature move
        } else if (roll < 50) {
            outInputBits |= (BTN_DOWN | BTN_RP); // Down+2 Launcher!
        } else if (roll < 65) {
            outInputBits |= (BTN_DOWN | BTN_LK); // Down+3 Low sweep!
        } else if (roll < 80) {
            outInputBits |= BTN_LP;
        } else {
            outInputBits |= BTN_RK;
        }
    }
}

void Fighter::Update(float deltaTime, int /*currentFrame*/, unsigned int inputBits, Fighter* opponent) {
    m_animTime += deltaTime;

    if (m_isAI) {
        UpdateAI(opponent, inputBits);
    }

    // Orient facing opponent
    if (opponent && m_actionState != ActionState::KNOCKED_DOWN) {
        Vector3 toOpponent = Vector3Subtract(opponent->GetPosition(), m_position);
        float angleRad = std::atan2(toOpponent.z, toOpponent.x);
        m_yaw = -angleRad * RAD2DEG;
    }

    // Stun decay
    if (m_stunFrames > 0) {
        m_stunFrames--;
        if (m_stunFrames <= 0) {
            if (m_actionState == ActionState::HIT_STUN || m_actionState == ActionState::BLOCK_STUN) {
                m_actionState = ActionState::IDLE;
            }
        }
    }

    if (m_actionState == ActionState::KNOCKED_DOWN) {
        if (m_stunFrames <= 0) {
            m_actionState = ActionState::IDLE;
            m_position.y = 0.0f;
        }
    }

    UpdateAttackFrames();

    // Movement & Inputs
    if (m_actionState == ActionState::IDLE || m_actionState == ActionState::WALK_FORWARD ||
        m_actionState == ActionState::WALK_BACK || m_actionState == ActionState::CROUCH_IDLE) {

        float moveSpeed = 3.6f;
        float sidestepSpeed = 4.2f;
        bool isFacingRight = (opponent && opponent->GetPosition().x > m_position.x);

        if (inputBits & BTN_UP) {
            m_velocity.z = -sidestepSpeed;
            m_actionState = ActionState::SIDESTEP_LEFT;
            m_stance = StanceState::SIDESTEP_LEFT;
        } else if (inputBits & BTN_DOWN) {
            if ((inputBits & BTN_LEFT) || (inputBits & BTN_RIGHT)) {
                m_velocity.z = sidestepSpeed;
                m_actionState = ActionState::SIDESTEP_RIGHT;
                m_stance = StanceState::SIDESTEP_RIGHT;
            } else {
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

        // =========================================================================
        // SPECIALIZED MOVESETS FOR EACH CHARACTER
        // =========================================================================

        if (m_characterId == 0) {
            // =====================================================================
            // OODIII JII: Heihachi Mishima + Raiden
            // =====================================================================
            if (inputBits & BTN_HEAT_RAGE) {
                Hitbox3D rage{};
                rage.moveName = "RAGE ART: MISHIMA THUNDER WRATH";
                rage.offset = Vector3{0.95f, 1.25f, 0.0f};
                rage.radius = 0.60f;
                rage.damage = IsInRage() ? 3300 : 1500;
                rage.height = AttackHeight::MID;
                rage.type = AttackType::RAGE_ART;
                rage.startupFrames = 20;
                rage.activeFrames = 6;
                rage.recoveryFrames = 40;
                rage.hitAdvantage = 15;
                rage.blockAdvantage = -18;
                rage.knockback = Vector3{isFacingRight ? 8.5f : -8.5f, 6.5f, 0.0f};
                rage.isLauncher = true;
                rage.causesSplat = true;
                ExecuteMove(rage);
            } else if ((inputBits & BTN_THROW) || ((inputBits & BTN_LP) && (inputBits & BTN_LK))) {
                // Throw: Mishima Stonehead Headbutt (1+3)
                Hitbox3D stonehead{};
                stonehead.moveName = "MISHIMA STONEHEAD HEADBUTT (THROW)";
                stonehead.offset = Vector3{0.70f, 1.40f, 0.0f};
                stonehead.radius = 0.50f;
                stonehead.damage = 1500;
                stonehead.height = AttackHeight::HIGH;
                stonehead.type = AttackType::THROW_SLAM;
                stonehead.startupFrames = 12;
                stonehead.activeFrames = 4;
                stonehead.recoveryFrames = 20;
                stonehead.hitAdvantage = 14;
                stonehead.blockAdvantage = -4;
                stonehead.knockback = Vector3{isFacingRight ? 6.5f : -6.5f, 1.0f, 0.0f};
                stonehead.isLauncher = false;
                stonehead.causesSplat = true;
                ExecuteMove(stonehead);
            } else if ((inputBits & BTN_LP) && (inputBits & BTN_RP)) {
                // 1+2: Heihachi Demon Breath (f+1+2 Palm Strike)
                Hitbox3D palm{};
                palm.moveName = "DEMON BREATH PALM (HEIHACHI 1+2)";
                palm.offset = Vector3{0.85f, 1.30f, 0.0f};
                palm.radius = 0.45f;
                palm.damage = 1450;
                palm.height = AttackHeight::MID;
                palm.type = AttackType::DUAL_STRIKE;
                palm.startupFrames = 15;
                palm.activeFrames = 4;
                palm.recoveryFrames = 18;
                palm.hitAdvantage = 8;
                palm.blockAdvantage = -4;
                palm.knockback = Vector3{isFacingRight ? 7.0f : -7.0f, 1.0f, 0.0f};
                palm.isLauncher = false;
                palm.causesSplat = true;
                ExecuteMove(palm);
            } else if ((inputBits & BTN_LK) && (inputBits & BTN_RK)) {
                // 3+4: Raiden Electric Torpedo Dive!
                Hitbox3D torpedo{};
                torpedo.moveName = "RAIDEN THUNDER TORPEDO DIVE (3+4)";
                torpedo.offset = Vector3{1.10f, 1.15f, 0.0f};
                torpedo.radius = 0.52f;
                torpedo.damage = 1600;
                torpedo.height = AttackHeight::MID;
                torpedo.type = AttackType::FLYING_DIVE_DROPKICK;
                torpedo.startupFrames = 18;
                torpedo.activeFrames = 6;
                torpedo.recoveryFrames = 22;
                torpedo.hitAdvantage = 14;
                torpedo.blockAdvantage = -11;
                torpedo.knockback = Vector3{isFacingRight ? 8.5f : -8.5f, 3.5f, 0.0f};
                torpedo.isLauncher = true;
                torpedo.causesSplat = true;
                m_velocity.x = isFacingRight ? 6.0f : -6.0f; // Dive forward!
                ExecuteMove(torpedo);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_RP)) {
                // Down+2: Electric Wind God Fist (EWGF launcher!)
                Hitbox3D ewgf{};
                ewgf.moveName = "ELECTRIC WIND GOD FIST (EWGF)";
                ewgf.offset = Vector3{0.75f, 1.35f, 0.0f};
                ewgf.radius = 0.48f;
                ewgf.damage = 1750;
                ewgf.height = AttackHeight::MID;
                ewgf.type = AttackType::UPPERCUT_LAUNCHER;
                ewgf.startupFrames = 14; // Fast electric Mishima frame!
                ewgf.activeFrames = 5;
                ewgf.recoveryFrames = 20;
                ewgf.hitAdvantage = 28;
                ewgf.blockAdvantage = +5; // +5 electric on block!
                ewgf.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 10.0f, 0.0f}; // Electric sky launch!
                ewgf.isLauncher = true;
                ewgf.causesSplat = true;
                ExecuteMove(ewgf);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_LK)) {
                // Down+3: Raiden Lightning Slide / Hell Sweep
                Hitbox3D hellsweep{};
                hellsweep.moveName = "MISHIMA HELL SWEEP (d+3)";
                hellsweep.offset = Vector3{0.80f, 0.22f, 0.0f};
                hellsweep.radius = 0.40f;
                hellsweep.damage = 1200;
                hellsweep.height = AttackHeight::LOW;
                hellsweep.type = AttackType::LOW_SWEEP;
                hellsweep.startupFrames = 16;
                hellsweep.activeFrames = 4;
                hellsweep.recoveryFrames = 22;
                hellsweep.hitAdvantage = 15;
                hellsweep.blockAdvantage = -14;
                hellsweep.knockback = Vector3{isFacingRight ? 4.0f : -4.0f, 1.8f, 0.0f};
                hellsweep.isLauncher = false;
                hellsweep.causesSplat = false;
                ExecuteMove(hellsweep);
            } else if (inputBits & BTN_LP) {
                // 1: Flash Jab
                Hitbox3D jab{};
                jab.moveName = "1: DEMON FLASH JAB";
                jab.offset = Vector3{0.75f, 1.45f, 0.0f};
                jab.radius = 0.35f;
                jab.damage = 650;
                jab.height = AttackHeight::HIGH;
                jab.type = AttackType::PUNCH_LEFT;
                jab.startupFrames = 10;
                jab.activeFrames = 3;
                jab.recoveryFrames = 11;
                jab.hitAdvantage = 8;
                jab.blockAdvantage = 1;
                jab.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
                ExecuteMove(jab);
            } else if (inputBits & BTN_RP) {
                // 2: Demon Straight
                Hitbox3D cross{};
                cross.moveName = "2: HEIHACHI DEMON STRAIGHT";
                cross.offset = Vector3{0.85f, 1.35f, 0.0f};
                cross.radius = 0.38f;
                cross.damage = 1000;
                cross.height = AttackHeight::MID;
                cross.type = AttackType::PUNCH_RIGHT;
                cross.startupFrames = 12;
                cross.activeFrames = 4;
                cross.recoveryFrames = 14;
                cross.hitAdvantage = 6;
                cross.blockAdvantage = -2;
                cross.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 0.0f, 0.0f};
                ExecuteMove(cross);
            } else if (inputBits & BTN_LK) {
                // 3: Raiden Spark Kick
                Hitbox3D kick{};
                kick.moveName = "3: RAIDEN SPARK KICK";
                kick.offset = Vector3{0.90f, 1.15f, 0.0f};
                kick.radius = 0.40f;
                kick.damage = 1250;
                kick.height = AttackHeight::MID;
                kick.type = AttackType::KICK_LEFT;
                kick.startupFrames = 14;
                kick.activeFrames = 4;
                kick.recoveryFrames = 18;
                kick.hitAdvantage = 7;
                kick.blockAdvantage = -5;
                kick.knockback = Vector3{isFacingRight ? 4.0f : -4.0f, 0.0f, 0.0f};
                ExecuteMove(kick);
            } else if (inputBits & BTN_RK) {
                // 4: Hell Axle Double Axe Kick
                Hitbox3D hellAxle{};
                hellAxle.moveName = "4: HELL AXLE JUMPING AXE";
                hellAxle.offset = Vector3{0.92f, 1.60f, 0.0f};
                hellAxle.radius = 0.44f;
                hellAxle.damage = 1550;
                hellAxle.height = AttackHeight::HIGH;
                hellAxle.type = AttackType::KICK_RIGHT;
                hellAxle.startupFrames = 16;
                hellAxle.activeFrames = 5;
                hellAxle.recoveryFrames = 22;
                hellAxle.hitAdvantage = 16;
                hellAxle.blockAdvantage = -8;
                hellAxle.knockback = Vector3{isFacingRight ? 6.5f : -6.5f, 4.5f, 0.0f};
                hellAxle.isLauncher = true;
                hellAxle.causesSplat = true;
                ExecuteMove(hellAxle);
            }

        } else if (m_characterId == 1) {
            // =====================================================================
            // LUHAR IHDNAG: Leo Kliesen (Bajiquan)
            // =====================================================================
            if (inputBits & BTN_HEAT_RAGE) {
                Hitbox3D rage{};
                rage.moveName = "RAGE ART: MOUNTAIN SHATTERER";
                rage.offset = Vector3{0.90f, 1.20f, 0.0f};
                rage.radius = 0.58f;
                rage.damage = IsInRage() ? 3200 : 1450;
                rage.height = AttackHeight::MID;
                rage.type = AttackType::RAGE_ART;
                rage.startupFrames = 20;
                rage.activeFrames = 6;
                rage.recoveryFrames = 40;
                rage.hitAdvantage = 15;
                rage.blockAdvantage = -18;
                rage.knockback = Vector3{isFacingRight ? 8.0f : -8.0f, 6.0f, 0.0f};
                rage.isLauncher = true;
                rage.causesSplat = true;
                ExecuteMove(rage);
            } else if ((inputBits & BTN_THROW) || ((inputBits & BTN_LP) && (inputBits & BTN_LK))) {
                // Throw: Baji Shoulder Hip Throw (1+3)
                Hitbox3D bajiThrow{};
                bajiThrow.moveName = "BAJI SHOULDER HIP THROW (THROW)";
                bajiThrow.offset = Vector3{0.70f, 1.30f, 0.0f};
                bajiThrow.radius = 0.48f;
                bajiThrow.damage = 1450;
                bajiThrow.height = AttackHeight::HIGH;
                bajiThrow.type = AttackType::THROW_SLAM;
                bajiThrow.startupFrames = 12;
                bajiThrow.activeFrames = 4;
                bajiThrow.recoveryFrames = 20;
                bajiThrow.hitAdvantage = 14;
                bajiThrow.blockAdvantage = -4;
                bajiThrow.knockback = Vector3{isFacingRight ? 6.0f : -6.0f, 1.0f, 0.0f};
                bajiThrow.isLauncher = false;
                bajiThrow.causesSplat = true;
                ExecuteMove(bajiThrow);
            } else if ((inputBits & BTN_LP) && (inputBits & BTN_RP)) {
                // 1+2: Tetsuzanko (Iron Body Shoulder Ram - Leo's signature armored tackle!)
                Hitbox3D tetsuzanko{};
                tetsuzanko.moveName = "TETSUZANKO (IRON BODY SHOULDER RAM)";
                tetsuzanko.offset = Vector3{0.80f, 1.20f, 0.0f};
                tetsuzanko.radius = 0.50f;
                tetsuzanko.damage = 1600;
                tetsuzanko.height = AttackHeight::MID;
                tetsuzanko.type = AttackType::DUAL_STRIKE;
                tetsuzanko.startupFrames = 13; // Fast Baji shoulder!
                tetsuzanko.activeFrames = 5;
                tetsuzanko.recoveryFrames = 20;
                tetsuzanko.hitAdvantage = 10;
                tetsuzanko.blockAdvantage = -8;
                tetsuzanko.knockback = Vector3{isFacingRight ? 8.0f : -8.0f, 1.5f, 0.0f};
                tetsuzanko.isLauncher = false;
                tetsuzanko.causesSplat = true;
                ExecuteMove(tetsuzanko);
            } else if ((inputBits & BTN_LK) && (inputBits & BTN_RK)) {
                // 3+4: Dual Crane Palm Thrust
                Hitbox3D crane{};
                crane.moveName = "DUAL CRANE PALM THRUST (3+4)";
                crane.offset = Vector3{0.88f, 1.25f, 0.0f};
                crane.radius = 0.46f;
                crane.damage = 1450;
                crane.height = AttackHeight::MID;
                crane.type = AttackType::DUAL_STRIKE;
                crane.startupFrames = 16;
                crane.activeFrames = 4;
                crane.recoveryFrames = 18;
                crane.hitAdvantage = 9;
                crane.blockAdvantage = -4;
                crane.knockback = Vector3{isFacingRight ? 6.0f : -6.0f, 1.0f, 0.0f};
                crane.isLauncher = false;
                crane.causesSplat = true;
                ExecuteMove(crane);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_RP)) {
                // Down+2: Bajiquan Rising Elbow Launcher
                Hitbox3D elbow{};
                elbow.moveName = "BAJIQUAN RISING ELBOW LAUNCHER (d+2)";
                elbow.offset = Vector3{0.70f, 1.35f, 0.0f};
                elbow.radius = 0.44f;
                elbow.damage = 1650;
                elbow.height = AttackHeight::MID;
                elbow.type = AttackType::UPPERCUT_LAUNCHER;
                elbow.startupFrames = 15;
                elbow.activeFrames = 5;
                elbow.recoveryFrames = 22;
                elbow.hitAdvantage = 26;
                elbow.blockAdvantage = -9;
                elbow.knockback = Vector3{isFacingRight ? 2.8f : -2.8f, 9.6f, 0.0f}; // Huge vertical launch!
                elbow.isLauncher = true;
                elbow.causesSplat = true;
                ExecuteMove(elbow);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_LK)) {
                // Down+3: Baji Dragon Sweep
                Hitbox3D bajiSweep{};
                bajiSweep.moveName = "BAJI DRAGON LOW SWEEP (d+3)";
                bajiSweep.offset = Vector3{0.85f, 0.20f, 0.0f};
                bajiSweep.radius = 0.40f;
                bajiSweep.damage = 1100;
                bajiSweep.height = AttackHeight::LOW;
                bajiSweep.type = AttackType::LOW_SWEEP;
                bajiSweep.startupFrames = 16;
                bajiSweep.activeFrames = 4;
                bajiSweep.recoveryFrames = 20;
                bajiSweep.hitAdvantage = 14;
                bajiSweep.blockAdvantage = -13;
                bajiSweep.knockback = Vector3{isFacingRight ? 4.5f : -4.5f, 1.5f, 0.0f};
                bajiSweep.isLauncher = false;
                bajiSweep.causesSplat = false;
                ExecuteMove(bajiSweep);
            } else if (inputBits & BTN_LP) {
                // 1: Lead Palm Jab
                Hitbox3D jab{};
                jab.moveName = "1: LEO LEAD PALM JAB";
                jab.offset = Vector3{0.75f, 1.45f, 0.0f};
                jab.radius = 0.35f;
                jab.damage = 620;
                jab.height = AttackHeight::HIGH;
                jab.type = AttackType::PUNCH_LEFT;
                jab.startupFrames = 10;
                jab.activeFrames = 3;
                jab.recoveryFrames = 11;
                jab.hitAdvantage = 8;
                jab.blockAdvantage = 1;
                jab.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
                ExecuteMove(jab);
            } else if (inputBits & BTN_RP) {
                // 2: Iron Fist Thrust
                Hitbox3D fist{};
                fist.moveName = "2: IRON FIST THRUST";
                fist.offset = Vector3{0.82f, 1.30f, 0.0f};
                fist.radius = 0.38f;
                fist.damage = 980;
                fist.height = AttackHeight::MID;
                fist.type = AttackType::PUNCH_RIGHT;
                fist.startupFrames = 12;
                fist.activeFrames = 4;
                fist.recoveryFrames = 14;
                fist.hitAdvantage = 6;
                fist.blockAdvantage = -3;
                fist.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 0.0f, 0.0f};
                ExecuteMove(fist);
            } else if (inputBits & BTN_LK) {
                // 3: Low Shin Check
                Hitbox3D shin{};
                shin.moveName = "3: LOW SHIN CHECK";
                shin.offset = Vector3{0.80f, 0.40f, 0.0f};
                shin.radius = 0.38f;
                shin.damage = 900;
                shin.height = AttackHeight::LOW;
                shin.type = AttackType::KICK_LEFT;
                shin.startupFrames = 13;
                shin.activeFrames = 3;
                shin.recoveryFrames = 16;
                shin.hitAdvantage = 5;
                shin.blockAdvantage = -11;
                shin.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
                ExecuteMove(shin);
            } else if (inputBits & BTN_RK) {
                // 4: Axe Kick & Stomp
                Hitbox3D axe{};
                axe.moveName = "4: AXE KICK & STOMP";
                axe.offset = Vector3{0.88f, 1.45f, 0.0f};
                axe.radius = 0.42f;
                axe.damage = 1400;
                axe.height = AttackHeight::MID;
                axe.type = AttackType::KICK_RIGHT;
                axe.startupFrames = 16;
                axe.activeFrames = 4;
                axe.recoveryFrames = 20;
                axe.hitAdvantage = 12;
                axe.blockAdvantage = -6;
                axe.knockback = Vector3{isFacingRight ? 5.5f : -5.5f, 2.0f, 0.0f};
                axe.isLauncher = false;
                axe.causesSplat = true;
                ExecuteMove(axe);
            }

        } else if (m_characterId == 2) {
            // =====================================================================
            // WINPAK: Jin Kazama (Kazama Karate / Devil)
            // =====================================================================
            if (inputBits & BTN_HEAT_RAGE) {
                Hitbox3D rage{};
                rage.moveName = "RAGE ART: KAZAMA DIVINE AVENGER";
                rage.offset = Vector3{0.92f, 1.25f, 0.0f};
                rage.radius = 0.58f;
                rage.damage = IsInRage() ? 3350 : 1500;
                rage.height = AttackHeight::MID;
                rage.type = AttackType::RAGE_ART;
                rage.startupFrames = 20;
                rage.activeFrames = 6;
                rage.recoveryFrames = 40;
                rage.hitAdvantage = 15;
                rage.blockAdvantage = -18;
                rage.knockback = Vector3{isFacingRight ? 8.5f : -8.5f, 6.5f, 0.0f};
                rage.isLauncher = true;
                rage.causesSplat = true;
                ExecuteMove(rage);
            } else if ((inputBits & BTN_THROW) || ((inputBits & BTN_LP) && (inputBits & BTN_LK))) {
                // Throw: Kazama Comstock Armlock Throw (1+3)
                Hitbox3D jinThrow{};
                jinThrow.moveName = "KAZAMA COMSTOCK ARMLOCK (THROW)";
                jinThrow.offset = Vector3{0.70f, 1.35f, 0.0f};
                jinThrow.radius = 0.50f;
                jinThrow.damage = 1500;
                jinThrow.height = AttackHeight::HIGH;
                jinThrow.type = AttackType::THROW_SLAM;
                jinThrow.startupFrames = 12;
                jinThrow.activeFrames = 4;
                jinThrow.recoveryFrames = 20;
                jinThrow.hitAdvantage = 14;
                jinThrow.blockAdvantage = -4;
                jinThrow.knockback = Vector3{isFacingRight ? 7.0f : -7.0f, 1.2f, 0.0f};
                jinThrow.isLauncher = false;
                jinThrow.causesSplat = true;
                ExecuteMove(jinThrow);
            } else if ((inputBits & BTN_LP) && (inputBits & BTN_RP)) {
                // 1+2: Demon Paw (f,f+2 Lunging Straight Fist with Red Lightning!)
                Hitbox3D demonPaw{};
                demonPaw.moveName = "DEMON PAW (JIN f,f+2)";
                demonPaw.offset = Vector3{1.05f, 1.35f, 0.0f};
                demonPaw.radius = 0.48f;
                demonPaw.damage = 1550;
                demonPaw.height = AttackHeight::MID;
                demonPaw.type = AttackType::DUAL_STRIKE;
                demonPaw.startupFrames = 14;
                demonPaw.activeFrames = 4;
                demonPaw.recoveryFrames = 17;
                demonPaw.hitAdvantage = 9;
                demonPaw.blockAdvantage = -6; // Relatively safe on block
                demonPaw.knockback = Vector3{isFacingRight ? 8.5f : -8.5f, 1.5f, 0.0f};
                demonPaw.isLauncher = false;
                demonPaw.causesSplat = true; // Massive wall splat!
                m_velocity.x = isFacingRight ? 4.5f : -4.5f; // Lunging dash!
                ExecuteMove(demonPaw);
            } else if ((inputBits & BTN_LK) && (inputBits & BTN_RK)) {
                // 3+4: Can-Can Double Launching Kicks (d+3+4)
                Hitbox3D cancan{};
                cancan.moveName = "CAN-CAN DOUBLE LAUNCH KICKS (d+3+4)";
                cancan.offset = Vector3{0.80f, 1.40f, 0.0f};
                cancan.radius = 0.48f;
                cancan.damage = 1700;
                cancan.height = AttackHeight::MID;
                cancan.type = AttackType::FLYING_DIVE_DROPKICK;
                cancan.startupFrames = 15;
                cancan.activeFrames = 6;
                cancan.recoveryFrames = 24;
                cancan.hitAdvantage = 28;
                cancan.blockAdvantage = -14;
                cancan.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 10.2f, 0.0f}; // Jin's signature double launch!
                cancan.isLauncher = true;
                cancan.causesSplat = true;
                ExecuteMove(cancan);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_RP)) {
                // Down+2: Thrusting Demon Uppercut (Jin Launcher)
                Hitbox3D jinUpper{};
                jinUpper.moveName = "THRUSTING DEMON UPPERCUT (d+2)";
                jinUpper.offset = Vector3{0.72f, 1.35f, 0.0f};
                jinUpper.radius = 0.45f;
                jinUpper.damage = 1650;
                jinUpper.height = AttackHeight::MID;
                jinUpper.type = AttackType::UPPERCUT_LAUNCHER;
                jinUpper.startupFrames = 15;
                jinUpper.activeFrames = 5;
                jinUpper.recoveryFrames = 22;
                jinUpper.hitAdvantage = 25;
                jinUpper.blockAdvantage = -10;
                jinUpper.knockback = Vector3{isFacingRight ? 2.9f : -2.9f, 9.5f, 0.0f};
                jinUpper.isLauncher = true;
                jinUpper.causesSplat = true;
                ExecuteMove(jinUpper);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_LK)) {
                // Down+3: Kazama Spinning Low Sweep
                Hitbox3D jinSweep{};
                jinSweep.moveName = "KAZAMA SPINNING LOW SWEEP (d+3)";
                jinSweep.offset = Vector3{0.82f, 0.22f, 0.0f};
                jinSweep.radius = 0.40f;
                jinSweep.damage = 1100;
                jinSweep.height = AttackHeight::LOW;
                jinSweep.type = AttackType::LOW_SWEEP;
                jinSweep.startupFrames = 15;
                jinSweep.activeFrames = 4;
                jinSweep.recoveryFrames = 20;
                jinSweep.hitAdvantage = 12;
                jinSweep.blockAdvantage = -13;
                jinSweep.knockback = Vector3{isFacingRight ? 4.2f : -4.2f, 1.2f, 0.0f};
                jinSweep.isLauncher = false;
                jinSweep.causesSplat = false;
                ExecuteMove(jinSweep);
            } else if (inputBits & BTN_LP) {
                // 1: Kazama Karate Jab
                Hitbox3D jab{};
                jab.moveName = "1: KAZAMA KARATE JAB";
                jab.offset = Vector3{0.75f, 1.45f, 0.0f};
                jab.radius = 0.35f;
                jab.damage = 650;
                jab.height = AttackHeight::HIGH;
                jab.type = AttackType::PUNCH_LEFT;
                jab.startupFrames = 10;
                jab.activeFrames = 3;
                jab.recoveryFrames = 11;
                jab.hitAdvantage = 8;
                jab.blockAdvantage = 1;
                jab.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
                ExecuteMove(jab);
            } else if (inputBits & BTN_RP) {
                // 2: Straight Punch
                Hitbox3D cross{};
                cross.moveName = "2: JIN STRAIGHT PUNCH";
                cross.offset = Vector3{0.85f, 1.35f, 0.0f};
                cross.radius = 0.38f;
                cross.damage = 1020;
                cross.height = AttackHeight::MID;
                cross.type = AttackType::PUNCH_RIGHT;
                cross.startupFrames = 12;
                cross.activeFrames = 4;
                cross.recoveryFrames = 14;
                cross.hitAdvantage = 6;
                cross.blockAdvantage = -3;
                cross.knockback = Vector3{isFacingRight ? 3.0f : -3.0f, 0.0f, 0.0f};
                ExecuteMove(cross);
            } else if (inputBits & BTN_LK) {
                // 3: Round Kick
                Hitbox3D kick{};
                kick.moveName = "3: KAZAMA ROUND KICK";
                kick.offset = Vector3{0.92f, 1.20f, 0.0f};
                kick.radius = 0.40f;
                kick.damage = 1200;
                kick.height = AttackHeight::MID;
                kick.type = AttackType::KICK_LEFT;
                kick.startupFrames = 14;
                kick.activeFrames = 4;
                kick.recoveryFrames = 18;
                kick.hitAdvantage = 7;
                kick.blockAdvantage = -5;
                kick.knockback = Vector3{isFacingRight ? 4.0f : -4.0f, 0.0f, 0.0f};
                ExecuteMove(kick);
            } else if (inputBits & BTN_RK) {
                // 4: Right Roundhouse Kick (Counter-Hit launcher!)
                Hitbox3D rrk{};
                rrk.moveName = "4: RIGHT ROUNDHOUSE KICK";
                rrk.offset = Vector3{0.90f, 1.55f, 0.0f};
                rrk.radius = 0.42f;
                rrk.damage = 1500;
                rrk.height = AttackHeight::HIGH;
                rrk.type = AttackType::KICK_RIGHT;
                rrk.startupFrames = 15;
                rrk.activeFrames = 5;
                rrk.recoveryFrames = 22;
                rrk.hitAdvantage = 15;
                rrk.blockAdvantage = -8;
                rrk.knockback = Vector3{isFacingRight ? 6.5f : -6.5f, 4.2f, 0.0f};
                rrk.isLauncher = true;
                rrk.causesSplat = true;
                ExecuteMove(rrk);
            }

        } else if (m_characterId == 3) {
            // =====================================================================
            // GANJA: Bryan Fury + Kangaroo (Roger)
            // =====================================================================
            if (inputBits & BTN_HEAT_RAGE) {
                Hitbox3D rage{};
                rage.moveName = "RAGE ART: FURY CHAOS BARRAGE";
                rage.offset = Vector3{0.95f, 1.25f, 0.0f};
                rage.radius = 0.60f;
                rage.damage = IsInRage() ? 3400 : 1550;
                rage.height = AttackHeight::MID;
                rage.type = AttackType::RAGE_ART;
                rage.startupFrames = 20;
                rage.activeFrames = 6;
                rage.recoveryFrames = 40;
                rage.hitAdvantage = 15;
                rage.blockAdvantage = -18;
                rage.knockback = Vector3{isFacingRight ? 9.0f : -9.0f, 6.5f, 0.0f};
                rage.isLauncher = true;
                rage.causesSplat = true;
                ExecuteMove(rage);
            } else if ((inputBits & BTN_THROW) || ((inputBits & BTN_LP) && (inputBits & BTN_LK))) {
                // Throw: Kangaroo Pouch Slam / Fisherman Suplex (1+3)
                Hitbox3D pouchSlam{};
                pouchSlam.moveName = "KANGAROO POUCH SLAM (THROW)";
                pouchSlam.offset = Vector3{0.70f, 1.30f, 0.0f};
                pouchSlam.radius = 0.50f;
                pouchSlam.damage = 1550;
                pouchSlam.height = AttackHeight::HIGH;
                pouchSlam.type = AttackType::THROW_SLAM;
                pouchSlam.startupFrames = 12;
                pouchSlam.activeFrames = 4;
                pouchSlam.recoveryFrames = 20;
                pouchSlam.hitAdvantage = 15;
                pouchSlam.blockAdvantage = -4;
                pouchSlam.knockback = Vector3{isFacingRight ? 7.5f : -7.5f, 1.5f, 0.0f};
                pouchSlam.isLauncher = false;
                pouchSlam.causesSplat = true;
                ExecuteMove(pouchSlam);
            } else if ((inputBits & BTN_LP) && (inputBits & BTN_RP)) {
                // 1+2: Nuclear Mach Punch (Bryan Fury f,f+2)
                Hitbox3D machPunch{};
                machPunch.moveName = "NUCLEAR MACH PUNCH (BRYAN f,f+2)";
                machPunch.offset = Vector3{1.10f, 1.40f, 0.0f};
                machPunch.radius = 0.50f;
                machPunch.damage = 1700; // Massive damage!
                machPunch.height = AttackHeight::HIGH;
                machPunch.type = AttackType::DUAL_STRIKE;
                machPunch.startupFrames = 15;
                machPunch.activeFrames = 4;
                machPunch.recoveryFrames = 19;
                machPunch.hitAdvantage = 12;
                machPunch.blockAdvantage = -9;
                machPunch.knockback = Vector3{isFacingRight ? 10.0f : -10.0f, 1.8f, 0.0f};
                machPunch.isLauncher = false;
                machPunch.causesSplat = true; // Violent wall splat!
                m_velocity.x = isFacingRight ? 5.5f : -5.5f; // Lunging Mach burst!
                ExecuteMove(machPunch);
            } else if ((inputBits & BTN_LK) && (inputBits & BTN_RK)) {
                // 3+4: Kangaroo Double Flying Dropkick!
                Hitbox3D dropkick{};
                dropkick.moveName = "KANGAROO DOUBLE FLYING DROPKICK (3+4)";
                dropkick.offset = Vector3{1.05f, 1.25f, 0.0f};
                dropkick.radius = 0.55f;
                dropkick.damage = 1600;
                dropkick.height = AttackHeight::MID;
                dropkick.type = AttackType::FLYING_DIVE_DROPKICK;
                dropkick.startupFrames = 17;
                dropkick.activeFrames = 6;
                dropkick.recoveryFrames = 24;
                dropkick.hitAdvantage = 16;
                dropkick.blockAdvantage = -10;
                dropkick.knockback = Vector3{isFacingRight ? 8.0f : -8.0f, 4.0f, 0.0f};
                dropkick.isLauncher = true;
                dropkick.causesSplat = true;
                m_velocity.x = isFacingRight ? 5.0f : -5.0f;
                m_velocity.y = 3.5f; // Flying jump!
                ExecuteMove(dropkick);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_RP)) {
                // Down+2: Jet Upper (Bryan's signature lightning-fast launcher!)
                Hitbox3D jetUpper{};
                jetUpper.moveName = "JET UPPER (BRYAN i14 LAUNCHER)";
                jetUpper.offset = Vector3{0.75f, 1.40f, 0.0f};
                jetUpper.radius = 0.46f;
                jetUpper.damage = 1700;
                jetUpper.height = AttackHeight::MID;
                jetUpper.type = AttackType::UPPERCUT_LAUNCHER;
                jetUpper.startupFrames = 14; // Ultra fast i14 launcher!
                jetUpper.activeFrames = 4;
                jetUpper.recoveryFrames = 22;
                jetUpper.hitAdvantage = 28;
                jetUpper.blockAdvantage = -9; // Safe on block (-9)
                jetUpper.knockback = Vector3{isFacingRight ? 2.8f : -2.8f, 9.8f, 0.0f}; // Sky-high launch!
                jetUpper.isLauncher = true;
                jetUpper.causesSplat = true;
                ExecuteMove(jetUpper);
            } else if ((inputBits & BTN_DOWN) && (inputBits & BTN_LK)) {
                // Down+3: Snake Edge (Bryan's infamous low launcher!)
                Hitbox3D snakeEdge{};
                snakeEdge.moveName = "SNAKE EDGE LOW LAUNCHER (d/f+3)";
                snakeEdge.offset = Vector3{0.95f, 0.20f, 0.0f};
                snakeEdge.radius = 0.44f;
                snakeEdge.damage = 1350;
                snakeEdge.height = AttackHeight::LOW;
                snakeEdge.type = AttackType::LOW_SWEEP;
                snakeEdge.startupFrames = 20; // Slower sweeping low
                snakeEdge.activeFrames = 5;
                snakeEdge.recoveryFrames = 26;
                snakeEdge.hitAdvantage = 24;
                snakeEdge.blockAdvantage = -24; // Very punishable on block
                snakeEdge.knockback = Vector3{isFacingRight ? 4.5f : -4.5f, 8.5f, 0.0f}; // Launches on hit!
                snakeEdge.isLauncher = true;
                snakeEdge.causesSplat = false;
                ExecuteMove(snakeEdge);
            } else if (inputBits & BTN_LP) {
                // 1: Bryan Lead Hook
                Hitbox3D hook{};
                hook.moveName = "1: BRYAN LEAD HOOK";
                hook.offset = Vector3{0.75f, 1.45f, 0.0f};
                hook.radius = 0.35f;
                hook.damage = 680;
                hook.height = AttackHeight::HIGH;
                hook.type = AttackType::PUNCH_LEFT;
                hook.startupFrames = 10;
                hook.activeFrames = 3;
                hook.recoveryFrames = 11;
                hook.hitAdvantage = 8;
                hook.blockAdvantage = 1;
                hook.knockback = Vector3{isFacingRight ? 2.0f : -2.0f, 0.0f, 0.0f};
                ExecuteMove(hook);
            } else if (inputBits & BTN_RP) {
                // 2: Bryan Body Blow
                Hitbox3D bodyBlow{};
                bodyBlow.moveName = "2: BRYAN BODY BLOW";
                bodyBlow.offset = Vector3{0.85f, 1.25f, 0.0f};
                bodyBlow.radius = 0.38f;
                bodyBlow.damage = 1050;
                bodyBlow.height = AttackHeight::MID;
                bodyBlow.type = AttackType::PUNCH_RIGHT;
                bodyBlow.startupFrames = 12;
                bodyBlow.activeFrames = 4;
                bodyBlow.recoveryFrames = 14;
                bodyBlow.hitAdvantage = 7;
                bodyBlow.blockAdvantage = -2;
                bodyBlow.knockback = Vector3{isFacingRight ? 3.2f : -3.2f, 0.0f, 0.0f};
                ExecuteMove(bodyBlow);
            } else if (inputBits & BTN_LK) {
                // 3: Kangaroo Low Shin Kick
                Hitbox3D kick{};
                kick.moveName = "3: KANGAROO LOW SHIN KICK";
                kick.offset = Vector3{0.82f, 0.35f, 0.0f};
                kick.radius = 0.38f;
                kick.damage = 950;
                kick.height = AttackHeight::LOW;
                kick.type = AttackType::KICK_LEFT;
                kick.startupFrames = 13;
                kick.activeFrames = 3;
                kick.recoveryFrames = 16;
                kick.hitAdvantage = 5;
                kick.blockAdvantage = -11;
                kick.knockback = Vector3{isFacingRight ? 2.5f : -2.5f, 0.0f, 0.0f};
                ExecuteMove(kick);
            } else if (inputBits & BTN_RK) {
                // 4: Brutal Knee Smash
                Hitbox3D knee{};
                knee.moveName = "4: BRUTAL KNEE SMASH";
                knee.offset = Vector3{0.80f, 1.20f, 0.0f};
                knee.radius = 0.44f;
                knee.damage = 1480;
                knee.height = AttackHeight::MID;
                knee.type = AttackType::KICK_RIGHT;
                knee.startupFrames = 15;
                knee.activeFrames = 4;
                knee.recoveryFrames = 20;
                knee.hitAdvantage = 14;
                knee.blockAdvantage = -5;
                knee.knockback = Vector3{isFacingRight ? 6.0f : -6.0f, 2.0f, 0.0f};
                knee.isLauncher = false;
                knee.causesSplat = true;
                ExecuteMove(knee);
            }
        }
    }

    UpdatePhysics(deltaTime);
}

void Fighter::UpdatePhysics(float deltaTime) {
    if (m_position.y > 0.0f || m_velocity.y != 0.0f) {
        float gravity = 24.0f;
        m_velocity.y -= gravity * deltaTime;
        m_position.y += m_velocity.y * deltaTime;

        if (m_position.y <= 0.0f) {
            m_position.y = 0.0f;
            m_velocity.y = 0.0f;

            if (m_actionState == ActionState::JUGGLED) {
                m_actionState = ActionState::KNOCKED_DOWN;
                m_stunFrames = 30;
                m_velocity.x = 0.0f;
                m_velocity.z = 0.0f;
            }
        }
    }

    m_position.x += m_velocity.x * deltaTime;
    m_position.z += m_velocity.z * deltaTime;

    m_velocity.x = Lerp(m_velocity.x, 0.0f, 10.0f * deltaTime);
    m_velocity.z = Lerp(m_velocity.z, 0.0f, 12.0f * deltaTime);
}

bool Fighter::ReceiveHit(const Hitbox3D& attack, Fighter* attacker) {
    int incomingDmg = attack.damage;

    m_health = std::max(0, m_health - incomingDmg);
    m_recoverableHealth = std::max(m_health, m_recoverableHealth - (incomingDmg / 2));

    m_stunFrames = std::max(12, attack.recoveryFrames / 2);
    m_actionState = ActionState::HIT_STUN;

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
    m_stunFrames = 35;
}

void Fighter::DrawProcedural3DMannequin() {
    float crouchY = (m_stance == StanceState::CROUCH) ? -0.42f : 0.0f;
    float breathe = std::sin(m_animTime * 4.0f) * 0.025f;
    float bounce = (m_actionState == ActionState::IDLE) ? std::abs(std::sin(m_animTime * 6.0f)) * 0.035f : 0.0f;
    Vector3 root = Vector3{m_position.x, m_position.y + crouchY, m_position.z};

    float rad = -m_yaw * DEG2RAD;
    Vector3 fwd = {std::cos(rad), 0.0f, std::sin(rad)};
    Vector3 rgt = {-std::sin(rad), 0.0f, std::cos(rad)};

    Color jointColor = Color{45, 50, 60, 255};
    Color skinColor = m_themeColor;

    if (IsInRage()) {
        float auraPulse = (std::sin(m_animTime * 12.0f) + 1.0f) * 0.5f;
        skinColor = ColorLerp(m_themeColor, Color{239, 68, 68, 255}, 0.5f + auraPulse * 0.4f);
    }

    // Motion parameters
    float t = (m_actionState == ActionState::ATTACKING) ? m_limbExtension : 0.0f;
    float snapT = t * t * (3.0f - 2.0f * t); // Smoothstep easing for crisp martial arts strikes
    AttackType atkType = (m_actionState == ActionState::ATTACKING) ? m_currentAttack.type : AttackType::NONE;

    // 1. Pelvis / Hips
    Vector3 pelvis = Vector3Add(root, Vector3{0.0f, 0.95f + breathe + bounce, 0.0f});
    if (atkType == AttackType::LOW_SWEEP) {
        pelvis.y -= 0.38f * snapT;
    } else if (atkType == AttackType::FLYING_DIVE_DROPKICK) {
        pelvis = Vector3Add(pelvis, Vector3Add(Vector3Scale(fwd, 0.35f * snapT), Vector3{0.0f, 0.15f * snapT, 0.0f}));
    } else if (atkType == AttackType::UPPERCUT_LAUNCHER) {
        pelvis.y += 0.15f * snapT;
    }
    ::DrawSphere(pelvis, 0.22f, jointColor);

    // Belts & Sashes
    if (m_characterId == 0) {
        // OODIII JII: Gold Obi Belt with hanging knot
        ::DrawCylinderEx(Vector3Add(pelvis, Vector3{0.0f, -0.06f, 0.0f}), Vector3Add(pelvis, Vector3{0.0f, 0.06f, 0.0f}), 0.24f, 0.24f, 8, Color{234, 179, 8, 255});
        Vector3 knot = Vector3Add(pelvis, Vector3Add(Vector3Scale(fwd, 0.22f), Vector3Scale(rgt, 0.08f)));
        ::DrawCube(Vector3Add(knot, Vector3{0.0f, -0.15f, 0.0f}), 0.06f, 0.25f, 0.04f, Color{234, 179, 8, 255});
    } else if (m_characterId == 1) {
        // LUHAR IHDNAG: Silver Kung-Fu Sash
        ::DrawCylinderEx(Vector3Add(pelvis, Vector3{0.0f, -0.05f, 0.0f}), Vector3Add(pelvis, Vector3{0.0f, 0.05f, 0.0f}), 0.24f, 0.24f, 8, Color{220, 225, 235, 255});
    } else if (m_characterId == 2) {
        // WINPAK: Dark Devil Belt
        ::DrawCylinderEx(Vector3Add(pelvis, Vector3{0.0f, -0.05f, 0.0f}), Vector3Add(pelvis, Vector3{0.0f, 0.05f, 0.0f}), 0.24f, 0.24f, 8, Color{20, 20, 25, 255});
    } else if (m_characterId == 3) {
        // GANJA: Tactical Skull Belt Buckle & Kangaroo tail
        Vector3 buckle = Vector3Add(pelvis, Vector3Scale(fwd, 0.23f));
        ::DrawCube(buckle, 0.10f, 0.08f, 0.04f, Color{210, 215, 220, 255});

        // Articulated curved Kangaroo tail
        Vector3 tailBase = Vector3Add(pelvis, Vector3Scale(fwd, -0.18f));
        Vector3 tailMid = Vector3Add(tailBase, Vector3Add(Vector3Scale(fwd, -0.42f), Vector3{0.0f, -0.28f, 0.0f}));
        Vector3 tailTip = Vector3Add(tailMid, Vector3Add(Vector3Scale(fwd, -0.32f), Vector3{0.0f, 0.18f, 0.0f}));
        ::DrawCylinderEx(tailBase, tailMid, 0.10f, 0.07f, 6, Color{130, 80, 50, 255});
        ::DrawCylinderEx(tailMid, tailTip, 0.07f, 0.03f, 6, Color{130, 80, 50, 255});
    }

    // 2. Torso / Chest (Articulates dynamically with strikes)
    Vector3 chest = Vector3Add(pelvis, Vector3{0.0f, 0.45f, 0.0f});
    if (atkType == AttackType::DUAL_STRIKE) {
        chest = Vector3Add(chest, Vector3Scale(fwd, 0.32f * snapT));
    } else if (atkType == AttackType::FLYING_DIVE_DROPKICK) {
        chest = Vector3Add(chest, Vector3Add(Vector3Scale(fwd, 0.45f * snapT), Vector3{0.0f, -0.20f * snapT, 0.0f}));
    } else if (atkType == AttackType::UPPERCUT_LAUNCHER) {
        chest = Vector3Add(chest, Vector3Add(Vector3Scale(fwd, 0.15f * snapT), Vector3{0.0f, 0.20f * snapT, 0.0f}));
    } else if (atkType == AttackType::LOW_SWEEP) {
        chest = Vector3Add(chest, Vector3{0.0f, -0.25f * snapT, 0.0f});
    } else if (m_actionState == ActionState::HIT_STUN) {
        chest = Vector3Add(chest, Vector3Scale(fwd, -0.22f));
    } else if (m_actionState == ActionState::JUGGLED) {
        chest = Vector3Add(chest, Vector3Scale(fwd, -0.35f));
    }

    ::DrawCube(chest, 0.42f, 0.45f, 0.26f, skinColor);
    ::DrawCubeWires(chest, 0.42f, 0.45f, 0.26f, jointColor);

    // 3. Head
    Vector3 head = Vector3Add(chest, Vector3{0.0f, 0.42f, 0.0f});
    ::DrawSphere(head, 0.20f, skinColor);

    // Eye visor
    Vector3 eyePos = Vector3Add(head, Vector3Scale(fwd, 0.16f));
    eyePos.y += 0.04f;
    Color eyeColor = (m_characterId == 0) ? Color{250, 204, 21, 255} : // Golden Mishima lightning eyes
                     (m_characterId == 1) ? Color{56, 189, 248, 255} :  // Jade Blue eyes
                     (m_characterId == 2) ? Color{239, 68, 68, 255} :   // Devil red eyes
                     Color{250, 204, 21, 255};                           // Bryan Fury cybernetic yellow eye
    ::DrawCube(eyePos, 0.12f, 0.06f, 0.26f, eyeColor);

    // Character 0 (OODIII JII): Heihachi white side-wing horns!
    if (m_characterId == 0) {
        Vector3 lHorn = Vector3Add(head, Vector3Add(Vector3Scale(rgt, -0.18f), Vector3{0.0f, 0.08f, -0.05f}));
        Vector3 rHorn = Vector3Add(head, Vector3Add(Vector3Scale(rgt, 0.18f), Vector3{0.0f, 0.08f, -0.05f}));
        Vector3 lHornTip = Vector3Add(lHorn, Vector3Add(Vector3Scale(rgt, -0.12f), Vector3{0.0f, 0.16f, 0.0f}));
        Vector3 rHornTip = Vector3Add(rHorn, Vector3Add(Vector3Scale(rgt, 0.12f), Vector3{0.0f, 0.16f, 0.0f}));
        ::DrawCylinderEx(lHorn, lHornTip, 0.08f, 0.02f, 5, Color{240, 240, 240, 255});
        ::DrawCylinderEx(rHorn, rHornTip, 0.08f, 0.02f, 5, Color{240, 240, 240, 255});
    }

    // Character 1 (LUHAR IHDNAG): Headband twin flowing ribbons
    if (m_characterId == 1) {
        Vector3 bandBase = Vector3Add(head, Vector3Add(Vector3Scale(fwd, -0.18f), Vector3{0.0f, 0.05f, 0.0f}));
        float sway = std::sin(m_animTime * 6.0f) * 0.08f;
        Vector3 ribbonEnd1 = Vector3Add(bandBase, Vector3Add(Vector3Scale(fwd, -0.35f + sway), Vector3{-0.06f, -0.25f, 0.0f}));
        Vector3 ribbonEnd2 = Vector3Add(bandBase, Vector3Add(Vector3Scale(fwd, -0.32f - sway), Vector3{ 0.06f, -0.22f, 0.0f}));
        ::DrawLine3D(bandBase, ribbonEnd1, Color{220, 38, 38, 255});
        ::DrawLine3D(bandBase, ribbonEnd2, Color{220, 38, 38, 255});
    }

    // Character 2 (WINPAK): Jin Kazama spiky fringe
    if (m_characterId == 2) {
        Vector3 fringe = Vector3Add(head, Vector3Add(Vector3Scale(fwd, 0.14f), Vector3{0.0f, 0.16f, 0.0f}));
        ::DrawCube(fringe, 0.08f, 0.10f, 0.20f, Color{20, 20, 25, 255});
    }

    // 4. Arms & Fists (ELBOW JOINTS & DYNAMIC PUNCH ARTICULATION)
    Vector3 lShoulder = Vector3Add(chest, Vector3Add(Vector3Scale(rgt, -0.28f), Vector3{0.0f, 0.15f, 0.0f}));
    Vector3 rShoulder = Vector3Add(chest, Vector3Add(Vector3Scale(rgt, 0.28f), Vector3{0.0f, 0.15f, 0.0f}));

    // Default Tekken high combat guard:
    Vector3 lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.40f), Vector3{0.0f, 0.02f, 0.0f}));
    Vector3 rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.30f), Vector3{0.0f, -0.04f, 0.0f}));

    // Attack / Stance Articulation for Arms:
    if (atkType == AttackType::PUNCH_LEFT) {
        // 1: Left Flash Jab / Lead Hook
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.40f + 0.68f * snapT), Vector3{0.0f, 0.05f * snapT, 0.0f}));
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.26f), Vector3{0.0f, 0.05f, 0.0f})); // guards chin
    } else if (atkType == AttackType::PUNCH_RIGHT) {
        // 2: Right Power Straight / Cross / Body Blow
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.30f + 0.88f * snapT), Vector3{0.0f, -0.04f * snapT, 0.0f}));
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.28f), Vector3{0.0f, 0.06f, 0.0f})); // guards face
    } else if (atkType == AttackType::UPPERCUT_LAUNCHER) {
        // Down+2: Electric Wind God Fist / Jet Upper / Rising Elbow Launcher!
        // Right arm explodes straight UPWARD toward the sky!
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.35f * snapT), Vector3{0.0f, 0.95f * snapT, 0.0f}));
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.25f), Vector3{0.0f, -0.15f * snapT, 0.0f}));
    } else if (atkType == AttackType::DUAL_STRIKE) {
        // 1+2: Demon Breath Palm / Nuclear Mach Punch / Tetsuzanko
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.40f + 0.72f * snapT), Vector3{0.0f, 0.0f, 0.0f}));
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.30f + 0.82f * snapT), Vector3{0.0f, 0.0f, 0.0f}));
    } else if (atkType == AttackType::THROW_SLAM) {
        // Throw Grab
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.55f * snapT), Vector3{0.0f, 0.10f, 0.0f}));
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.55f * snapT), Vector3{0.0f, 0.10f, 0.0f}));
    } else if (atkType == AttackType::RAGE_ART) {
        // Rage Art Cinematic Strike Flurry
        float flurry = std::sin(m_animTime * 24.0f) * 0.4f;
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, 0.40f + 0.70f * (snapT + flurry)), Vector3{0.0f, 0.0f, 0.0f}));
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, 0.30f + 0.70f * (snapT - flurry)), Vector3{0.0f, 0.0f, 0.0f}));
    } else if (IsBlocking()) {
        // Tekken Guard: crossed forearms protecting head & ribs!
        lFist = Vector3Add(chest, Vector3Add(Vector3Scale(fwd, 0.24f), Vector3Add(Vector3Scale(rgt, 0.06f), Vector3{0.0f, 0.20f, 0.0f})));
        rFist = Vector3Add(chest, Vector3Add(Vector3Scale(fwd, 0.22f), Vector3Add(Vector3Scale(rgt, -0.06f), Vector3{0.0f, 0.18f, 0.0f})));
    } else if (m_actionState == ActionState::HIT_STUN || m_actionState == ActionState::JUGGLED) {
        // Flinching back
        lFist = Vector3Add(lShoulder, Vector3Add(Vector3Scale(fwd, -0.15f), Vector3{0.0f, 0.15f, 0.0f}));
        rFist = Vector3Add(rShoulder, Vector3Add(Vector3Scale(fwd, -0.15f), Vector3{0.0f, 0.15f, 0.0f}));
    }

    // Articulated Elbow joints:
    Vector3 lElbow = Vector3Lerp(lShoulder, lFist, 0.5f);
    Vector3 rElbow = Vector3Lerp(rShoulder, rFist, 0.5f);
    if (atkType == AttackType::UPPERCUT_LAUNCHER) {
        rElbow = Vector3Add(rElbow, Vector3Add(Vector3Scale(fwd, 0.20f), Vector3{0.0f, 0.40f * snapT, 0.0f}));
    } else {
        lElbow = Vector3Add(lElbow, Vector3Add(Vector3Scale(rgt, -0.14f), Vector3{0.0f, -0.10f, 0.0f}));
        rElbow = Vector3Add(rElbow, Vector3Add(Vector3Scale(rgt, 0.14f), Vector3{0.0f, -0.10f, 0.0f}));
    }

    Color gloveColor = skinColor;
    if (m_characterId == 3) {
        gloveColor = Color{220, 38, 38, 255}; // Red boxing gloves for Ganja / Kangaroo!
    } else if (m_characterId == 2) {
        gloveColor = Color{185, 28, 28, 255}; // Jin's crimson gauntlets
    }

    // DRAW ARTICULATED ARMS (BICEPS, ELBOWS, FOREARMS & FISTS):
    // Left arm
    ::DrawCylinderEx(lShoulder, lElbow, 0.08f, 0.07f, 6, jointColor);
    ::DrawSphere(lElbow, 0.075f, jointColor);
    ::DrawCylinderEx(lElbow, lFist, 0.07f, 0.065f, 6, jointColor);
    ::DrawSphere(lFist, (m_characterId == 3) ? 0.16f : 0.12f, gloveColor);

    // Right arm
    ::DrawCylinderEx(rShoulder, rElbow, 0.08f, 0.07f, 6, jointColor);
    ::DrawSphere(rElbow, 0.075f, jointColor);
    ::DrawCylinderEx(rElbow, rFist, 0.07f, 0.065f, 6, jointColor);
    ::DrawSphere(rFist, (m_characterId == 3) ? 0.16f : 0.12f, gloveColor);

    // 5. Legs & Feet (KNEE JOINTS & DYNAMIC KICK ARTICULATION)
    Vector3 lHip = Vector3Add(pelvis, Vector3Scale(rgt, -0.16f));
    Vector3 rHip = Vector3Add(pelvis, Vector3Scale(rgt, 0.16f));

    // Stride walk cycle
    float walkCycle = 0.0f;
    float walkLiftL = 0.0f;
    float walkLiftR = 0.0f;
    if (m_actionState == ActionState::WALK_FORWARD || m_actionState == ActionState::WALK_BACK) {
        walkCycle = std::sin(m_animTime * 9.0f);
        if (walkCycle > 0.0f) walkLiftL = walkCycle * 0.14f;
        else walkLiftR = -walkCycle * 0.14f;
    }

    Vector3 lFoot = Vector3Add(root, Vector3Add(Vector3Scale(rgt, -0.22f), Vector3Scale(fwd, 0.16f + walkCycle * 0.26f)));
    Vector3 rFoot = Vector3Add(root, Vector3Add(Vector3Scale(rgt, 0.22f), Vector3Scale(fwd, -0.16f - walkCycle * 0.26f)));
    lFoot.y += walkLiftL;
    rFoot.y += walkLiftR;

    Vector3 lKnee = Vector3Lerp(lHip, lFoot, 0.5f);
    Vector3 rKnee = Vector3Lerp(rHip, rFoot, 0.5f);
    lKnee = Vector3Add(lKnee, Vector3Scale(fwd, 0.10f));
    rKnee = Vector3Add(rKnee, Vector3Scale(fwd, 0.08f));

    // Dynamic Attack Articulation for Legs:
    if (atkType == AttackType::KICK_LEFT) {
        // 3: Left Snap Kick / Mid Round Kick / Low Shin Check
        float kickH = (m_currentAttack.height == AttackHeight::LOW) ? 0.22f : 1.15f;
        lKnee = Vector3Add(lHip, Vector3Add(Vector3Scale(fwd, 0.52f * snapT), Vector3{0.0f, 0.38f * snapT, 0.0f}));
        lFoot = Vector3Add(root, Vector3Add(Vector3Scale(fwd, 0.98f * snapT), Vector3{0.0f, kickH * snapT, 0.0f}));
        lFoot = Vector3Add(lFoot, Vector3Scale(rgt, -0.08f));
        rKnee = Vector3Add(rKnee, Vector3Scale(fwd, 0.05f));
    } else if (atkType == AttackType::KICK_RIGHT) {
        // 4: Right Roundhouse Kick / Brutal Knee Smash
        if (m_characterId == 3) {
            // Ganja Knee Smash: Right knee drives violently upward!
            rKnee = Vector3Add(rHip, Vector3Add(Vector3Scale(fwd, 0.75f * snapT), Vector3{0.0f, 0.48f * snapT, 0.0f}));
            rFoot = Vector3Add(rKnee, Vector3Add(Vector3Scale(fwd, -0.08f), Vector3{0.0f, -0.32f, 0.0f}));
        } else {
            // High Roundhouse Whip Kick to head height!
            rKnee = Vector3Add(rHip, Vector3Add(Vector3Scale(fwd, 0.45f * snapT), Vector3Add(Vector3Scale(rgt, 0.22f * snapT), Vector3{0.0f, 0.45f * snapT, 0.0f})));
            rFoot = Vector3Add(root, Vector3Add(Vector3Scale(fwd, 0.94f * snapT), Vector3{0.0f, 1.50f * snapT, 0.0f}));
            rFoot = Vector3Add(rFoot, Vector3Scale(rgt, 0.10f));
        }
    } else if (atkType == AttackType::LOW_SWEEP) {
        // d+3: Hell Sweep / Snake Edge / Dragon Low Sweep
        // Leg sweeps low across the floor in a wide arc!
        float sweepAngle = snapT * 3.14159f;
        lFoot = Vector3Add(root, Vector3Add(Vector3Scale(rgt, -0.28f), Vector3Scale(fwd, -0.15f)));
        lKnee = Vector3Lerp(lHip, lFoot, 0.5f);
        rKnee = Vector3Add(rHip, Vector3Add(Vector3Scale(fwd, 0.50f * snapT), Vector3{0.0f, -0.35f * snapT, 0.0f}));
        rFoot = Vector3Add(root, Vector3Add(Vector3Scale(fwd, 1.05f * std::sin(sweepAngle)), Vector3Add(Vector3Scale(rgt, 0.45f * std::cos(sweepAngle)), Vector3{0.0f, 0.08f, 0.0f})));
    } else if (atkType == AttackType::FLYING_DIVE_DROPKICK) {
        // 3+4: Raiden Electric Torpedo Dive / Kangaroo Double Dropkick / Can-Can Kicks
        if (m_characterId == 2) {
            // Can-Can double rising kicks (Jin)
            lKnee = Vector3Add(lHip, Vector3Add(Vector3Scale(fwd, 0.45f * snapT), Vector3{0.0f, 0.48f * snapT, 0.0f}));
            rKnee = Vector3Add(rHip, Vector3Add(Vector3Scale(fwd, 0.55f * snapT), Vector3{0.0f, 0.58f * snapT, 0.0f}));
            lFoot = Vector3Add(root, Vector3Add(Vector3Scale(fwd, 0.75f * snapT), Vector3{0.0f, 1.30f * snapT, 0.0f}));
            rFoot = Vector3Add(root, Vector3Add(Vector3Scale(fwd, 0.95f * snapT), Vector3{0.0f, 1.65f * snapT, 0.0f}));
        } else {
            // Horizontal flying dive: both legs shoot straight forward!
            lKnee = Vector3Add(lHip, Vector3Add(Vector3Scale(fwd, 0.55f * snapT), Vector3{0.0f, 0.12f * snapT, 0.0f}));
            rKnee = Vector3Add(rHip, Vector3Add(Vector3Scale(fwd, 0.55f * snapT), Vector3{0.0f, 0.12f * snapT, 0.0f}));
            lFoot = Vector3Add(pelvis, Vector3Add(Vector3Scale(fwd, 1.05f * snapT), Vector3Add(Vector3Scale(rgt, -0.12f), Vector3{0.0f, 0.08f * snapT, 0.0f})));
            rFoot = Vector3Add(pelvis, Vector3Add(Vector3Scale(fwd, 1.05f * snapT), Vector3Add(Vector3Scale(rgt, 0.12f), Vector3{0.0f, 0.08f * snapT, 0.0f})));
        }
    }

    // DRAW ARTICULATED LEGS (THIGHS, KNEES, SHINS & FEET):
    // Left leg
    ::DrawCylinderEx(lHip, lKnee, 0.11f, 0.09f, 8, jointColor);
    ::DrawSphere(lKnee, 0.095f, jointColor);
    ::DrawCylinderEx(lKnee, lFoot, 0.09f, 0.075f, 8, jointColor);
    ::DrawCube(lFoot, 0.14f, 0.10f, 0.28f, skinColor);

    // Right leg
    ::DrawCylinderEx(rHip, rKnee, 0.11f, 0.09f, 8, jointColor);
    ::DrawSphere(rKnee, 0.095f, jointColor);
    ::DrawCylinderEx(rKnee, rFoot, 0.09f, 0.075f, 8, jointColor);
    ::DrawCube(rFoot, 0.14f, 0.10f, 0.28f, skinColor);

    // WINPAK: Crimson flame stripe on right leg
    if (m_characterId == 2) {
        Vector3 flameLeg = Vector3Lerp(rHip, rFoot, 0.5f);
        ::DrawCube(flameLeg, 0.14f, 0.25f, 0.12f, Color{225, 29, 72, 240});
    }

    // Elemental Strike FX:
    if (m_actionState == ActionState::ATTACKING) {
        if (m_characterId == 0) {
            // Mishima / Raiden Thunder lightning sparks
            float sparkOsc = std::sin(m_animTime * 25.0f) * 0.12f;
            Vector3 sparkPos = (atkType == AttackType::KICK_LEFT || atkType == AttackType::KICK_RIGHT || atkType == AttackType::LOW_SWEEP) ? rFoot : rFist;
            ::DrawSphere(Vector3Add(sparkPos, Vector3{sparkOsc, 0.05f, 0.0f}), 0.10f, Color{250, 204, 21, 230});
            ::DrawLine3D(sparkPos, Vector3Add(sparkPos, Vector3{sparkOsc * 2.0f, 0.25f, -sparkOsc}), Color{255, 240, 100, 255});
        } else if (m_characterId == 2) {
            // Jin Devil Red lightning
            float sparkOsc = std::cos(m_animTime * 24.0f) * 0.10f;
            Vector3 sparkPos = (atkType == AttackType::KICK_LEFT || atkType == AttackType::KICK_RIGHT || atkType == AttackType::FLYING_DIVE_DROPKICK) ? rFoot : rFist;
            ::DrawSphere(Vector3Add(sparkPos, Vector3{0.0f, sparkOsc, 0.0f}), 0.10f, Color{239, 68, 68, 230});
        }
    }

    // Dynamic Altitude-Scaled Soft Shadow
    float groundDist = std::max(0.0f, m_position.y);
    float shadowScale = std::clamp(1.0f - (groundDist * 0.12f), 0.35f, 1.0f);
    unsigned char shadowAlpha = static_cast<unsigned char>(std::clamp(120.0f - (groundDist * 25.0f), 25.0f, 120.0f));
    ::DrawCircle3D(Vector3{m_position.x, 0.02f, m_position.z}, 0.58f * shadowScale, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, Color{0, 0, 0, shadowAlpha});
}

void Fighter::Draw3D(bool showHitboxes) {
    if (m_hasBlenderModel) {
        Vector3 rotAxis = {0.0f, 1.0f, 0.0f};
        ::DrawModelEx(m_model, m_position, rotAxis, m_yaw, Vector3{1.0f, 1.0f, 1.0f}, WHITE);
    } else {
        DrawProcedural3DMannequin();
    }

    if (showHitboxes) {
        auto hurts = GetHurtboxes();
        for (const auto& box : hurts) {
            ::DrawBoundingBox(box, Color{34, 197, 94, 255});
        }

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
