#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"

namespace DarkLotus {

struct AnimationClip {
    std::string name;
    int totalFrames;
    int fps;
    bool loop;
    std::unordered_map<int, std::string> frameEvents; // e.g. Frame 10: "SPAWN_HITBOX"
};

class AnimationManager {
public:
    AnimationManager();
    ~AnimationManager();

    void Init();
    void RegisterClip(const AnimationClip& clip);
    
    // Playback control
    void Play(const std::string& clipName, bool restartIfPlaying = false);
    void Update(float deltaTime);

    int GetCurrentFrame() const { return m_currentFrame; }
    float GetProgress() const;
    const std::string& GetCurrentClipName() const { return m_currentClipName; }
    bool IsFinished() const { return m_finished; }

    // Event polling
    bool HasEventThisFrame(std::string* outEventName = nullptr);

    // Procedural bone transform interpolator (for procedural mannequin animation)
    static float EvaluateSineCurve(float time, float speed, float amplitude);
    static Vector3 InterpolateLimb(const Vector3& start, const Vector3& target, float t);

private:
    std::unordered_map<std::string, AnimationClip> m_clips;
    std::string m_currentClipName;
    float m_clipTime{0.0f};
    int m_currentFrame{0};
    int m_prevFrame{-1};
    bool m_finished{false};
};

} // namespace DarkLotus
