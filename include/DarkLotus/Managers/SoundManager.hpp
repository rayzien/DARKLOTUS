#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Managers/SettingsManager.hpp"

namespace DarkLotus {

enum class SoundEffectType {
    PUNCH_LIGHT,
    PUNCH_HEAVY,
    KICK_LIGHT,
    KICK_HEAVY,
    BLOCK,
    COUNTER_HIT,
    WHOOSH,
    JUMP,
    LANDING,
    HEAT_ACTIVATION,
    ROUND_START_BELL,
    KO_CRASH,
    MENU_CLICK,
    MENU_SELECT
};

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    void Init(const SettingsManager& settings);
    void Shutdown();

    void PlaySFX(SoundEffectType type);
    void PlayMusic(const std::string& musicPath);
    void UpdateMusic();

    void SetVolume(float master, float sfx, float bgm);

private:
    void GenerateProceduralSounds();

private:
    bool m_audioReady{false};
    float m_masterVolume{0.8f};
    float m_sfxVolume{0.9f};
    float m_bgmVolume{0.7f};

    std::unordered_map<SoundEffectType, Sound> m_sfxBank;
};

} // namespace DarkLotus
