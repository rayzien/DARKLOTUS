#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Entities/Fighter.hpp"

namespace DarkLotus {

struct CharacterInfo {
    int id;
    std::string name;
    std::string style;
    std::string description;
    Color primaryColor;
    std::string modelGlbPath;
};

class CharacterManager {
public:
    CharacterManager();
    ~CharacterManager();

    void Init();
    void SetupMatch(int p1CharId, int p2CharId, bool vsAI = false);
    void Update(float deltaTime, int currentFrame, unsigned int p1Inputs, unsigned int p2Inputs);
    void Draw3D(bool showHitboxes = false);
    void ResetRound();

    Fighter* GetPlayer1() { return m_player1.get(); }
    Fighter* GetPlayer2() { return m_player2.get(); }
    const Fighter* GetPlayer1() const { return m_player1.get(); }
    const Fighter* GetPlayer2() const { return m_player2.get(); }

    const std::vector<CharacterInfo>& GetRoster() const { return m_roster; }
    const CharacterInfo* GetCharacterInfo(int id) const;

private:
    std::vector<CharacterInfo> m_roster;
    std::unique_ptr<Fighter> m_player1;
    std::unique_ptr<Fighter> m_player2;
};

} // namespace DarkLotus
