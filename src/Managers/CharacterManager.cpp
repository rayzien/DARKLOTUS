#include "DarkLotus/Managers/CharacterManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

CharacterManager::CharacterManager() = default;
CharacterManager::~CharacterManager() = default;

void CharacterManager::Init() {
    m_roster.clear();

    m_roster.push_back({
        0,
        "LOTUS",
        "Mishima Karate / Dark Lotus",
        "Master of explosive strikes, electric wind god fist, and devastating juggle combos.",
        Color{220, 38, 38, 255}, // Crimson Red
        "assets/models/fighters/lotus.glb"
    });

    m_roster.push_back({
        1,
        "KAGE",
        "Shadow Ninjutsu",
        "Lightning fast evasive sidestepper, high/low mixup specialist with lethal sweeps.",
        Color{59, 130, 246, 255}, // Cobalt Blue
        "assets/models/fighters/kage.glb"
    });

    m_roster.push_back({
        2,
        "RAIJIN",
        "Thunder Brawler",
        "Armored power crushes, wall carry juggles, and ferocious counter hit setups.",
        Color{168, 85, 247, 255}, // Raijin Purple
        "assets/models/fighters/raijin.glb"
    });

    // Default match setup: Lotus (P1) vs Kage (P2 CPU)
    SetupMatch(0, 1, true);

    Logger::Info("CharacterManager", "Fighter roster initialized with 3 fighters.");
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

    std::string name1 = info1 ? info1->name : "LOTUS";
    Color color1 = info1 ? info1->primaryColor : Color{220, 38, 38, 255};
    std::string glb1 = info1 ? info1->modelGlbPath : "";

    std::string name2 = info2 ? info2->name : "KAGE";
    Color color2 = info2 ? info2->primaryColor : Color{59, 130, 246, 255};
    std::string glb2 = info2 ? info2->modelGlbPath : "";

    // P1 spawns at -2.5m on X-axis, P2 spawns at +2.5m
    m_player1 = std::make_unique<Fighter>(0, name1, color1, Vector3{-2.5f, 0.0f, 0.0f});
    m_player2 = std::make_unique<Fighter>(1, name2, color2, Vector3{ 2.5f, 0.0f, 0.0f});

    m_player1->Init();
    m_player2->Init();

    m_player2->SetIsAI(vsAI);

    // Attempt to load Blender models if user created them
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
