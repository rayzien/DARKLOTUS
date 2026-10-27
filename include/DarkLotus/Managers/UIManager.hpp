#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"

namespace DarkLotus {

class Fighter;
class ButtonManager;
class SettingsManager;

struct CombatTextPopup {
    std::string text;
    Vector2 screenPos;
    Color color;
    float lifeTime;
    float maxLifeTime;
    float scale;
};

class UIManager {
public:
    UIManager();
    ~UIManager();

    void Init();
    void Update(float deltaTime);

    // HUD & In-game displays
    void DrawCombatHUD(const Fighter* p1, const Fighter* p2, int roundTimer, int p1RoundsWon, int p2RoundsWon, int roundsToWin);
    void DrawComboCounter(int playerIndex, int hits, int totalDamage);
    void TriggerCombatAnnouncement(const std::string& text, Color color, float duration = 1.2f, float scale = 2.0f);
    void DrawAnnouncements();

    // Menus
    void DrawMainMenu(ButtonManager* btnMgr);
    void DrawCharacterSelect(ButtonManager* btnMgr, int p1Selected, int p2Selected, bool p1Locked, bool p2Locked);
    void DrawSettingsMenu(ButtonManager* btnMgr, SettingsManager* settings);
    void DrawPauseMenu(ButtonManager* btnMgr);
    void DrawRoundEnd(FightResult result);

private:
    void DrawTekkenHealthBar(float x, float y, float width, float height, float hpPercent, float redPercent, bool inRage, bool isP1);

private:
    // Animated smooth damage drain
    float m_p1DisplayHp{1.0f};
    float m_p2DisplayHp{1.0f};

    std::vector<CombatTextPopup> m_popups;
    float m_pulseTime{0.0f};
};

} // namespace DarkLotus
