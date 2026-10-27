#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"
#include "DarkLotus/Entities/Fighter.hpp"

namespace DarkLotus {

class GraphicsManager;
class SoundManager;

struct CombatEvent {
    int attackerIndex;
    int defenderIndex;
    std::string moveName;
    int damage;
    bool isCounterHit;
    bool isBlocked;
    bool isLauncher;
    Vector3 hitLocation;
};

class CombatManager {
public:
    CombatManager();
    ~CombatManager();

    void Init();
    void Update(Fighter* p1, Fighter* p2, GraphicsManager* graphics, SoundManager* sounds, int currentFrame);

    const std::vector<CombatEvent>& GetRecentEvents() const { return m_recentEvents; }
    void ClearEvents() { m_recentEvents.clear(); }

    int GetComboCount(int playerIndex) const { return m_comboCount[playerIndex]; }
    int GetComboDamage(int playerIndex) const { return m_comboDamage[playerIndex]; }

private:
    bool CheckCollision3D(const Hitbox3D& hitbox, const Vector3& attackerPos, float attackerYaw,
                          const Fighter* defender, Vector3* outContactPoint);
    void ResolveHit(Fighter* attacker, Fighter* defender, const Hitbox3D& attack,
                    const Vector3& contactPoint, GraphicsManager* graphics, SoundManager* sounds);

private:
    std::vector<CombatEvent> m_recentEvents;
    int m_comboCount[2]{0, 0};
    int m_comboDamage[2]{0, 0};
    int m_comboResetTimer[2]{0, 0};
};

} // namespace DarkLotus
