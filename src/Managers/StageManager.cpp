#include "DarkLotus/Managers/StageManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

StageManager::StageManager() = default;
StageManager::~StageManager() = default;

void StageManager::Init() {
    SetStage(StageType::NEON_DOJO);
    Logger::Info("StageManager", "3D Stage and Arena boundary manager initialized.");
}

void StageManager::SetStage(StageType stageType) {
    switch (stageType) {
        case StageType::NEON_DOJO:
            m_currentStage = {
                "Neon Cyber Dojo",
                StageType::NEON_DOJO,
                16.0f, // 16m wide
                16.0f, // 16m long
                true,  // Has breakable/splat walls!
                Color{22, 24, 34, 255},
                Color{220, 38, 38, 255} // Tekken Red neon
            };
            break;
        case StageType::MISHIMA_SANCTUARY:
            m_currentStage = {
                "Dark Lotus Sanctuary",
                StageType::MISHIMA_SANCTUARY,
                20.0f,
                20.0f,
                true,
                Color{18, 16, 26, 255},
                Color{147, 51, 234, 255} // Dark Lotus Purple
            };
            break;
        case StageType::INFINITE_VOID:
            m_currentStage = {
                "Infinite Astral Void",
                StageType::INFINITE_VOID,
                80.0f,
                80.0f,
                false, // Infinite stage
                Color{10, 10, 18, 255},
                Color{56, 189, 248, 255} // Cyan
            };
            break;
    }
}

void StageManager::Update(float deltaTime) {
    m_gridPulse += deltaTime * 2.0f;
}

void StageManager::Draw3D() {
    float halfW = m_currentStage.arenaWidth * 0.5f;
    float halfL = m_currentStage.arenaLength * 0.5f;

    // 1. Draw floor plane
    ::DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{m_currentStage.arenaWidth, m_currentStage.arenaLength}, m_currentStage.floorColor);

    // 2. Draw 3D tactical grid on floor
    int slices = static_cast<int>(m_currentStage.arenaWidth);
    ::DrawGrid(slices, 1.0f);

    // 3. Center ring decal / Lotus fighting circle
    ::DrawCircle3D(Vector3{0.0f, 0.01f, 0.0f}, 4.0f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, Color{m_currentStage.accentColor.r, m_currentStage.accentColor.g, m_currentStage.accentColor.b, 60});
    ::DrawCircle3D(Vector3{0.0f, 0.01f, 0.0f}, 8.0f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, Color{255, 255, 255, 40});

    // 4. Boundary walls / Tekken octagonal energy pillars
    if (m_currentStage.hasWalls) {
        // Draw 4 corner boundary pillars
        float pillarH = 5.0f;
        Color wallColor = Color{m_currentStage.accentColor.r, m_currentStage.accentColor.g, m_currentStage.accentColor.b, 180};

        ::DrawCube(Vector3{-halfW, pillarH * 0.5f, -halfL}, 0.6f, pillarH, 0.6f, wallColor);
        ::DrawCube(Vector3{ halfW, pillarH * 0.5f, -halfL}, 0.6f, pillarH, 0.6f, wallColor);
        ::DrawCube(Vector3{-halfW, pillarH * 0.5f,  halfL}, 0.6f, pillarH, 0.6f, wallColor);
        ::DrawCube(Vector3{ halfW, pillarH * 0.5f,  halfL}, 0.6f, pillarH, 0.6f, wallColor);

        // Draw boundary floor rails
        ::DrawLine3D(Vector3{-halfW, 0.1f, -halfL}, Vector3{ halfW, 0.1f, -halfL}, m_currentStage.accentColor);
        ::DrawLine3D(Vector3{ halfW, 0.1f, -halfL}, Vector3{ halfW, 0.1f,  halfL}, m_currentStage.accentColor);
        ::DrawLine3D(Vector3{ halfW, 0.1f,  halfL}, Vector3{-halfW, 0.1f,  halfL}, m_currentStage.accentColor);
        ::DrawLine3D(Vector3{-halfW, 0.1f,  halfL}, Vector3{-halfW, 0.1f, -halfL}, m_currentStage.accentColor);

        // Top barrier lines
        ::DrawLine3D(Vector3{-halfW, pillarH, -halfL}, Vector3{ halfW, pillarH, -halfL}, wallColor);
        ::DrawLine3D(Vector3{ halfW, pillarH, -halfL}, Vector3{ halfW, pillarH,  halfL}, wallColor);
        ::DrawLine3D(Vector3{ halfW, pillarH,  halfL}, Vector3{-halfW, pillarH,  halfL}, wallColor);
        ::DrawLine3D(Vector3{-halfW, pillarH,  halfL}, Vector3{-halfW, pillarH, -halfL}, wallColor);
    }
}

bool StageManager::ConstrainFighter(Vector3& inOutPos, bool* outHitWall) const {
    if (!m_currentStage.hasWalls) {
        if (outHitWall) *outHitWall = false;
        return false;
    }

    float halfW = (m_currentStage.arenaWidth * 0.5f) - 0.7f;
    float halfL = (m_currentStage.arenaLength * 0.5f) - 0.7f;
    bool collided = false;

    if (inOutPos.x > halfW) {
        inOutPos.x = halfW;
        collided = true;
    } else if (inOutPos.x < -halfW) {
        inOutPos.x = -halfW;
        collided = true;
    }

    if (inOutPos.z > halfL) {
        inOutPos.z = halfL;
        collided = true;
    } else if (inOutPos.z < -halfL) {
        inOutPos.z = -halfL;
        collided = true;
    }

    if (outHitWall) *outHitWall = collided;
    return collided;
}

} // namespace DarkLotus
