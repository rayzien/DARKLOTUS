#include "DarkLotus/Managers/CharacterManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

CharacterManager::CharacterManager() = default;
CharacterManager::~CharacterManager() = default;

void CharacterManager::Init() {
    m_roster.clear();

    // 1. OODIII JII (Heihachi Mishima + Raiden)
    m_roster.push_back({
        0,
        "OODIII JII",
        "Mishima Karate & Thunder God (Heihachi + Raiden)",
        "Electric Wind God Fist (EWGF), Demon Breath Palm, Hell Axle, and Raiden Lightning Torpedo Dive.",
        Color{234, 179, 8, 255}, // Thunder Gold
        "assets/characters/oodiii_jii/model.glb"
    });

    // 2. LUHAR IHDNAG (Leo Kliesen / Bajiquan)
    m_roster.push_back({
        1,
        "LUHAR IHDNAG",
        "Bajiquan Chinese Martial Arts (Leo Kliesen)",
        "Tetsuzanko Iron Body Shoulder Ram, Rising Elbow Launcher, Dragon Sweep, and Mountain Shatterer.",
        Color{34, 197, 94, 255}, // Jade Emerald
        "assets/characters/luhar_ihdnag/model.glb"
    });

    // 3. WINPAK (Jin Kazama)
    m_roster.push_back({
        2,
        "WINPAK",
        "Kazama Style Karate & Omen (Jin Kazama)",
        "Demon Paw lunging straight fist, Can-Can Double Launch Kicks, Thrusting Uppercut, and Divine Avenger.",
        Color{225, 29, 72, 255}, // Crimson Flame
        "assets/characters/winpak/model.glb"
    });

    // 4. GANJA (Bryan Fury + Kangaroo)
    m_roster.push_back({
        3,
        "GANJA",
        "Brutal Kickboxing & Marsupial Brawler (Bryan Fury + Kangaroo)",
        "Nuclear Mach Punch wall splat, Jet Upper i14 launcher, Snake Edge low, and Kangaroo Dropkicks.",
        Color{168, 85, 247, 255}, // Toxic Purple
        "assets/characters/ganja/model.glb"
    });

    // Default starting match: OODIII JII vs WINPAK
    SetupMatch(0, 2, true);

    Logger::Info("CharacterManager", "Fighter roster initialized with 4 specialized characters: OODIII JII, LUHAR IHDNAG, WINPAK, GANJA.");
}

const CharacterInfo* CharacterManager::GetCharacterInfo(int id) const {
    for (const auto& c : m_roster) {
        if (c.id == id) return &c;
    }
    return nullptr;
}

void CharacterManager::SetupMatch(int p1CharId, int p2CharId, bool vsAI) {
    const CharacterInfo* info1 = GetCharacterInfo(p1CharId);
    const CharacterInfo* info2 = GetCharacterInfo(p2CharId);

    std::string name1 = info1 ? info1->name : "OODIII JII";
    Color color1 = info1 ? info1->primaryColor : Color{234, 179, 8, 255};
    std::string glb1 = info1 ? info1->modelGlbPath : "";

    std::string name2 = info2 ? info2->name : "WINPAK";
    Color color2 = info2 ? info2->primaryColor : Color{225, 29, 72, 255};
    std::string glb2 = info2 ? info2->modelGlbPath : "";

    // P1 spawns at -2.5m on X-axis, P2 spawns at +2.5m
    m_player1 = std::make_unique<Fighter>(0, p1CharId, name1, color1, Vector3{-2.5f, 0.0f, 0.0f});
    m_player2 = std::make_unique<Fighter>(1, p2CharId, name2, color2, Vector3{ 2.5f, 0.0f, 0.0f});

    m_player1->Init();
    m_player2->Init();

    m_player2->SetIsAI(vsAI);

    // Attempt to load Blender models if available
    if (!glb1.empty()) m_player1->LoadBlenderModel(glb1);
    if (!glb2.empty()) m_player2->LoadBlenderModel(glb2);

    Logger::Info("CharacterManager", "Match configured: " + name1 + " vs " + name2 + (vsAI ? " [VS CPU]" : " [VS P2]"));
}

void CharacterManager::Update(float deltaTime, int currentFrame, unsigned int p1Inputs, unsigned int p2Inputs) {
    if (m_player1 && m_player2) {
        m_player1->Update(deltaTime, currentFrame, p1Inputs, m_player2.get());
        m_player2->Update(deltaTime, currentFrame, p2Inputs, m_player1.get());
    }
}

void CharacterManager::Draw3D(bool showHitboxes) {
    if (m_player1) m_player1->Draw3D(showHitboxes);
    if (m_player2) m_player2->Draw3D(showHitboxes);
}

void CharacterManager::ResetRound() {
    if (m_player1) m_player1->ResetRound(Vector3{-2.5f, 0.0f, 0.0f});
    if (m_player2) m_player2->ResetRound(Vector3{ 2.5f, 0.0f, 0.0f});
}

} // namespace DarkLotus
