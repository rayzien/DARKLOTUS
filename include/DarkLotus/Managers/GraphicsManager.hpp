#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Managers/SettingsManager.hpp"

namespace DarkLotus {

struct HitParticle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float size;
    float lifeTime;
    float maxLifeTime;
};

class GraphicsManager {
public:
    GraphicsManager();
    ~GraphicsManager();

    bool Init(const SettingsManager& settings);
    void Shutdown();

    // Camera control (Tekken 3D dual-fighter tracking camera)
    void UpdateCamera(const Vector3& p1Pos, const Vector3& p2Pos, float deltaTime);
    Camera3D GetCamera() const { return m_camera; }
    void AddCameraTrauma(float trauma); // Screen shake on impact

    // Slow motion control (for KO slow-mo or Counter-hit freeze)
    void TriggerSlowMo(float durationSeconds, float speedScale = 0.25f);
    void UpdateSlowMo(float deltaTime);
    float GetTimeScale() const { return m_timeScale; }

    // Particle FX (Tekken electric / hit sparks)
    void SpawnHitSparks(const Vector3& worldPos, Color sparkColor, int count = 25);
    void UpdateParticles(float deltaTime);
    void DrawParticles();

    // Render loop wrapping
    void BeginFrame();
    void EndFrame();
    void Begin3D();
    void End3D();

    int GetRenderWidth() const { return ::GetRenderWidth(); }
    int GetRenderHeight() const { return ::GetRenderHeight(); }

private:
    Camera3D m_camera{};
    Vector3 m_targetCameraPos{0.0f, 2.5f, 7.0f};
    Vector3 m_targetLookAt{0.0f, 1.2f, 0.0f};

    // Camera shake
    float m_shakeTrauma{0.0f};
    float m_shakeTime{0.0f};

    // Slow-mo
    float m_timeScale{1.0f};
    float m_slowMoTimer{0.0f};

    // Particle pool
    std::vector<HitParticle> m_particles;
};

} // namespace DarkLotus
