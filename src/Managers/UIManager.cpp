#include "DarkLotus/Managers/UIManager.hpp"
#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Managers/SettingsManager.hpp"
#include "DarkLotus/Entities/Fighter.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

UIManager::UIManager() = default;
UIManager::~UIManager() = default;

void UIManager::Init() {
    m_popups.clear();
    m_p1DisplayHp = 1.0f;
    m_p2DisplayHp = 1.0f;
    Logger::Info("UIManager", "Tekken 8-styled HUD and interface manager initialized.");
}

void UIManager::Update(float deltaTime) {
    m_pulseTime += deltaTime;

    // Update floating popups
    for (auto it = m_popups.begin(); it != m_popups.end();) {
        it->lifeTime -= deltaTime;
        if (it->lifeTime <= 0.0f) {
            it = m_popups.erase(it);
        } else {
            it->screenPos.y -= 25.0f * deltaTime; // Float upwards
            ++it;
        }
    }
}

void UIManager::TriggerCombatAnnouncement(const std::string& text, Color color, float duration, float scale) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());
    m_popups.push_back({
        text,
        Vector2{sw * 0.5f, sh * 0.38f},
        color,
        duration,
        duration,
        scale
    });
}

void UIManager::DrawTekkenHealthBar(float x, float y, float width, float height, float hpPercent, float redPercent, bool inRage, bool isP1) {
    // Outer border & dark background
    Rectangle outerBg = {x, y, width, height};
    ::DrawRectangleRec(outerBg, Color{20, 20, 25, 230});

    // Tekken Rage pulsing border
    if (inRage) {
        float pulse = (std::sin(m_pulseTime * 8.0f) + 1.0f) * 0.5f;
        Color rageColor = Color{239, 68, 68, static_cast<unsigned char>(180 + pulse * 75)};
        ::DrawRectangleLinesEx(outerBg, 3.0f, rageColor);
    } else {
        ::DrawRectangleLinesEx(outerBg, 2.0f, Color{70, 75, 85, 255});
    }

    // Recoverable health (Red gauge)
    float redW = width * std::clamp(redPercent, 0.0f, 1.0f);
    float redX = isP1 ? (x + width - redW) : x;
    ::DrawRectangleRec({redX, y + 2, redW, height - 4}, Color{185, 28, 28, 200});

    // Current health bar (Tekken Neon Yellow/Orange or Red when in Rage)
    float hpW = width * std::clamp(hpPercent, 0.0f, 1.0f);
    float hpX = isP1 ? (x + width - hpW) : x;
    Color healthColor = inRage ? Color{239, 68, 68, 255} : Color{250, 204, 21, 255}; // Yellow-gold
    ::DrawRectangleRec({hpX, y + 2, hpW, height - 4}, healthColor);

    // Diagonal segment notches
    for (int i = 1; i <= 4; ++i) {
        float notchX = x + (width / 5.0f) * i;
        ::DrawLine(static_cast<int>(notchX), static_cast<int>(y),
                   static_cast<int>(notchX), static_cast<int>(y + height),
                   Color{10, 10, 15, 120});
    }
}

void UIManager::DrawCombatHUD(const Fighter* p1, const Fighter* p2, int roundTimer,
                             int p1RoundsWon, int p2RoundsWon, int roundsToWin) {
    float sw = static_cast<float>(::GetScreenWidth());
    float barWidth = sw * 0.40f;
    float barHeight = 24.0f;
    float topMargin = 35.0f;

    // Smooth damage drain lerp
    float p1TargetHp = static_cast<float>(p1->GetHealth()) / p1->GetMaxHealth();
    float p2TargetHp = static_cast<float>(p2->GetHealth()) / p2->GetMaxHealth();
    m_p1DisplayHp = Lerp(m_p1DisplayHp, p1TargetHp, 0.08f);
    m_p2DisplayHp = Lerp(m_p2DisplayHp, p2TargetHp, 0.08f);

    float p1Red = static_cast<float>(p1->GetRecoverableHealth()) / p1->GetMaxHealth();
    float p2Red = static_cast<float>(p2->GetRecoverableHealth()) / p2->GetMaxHealth();

    // 1. Health bars
    // P1 (Left side, drains to center)
    DrawTekkenHealthBar(sw * 0.05f, topMargin, barWidth, barHeight, m_p1DisplayHp, p1Red, p1->IsInRage(), true);
    // P2 (Right side, drains to center)
    DrawTekkenHealthBar(sw * 0.55f, topMargin, barWidth, barHeight, m_p2DisplayHp, p2Red, p2->IsInRage(), false);

    // 2. Fighter Names
    ::DrawText(p1->GetName().c_str(), static_cast<int>(sw * 0.05f), static_cast<int>(topMargin + barHeight + 6), 22, Color{245, 245, 245, 255});
    int p2NameW = ::MeasureText(p2->GetName().c_str(), 22);
    ::DrawText(p2->GetName().c_str(), static_cast<int>(sw * 0.95f - p2NameW), static_cast<int>(topMargin + barHeight + 6), 22, Color{245, 245, 245, 255});

    // 3. Tekken Heat Gauge (below health bar)
    float heatWidth = barWidth * 0.7f;
    float heatHeight = 6.0f;
    // P1 Heat
    float p1Heat = (p1->GetHeatGauge() / 100.0f) * heatWidth;
    ::DrawRectangleRec({sw * 0.05f, topMargin + barHeight + 32, heatWidth, heatHeight}, Color{30, 30, 40, 200});
    ::DrawRectangleRec({sw * 0.05f, topMargin + barHeight + 32, p1Heat, heatHeight}, Color{59, 130, 246, 255}); // Blue Heat
    // P2 Heat
    float p2Heat = (p2->GetHeatGauge() / 100.0f) * heatWidth;
    ::DrawRectangleRec({sw * 0.95f - heatWidth, topMargin + barHeight + 32, heatWidth, heatHeight}, Color{30, 30, 40, 200});
    ::DrawRectangleRec({sw * 0.95f - p2Heat, topMargin + barHeight + 32, p2Heat, heatHeight}, Color{59, 130, 246, 255});

    // 4. Round Icons (Tekken style round indicators)
    for (int r = 0; r < roundsToWin; ++r) {
        Color p1RoundCol = (r < p1RoundsWon) ? Color{250, 204, 21, 255} : Color{50, 50, 60, 200};
        Color p2RoundCol = (r < p2RoundsWon) ? Color{250, 204, 21, 255} : Color{50, 50, 60, 200};

        ::DrawPoly(Vector2{sw * 0.43f - (r * 18.0f), topMargin + 35.0f}, 4, 7.0f, 45.0f, p1RoundCol);
        ::DrawPoly(Vector2{sw * 0.57f + (r * 18.0f), topMargin + 35.0f}, 4, 7.0f, 45.0f, p2RoundCol);
    }

    // 5. Digital Round Timer in Center
    std::string timerStr = (roundTimer >= 0) ? std::to_string(roundTimer) : "∞";
    int timerFontSize = 48;
    int timerW = ::MeasureText(timerStr.c_str(), timerFontSize);
    Color timerColor = (roundTimer <= 10) ? Color{239, 68, 68, 255} : Color{255, 255, 255, 255};

    // Center timer box
    Rectangle timerBox = {sw * 0.5f - 40.0f, topMargin - 8.0f, 80.0f, 48.0f};
    ::DrawRectangleRec(timerBox, Color{15, 17, 24, 240});
    ::DrawRectangleLinesEx(timerBox, 2.0f, Color{220, 38, 38, 255});
    ::DrawText(timerStr.c_str(), static_cast<int>(sw * 0.5f - timerW * 0.5f), static_cast<int>(topMargin - 4), timerFontSize, timerColor);

    // 6. Draw active popups
    DrawAnnouncements();
}

void UIManager::DrawComboCounter(int playerIndex, int hits, int totalDamage) {
    if (hits < 2) return;

    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());
    float posX = (playerIndex == 0) ? sw * 0.15f : sw * 0.72f;
    float posY = sh * 0.45f;

    std::string hitText = std::to_string(hits) + " HITS!";
    std::string dmgText = std::to_string(totalDamage) + " DMG";

    ::DrawText(hitText.c_str(), static_cast<int>(posX), static_cast<int>(posY), 32, Color{250, 204, 21, 255});
    ::DrawText(dmgText.c_str(), static_cast<int>(posX + 4), static_cast<int>(posY + 36), 22, Color{240, 240, 240, 220});
}

void UIManager::DrawAnnouncements() {
    for (const auto& popup : m_popups) {
        float alpha = popup.lifeTime / popup.maxLifeTime;
        Color c = popup.color;
        c.a = static_cast<unsigned char>(alpha * 255.0f);

        int fontSize = static_cast<int>(32.0f * popup.scale);
        int textW = ::MeasureText(popup.text.c_str(), fontSize);

        // Shadow
        ::DrawText(popup.text.c_str(),
                   static_cast<int>(popup.screenPos.x - textW * 0.5f + 3),
                   static_cast<int>(popup.screenPos.y + 3),
                   fontSize, Color{0, 0, 0, static_cast<unsigned char>(alpha * 200.0f)});

        ::DrawText(popup.text.c_str(),
                   static_cast<int>(popup.screenPos.x - textW * 0.5f),
                   static_cast<int>(popup.screenPos.y),
                   fontSize, c);
    }
}

void UIManager::DrawMainMenu(ButtonManager* btnMgr) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    // Title / Logo
    int logoSize = 64;
    int logoW = ::MeasureText(GAME_NAME, logoSize);
    ::DrawText(GAME_NAME, static_cast<int>(sw * 0.5f - logoW * 0.5f), static_cast<int>(sh * 0.18f), logoSize, Color{220, 38, 38, 255});

    int subSize = 20;
    int subW = ::MeasureText(GAME_SUBTITLE, subSize);
    ::DrawText(GAME_SUBTITLE, static_cast<int>(sw * 0.5f - subW * 0.5f), static_cast<int>(sh * 0.18f + 70), subSize, Color{180, 185, 200, 255});

    // Draw buttons registered in ButtonManager
    btnMgr->DrawUI();
}

void UIManager::DrawCharacterSelect(ButtonManager* btnMgr, int p1Selected, int p2Selected, bool p1Locked, bool p2Locked) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    int titleSize = 40;
    int titleW = ::MeasureText("SELECT YOUR FIGHTER", titleSize);
    ::DrawText("SELECT YOUR FIGHTER", static_cast<int>(sw * 0.5f - titleW * 0.5f), 40, titleSize, Color{245, 245, 245, 255});

    const char* rosterNames[] = {"LOTUS (Mishima Karate)", "KAGE (Shadow Ninjutsu)", "RAIJIN (Thunder Brawler)"};
    const char* p1Name = (p1Selected >= 0 && p1Selected < 3) ? rosterNames[p1Selected] : "SELECTING...";
    const char* p2Name = (p2Selected >= 0 && p2Selected < 3) ? rosterNames[p2Selected] : "SELECTING...";

    // P1 side card
    Rectangle p1Card = {sw * 0.10f, sh * 0.25f, sw * 0.35f, sh * 0.50f};
    ::DrawRectangleRec(p1Card, Color{25, 28, 38, 240});
    ::DrawRectangleLinesEx(p1Card, p1Locked ? 4.0f : 2.0f, p1Locked ? Color{34, 197, 94, 255} : Color{220, 38, 38, 255});
    ::DrawText("PLAYER 1", static_cast<int>(p1Card.x + 20), static_cast<int>(p1Card.y + 20), 24, Color{220, 38, 38, 255});
    ::DrawText(p1Name, static_cast<int>(p1Card.x + 20), static_cast<int>(p1Card.y + 70), 20, Color{240, 240, 240, 255});
    if (p1Locked) {
        ::DrawText("[READY]", static_cast<int>(p1Card.x + p1Card.width - 120), static_cast<int>(p1Card.y + 20), 22, Color{34, 197, 94, 255});
    }

    // P2 side card
    Rectangle p2Card = {sw * 0.55f, sh * 0.25f, sw * 0.35f, sh * 0.50f};
    ::DrawRectangleRec(p2Card, Color{25, 28, 38, 240});
    ::DrawRectangleLinesEx(p2Card, p2Locked ? 4.0f : 2.0f, p2Locked ? Color{34, 197, 94, 255} : Color{59, 130, 246, 255});
    ::DrawText("PLAYER 2 / CPU", static_cast<int>(p2Card.x + 20), static_cast<int>(p2Card.y + 20), 24, Color{59, 130, 246, 255});
    ::DrawText(p2Name, static_cast<int>(p2Card.x + 20), static_cast<int>(p2Card.y + 70), 20, Color{240, 240, 240, 255});
    if (p2Locked) {
        ::DrawText("[READY]", static_cast<int>(p2Card.x + p2Card.width - 120), static_cast<int>(p2Card.y + 20), 22, Color{34, 197, 94, 255});
    }

    btnMgr->DrawUI();
}

void UIManager::DrawSettingsMenu(ButtonManager* btnMgr, SettingsManager* /*settings*/) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    int titleSize = 36;
    int titleW = ::MeasureText("SYSTEM SETTINGS", titleSize);
    ::DrawText("SYSTEM SETTINGS", static_cast<int>(sw * 0.5f - titleW * 0.5f), static_cast<int>(sh * 0.15f), titleSize, Color{245, 245, 245, 255});

    btnMgr->DrawUI();
}

void UIManager::DrawPauseMenu(ButtonManager* btnMgr) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    // Dim overlay
    ::DrawRectangle(0, 0, static_cast<int>(sw), static_cast<int>(sh), Color{0, 0, 0, 180});

    int pauseSize = 48;
    int pauseW = ::MeasureText("PAUSED", pauseSize);
    ::DrawText("PAUSED", static_cast<int>(sw * 0.5f - pauseW * 0.5f), static_cast<int>(sh * 0.25f), pauseSize, Color{245, 245, 245, 255});

    btnMgr->DrawUI();
}

void UIManager::DrawRoundEnd(FightResult result) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    const char* winText = "K.O.";
    Color textColor = Color{220, 38, 38, 255};

    if (result == FightResult::PLAYER_1_WIN) {
        winText = "PLAYER 1 WINS!";
        textColor = Color{250, 204, 21, 255};
    } else if (result == FightResult::PLAYER_2_WIN) {
        winText = "PLAYER 2 WINS!";
        textColor = Color{59, 130, 246, 255};
    } else if (result == FightResult::DRAW) {
        winText = "DOUBLE K.O.";
    }

    int sz = 60;
    int w = ::MeasureText(winText, sz);
    ::DrawText(winText, static_cast<int>(sw * 0.5f - w * 0.5f), static_cast<int>(sh * 0.42f), sz, textColor);
}

} // namespace DarkLotus
