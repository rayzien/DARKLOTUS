#include "DarkLotus/Managers/StageManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

StageManager::StageManager() = default;

StageManager::~StageManager() {
    UnloadCurrentModel();
}

void StageManager::UnloadCurrentModel() {
    if (m_hasCustomModel) {
        ::UnloadModel(m_stageModel);
        m_hasCustomModel = false;
    }
}

void StageManager::Init() {
    SetStage(StageType::LOTUS_TEMPLE);
    Logger::Info("StageManager", "3D Stage and Arena boundary manager initialized.");
}

void StageManager::SetStage(StageType stageType) {
    UnloadCurrentModel();

    switch (stageType) {
        case StageType::LOTUS_TEMPLE:
            m_currentStage = {
                "The Lotus Temple",
                StageType::LOTUS_TEMPLE,
                21.0f, // 21m diameter circular fighting dais
                21.0f,
                true,  // Has 9 pagoda boundary lanterns for wall splat!
                Color{8, 10, 16, 255},   // Dark obsidian floor
                Color{245, 158, 11, 255} // Lotus Imperial Gold
            };
            if (::FileExists("assets/models/arenas/lotus_temple.glb")) {
                m_stageModel = ::LoadModel("assets/models/arenas/lotus_temple.glb");
                m_hasCustomModel = true;
                Logger::Info("StageManager", "Loaded high-fidelity 3D Lotus Temple arena (.glb) successfully!");
            } else {
                Logger::Warn("StageManager", "Lotus Temple .glb not found, using procedural fallback.");
            }
            break;
        case StageType::NEON_DOJO:
            m_currentStage = {
                "Neon Cyber Dojo",
                StageType::NEON_DOJO,
                16.0f,
                16.0f,
                true,
                Color{16, 18, 26, 255},
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
                Color{14, 12, 22, 255},
                Color{147, 51, 234, 255} // Dark Lotus Purple
            };
            break;
        case StageType::INFINITE_VOID:
            m_currentStage = {
                "Infinite Astral Void",
                StageType::INFINITE_VOID,
                80.0f,
                80.0f,
                false,
                Color{10, 10, 18, 255},
                Color{56, 189, 248, 255} // Cyan
            };
            break;
    }
}

void StageManager::Update(float deltaTime) {
    m_gridPulse += deltaTime * 2.5f;
}

void StageManager::Draw3D() {
    // If a high-fidelity 3D arena model (.glb) is loaded, draw it directly!
    if (m_hasCustomModel) {
        ::DrawModel(m_stageModel, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

        // Tactical combat boundary guide circle (matches the 9 pagoda boundary lanterns)
        float pulse = (std::sin(m_gridPulse) + 1.0f) * 0.5f;
        Color ringCol = m_currentStage.accentColor;
        ringCol.a = static_cast<unsigned char>(60 + pulse * 50);
        ::DrawCircle3D(Vector3{0.0f, 0.36f, 0.0f}, 10.4f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, ringCol);
        return;
    }

    float halfW = m_currentStage.arenaWidth * 0.5f;
    float halfL = m_currentStage.arenaLength * 0.5f;

    // 1. Dark reflective floor base
    ::DrawPlane(Vector3{0.0f, -0.01f, 0.0f}, Vector2{m_currentStage.arenaWidth * 1.5f, m_currentStage.arenaLength * 1.5f}, Color{10, 12, 16, 255});
    ::DrawPlane(Vector3{0.0f, 0.0f, 0.0f}, Vector2{m_currentStage.arenaWidth, m_currentStage.arenaLength}, m_currentStage.floorColor);

    // 2. High-tech tactical floor grid
    int slices = static_cast<int>(m_currentStage.arenaWidth);
    ::DrawGrid(slices, 1.0f);

    // 3. Dynamic Center Fighting Rings (Tekken Lotus Emblem)
    float pulse1 = (std::sin(m_gridPulse) + 1.0f) * 0.5f;
    float pulse2 = (std::cos(m_gridPulse * 1.4f) + 1.0f) * 0.5f;

    Color ringCol1 = m_currentStage.accentColor;
    ringCol1.a = static_cast<unsigned char>(50 + pulse1 * 60);

    Color ringCol2 = Color{250, 204, 21, static_cast<unsigned char>(40 + pulse2 * 50)};

    // Concentric glowing rings
    ::DrawCircle3D(Vector3{0.0f, 0.01f, 0.0f}, 2.0f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, ringCol2);
    ::DrawCircle3D(Vector3{0.0f, 0.015f, 0.0f}, 4.5f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, ringCol1);
    ::DrawCircle3D(Vector3{0.0f, 0.02f, 0.0f}, 7.5f, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, Color{255, 255, 255, 35});

    // Crosshairs on center ring
    ::DrawLine3D(Vector3{-2.5f, 0.02f, 0.0f}, Vector3{2.5f, 0.02f, 0.0f}, ringCol1);
    ::DrawLine3D(Vector3{0.0f, 0.02f, -2.5f}, Vector3{0.0f, 0.02f, 2.5f}, ringCol1);

    // 4. Boundary energy barrier & 8 Pylons (Tekken octagonal walls)
    if (m_currentStage.hasWalls) {
        float pillarH = 5.5f;
        Color wallColor = m_currentStage.accentColor;
        wallColor.a = static_cast<unsigned char>(160 + pulse1 * 60);

        // 8 Energy Pylons (4 corners + 4 mid-points)
        Vector3 pylonPositions[8] = {
            {-halfW, 0.0f, -halfL},
            { halfW, 0.0f, -halfL},
            { halfW, 0.0f,  halfL},
            {-halfW, 0.0f,  halfL},
            { 0.0f,  0.0f, -halfL},
            { 0.0f,  0.0f,  halfL},
            {-halfW, 0.0f,  0.0f},
            { halfW, 0.0f,  0.0f}
        };

        for (int i = 0; i < 8; ++i) {
            Vector3 pos = pylonPositions[i];
            // Base pedestal
            ::DrawCube(Vector3{pos.x, 0.4f, pos.z}, 0.8f, 0.8f, 0.8f, Color{30, 32, 44, 255});
            ::DrawCubeWires(Vector3{pos.x, 0.4f, pos.z}, 0.8f, 0.8f, 0.8f, m_currentStage.accentColor);

            // Glowing vertical beam
            ::DrawCylinderEx(Vector3{pos.x, 0.8f, pos.z}, Vector3{pos.x, pillarH, pos.z}, 0.12f, 0.08f, 8, wallColor);
            // Pylon top crystal
            ::DrawSphere(Vector3{pos.x, pillarH, pos.z}, 0.22f, Color{255, 255, 255, 240});
        }

        // Perimeter laser barrier lines (Bottom, Mid, Top)
        float heights[3] = {0.2f, 2.2f, pillarH};
        for (float h : heights) {
            Color beamCol = wallColor;
            beamCol.a = (h == heights[1]) ? 100 : 180;

            ::DrawLine3D(Vector3{-halfW, h, -halfL}, Vector3{ halfW, h, -halfL}, beamCol);
            ::DrawLine3D(Vector3{ halfW, h, -halfL}, Vector3{ halfW, h,  halfL}, beamCol);
            ::DrawLine3D(Vector3{ halfW, h,  halfL}, Vector3{-halfW, h,  halfL}, beamCol);
            ::DrawLine3D(Vector3{-halfW, h,  halfL}, Vector3{-halfW, h, -halfL}, beamCol);
        }
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
