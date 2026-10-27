#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"

namespace DarkLotus {

class AnimationManager;

class Fighter {
public:
    Fighter(int playerIndex, const std::string& name, Color themeColor, Vector3 startPos);
    ~Fighter();

    void Init();
    void Update(float deltaTime, int currentFrame, unsigned int inputBits, Fighter* opponent);
    void Draw3D(bool showHitboxes = false);

    // Damage & combat resolution
    bool ReceiveHit(const Hitbox3D& attack, Fighter* attacker);
    void TriggerJuggleLaunch(const Vector3& launchImpulse);
    void TriggerWallSplat();
    void ResetRound(Vector3 spawnPos);

    // Getters
    int GetPlayerIndex() const { return m_playerIndex; }
    const std::string& GetName() const { return m_name; }
    Vector3 GetPosition() const { return m_position; }
    Vector3 GetVelocity() const { return m_velocity; }
    float GetYaw() const { return m_yaw; }
    int GetHealth() const { return m_health; }
    int GetMaxHealth() const { return m_maxHealth; }
    int GetRecoverableHealth() const { return m_recoverableHealth; }
    bool IsInRage() const { return m_health <= m_maxHealth * 0.25f; }
    float GetHeatGauge() const { return m_heatGauge; }
    bool IsHeatActive() const { return m_heatActive; }

    StanceState GetStance() const { return m_stance; }
    ActionState GetActionState() const { return m_actionState; }
    bool IsAttacking() const { return m_actionState == ActionState::ATTACKING; }
    bool IsAirborne() const { return m_position.y > 0.05f || m_actionState == ActionState::JUGGLED; }
    bool IsBlocking() const;
    bool IsCrouching() const { return m_stance == StanceState::CROUCH; }

    // Hitbox / Hurtbox queries
    const Hitbox3D* GetActiveHitbox() const;
    std::vector<BoundingBox> GetHurtboxes() const;

    // Model loading (Blender .glb support)
    bool LoadBlenderModel(const std::string& glbFilePath);

    // Setters
    void SetPosition(const Vector3& pos) { m_position = pos; }
    void SetIsAI(bool isAI) { m_isAI = isAI; }
    bool IsAI() const { return m_isAI; }

private:
    void ExecuteMove(const Hitbox3D& move);
    void UpdatePhysics(float deltaTime);
    void UpdateAttackFrames();
    void UpdateAI(Fighter* opponent, unsigned int& outInputBits);
    void DrawProcedural3DMannequin();

private:
    int m_playerIndex;
    std::string m_name;
    Color m_themeColor;
    bool m_isAI{false};

    Vector3 m_position{0.0f, 0.0f, 0.0f};
    Vector3 m_velocity{0.0f, 0.0f, 0.0f};
    float m_yaw{0.0f};

    int m_health{10000};
    int m_maxHealth{10000};
    int m_recoverableHealth{10000};
    float m_heatGauge{100.0f};
    bool m_heatActive{false};

    StanceState m_stance{StanceState::STAND};
    ActionState m_actionState{ActionState::IDLE};

    // Attack frame data state
    Hitbox3D m_currentAttack{};
    int m_moveFrame{0};
    int m_stunFrames{0};
    bool m_hitboxEmitted{false};

    // Blender Model support
    Model m_model{};
    bool m_hasBlenderModel{false};

    // Procedural mannequin joint animation phases
    float m_animTime{0.0f};
    float m_limbExtension{0.0f}; // For punch / kick visual reach
};

} // namespace DarkLotus
