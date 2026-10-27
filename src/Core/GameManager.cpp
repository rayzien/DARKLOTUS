#include "DarkLotus/Core/GameManager.hpp"
#include "DarkLotus/Managers/ClickManager.hpp"
#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Managers/SettingsManager.hpp"
#include "DarkLotus/Managers/GraphicsManager.hpp"
#include "DarkLotus/Managers/CharacterManager.hpp"
#include "DarkLotus/Managers/AnimationManager.hpp"
#include "DarkLotus/Managers/UIManager.hpp"
#include "DarkLotus/Managers/CombatManager.hpp"
#include "DarkLotus/Managers/SoundManager.hpp"
#include "DarkLotus/Managers/StageManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

GameManager& GameManager::Instance() {
    static GameManager s_instance;
    return s_instance;
}

GameManager::GameManager()
    : m_clickManager(std::make_unique<ClickManager>()),
      m_buttonManager(std::make_unique<ButtonManager>()),
      m_settingsManager(std::make_unique<SettingsManager>()),
      m_graphicsManager(std::make_unique<GraphicsManager>()),
      m_characterManager(std::make_unique<CharacterManager>()),
      m_animationManager(std::make_unique<AnimationManager>()),
      m_uiManager(std::make_unique<UIManager>()),
      m_combatManager(std::make_unique<CombatManager>()),
      m_soundManager(std::make_unique<SoundManager>()),
      m_stageManager(std::make_unique<StageManager>()) {
}

GameManager::~GameManager() = default;

bool GameManager::Init() {
    Logger::Info("GameManager", "Initializing DARKLOTUS Engine Core...");

    m_settingsManager->Init();
    if (!m_graphicsManager->Init(*m_settingsManager)) {
        Logger::Error("GameManager", "Failed to initialize graphics subsystem!");
        return false;
    }

    m_soundManager->Init(*m_settingsManager);
    m_clickManager->Init();
    m_buttonManager->Init();
    m_stageManager->Init();
    m_characterManager->Init();
    m_animationManager->Init();
    m_uiManager->Init();
    m_combatManager->Init();

    SetState(GameState::MAIN_MENU);
    m_isRunning = true;

    Logger::Info("GameManager", "DARKLOTUS Ready for Battle.");
    return true;
}

void GameManager::Shutdown() {
    m_soundManager->Shutdown();
    m_graphicsManager->Shutdown();
    Logger::Info("GameManager", "DARKLOTUS Shutdown completed successfully.");
}

void GameManager::SetState(GameState newState) {
    m_currentState = newState;
    m_buttonManager->ClearButtons();

    switch (m_currentState) {
        case GameState::MAIN_MENU:
            SetupMainMenuButtons();
            break;
        case GameState::CHARACTER_SELECT:
            m_p1Locked = false;
            m_p2Locked = false;
            SetupCharSelectButtons();
            break;
        case GameState::SETTINGS_MENU:
            SetupSettingsButtons();
            break;
        case GameState::PAUSED:
            SetupPauseButtons();
            break;
        case GameState::FIGHT_INTRO:
            m_roundNumber = 1;
            m_p1RoundsWon = 0;
            m_p2RoundsWon = 0;
            m_roundTimer = static_cast<float>(m_settingsManager->GetGameplay().roundTimerSeconds);
            m_characterManager->ResetRound();
            m_soundManager->PlaySFX(SoundEffectType::ROUND_START_BELL);
            m_uiManager->TriggerCombatAnnouncement("ROUND 1", Color{250, 204, 21, 255}, 1.5f, 2.4f);
            SetState(GameState::FIGHT_ACTIVE);
            break;
        default:
            break;
    }
}

void GameManager::SetupMainMenuButtons() {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());
    float btnW = 280.0f;
    float btnH = 50.0f;
    float startY = sh * 0.42f;

    m_buttonManager->RegisterButton({
        "btn_arcade", "ARCADE (VS CPU)",
        Rectangle{sw * 0.5f - btnW * 0.5f, startY, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_SELECT);
            m_vsAI = true;
            SetState(GameState::CHARACTER_SELECT);
        }
    });

    m_buttonManager->RegisterButton({
        "btn_versus", "LOCAL 2P VERSUS",
        Rectangle{sw * 0.5f - btnW * 0.5f, startY + 65.0f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_SELECT);
            m_vsAI = false;
            SetState(GameState::CHARACTER_SELECT);
        }
    });

    m_buttonManager->RegisterButton({
        "btn_settings", "SETTINGS",
        Rectangle{sw * 0.5f - btnW * 0.5f, startY + 130.0f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_SELECT);
            SetState(GameState::SETTINGS_MENU);
        }
    });

    m_buttonManager->RegisterButton({
        "btn_exit", "EXIT",
        Rectangle{sw * 0.5f - btnW * 0.5f, startY + 195.0f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{120, 20, 20, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_isRunning = false;
        }
    });
}

void GameManager::SetupCharSelectButtons() {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());

    const auto& roster = m_characterManager->GetRoster();
    float slotW = 140.0f;
    float slotH = 45.0f;
    float totalW = roster.size() * (slotW + 20.0f);
    float startX = (sw - totalW) * 0.5f;

    for (size_t i = 0; i < roster.size(); ++i) {
        int charId = roster[i].id;
        std::string label = roster[i].name;

        m_buttonManager->RegisterButton({
            "char_btn_" + std::to_string(charId), label,
            Rectangle{startX + (i * (slotW + 20.0f)), sh * 0.80f, slotW, slotH},
            ButtonVisualState::NORMAL,
            Color{40, 44, 55, 230}, roster[i].primaryColor, Color{245, 245, 245, 255},
            [this, charId]() {
                m_soundManager->PlaySFX(SoundEffectType::MENU_CLICK);
                if (!m_p1Locked) {
                    m_p1SelectedChar = charId;
                    m_p1Locked = true;
                    if (m_vsAI) {
                        m_p2SelectedChar = (charId == 0) ? 1 : 0;
                        m_p2Locked = true;
                    }
                } else if (!m_p2Locked) {
                    m_p2SelectedChar = charId;
                    m_p2Locked = true;
                }

                if (m_p1Locked && m_p2Locked) {
                    m_characterManager->SetupMatch(m_p1SelectedChar, m_p2SelectedChar, m_vsAI);
                    SetState(GameState::FIGHT_INTRO);
                }
            }
        });
    }

    m_buttonManager->RegisterButton({
        "btn_back_menu", "BACK TO MENU",
        Rectangle{40.0f, sh - 60.0f, 180.0f, 40.0f},
        ButtonVisualState::NORMAL,
        Color{30, 32, 42, 230}, Color{220, 38, 38, 255}, Color{220, 220, 220, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_CLICK);
            SetState(GameState::MAIN_MENU);
        }
    });
}

void GameManager::SetupSettingsButtons() {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());
    float btnW = 320.0f;
    float btnH = 46.0f;

    m_buttonManager->RegisterButton({
        "btn_toggle_fs", "TOGGLE FULLSCREEN",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.35f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_settingsManager->ToggleFullscreen();
        }
    });

    m_buttonManager->RegisterButton({
        "btn_res_720", "RESOLUTION: 1280x720",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.44f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_settingsManager->SetResolution(1280, 720);
        }
    });

    m_buttonManager->RegisterButton({
        "btn_res_1080", "RESOLUTION: 1920x1080",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.53f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_settingsManager->SetResolution(1920, 1080);
        }
    });

    m_buttonManager->RegisterButton({
        "btn_back_from_settings", "SAVE & BACK",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.65f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{30, 32, 42, 230}, Color{34, 197, 94, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_settingsManager->SaveConfig();
            m_soundManager->PlaySFX(SoundEffectType::MENU_CLICK);
            SetState(GameState::MAIN_MENU);
        }
    });
}

void GameManager::SetupPauseButtons() {
    float sw = static_cast<float>(::GetScreenWidth());
    float sh = static_cast<float>(::GetScreenHeight());
    float btnW = 260.0f;
    float btnH = 46.0f;

    m_buttonManager->RegisterButton({
        "btn_resume", "RESUME FIGHT",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.40f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{34, 197, 94, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_CLICK);
            m_currentState = GameState::FIGHT_ACTIVE;
            m_buttonManager->ClearButtons();
        }
    });

    m_buttonManager->RegisterButton({
        "btn_quit_to_menu", "QUIT TO MENU",
        Rectangle{sw * 0.5f - btnW * 0.5f, sh * 0.49f, btnW, btnH},
        ButtonVisualState::NORMAL,
        Color{35, 38, 48, 230}, Color{220, 38, 38, 255}, Color{245, 245, 245, 255},
        [this]() {
            m_soundManager->PlaySFX(SoundEffectType::MENU_CLICK);
            SetState(GameState::MAIN_MENU);
        }
    });
}

void GameManager::UpdateFight(float deltaTime) {
    if (::IsKeyPressed(KEY_ESCAPE)) {
        SetState(GameState::PAUSED);
        return;
    }

    // Decrement round timer
    if (m_roundTimer > 0.0f) {
        m_roundTimer -= deltaTime;
        if (m_roundTimer <= 0.0f) {
            m_roundTimer = 0.0f;
            // Time out!
            Fighter* p1 = m_characterManager->GetPlayer1();
            Fighter* p2 = m_characterManager->GetPlayer2();
            if (p1->GetHealth() > p2->GetHealth()) {
                m_roundResult = FightResult::PLAYER_1_WIN;
                m_p1RoundsWon++;
            } else if (p2->GetHealth() > p1->GetHealth()) {
                m_roundResult = FightResult::PLAYER_2_WIN;
                m_p2RoundsWon++;
            } else {
                m_roundResult = FightResult::DRAW;
            }
            SetState(GameState::ROUND_OVER);
            return;
        }
    }

    // Fighter inputs from ButtonManager
    unsigned int p1Inputs = m_buttonManager->GetPlayerInput(0);
    unsigned int p2Inputs = m_buttonManager->GetPlayerInput(1);

    // Update characters
    m_characterManager->Update(deltaTime, m_currentFrame, p1Inputs, p2Inputs);

    // Arena boundary constraints (Tekken stage limits / wall splat)
    Fighter* p1 = m_characterManager->GetPlayer1();
    Fighter* p2 = m_characterManager->GetPlayer2();
    if (p1 && p2) {
        Vector3 pos1 = p1->GetPosition();
        Vector3 pos2 = p2->GetPosition();
        bool splat1 = false;
        bool splat2 = false;

        if (m_stageManager->ConstrainFighter(pos1, &splat1)) {
            p1->SetPosition(pos1);
            if (splat1 && p1->GetActionState() == ActionState::HIT_STUN) {
                p1->TriggerWallSplat();
            }
        }

        if (m_stageManager->ConstrainFighter(pos2, &splat2)) {
            p2->SetPosition(pos2);
            if (splat2 && p2->GetActionState() == ActionState::HIT_STUN) {
                p2->TriggerWallSplat();
            }
        }

        // Combat Collision resolution
        m_combatManager->Update(p1, p2, m_graphicsManager.get(), m_soundManager.get(), m_currentFrame);

        // Check recent combat events for popups
        for (const auto& ev : m_combatManager->GetRecentEvents()) {
            if (ev.isCounterHit) {
                m_uiManager->TriggerCombatAnnouncement("COUNTER HIT", Color{250, 204, 21, 255}, 0.9f, 1.8f);
            }
        }
        m_combatManager->ClearEvents();

        // KO Detection
        if (p1->GetHealth() <= 0 || p2->GetHealth() <= 0) {
            if (p1->GetHealth() <= 0 && p2->GetHealth() <= 0) {
                m_roundResult = FightResult::DRAW;
            } else if (p1->GetHealth() <= 0) {
                m_roundResult = FightResult::PLAYER_2_WIN;
                m_p2RoundsWon++;
            } else {
                m_roundResult = FightResult::PLAYER_1_WIN;
                m_p1RoundsWon++;
            }
            m_roundEndDelay = 2.5f; // Pause to savor K.O.
            SetState(GameState::ROUND_OVER);
        }
    }
}

void GameManager::UpdateRoundOver(float deltaTime) {
    m_roundEndDelay -= deltaTime;
    if (m_roundEndDelay <= 0.0f) {
        int roundsNeeded = m_settingsManager->GetGameplay().roundsToWin;
        if (m_p1RoundsWon >= roundsNeeded || m_p2RoundsWon >= roundsNeeded) {
            // Match over! Back to character select
            SetState(GameState::CHARACTER_SELECT);
        } else {
            // Next round
            m_roundNumber++;
            m_roundTimer = static_cast<float>(m_settingsManager->GetGameplay().roundTimerSeconds);
            m_characterManager->ResetRound();
            m_soundManager->PlaySFX(SoundEffectType::ROUND_START_BELL);
            m_uiManager->TriggerCombatAnnouncement("ROUND " + std::to_string(m_roundNumber), Color{250, 204, 21, 255}, 1.2f, 2.2f);
            SetState(GameState::FIGHT_ACTIVE);
        }
    }
}

void GameManager::Update(float deltaTime) {
    m_currentFrame++;

    // 1. Update Click & Button Managers
    m_clickManager->Update(m_graphicsManager->GetCamera());
    m_buttonManager->Update(deltaTime, m_currentFrame);

    // 2. Slow motion & Particles
    m_graphicsManager->UpdateSlowMo(deltaTime);
    float scaledDelta = deltaTime * m_graphicsManager->GetTimeScale();
    m_graphicsManager->UpdateParticles(scaledDelta);

    // 3. UI
    m_uiManager->Update(deltaTime);
    m_stageManager->Update(scaledDelta);

    // 4. Update Game State
    switch (m_currentState) {
        case GameState::MAIN_MENU:
            break;
        case GameState::CHARACTER_SELECT:
            break;
        case GameState::SETTINGS_MENU:
            break;
        case GameState::FIGHT_ACTIVE:
            UpdateFight(scaledDelta);
            break;
        case GameState::ROUND_OVER:
            UpdateRoundOver(scaledDelta);
            break;
        case GameState::PAUSED:
            break;
        default:
            break;
    }

    // 5. Update Dynamic 3D Camera (tracks fighters)
    Fighter* p1 = m_characterManager->GetPlayer1();
    Fighter* p2 = m_characterManager->GetPlayer2();
    if (p1 && p2) {
        m_graphicsManager->UpdateCamera(p1->GetPosition(), p2->GetPosition(), deltaTime);
    }
}

void GameManager::Render() {
    m_graphicsManager->BeginFrame();

    // ==========================================
    // 3D SCENE PASS
    // ==========================================
    m_graphicsManager->Begin3D();

    m_stageManager->Draw3D();
    m_characterManager->Draw3D(false);
    m_graphicsManager->DrawParticles();

    m_graphicsManager->End3D();

    // ==========================================
    // 2D UI & HUD PASS
    // ==========================================
    switch (m_currentState) {
        case GameState::MAIN_MENU:
            m_uiManager->DrawMainMenu(m_buttonManager.get());
            break;
        case GameState::CHARACTER_SELECT:
            m_uiManager->DrawCharacterSelect(m_buttonManager.get(), m_p1SelectedChar, m_p2SelectedChar, m_p1Locked, m_p2Locked);
            break;
        case GameState::SETTINGS_MENU:
            m_uiManager->DrawSettingsMenu(m_buttonManager.get(), m_settingsManager.get());
            break;
        case GameState::FIGHT_ACTIVE: {
            Fighter* p1 = m_characterManager->GetPlayer1();
            Fighter* p2 = m_characterManager->GetPlayer2();
            if (p1 && p2) {
                m_uiManager->DrawCombatHUD(p1, p2, static_cast<int>(m_roundTimer),
                                           m_p1RoundsWon, m_p2RoundsWon,
                                           m_settingsManager->GetGameplay().roundsToWin);

                // Combo displays
                m_uiManager->DrawComboCounter(0, m_combatManager->GetComboCount(0), m_combatManager->GetComboDamage(0));
                m_uiManager->DrawComboCounter(1, m_combatManager->GetComboCount(1), m_combatManager->GetComboDamage(1));
            }
            break;
        }
        case GameState::ROUND_OVER: {
            Fighter* p1 = m_characterManager->GetPlayer1();
            Fighter* p2 = m_characterManager->GetPlayer2();
            if (p1 && p2) {
                m_uiManager->DrawCombatHUD(p1, p2, static_cast<int>(m_roundTimer),
                                           m_p1RoundsWon, m_p2RoundsWon,
                                           m_settingsManager->GetGameplay().roundsToWin);
            }
            m_uiManager->DrawRoundEnd(m_roundResult);
            break;
        }
        case GameState::PAUSED:
            m_uiManager->DrawPauseMenu(m_buttonManager.get());
            break;
        default:
            break;
    }

    // Show FPS
    if (m_settingsManager->GetVideo().showFps) {
        ::DrawFPS(10, 10);
    }

    m_graphicsManager->EndFrame();
}

void GameManager::Run() {
    Logger::Info("GameManager", "Entering Main Battle Loop...");

    while (m_isRunning && !::WindowShouldClose()) {
        float dt = ::GetFrameTime();
        // Clamp delta time to avoid large physics steps
        dt = std::min(dt, 0.05f);

        Update(dt);
        Render();
    }
}

} // namespace DarkLotus
