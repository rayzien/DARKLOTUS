#pragma once

#include "DarkLotus/Core/Common.hpp"

namespace DarkLotus {

struct VideoSettings {
    int screenWidth{DEFAULT_SCREEN_WIDTH};
    int screenHeight{DEFAULT_SCREEN_HEIGHT};
    bool fullscreen{false};
    bool vsync{true};
    int targetFps{TARGET_FPS};
    bool showFps{true};
    bool enableBloom{true};
    bool enableHitSparks{true};
};

struct AudioSettings {
    float masterVolume{0.8f};
    float bgmVolume{0.7f};
    float sfxVolume{0.9f};
    float voiceVolume{0.85f};
    bool muteAll{false};
};

struct GameplaySettings {
    int roundsToWin{2};
    int roundTimerSeconds{60};
    int aiDifficulty{2}; // 1 = Easy, 2 = Medium, 3 = Hard, 4 = Tekken Master
};

class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();

    void Init(const std::string& configFilePath = "assets/config/settings.cfg");

    void SaveConfig();
    void LoadConfig();

    VideoSettings& GetVideo() { return m_video; }
    const VideoSettings& GetVideo() const { return m_video; }

    AudioSettings& GetAudio() { return m_audio; }
    const AudioSettings& GetAudio() const { return m_audio; }

    GameplaySettings& GetGameplay() { return m_gameplay; }
    const GameplaySettings& GetGameplay() const { return m_gameplay; }

    void SetResolution(int width, int height);
    void ToggleFullscreen();
    void SetMasterVolume(float volume);

private:
    std::string m_configPath;
    VideoSettings m_video;
    AudioSettings m_audio;
    GameplaySettings m_gameplay;
};

} // namespace DarkLotus
