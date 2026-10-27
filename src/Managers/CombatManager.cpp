#include "DarkLotus/Managers/CombatManager.hpp"
#include "DarkLotus/Managers/GraphicsManager.hpp"
#include "DarkLotus/Managers/SoundManager.hpp"
#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

CombatManager::CombatManager() = default;
CombatManager::~CombatManager() = default;

void CombatManager::Init() {
    m_recentEvents.clear();
    m_comboCount[0] = 0;
    m_comboCount[1] = 0;
    m_comboDamage[0] = 0;
    m_comboDamage[1] = 0;
    Logger::Info("CombatManager", "Tekken 3D Hitbox/Hurtbox and Juggle combat engine initialized.");
}

bool CombatManager::CheckCollision3D(const Hitbox3D& hitbox, const Vector3& attackerPos, float attackerYaw,
                                     const Fighter* defender, Vector3* outContactPoint) {
    if (!defender) return false;

    // Calculate world position of attacker's hitbox
    float rad = -attackerYaw * DEG2RAD;
    Vector3 fwd = {std::cos(rad), 0.0f, std::sin(rad)};
    Vector3 worldHitPos = Vector3Add(attackerPos, Vector3{
        fwd.x * hitbox.offset.x,
        hitbox.offset.y,
        fwd.z * hitbox.offset.x
    });

    // Check collision against all defender hurtboxes
    auto hurtboxes = defender->GetHurtboxes();
    for (const auto& box : hurtboxes) {
        if (::CheckCollisionBoxSphere(box, worldHitPos, hitbox.radius)) {
            if (outContactPoint) {
                // Approximate contact point
                *outContactPoint = Vector3Lerp(worldHitPos, Vector3{
                    (box.min.x + box.max.x) * 0.5f,
                    (box.min.y + box.max.y) * 0.5f,
                    (box.min.z + box.max.z) * 0.5f
                }, 0.5f);
            }
            return true;
        }
    }
    return false;
}

void CombatManager::ResolveHit(Fighter* attacker, Fighter* defender, const Hitbox3D& attack,
                              const Vector3& contactPoint, GraphicsManager* graphics, SoundManager* sounds, ButtonManager* buttons) {
    int atkIdx = attacker->GetPlayerIndex();
    int defIdx = defender->GetPlayerIndex();

    bool isCounterHit = (defender->GetActionState() == ActionState::ATTACKING);
    bool isBlocked = false;

    // ==========================================
    // TEKKEN GUARD EVALUATION
    // ==========================================
    if (defender->IsBlocking()) {
        if (defender->IsCrouching()) {
            // Crouch Guard
            if (attack.height == AttackHeight::HIGH) {
                // High attack whiffs over crouching opponent!
                return;
            } else if (attack.height == AttackHeight::LOW) {
                isBlocked = true; // Blocked low!
            } else if (attack.height == AttackHeight::MID) {
                isBlocked = false; // Mid crushes crouch guard!
            }
        } else {
            // Standing Guard
            if (attack.height == AttackHeight::HIGH || attack.height == AttackHeight::MID || attack.height == AttackHeight::SPECIAL_MID) {
                isBlocked = true; // Blocked!
            } else if (attack.height == AttackHeight::LOW) {
                isBlocked = false; // Low sweeps standing guard!
            }
        }
    }

    if (attack.height == AttackHeight::UNBLOCKABLE) {
        isBlocked = false;
    }

    if (isBlocked) {
        // Attack was blocked!
        sounds->PlaySFX(SoundEffectType::BLOCK);
        graphics->SpawnHitSparks(contactPoint, Color{220, 220, 255, 255}, 12); // Blue/white spark
        graphics->AddCameraTrauma(0.10f);

        if (buttons) {
            buttons->TriggerVibration(atkIdx, 0.15f, 0.15f, 0.08f);
            buttons->TriggerVibration(defIdx, 0.20f, 0.20f, 0.10f);
        }

        // Reset attacker combo
        m_comboCount[atkIdx] = 0;
        m_comboDamage[atkIdx] = 0;

        m_recentEvents.push_back({
            atkIdx, defIdx, attack.moveName, 0, false, true, false, contactPoint
        });
        return;
    }

    // HIT CONFIRMED!
    int finalDamage = attack.damage;
    if (isCounterHit) {
        finalDamage = static_cast<int>(finalDamage * 1.25f); // 25% Counter Hit bonus
    }

    // Air Juggle damage scaling (Tekken juggle combo scaling)
    if (defender->IsAirborne()) {
        int comboHits = m_comboCount[atkIdx];
        float scaling = std::max(0.40f, 1.0f - (comboHits * 0.12f));
        finalDamage = static_cast<int>(finalDamage * scaling);
    }

    // Apply damage & stun to defender
    Hitbox3D finalAttack = attack;
    finalAttack.damage = finalDamage;
    bool isKO = defender->ReceiveHit(finalAttack, attacker);

    // Update Combo Counter
    m_comboCount[atkIdx]++;
    m_comboDamage[atkIdx] += finalDamage;
    m_comboResetTimer[atkIdx] = 70; // 70 frames to drop combo

    // Controller Rumble / Haptic Feedback
    if (buttons) {
        if (isKO) {
            buttons->TriggerVibration(atkIdx, 1.0f, 1.0f, 0.50f);
            buttons->TriggerVibration(defIdx, 1.0f, 1.0f, 0.50f);
        } else if (isCounterHit) {
            buttons->TriggerVibration(atkIdx, 0.80f, 0.80f, 0.25f);
            buttons->TriggerVibration(defIdx, 0.85f, 0.85f, 0.28f);
        } else if (attack.isLauncher || attack.damage >= 1200) {
            buttons->TriggerVibration(atkIdx, 0.55f, 0.55f, 0.18f);
            buttons->TriggerVibration(defIdx, 0.60f, 0.60f, 0.20f);
        } else {
            buttons->TriggerVibration(atkIdx, 0.25f, 0.25f, 0.10f);
            buttons->TriggerVibration(defIdx, 0.35f, 0.35f, 0.12f);
        }
    }

    // Visuals & Sound FX
    if (isKO) {
        sounds->PlaySFX(SoundEffectType::KO_CRASH);
        graphics->AddCameraTrauma(0.70f);
        graphics->TriggerSlowMo(1.5f, 0.20f); // Cinematic KO slow-mo!
    } else if (isCounterHit) {
        sounds->PlaySFX(SoundEffectType::COUNTER_HIT);
        graphics->AddCameraTrauma(0.45f);
        graphics->TriggerSlowMo(0.12f, 0.10f); // Impact frame freeze!
    } else if (attack.isLauncher || attack.damage >= 1200) {
        sounds->PlaySFX(SoundEffectType::PUNCH_HEAVY);
        graphics->AddCameraTrauma(0.35f);
    } else {
        sounds->PlaySFX(SoundEffectType::PUNCH_LIGHT);
        graphics->AddCameraTrauma(0.15f);
    }

    // Tekken elemental hit sparks tailored per character
    Color sparkCol = Color{250, 204, 21, 255};
    int charId = attacker->GetCharacterId();
    if (charId == 0) sparkCol = Color{250, 204, 21, 255};      // Golden Lightning (Heihachi/Raiden)
    else if (charId == 1) sparkCol = Color{34, 197, 94, 255};   // Emerald Chi (Leo Bajiquan)
    else if (charId == 2) sparkCol = Color{239, 68, 68, 255};   // Crimson Flame Omen (Jin Kazama)
    else if (charId == 3) sparkCol = Color{168, 85, 247, 255};  // Toxic Purple & Mach Sparks (Bryan/Kangaroo)

    if (isCounterHit) {
        sparkCol = Color{255, 248, 140, 255}; // Blinding electric white-gold Counter Hit
    }
    graphics->SpawnHitSparks(contactPoint, sparkCol, isCounterHit ? 42 : 24);

    m_recentEvents.push_back({
        atkIdx, defIdx, attack.moveName, finalDamage, isCounterHit, false, attack.isLauncher, contactPoint
    });
}

void CombatManager::Update(Fighter* p1, Fighter* p2, GraphicsManager* graphics, SoundManager* sounds, ButtonManager* buttons, int /*currentFrame*/) {
    if (!p1 || !p2 || !graphics || !sounds) return;

    // Decay combo reset timers
    for (int p = 0; p < 2; ++p) {
        if (m_comboResetTimer[p] > 0) {
            m_comboResetTimer[p]--;
            if (m_comboResetTimer[p] <= 0) {
                m_comboCount[p] = 0;
                m_comboDamage[p] = 0;
            }
        }
    }

    // 1. Check P1 attacking P2
    const Hitbox3D* p1Atk = p1->GetActiveHitbox();
    if (p1Atk) {
        Vector3 contactPoint;
        if (CheckCollision3D(*p1Atk, p1->GetPosition(), p1->GetYaw(), p2, &contactPoint)) {
            ResolveHit(p1, p2, *p1Atk, contactPoint, graphics, sounds, buttons);
        }
    }

    // 2. Check P2 attacking P1
    const Hitbox3D* p2Atk = p2->GetActiveHitbox();
    if (p2Atk) {
        Vector3 contactPoint;
        if (CheckCollision3D(*p2Atk, p2->GetPosition(), p2->GetYaw(), p1, &contactPoint)) {
            ResolveHit(p2, p1, *p2Atk, contactPoint, graphics, sounds, buttons);
        }
    }
}

} // namespace DarkLotus
