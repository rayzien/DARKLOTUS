#include "DarkLotus/Managers/AnimationManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

AnimationManager::AnimationManager() = default;
AnimationManager::~AnimationManager() = default;

void AnimationManager::Init() {
    // Register standard Tekken martial arts animation clips
    RegisterClip({"IDLE", 60, 60, true, {}});
    RegisterClip({"WALK_FWD", 40, 60, true, {}});
    RegisterClip({"WALK_BACK", 40, 60, true, {}});
    RegisterClip({"SIDESTEP_LEFT", 25, 60, false, {}});
    RegisterClip({"SIDESTEP_RIGHT", 25, 60, false, {}});
    RegisterClip({"LP_JAB", 20, 60, false, {{10, "SPAWN_HITBOX"}, {12, "HITBOX_END"}}});
    RegisterClip({"RP_CROSS", 24, 60, false, {{12, "SPAWN_HITBOX"}, {15, "HITBOX_END"}}});
    RegisterClip({"LK_MID", 28, 60, false, {{14, "SPAWN_HITBOX"}, {18, "HITBOX_END"}}});
    RegisterClip({"RK_ROUNDHOUSE", 32, 60, false, {{16, "SPAWN_HITBOX"}, {20, "HITBOX_END"}}});
    RegisterClip({"UPPERCUT_LAUNCHER", 34, 60, false, {{15, "SPAWN_HITBOX"}, {19, "HITBOX_END"}}});
    RegisterClip({"LOW_SWEEP", 30, 60, false, {{16, "SPAWN_HITBOX"}, {20, "HITBOX_END"}}});
    RegisterClip({"RAGE_ART", 90, 60, false, {{20, "SPAWN_HITBOX"}, {25, "HITBOX_END"}}});
    RegisterClip({"HIT_STUN", 22, 60, false, {}});
    RegisterClip({"BLOCK_STUN", 16, 60, false, {}});
    RegisterClip({"KNOCKDOWN", 50, 60, false, {}});

    Play("IDLE");
    Logger::Info("AnimationManager", "Skeletal and procedural animation clip system initialized.");
}

void AnimationManager::RegisterClip(const AnimationClip& clip) {
    m_clips[clip.name] = clip;
}

void AnimationManager::Play(const std::string& clipName, bool restartIfPlaying) {
    if (m_currentClipName == clipName && !restartIfPlaying) {
        return;
    }

    auto it = m_clips.find(clipName);
    if (it != m_clips.end()) {
        m_currentClipName = clipName;
        m_clipTime = 0.0f;
        m_currentFrame = 0;
        m_prevFrame = -1;
        m_finished = false;
    }
}

void AnimationManager::Update(float deltaTime) {
    auto it = m_clips.find(m_currentClipName);
    if (it == m_clips.end()) return;

    const auto& clip = it->second;
    m_prevFrame = m_currentFrame;
    m_clipTime += deltaTime;

    m_currentFrame = static_cast<int>(m_clipTime * clip.fps);

    if (m_currentFrame >= clip.totalFrames) {
        if (clip.loop) {
            m_clipTime = std::fmod(m_clipTime, static_cast<float>(clip.totalFrames) / clip.fps);
            m_currentFrame = static_cast<int>(m_clipTime * clip.fps);
        } else {
            m_currentFrame = clip.totalFrames - 1;
            m_finished = true;
        }
    }
}

float AnimationManager::GetProgress() const {
    auto it = m_clips.find(m_currentClipName);
    if (it == m_clips.end() || it->second.totalFrames == 0) return 0.0f;
    return static_cast<float>(m_currentFrame) / static_cast<float>(it->second.totalFrames);
}

bool AnimationManager::HasEventThisFrame(std::string* outEventName) {
    auto it = m_clips.find(m_currentClipName);
    if (it == m_clips.end()) return false;

    const auto& clip = it->second;
    if (m_currentFrame != m_prevFrame) {
        auto evIt = clip.frameEvents.find(m_currentFrame);
        if (evIt != clip.frameEvents.end()) {
            if (outEventName) *outEventName = evIt->second;
            return true;
        }
    }
    return false;
}

float AnimationManager::EvaluateSineCurve(float time, float speed, float amplitude) {
    return std::sin(time * speed) * amplitude;
}

Vector3 AnimationManager::InterpolateLimb(const Vector3& start, const Vector3& target, float t) {
    return Vector3Lerp(start, target, t);
}

} // namespace DarkLotus
