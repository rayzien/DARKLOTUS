#include "DarkLotus/Managers/SettingsManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"
#include <fstream>
#include <sstream>

namespace DarkLotus {

SettingsManager::SettingsManager() = default;
SettingsManager::~SettingsManager() = default;

void SettingsManager::Init(const std::string& configFilePath) {
    m_configPath = configFilePath;
    LoadConfig();
    Logger::Info("SettingsManager", "Settings initialized with config: " + m_configPath);
}

void SettingsManager::SaveConfig() {
    std::ofstream out(m_configPath);
    if (!out.is_open()) {
        Logger::Warn("SettingsManager", "Could not open config file for writing: " + m_configPath);
        return;
    }

    out << "# DarkLotus Engine Settings\n";
    out << "screenWidth=" << m_video.screenWidth << "\n";
    out << "screenHeight=" << m_video.screenHeight << "\n";
    out << "fullscreen=" << (m_video.fullscreen ? 1 : 0) << "\n";
    out << "vsync=" << (m_video.vsync ? 1 : 0) << "\n";
    out << "targetFps=" << m_video.targetFps << "\n";
    out << "showFps=" << (m_video.showFps ? 1 : 0) << "\n";
    out << "enableHitSparks=" << (m_video.enableHitSparks ? 1 : 0) << "\n";
    out << "masterVolume=" << m_audio.masterVolume << "\n";
    out << "bgmVolume=" << m_audio.bgmVolume << "\n";
    out << "sfxVolume=" << m_audio.sfxVolume << "\n";
    out << "roundsToWin=" << m_gameplay.roundsToWin << "\n";
    out << "roundTimerSeconds=" << m_gameplay.roundTimerSeconds << "\n";
    out << "aiDifficulty=" << m_gameplay.aiDifficulty << "\n";

    Logger::Info("SettingsManager", "Settings saved successfully.");
}

void SettingsManager::LoadConfig() {
    std::ifstream in(m_configPath);
    if (!in.is_open()) {
        Logger::Info("SettingsManager", "No config file found. Using default Tekken tournament settings.");
        SaveConfig();
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        try {
            if (key == "screenWidth") m_video.screenWidth = std::stoi(val);
            else if (key == "screenHeight") m_video.screenHeight = std::stoi(val);
            else if (key == "fullscreen") m_video.fullscreen = (std::stoi(val) != 0);
            else if (key == "vsync") m_video.vsync = (std::stoi(val) != 0);
            else if (key == "targetFps") m_video.targetFps = std::stoi(val);
            else if (key == "showFps") m_video.showFps = (std::stoi(val) != 0);
            else if (key == "enableHitSparks") m_video.enableHitSparks = (std::stoi(val) != 0);
            else if (key == "masterVolume") m_audio.masterVolume = std::stof(val);
            else if (key == "bgmVolume") m_audio.bgmVolume = std::stof(val);
            else if (key == "sfxVolume") m_audio.sfxVolume = std::stof(val);
            else if (key == "roundsToWin") m_gameplay.roundsToWin = std::stoi(val);
            else if (key == "roundTimerSeconds") m_gameplay.roundTimerSeconds = std::stoi(val);
            else if (key == "aiDifficulty") m_gameplay.aiDifficulty = std::stoi(val);
        } catch (...) {
            Logger::Warn("SettingsManager", "Failed to parse setting line: " + line);
        }
    }
    Logger::Info("SettingsManager", "Settings loaded successfully.");
}

void SettingsManager::SetResolution(int width, int height) {
    m_video.screenWidth = width;
    m_video.screenHeight = height;
    ::SetWindowSize(width, height);
}

void SettingsManager::ToggleFullscreen() {
    m_video.fullscreen = !m_video.fullscreen;
    ::ToggleFullscreen();
}

void SettingsManager::SetMasterVolume(float volume) {
    m_audio.masterVolume = std::clamp(volume, 0.0f, 1.0f);
    ::SetMasterVolume(m_audio.masterVolume);
}

} // namespace DarkLotus
