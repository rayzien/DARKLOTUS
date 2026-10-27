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

struct CharDisplayData {
    const char* name;
    const char* style;
    const char* launcher;
    const char* wallSplat;
    const char* lowMove;
    const char* special;
    const char* rageArt;
    Color primaryColor;
};

static const CharDisplayData s_charData[] = {
    {
        "OODIII JII",
        "Mishima Karate & Thunder God (Heihachi + Raiden)",
        "Electric Wind God Fist (d+2)",
        "Demon Breath Palm (1+2)",
        "Hell Sweep (d+3)",
        "Raiden Torpedo Dive (3+4)",
        "RAGE ART: Thunder Wrath",
        Color{234, 179, 8, 255} // Thunder Gold
    },
    {
        "LUHAR IHDNAG",
        "Bajiquan Chinese Martial Arts (Leo Kliesen)",
        "Baji Rising Elbow (d+2)",
        "Tetsuzanko Shoulder Ram (1+2)",
        "Dragon Low Sweep (d+3)",
        "Dual Crane Palms (3+4)",
        "RAGE ART: Mountain Shatterer",
        Color{34, 197, 94, 255} // Jade Emerald
    },
    {
        "WINPAK",
        "Kazama Style Karate & Omen (Jin Kazama)",
        "Thrusting Demon Uppercut (d+2)",
        "Demon Paw Lunging Fist (1+2)",
        "Kazama Low Sweep (d+3)",
        "Can-Can Double Kicks (3+4)",
        "RAGE ART: Divine Avenger",
        Color{225, 29, 72, 255} // Crimson Flame
    },
    {
        "GANJA",
        "Brutal Kickboxing & Marsupial (Bryan + Kangaroo)",
        "Jet Upper i14 Launcher (d+2)",
        "Nuclear Mach Punch (1+2)",
        "Snake Edge Low Launcher (d+3)",
        "Kangaroo Flying Dropkick (3+4)",
        "RAGE ART: Fury Chaos Barrage",
        Color{168, 85, 247, 255} // Toxic Purple
    }
};

void UIManager::DrawCharacterSelect(ButtonManager* btnMgr, int p1Selected, int p2Selected, bool p1Locked, bool p2Locked) {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    int titleSize = 38;
    int titleW = ::MeasureText("SELECT YOUR FIGHTER", titleSize);
    ::DrawText("SELECT YOUR FIGHTER", static_cast<int>(sw * 0.5f - titleW * 0.5f), 30, titleSize, Color{245, 245, 245, 255});

    auto drawCard = [&](float cardX, float cardY, float cardW, float cardH, int charIdx, bool isLocked, const char* playerLabel, Color headerColor) {
        if (charIdx < 0 || charIdx >= 4) return;
        const auto& d = s_charData[charIdx];

        Rectangle card = {cardX, cardY, cardW, cardH};
        ::DrawRectangleRec(card, Color{20, 22, 32, 245});
        ::DrawRectangleLinesEx(card, isLocked ? 4.0f : 2.0f, isLocked ? Color{34, 197, 94, 255} : d.primaryColor);

        // Header
        ::DrawText(playerLabel, static_cast<int>(cardX + 16), static_cast<int>(cardY + 14), 22, headerColor);
        if (isLocked) {
            ::DrawText("[LOCKED IN]", static_cast<int>(cardX + cardW - 130), static_cast<int>(cardY + 14), 20, Color{34, 197, 94, 255});
        }

        // Character Name
        ::DrawText(d.name, static_cast<int>(cardX + 16), static_cast<int>(cardY + 48), 28, d.primaryColor);

        // Fighting Style
        ::DrawText(d.style, static_cast<int>(cardX + 16), static_cast<int>(cardY + 84), 16, Color{190, 200, 215, 255});

        // Moveset List
        float listY = cardY + 120.0f;
        ::DrawText("SIGNATURE MOVES:", static_cast<int>(cardX + 16), static_cast<int>(listY), 16, Color{250, 204, 21, 255});
        listY += 24.0f;

        ::DrawText(TextFormat("• Launcher: %s", d.launcher), static_cast<int>(cardX + 16), static_cast<int>(listY), 15, Color{230, 230, 230, 255});
        listY += 22.0f;
        ::DrawText(TextFormat("• Wall Splat: %s", d.wallSplat), static_cast<int>(cardX + 16), static_cast<int>(listY), 15, Color{230, 230, 230, 255});
        listY += 22.0f;
        ::DrawText(TextFormat("• Low Attack: %s", d.lowMove), static_cast<int>(cardX + 16), static_cast<int>(listY), 15, Color{230, 230, 230, 255});
        listY += 22.0f;
        ::DrawText(TextFormat("• Special: %s", d.special), static_cast<int>(cardX + 16), static_cast<int>(listY), 15, Color{230, 230, 230, 255});
        listY += 22.0f;
        ::DrawText(TextFormat("• Super: %s", d.rageArt), static_cast<int>(cardX + 16), static_cast<int>(listY), 15, Color{239, 68, 68, 255});
    };

    float cardW = sw * 0.42f;
    float cardH = sh * 0.44f;
    float cardY = sh * 0.16f;

    // Draw P1 Card (Left)
    drawCard(sw * 0.06f, cardY, cardW, cardH, p1Selected, p1Locked, "PLAYER 1", Color{220, 38, 38, 255});

    // Draw P2 / CPU Card (Right)
    drawCard(sw * 0.52f, cardY, cardW, cardH, p2Selected, p2Locked, "PLAYER 2 / CPU", Color{59, 130, 246, 255});

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
