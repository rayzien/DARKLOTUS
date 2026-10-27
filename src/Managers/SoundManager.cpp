#include "DarkLotus/Managers/SoundManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

SoundManager::SoundManager() = default;
SoundManager::~SoundManager() = default;

void SoundManager::Init(const SettingsManager& settings) {
    ::InitAudioDevice();
    if (::IsAudioDeviceReady()) {
        m_audioReady = true;
        m_masterVolume = settings.GetAudio().masterVolume;
        m_sfxVolume = settings.GetAudio().sfxVolume;
        m_bgmVolume = settings.GetAudio().bgmVolume;
        ::SetMasterVolume(m_masterVolume);

        GenerateProceduralSounds();
        Logger::Info("SoundManager", "3D Audio subsystem initialized with procedural sound synthesis.");
    } else {
        Logger::Warn("SoundManager", "Audio device could not be initialized.");
    }
}

void SoundManager::Shutdown() {
    if (m_audioReady) {
        for (auto& [type, snd] : m_sfxBank) {
            ::UnloadSound(snd);
        }
        m_sfxBank.clear();
        ::CloseAudioDevice();
        m_audioReady = false;
        Logger::Info("SoundManager", "Audio device closed.");
    }
}

// Procedurally synthesized combat audio
void SoundManager::GenerateProceduralSounds() {
    constexpr int sampleRate = 44100;

    auto createSound = [&](int sampleCount, std::function<short(int, int)> generator) -> Sound {
        Wave wave{};
        wave.frameCount = sampleCount;
        wave.sampleRate = sampleRate;
        wave.sampleSize = 16;
        wave.channels = 1;
        short* data = new short[sampleCount];

        for (int i = 0; i < sampleCount; ++i) {
            data[i] = generator(i, sampleCount);
        }
        wave.data = data;
        Sound snd = ::LoadSoundFromWave(wave);
        delete[] data;
        return snd;
    };

    // 1. PUNCH_LIGHT: Crisp high snap with quick exponential decay
    m_sfxBank[SoundEffectType::PUNCH_LIGHT] = createSound(sampleRate * 0.08, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 22.0f);
        float freq = 320.0f - t * 180.0f;
        float noise = (std::rand() % 2000 - 1000) * (1.0f - t);
        float tone = std::sin(i * freq * (2.0f * 3.14159f / 44100.0f)) * 22000.0f;
        return static_cast<short>((tone * 0.7f + noise * 0.3f) * decay);
    });

    // 2. PUNCH_HEAVY / KICK_HEAVY: Deep bass punch impact with distortion
    m_sfxBank[SoundEffectType::PUNCH_HEAVY] = createSound(sampleRate * 0.18, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 14.0f);
        float freq = 160.0f - t * 90.0f;
        float tone = std::sin(i * freq * (2.0f * 3.14159f / 44100.0f)) * 28000.0f;
        float sub = std::sin(i * (freq * 0.5f) * (2.0f * 3.14159f / 44100.0f)) * 16000.0f;
        return static_cast<short>((tone + sub) * 0.7f * decay);
    });

    // 3. KICK_LIGHT: Quick swoosh-thump
    m_sfxBank[SoundEffectType::KICK_LIGHT] = m_sfxBank[SoundEffectType::PUNCH_LIGHT];
    m_sfxBank[SoundEffectType::KICK_HEAVY] = m_sfxBank[SoundEffectType::PUNCH_HEAVY];

    // 4. BLOCK: Metallic / dull thud
    m_sfxBank[SoundEffectType::BLOCK] = createSound(sampleRate * 0.09, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 28.0f);
        float freq = 500.0f;
        float tone = std::sin(i * freq * (2.0f * 3.14159f / 44100.0f)) * 20000.0f;
        return static_cast<short>(tone * decay);
    });

    // 5. COUNTER_HIT: Boom bass shockwave with resonance
    m_sfxBank[SoundEffectType::COUNTER_HIT] = createSound(sampleRate * 0.35, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 7.0f);
        float freq = 220.0f - t * 140.0f;
        float bass = std::sin(i * 65.0f * (2.0f * 3.14159f / 44100.0f)) * 30000.0f;
        float mid = std::sin(i * freq * (2.0f * 3.14159f / 44100.0f)) * 18000.0f;
        return static_cast<short>((bass * 0.7f + mid * 0.3f) * decay);
    });

    // 6. WHOOSH: Wind slash on swing
    m_sfxBank[SoundEffectType::WHOOSH] = createSound(sampleRate * 0.12, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float env = std::sin(t * 3.14159f);
        float noise = (std::rand() % 28000 - 14000);
        return static_cast<short>(noise * env * 0.45f);
    });

    // 7. MENU_CLICK / MENU_SELECT
    m_sfxBank[SoundEffectType::MENU_CLICK] = createSound(sampleRate * 0.04, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 40.0f);
        return static_cast<short>(std::sin(i * 1200.0f * (2.0f * 3.14159f / 44100.0f)) * 22000.0f * decay);
    });

    m_sfxBank[SoundEffectType::MENU_SELECT] = createSound(sampleRate * 0.15, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 12.0f);
        float freq = 880.0f + t * 440.0f;
        return static_cast<short>(std::sin(i * freq * (2.0f * 3.14159f / 44100.0f)) * 24000.0f * decay);
    });

    // 8. KO_CRASH
    m_sfxBank[SoundEffectType::KO_CRASH] = createSound(sampleRate * 0.6, [](int i, int total) {
        float t = static_cast<float>(i) / total;
        float decay = std::exp(-t * 4.0f);
        float bass = std::sin(i * 55.0f * (2.0f * 3.14159f / 44100.0f)) * 31000.0f;
        float noise = (std::rand() % 16000 - 8000);
        return static_cast<short>((bass * 0.8f + noise * 0.2f) * decay);
    });
}

void SoundManager::PlaySFX(SoundEffectType type) {
    if (!m_audioReady) return;
    auto it = m_sfxBank.find(type);
    if (it != m_sfxBank.end()) {
        ::SetSoundVolume(it->second, m_sfxVolume * m_masterVolume);
        ::PlaySound(it->second);
    }
}

void SoundManager::PlayMusic(const std::string& /*musicPath*/) {
    // Music streamer hook
}

void SoundManager::UpdateMusic() {
    // Stream update hook
}

void SoundManager::SetVolume(float master, float sfx, float bgm) {
    m_masterVolume = std::clamp(master, 0.0f, 1.0f);
    m_sfxVolume = std::clamp(sfx, 0.0f, 1.0f);
    m_bgmVolume = std::clamp(bgm, 0.0f, 1.0f);
    if (m_audioReady) {
        ::SetMasterVolume(m_masterVolume);
    }
}

} // namespace DarkLotus
