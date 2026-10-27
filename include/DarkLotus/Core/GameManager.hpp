#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"

// Forward declarations of all managers
namespace DarkLotus {

class ClickManager;
class ButtonManager;
class SettingsManager;
class GraphicsManager;
class CharacterManager;
class AnimationManager;
class UIManager;
class CombatManager;
class SoundManager;
class StageManager;

class GameManager {
public:
    static GameManager& Instance();

    bool Init();
    void Run();
    void Shutdown();

    // Game state transitions
    void SetState(GameState newState);
    GameState GetState() const { return m_currentState; }

    // Access to all specialized managers
    ClickManager& GetClickManager() { return *m_clickManager; }
    ButtonManager& GetButtonManager() { return *m_buttonManager; }
    SettingsManager& GetSettingsManager() { return *m_settingsManager; }
    GraphicsManager& GetGraphicsManager() { return *m_graphicsManager; }
    CharacterManager& GetCharacterManager() { return *m_characterManager; }
    AnimationManager& GetAnimationManager() { return *m_animationManager; }
    UIManager& GetUIManager() { return *m_uiManager; }
    CombatManager& GetCombatManager() { return *m_combatManager; }
    SoundManager& GetSoundManager() { return *m_soundManager; }
    StageManager& GetStageManager() { return *m_stageManager; }

private:
    GameManager();
    ~GameManager();
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    void Update(float deltaTime);
    void Render();

    void UpdateMainMenu();
    void UpdateCharacterSelect();
    void UpdateFight(float deltaTime);
    void UpdateRoundOver(float deltaTime);

    void SetupMainMenuButtons();
    void SetupCharSelectButtons();
    void SetupSettingsButtons();
    void SetupPauseButtons();

private:
    bool m_isRunning{false};
    GameState m_currentState{GameState::MAIN_MENU};
    int m_currentFrame{0};

    // Match variables
    int m_roundNumber{1};
    int m_p1RoundsWon{0};
    int m_p2RoundsWon{0};
    float m_roundTimer{60.0f};
    FightResult m_roundResult{FightResult::NONE};
    float m_roundEndDelay{0.0f};

    // Selection cursors
    int m_p1SelectedChar{0};
    int m_p2SelectedChar{1};
    bool m_p1Locked{false};
    bool m_p2Locked{false};
    bool m_vsAI{true};

    // Specialized Managers
    std::unique_ptr<ClickManager> m_clickManager;
    std::unique_ptr<ButtonManager> m_buttonManager;
    std::unique_ptr<SettingsManager> m_settingsManager;
    std::unique_ptr<GraphicsManager> m_graphicsManager;
    std::unique_ptr<CharacterManager> m_characterManager;
    std::unique_ptr<AnimationManager> m_animationManager;
    std::unique_ptr<UIManager> m_uiManager;
    std::unique_ptr<CombatManager> m_combatManager;
    std::unique_ptr<SoundManager> m_soundManager;
    std::unique_ptr<StageManager> m_stageManager;
};

} // namespace DarkLotus
