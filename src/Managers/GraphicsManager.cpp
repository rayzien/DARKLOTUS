#include "DarkLotus/Managers/GraphicsManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

GraphicsManager::GraphicsManager() = default;
GraphicsManager::~GraphicsManager() = default;

bool GraphicsManager::Init(const SettingsManager& settings) {
    const auto& vid = settings.GetVideo();

    unsigned int flags = FLAG_WINDOW_RESIZABLE;
    if (vid.vsync) flags |= FLAG_VSYNC_HINT;
    if (vid.fullscreen) flags |= FLAG_FULLSCREEN_MODE;
    flags |= FLAG_MSAA_4X_HINT; // Crisp 3D geometry anti-aliasing

    ::SetConfigFlags(flags);
    ::InitWindow(vid.screenWidth, vid.screenHeight, GAME_NAME);
    ::SetTargetFPS(vid.targetFps);

    // Initialize Tekken-style 3D Camera
    m_camera.position = Vector3{0.0f, 2.8f, 7.5f};
    m_camera.target = Vector3{0.0f, 1.4f, 0.0f};
    m_camera.up = Vector3{0.0f, 1.0f, 0.0f};
    m_camera.fovy = 45.0f;
    m_camera.projection = CAMERA_PERSPECTIVE;

    m_particles.reserve(500);

    Logger::Info("GraphicsManager", "3D Graphics pipeline initialized successfully.");
    return true;
}

void GraphicsManager::Shutdown() {
    ::CloseWindow();
    Logger::Info("GraphicsManager", "Graphics engine shut down.");
}

void GraphicsManager::UpdateCamera(const Vector3& p1Pos, const Vector3& p2Pos, float deltaTime) {
    // 1. Calculate midpoint between both fighters (Tekken dynamic center)
    Vector3 midPoint = Vector3{
        (p1Pos.x + p2Pos.x) * 0.5f,
        (p1Pos.y + p2Pos.y) * 0.5f + 1.3f, // Target chest height
        (p1Pos.z + p2Pos.z) * 0.5f
    };

    // 2. Calculate distance between fighters to adjust camera distance (zoom)
    float fighterDist = Vector3Distance(p1Pos, p2Pos);
    float desiredDist = std::clamp(fighterDist * 1.35f + 4.5f, 5.0f, 11.0f);

    // 3. Normal vector perpendicular to the line connecting fighters (for 3D side framing)
    Vector3 diff = Vector3Subtract(p2Pos, p1Pos);
    Vector3 perp = Vector3Normalize(Vector3{-diff.z, 0.0f, diff.x});
    if (perp.z < 0.0f) {
        perp = Vector3Scale(perp, -1.0f);
    }
    // Prevent zero division
    if (Vector3Length(perp) < 0.001f) {
        perp = Vector3{0.0f, 0.0f, 1.0f};
    }

    // Camera target position with elevation
    Vector3 desiredPos = Vector3{
        midPoint.x + perp.x * desiredDist * 0.25f,
        midPoint.y + 1.2f + (desiredDist * 0.12f),
        midPoint.z + desiredDist
    };

    // Smoothly interpolate (spring-damper / lerp) camera position and target
    float lerpSpeed = 8.0f * deltaTime;
    m_camera.position = Vector3Lerp(m_camera.position, desiredPos, lerpSpeed);
    m_camera.target = Vector3Lerp(m_camera.target, midPoint, lerpSpeed);

    // 4. Camera Trauma / Shake
    if (m_shakeTrauma > 0.001f) {
        m_shakeTime += deltaTime * 25.0f;
        float shakeAmount = m_shakeTrauma * m_shakeTrauma; // Non-linear decay
        float offsetX = std::sin(m_shakeTime * 1.7f) * shakeAmount * 0.35f;
        float offsetY = std::cos(m_shakeTime * 2.3f) * shakeAmount * 0.35f;

        m_camera.position.x += offsetX;
        m_camera.position.y += offsetY;
        m_camera.target.x += offsetX * 0.5f;

        // Decay trauma over time
        m_shakeTrauma = std::max(0.0f, m_shakeTrauma - deltaTime * 2.5f);
    }
}

void GraphicsManager::AddCameraTrauma(float trauma) {
    m_shakeTrauma = std::clamp(m_shakeTrauma + trauma, 0.0f, 1.0f);
}

void GraphicsManager::TriggerSlowMo(float durationSeconds, float speedScale) {
    m_slowMoTimer = durationSeconds;
    m_timeScale = speedScale;
}

void GraphicsManager::UpdateSlowMo(float deltaTime) {
    if (m_slowMoTimer > 0.0f) {
        m_slowMoTimer -= deltaTime;
        if (m_slowMoTimer <= 0.0f) {
            m_timeScale = 1.0f; // Return to normal speed
        }
    }
}

void GraphicsManager::SpawnHitSparks(const Vector3& worldPos, Color sparkColor, int count) {
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(::GetRandomValue(0, 360)) * DEG2RAD;
        float elev = static_cast<float>(::GetRandomValue(-40, 60)) * DEG2RAD;
        float speed = static_cast<float>(::GetRandomValue(40, 160)) * 0.1f;

        Vector3 vel = {
            std::cos(angle) * std::cos(elev) * speed,
            std::sin(elev) * speed + 2.0f,
            std::sin(angle) * std::cos(elev) * speed
        };

        float life = static_cast<float>(::GetRandomValue(20, 45)) * 0.01f;
        float sz = static_cast<float>(::GetRandomValue(6, 16)) * 0.01f;

        m_particles.push_back({worldPos, vel, sparkColor, sz, life, life});
    }
}

void GraphicsManager::UpdateParticles(float deltaTime) {
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->lifeTime -= deltaTime;
        if (it->lifeTime <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            // Gravity and velocity
            it->velocity.y -= 15.0f * deltaTime;
            it->position = Vector3Add(it->position, Vector3Scale(it->velocity, deltaTime));
            ++it;
        }
    }
}

void GraphicsManager::DrawParticles() {
    ::rlSetBlendMode(BLEND_ADDITIVE);
    for (const auto& p : m_particles) {
        float alpha = p.lifeTime / p.maxLifeTime;
        Color c = p.color;
        c.a = static_cast<unsigned char>(alpha * 255.0f);

        // Draw glowing 3D diamond/cube spark
        ::DrawCube(p.position, p.size, p.size, p.size, c);
        ::DrawCubeWires(p.position, p.size * 1.2f, p.size * 1.2f, p.size * 1.2f, Color{255, 255, 255, c.a});
    }
    ::rlSetBlendMode(BLEND_ALPHA);
}

void GraphicsManager::BeginFrame() {
    ::BeginDrawing();
    ::ClearBackground(Color{12, 14, 20, 255}); // Dark moody atmospheric background
}

void GraphicsManager::EndFrame() {
    ::EndDrawing();
}

void GraphicsManager::Begin3D() {
    ::BeginMode3D(m_camera);
}

void GraphicsManager::End3D() {
    ::EndMode3D();
}

} // namespace DarkLotus
