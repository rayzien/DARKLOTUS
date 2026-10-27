#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

ButtonManager::ButtonManager() = default;
ButtonManager::~ButtonManager() = default;

void ButtonManager::Init() {
    m_buttons.clear();
    m_buttonOrder.clear();
    m_focusedButtonIdx = -1;
    m_navCooldown = 0.0f;
    m_inputHistory[0].reserve(BUFFER_SIZE);
    m_inputHistory[1].reserve(BUFFER_SIZE);

    if (::IsGamepadAvailable(0)) {
        Logger::Info("ButtonManager", std::string("Gamepad 0 detected: ") + ::GetGamepadName(0));
    }
    if (::IsGamepadAvailable(1)) {
        Logger::Info("ButtonManager", std::string("Gamepad 1 detected: ") + ::GetGamepadName(1));
    }

    Logger::Info("ButtonManager", "Controller and Input buffering system initialized.");
}

void ButtonManager::RegisterButton(const UIButton& button) {
    if (m_buttons.find(button.id) == m_buttons.end()) {
        m_buttonOrder.push_back(button.id);
    }
    m_buttons[button.id] = button;
    if (m_focusedButtonIdx < 0 && !m_buttonOrder.empty()) {
        m_focusedButtonIdx = 0;
    }
}

void ButtonManager::ClearButtons() {
    m_buttons.clear();
    m_buttonOrder.clear();
    m_focusedButtonIdx = -1;
}

bool ButtonManager::IsButtonClicked(const std::string& id) const {
    return m_lastClickedButton == id;
}

bool ButtonManager::IsGamepadConnected(int playerIndex) const {
    return ::IsGamepadAvailable(playerIndex);
}

std::string ButtonManager::GetGamepadDeviceName(int playerIndex) const {
    if (::IsGamepadAvailable(playerIndex)) {
        const char* name = ::GetGamepadName(playerIndex);
        return name ? std::string(name) : "Generic Gamepad";
    }
    return "Disconnected";
}

void ButtonManager::TriggerVibration(int playerIndex, float leftMotor, float rightMotor, float duration) {
    if (::IsGamepadAvailable(playerIndex)) {
        ::SetGamepadVibration(playerIndex, leftMotor, rightMotor, duration);
    }
}

bool ButtonManager::WasGamepadButtonPressed(int playerIndex, int gamepadButton) const {
    if (!::IsGamepadAvailable(playerIndex)) return false;
    return ::IsGamepadButtonPressed(playerIndex, gamepadButton);
}

bool ButtonManager::WasGamepadButtonReleased(int playerIndex, int gamepadButton) const {
    if (!::IsGamepadAvailable(playerIndex)) return false;
    return ::IsGamepadButtonReleased(playerIndex, gamepadButton);
}

float ButtonManager::GetGamepadAxis(int playerIndex, int axis) const {
    if (!::IsGamepadAvailable(playerIndex)) return 0.0f;
    return ::GetGamepadAxisMovement(playerIndex, axis);
}

void ButtonManager::NavigateFocusNext() {
    if (m_buttonOrder.empty()) return;
    m_focusedButtonIdx = (m_focusedButtonIdx + 1) % static_cast<int>(m_buttonOrder.size());
}

void ButtonManager::NavigateFocusPrev() {
    if (m_buttonOrder.empty()) return;
    m_focusedButtonIdx--;
    if (m_focusedButtonIdx < 0) {
        m_focusedButtonIdx = static_cast<int>(m_buttonOrder.size()) - 1;
    }
}

void ButtonManager::ActivateFocusedButton() {
    if (m_focusedButtonIdx >= 0 && m_focusedButtonIdx < static_cast<int>(m_buttonOrder.size())) {
        const std::string& id = m_buttonOrder[m_focusedButtonIdx];
        auto it = m_buttons.find(id);
        if (it != m_buttons.end() && it->second.enabled) {
            m_lastClickedButton = id;
            auto cb = it->second.onClick;
            if (cb) {
                cb();
            }
        }
    }
}

void ButtonManager::Update(float deltaTime, int currentFrame) {
    m_lastClickedButton = "";
    Vector2 mousePos = ::GetMousePosition();
    bool mouseClicked = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    std::function<void()> pendingAction = nullptr;

    // ==========================================
    // GAMEPAD MENU NAVIGATION
    // ==========================================
    if (m_navCooldown > 0.0f) {
        m_navCooldown -= deltaTime;
    }

    if (!m_buttonOrder.empty()) {
        bool padUp = false;
        bool padDown = false;
        bool padConfirm = false;

        // Check Gamepad 0 or Keyboard arrow keys for menu navigation
        if (::IsGamepadAvailable(0)) {
            float stickY = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
            float stickX = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);

            if (::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP) || stickY < -0.6f) padUp = true;
            if (::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || stickY > 0.6f) padDown = true;
            if (::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || stickX < -0.6f) padUp = true;
            if (::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || stickX > 0.6f) padDown = true;

            // Confirm on A / Cross or Start
            if (::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                ::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
                padConfirm = true;
            }
        }

        if (::IsKeyPressed(KEY_UP) || ::IsKeyPressed(KEY_W)) padUp = true;
        if (::IsKeyPressed(KEY_DOWN) || ::IsKeyPressed(KEY_S)) padDown = true;
        if (::IsKeyPressed(KEY_ENTER) || ::IsKeyPressed(KEY_SPACE)) padConfirm = true;

        if (m_navCooldown <= 0.0f) {
            if (padUp) {
                NavigateFocusPrev();
                m_navCooldown = 0.22f;
            } else if (padDown) {
                NavigateFocusNext();
                m_navCooldown = 0.22f;
            }
        }

        if (padConfirm && m_focusedButtonIdx >= 0 && m_focusedButtonIdx < static_cast<int>(m_buttonOrder.size())) {
            const std::string& id = m_buttonOrder[m_focusedButtonIdx];
            auto it = m_buttons.find(id);
            if (it != m_buttons.end() && it->second.enabled) {
                m_lastClickedButton = id;
                pendingAction = it->second.onClick;
            }
        }
    }

    // Update UI buttons hover / click
    for (size_t i = 0; i < m_buttonOrder.size(); ++i) {
        const auto& id = m_buttonOrder[i];
        auto it = m_buttons.find(id);
        if (it == m_buttons.end()) continue;
        auto& btn = it->second;

        if (!btn.enabled) {
            btn.state = ButtonVisualState::DISABLED;
            continue;
        }

        bool isMouseHover = ::CheckCollisionPointRec(mousePos, btn.bounds);
        bool isFocusedByPad = (static_cast<int>(i) == m_focusedButtonIdx);

        if (isMouseHover) {
            m_focusedButtonIdx = static_cast<int>(i);
            if (::IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                btn.state = ButtonVisualState::PRESSED;
            } else {
                btn.state = ButtonVisualState::HOVERED;
            }

            if (mouseClicked) {
                m_lastClickedButton = id;
                if (btn.onClick) {
                    pendingAction = btn.onClick;
                }
            }
        } else if (isFocusedByPad) {
            btn.state = ButtonVisualState::HOVERED;
        } else {
            btn.state = ButtonVisualState::NORMAL;
        }
    }

    // Save previous frame inputs
    m_previousInputs[0] = m_currentInputs[0];
    m_previousInputs[1] = m_currentInputs[1];

    // ==========================================
    // POLL PLAYER 1 INPUTS (Keyboard & Gamepad 0)
    // ==========================================
    unsigned int p1 = BTN_NONE;

    // Movement (Keyboard)
    if (::IsKeyDown(KEY_W)) p1 |= BTN_UP;
    if (::IsKeyDown(KEY_S)) p1 |= BTN_DOWN;
    if (::IsKeyDown(KEY_A)) p1 |= BTN_LEFT;
    if (::IsKeyDown(KEY_D)) p1 |= BTN_RIGHT;

    // Tekken 4-buttons (Keyboard): 1=LP (U), 2=RP (I), 3=LK (J), 4=RK (K)
    if (::IsKeyDown(KEY_U)) p1 |= BTN_LP;
    if (::IsKeyDown(KEY_I)) p1 |= BTN_RP;
    if (::IsKeyDown(KEY_J)) p1 |= BTN_LK;
    if (::IsKeyDown(KEY_K)) p1 |= BTN_RK;

    // Shortcuts (Keyboard)
    if (::IsKeyDown(KEY_O)) p1 |= (BTN_LP | BTN_RP); // 1+2
    if (::IsKeyDown(KEY_L)) p1 |= (BTN_LK | BTN_RK); // 3+4
    if (::IsKeyDown(KEY_SPACE)) p1 |= BTN_HEAT_RAGE; // Rage Art / Heat Burst

    // ==========================================
    // GAMEPAD 0 SUPPORT (Player 1)
    // ==========================================
    if (::IsGamepadAvailable(0)) {
        // D-Pad Movement
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) p1 |= BTN_UP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) p1 |= BTN_DOWN;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) p1 |= BTN_LEFT;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) p1 |= BTN_RIGHT;

        // Left Analog Stick Movement
        float stickX = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float stickY = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (stickX > 0.45f) p1 |= BTN_RIGHT;
        if (stickX < -0.45f) p1 |= BTN_LEFT;
        if (stickY > 0.45f) p1 |= BTN_DOWN;
        if (stickY < -0.45f) p1 |= BTN_UP;

        // Tekken 4-Face Buttons:
        // X (Square) -> 1: Left Punch
        // Y (Triangle) -> 2: Right Punch
        // A (Cross) -> 3: Left Kick
        // B (Circle) -> 4: Right Kick
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))  p1 |= BTN_LP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))    p1 |= BTN_RP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  p1 |= BTN_LK;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) p1 |= BTN_RK;

        // Bumpers & Triggers (Tekken Shortcuts)
        // RB (R1): 1 + 2 (Dual Punch / Demon Breath / Mach Punch / Tetsuzanko / Demon Paw)
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p1 |= (BTN_LP | BTN_RP);

        // RT (R2): 3 + 4 (Dual Kick / Raiden Torpedo / Kangaroo Dropkick / Can-Can)
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2) ||
            ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.4f) {
            p1 |= (BTN_LK | BTN_RK);
        }

        // LB (L1): Heat Burst / Rage Art
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
            ::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_THUMB)) {
            p1 |= BTN_HEAT_RAGE;
        }

        // LT (L2): 1 + 3 (Throw)
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
            ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER) > 0.4f) {
            p1 |= (BTN_LP | BTN_LK);
        }
    }

    m_currentInputs[0] = p1;

    // ==========================================
    // POLL PLAYER 2 INPUTS (Keyboard & Gamepad 1)
    // ==========================================
    unsigned int p2 = BTN_NONE;
    if (::IsKeyDown(KEY_UP))    p2 |= BTN_UP;
    if (::IsKeyDown(KEY_DOWN))  p2 |= BTN_DOWN;
    if (::IsKeyDown(KEY_LEFT))  p2 |= BTN_LEFT;
    if (::IsKeyDown(KEY_RIGHT)) p2 |= BTN_RIGHT;

    // Numpad 4, 5, 1, 2
    if (::IsKeyDown(KEY_KP_4)) p2 |= BTN_LP;
    if (::IsKeyDown(KEY_KP_5)) p2 |= BTN_RP;
    if (::IsKeyDown(KEY_KP_1)) p2 |= BTN_LK;
    if (::IsKeyDown(KEY_KP_2)) p2 |= BTN_RK;
    if (::IsKeyDown(KEY_KP_6)) p2 |= (BTN_LP | BTN_RP);
    if (::IsKeyDown(KEY_KP_3)) p2 |= (BTN_LK | BTN_RK);
    if (::IsKeyDown(KEY_KP_ENTER)) p2 |= BTN_HEAT_RAGE;

    // Gamepad 1 support
    if (::IsGamepadAvailable(1)) {
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_UP)) p2 |= BTN_UP;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) p2 |= BTN_DOWN;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) p2 |= BTN_LEFT;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) p2 |= BTN_RIGHT;

        float stick2X = ::GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_X);
        float stick2Y = ::GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_Y);
        if (stick2X > 0.45f) p2 |= BTN_RIGHT;
        if (stick2X < -0.45f) p2 |= BTN_LEFT;
        if (stick2Y > 0.45f) p2 |= BTN_DOWN;
        if (stick2Y < -0.45f) p2 |= BTN_UP;

        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))  p2 |= BTN_LP;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_UP))    p2 |= BTN_RP;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  p2 |= BTN_LK;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) p2 |= BTN_RK;

        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p2 |= (BTN_LP | BTN_RP);
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_TRIGGER_2) ||
            ::GetGamepadAxisMovement(1, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.4f) {
            p2 |= (BTN_LK | BTN_RK);
        }
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
            ::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_THUMB)) {
            p2 |= BTN_HEAT_RAGE;
        }
    }

    m_currentInputs[1] = p2;

    // Record into input history
    for (int p = 0; p < 2; ++p) {
        if (m_inputHistory[p].size() >= BUFFER_SIZE) {
            m_inputHistory[p].erase(m_inputHistory[p].begin());
        }
        m_inputHistory[p].push_back({m_currentInputs[p], currentFrame});
    }

    // Safely execute pending button action AFTER loops and updates to prevent iterator invalidation
    if (pendingAction) {
        pendingAction();
    }
}

void ButtonManager::DrawUI() {
    for (size_t i = 0; i < m_buttonOrder.size(); ++i) {
        const auto& id = m_buttonOrder[i];
        auto it = m_buttons.find(id);
        if (it == m_buttons.end()) continue;
        const auto& btn = it->second;
        Color bgColor = btn.normalColor;
        Color borderColor = Color{80, 80, 90, 255};

        bool isFocused = (static_cast<int>(i) == m_focusedButtonIdx);

        if (btn.state == ButtonVisualState::HOVERED || isFocused) {
            bgColor = btn.hoverColor;
            borderColor = Color{255, 255, 255, 255};
        } else if (btn.state == ButtonVisualState::PRESSED) {
            bgColor = Color{160, 20, 20, 255};
            borderColor = Color{255, 200, 50, 255};
        } else if (btn.state == ButtonVisualState::DISABLED) {
            bgColor = Color{25, 25, 25, 180};
            borderColor = Color{50, 50, 50, 255};
        }

        // Tekken-style angular beveled rectangle
        ::DrawRectangleRec(btn.bounds, bgColor);
        ::DrawRectangleLinesEx(btn.bounds, isFocused ? 3.0f : 2.0f, isFocused ? Color{250, 204, 21, 255} : borderColor);

        // Center text
        int fontSize = 20;
        int textWidth = ::MeasureText(btn.label.c_str(), fontSize);
        int textX = static_cast<int>(btn.bounds.x + (btn.bounds.width - textWidth) / 2.0f);
        int textY = static_cast<int>(btn.bounds.y + (btn.bounds.height - fontSize) / 2.0f);

        ::DrawText(btn.label.c_str(), textX, textY, fontSize, btn.textColor);
    }

    // Draw Controller connection status banner
    float sh = static_cast<float>(::GetScreenHeight());
    if (::IsGamepadAvailable(0)) {
        std::string padName = "🎮 Controller: " + GetGamepadDeviceName(0) + " [Connected]";
        ::DrawText(padName.c_str(), 20, static_cast<int>(sh - 30), 16, Color{34, 197, 94, 255});
    } else {
        ::DrawText("🎮 No Controller detected (Using Keyboard)", 20, static_cast<int>(sh - 30), 16, Color{160, 165, 180, 200});
    }
}

unsigned int ButtonManager::GetPlayerInput(int playerIndex) const {
    if (playerIndex < 0 || playerIndex > 1) return BTN_NONE;
    return m_currentInputs[playerIndex];
}

bool ButtonManager::WasButtonPressed(int playerIndex, InputButtons button) const {
    if (playerIndex < 0 || playerIndex > 1) return false;
    return (m_currentInputs[playerIndex] & button) && !(m_previousInputs[playerIndex] & button);
}

bool ButtonManager::IsButtonHeld(int playerIndex, InputButtons button) const {
    if (playerIndex < 0 || playerIndex > 1) return false;
    return (m_currentInputs[playerIndex] & button) != 0;
}

bool ButtonManager::CheckInputSequence(int playerIndex, const std::vector<unsigned int>& sequence, int windowFrames, int currentFrame) const {
    if (playerIndex < 0 || playerIndex > 1 || sequence.empty()) return false;
    const auto& history = m_inputHistory[playerIndex];
    if (history.empty()) return false;

    int seqIdx = static_cast<int>(sequence.size()) - 1;
    for (int i = static_cast<int>(history.size()) - 1; i >= 0; --i) {
        if (currentFrame - history[i].frame > windowFrames) {
            break;
        }

        if ((history[i].buttons & sequence[seqIdx]) == sequence[seqIdx]) {
            seqIdx--;
            if (seqIdx < 0) {
                return true;
            }
        }
    }
    return false;
}

} // namespace DarkLotus
