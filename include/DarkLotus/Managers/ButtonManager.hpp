#pragma once

#include "DarkLotus/Core/Common.hpp"
#include "DarkLotus/Core/Types.hpp"

namespace DarkLotus {

enum class ButtonVisualState {
    NORMAL,
    HOVERED,
    PRESSED,
    DISABLED
};

struct UIButton {
    std::string id;
    std::string label;
    Rectangle bounds;
    ButtonVisualState state{ButtonVisualState::NORMAL};
    Color normalColor{40, 44, 52, 230};
    Color hoverColor{220, 38, 38, 255}; // Tekken Crimson Red
    Color textColor{240, 240, 240, 255};
    std::function<void()> onClick;
    bool enabled{true};
};

struct BufferedInput {
    unsigned int buttons; // Bitmask of InputButtons
    int frame;            // Game frame when pressed
};

class ButtonManager {
public:
    ButtonManager();
    ~ButtonManager();

    void Init();
    void Update(float deltaTime, int currentFrame);
    void DrawUI();

    // UI Button management
    void RegisterButton(const UIButton& button);
    void ClearButtons();
    bool IsButtonClicked(const std::string& id) const;

    // Fighting game button state polling
    unsigned int GetPlayerInput(int playerIndex) const;
    bool WasButtonPressed(int playerIndex, InputButtons button) const;
    bool IsButtonHeld(int playerIndex, InputButtons button) const;

    // Input buffer for combo execution
    bool CheckInputSequence(int playerIndex, const std::vector<unsigned int>& sequence, int windowFrames, int currentFrame) const;

    // Gamepad detection and query
    bool IsGamepadConnected(int playerIndex = 0) const;
    std::string GetGamepadDeviceName(int playerIndex = 0) const;
    void TriggerVibration(int playerIndex, float leftMotor, float rightMotor, float duration);

    // Controller Navigation
    bool WasGamepadButtonPressed(int playerIndex, int gamepadButton) const;
    bool WasGamepadButtonReleased(int playerIndex, int gamepadButton) const;
    float GetGamepadAxis(int playerIndex, int axis) const;

    // Menu Focus Navigation
    void SetFocusedButtonIndex(int index) { m_focusedButtonIdx = index; }
    int GetFocusedButtonIndex() const { return m_focusedButtonIdx; }
    int GetButtonCount() const { return static_cast<int>(m_buttonOrder.size()); }
    void NavigateFocusNext();
    void NavigateFocusPrev();
    void ActivateFocusedButton();

private:
    std::unordered_map<std::string, UIButton> m_buttons;
    std::vector<std::string> m_buttonOrder;
    int m_focusedButtonIdx{-1};
    std::string m_lastClickedButton;

    // Input bitmasks for P1 and P2
    unsigned int m_currentInputs[2]{0, 0};
    unsigned int m_previousInputs[2]{0, 0};

    // Controller navigation cooldown timer
    float m_navCooldown{0.0f};

    // Circular input history buffer (60 frames)
    static constexpr size_t BUFFER_SIZE = 60;
    std::vector<BufferedInput> m_inputHistory[2];
};

} // namespace DarkLotus
