#pragma once

#include "DarkLotus/Core/Common.hpp"

namespace DarkLotus {

enum class StageType {
    NEON_DOJO,
    MISHIMA_SANCTUARY,
    INFINITE_VOID
};

struct StageInfo {
    std::string name;
    StageType type;
    float arenaWidth;
    float arenaLength;
    bool hasWalls;
    Color floorColor;
    Color accentColor;
};

class StageManager {
public:
    StageManager();
    ~StageManager();

    void Init();
    void SetStage(StageType stageType);
    void Update(float deltaTime);
    void Draw3D();

    // Arena boundary collision constraint (prevents fighters leaving the stage, triggers wall splat)
    bool ConstrainFighter(Vector3& inOutPos, bool* outHitWall = nullptr) const;

    float GetArenaWidth() const { return m_currentStage.arenaWidth; }
    float GetArenaLength() const { return m_currentStage.arenaLength; }

private:
    StageInfo m_currentStage;
    float m_gridPulse{0.0f};
};

} // namespace DarkLotus
