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

int ButtonManager::GetPrimaryGamepadIndex() const {
    int fallback = -1;
    for (int i = 0; i < 4; ++i) {
        if (::IsGamepadAvailable(i)) {
            const char* name = ::GetGamepadName(i);
            std::string n = name ? name : "";
            // Prioritize physical Xbox / 360 / ShanWan controller over virtual devices
            if (n.find("Virtual") == std::string::npos &&
               (n.find("Xbox") != std::string::npos ||
                n.find("X-Box") != std::string::npos ||
                n.find("360") != std::string::npos ||
                n.find("ShanWan") != std::string::npos ||
                n.find("Pad") != std::string::npos ||
                n.find("Controller") != std::string::npos)) {
                return i;
            }
            if (fallback < 0) fallback = i;
        }
    }
    return fallback;
}

bool ButtonManager::IsAnyGamepadConnected() const {
    for (int i = 0; i < 4; ++i) {
        if (::IsGamepadAvailable(i)) return true;
    }
    return false;
}

bool ButtonManager::IsGamepadConnected(int playerIndex) const {
    if (playerIndex == 0) {
        return IsAnyGamepadConnected();
    }
    return ::IsGamepadAvailable(playerIndex);
}

std::string ButtonManager::GetGamepadDeviceName(int playerIndex) const {
    if (playerIndex == 0) {
        int pad = GetPrimaryGamepadIndex();
        if (pad >= 0 && ::IsGamepadAvailable(pad)) {
            const char* name = ::GetGamepadName(pad);
            return name ? std::string(name) : "Xbox 360 Controller";
        }
    } else {
        if (::IsGamepadAvailable(playerIndex)) {
            const char* name = ::GetGamepadName(playerIndex);
            return name ? std::string(name) : "Gamepad";
        }
    }
    return "Disconnected";
}

Vector2 ButtonManager::GetGamepadStick(int playerIndex) const {
    int pad = (playerIndex == 0) ? GetPrimaryGamepadIndex() : playerIndex;
    if (pad >= 0 && ::IsGamepadAvailable(pad)) {
        return Vector2{
            ::GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X),
            ::GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y)
        };
    }
    return Vector2{0.0f, 0.0f};
}

void ButtonManager::TriggerVibration(int playerIndex, float leftMotor, float rightMotor, float duration) {
    int pad = (playerIndex == 0) ? GetPrimaryGamepadIndex() : playerIndex;
    if (pad >= 0 && ::IsGamepadAvailable(pad)) {
        ::SetGamepadVibration(pad, leftMotor, rightMotor, duration);
    }
}

bool ButtonManager::WasGamepadButtonPressed(int playerIndex, int gamepadButton) const {
    if (playerIndex == 0) {
        int pad = GetPrimaryGamepadIndex();
        if (pad >= 0 && ::IsGamepadButtonPressed(pad, gamepadButton)) return true;
        for (int i = 0; i < 4; ++i) {
            if (::IsGamepadAvailable(i) && ::IsGamepadButtonPressed(i, gamepadButton)) return true;
        }
        return false;
    }
    if (!::IsGamepadAvailable(playerIndex)) return false;
    return ::IsGamepadButtonPressed(playerIndex, gamepadButton);
}

bool ButtonManager::WasGamepadButtonReleased(int playerIndex, int gamepadButton) const {
    if (playerIndex == 0) {
        int pad = GetPrimaryGamepadIndex();
        if (pad >= 0 && ::IsGamepadButtonReleased(pad, gamepadButton)) return true;
        for (int i = 0; i < 4; ++i) {
            if (::IsGamepadAvailable(i) && ::IsGamepadButtonReleased(i, gamepadButton)) return true;
        }
        return false;
    }
    if (!::IsGamepadAvailable(playerIndex)) return false;
    return ::IsGamepadButtonReleased(playerIndex, gamepadButton);
}

float ButtonManager::GetGamepadAxis(int playerIndex, int axis) const {
    if (playerIndex == 0) {
        int pad = GetPrimaryGamepadIndex();
        if (pad >= 0 && ::IsGamepadAvailable(pad)) {
            return ::GetGamepadAxisMovement(pad, axis);
        }
        for (int i = 0; i < 4; ++i) {
            if (::IsGamepadAvailable(i)) {
                float v = ::GetGamepadAxisMovement(i, axis);
                if (std::abs(v) > 0.15f) return v;
            }
        }
        return 0.0f;
    }
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

        // Check ALL connected gamepads and keyboard for menu navigation
        for (int g = 0; g < 4; ++g) {
            if (!::IsGamepadAvailable(g)) continue;

            float stickY = ::GetGamepadAxisMovement(g, GAMEPAD_AXIS_LEFT_Y);
            float stickX = ::GetGamepadAxisMovement(g, GAMEPAD_AXIS_LEFT_X);

            if (::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_LEFT_FACE_UP) || stickY < -0.6f) padUp = true;
            if (::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_LEFT_FACE_DOWN) || stickY > 0.6f) padDown = true;
            if (::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_LEFT_FACE_LEFT) || stickX < -0.6f) padUp = true;
            if (::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) || stickX > 0.6f) padDown = true;

            // Confirm on A / Cross or Start / Menu button
            if (::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                ::IsGamepadButtonPressed(g, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
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
    // POLL PLAYER 1 INPUTS (Keyboard & Gamepad)
    // ==========================================
    unsigned int p1 = BTN_NONE;

    // Movement (Keyboard):
    // In single player / VS CPU / practice, Arrow Keys AND WASD both move Player 1!
    if (m_twoPlayerMode) {
        if (::IsKeyDown(KEY_W)) p1 |= BTN_UP;
        if (::IsKeyDown(KEY_S)) p1 |= BTN_DOWN;
        if (::IsKeyDown(KEY_A)) p1 |= BTN_LEFT;
        if (::IsKeyDown(KEY_D)) p1 |= BTN_RIGHT;
    } else {
        if (::IsKeyDown(KEY_W) || ::IsKeyDown(KEY_UP))    p1 |= BTN_UP;
        if (::IsKeyDown(KEY_S) || ::IsKeyDown(KEY_DOWN))  p1 |= BTN_DOWN;
        if (::IsKeyDown(KEY_A) || ::IsKeyDown(KEY_LEFT))  p1 |= BTN_LEFT;
        if (::IsKeyDown(KEY_D) || ::IsKeyDown(KEY_RIGHT)) p1 |= BTN_RIGHT;
    }

    // Tekken 4-buttons (Keyboard):
    // Standard Tekken layout: 1=LP (U), 2=RP (I), 3=LK (J), 4=RK (K)
    // Classic arcade bottom-row: 1=LP (Z), 2=RP (X), 3=LK (C), 4=RK (V)
    if (::IsKeyDown(KEY_U) || ::IsKeyDown(KEY_Z)) p1 |= BTN_LP;
    if (::IsKeyDown(KEY_I) || ::IsKeyDown(KEY_X)) p1 |= BTN_RP;
    if (::IsKeyDown(KEY_J) || ::IsKeyDown(KEY_C)) p1 |= BTN_LK;
    if (::IsKeyDown(KEY_K) || ::IsKeyDown(KEY_V)) p1 |= BTN_RK;

    // Shortcuts (Keyboard):
    if (::IsKeyDown(KEY_O) || ::IsKeyDown(KEY_F)) p1 |= (BTN_LP | BTN_RP); // 1+2
    if (::IsKeyDown(KEY_L) || ::IsKeyDown(KEY_G)) p1 |= (BTN_LK | BTN_RK); // 3+4
    if (::IsKeyDown(KEY_H) || ::IsKeyDown(KEY_T)) p1 |= BTN_THROW;          // Throw (1+3)
    if (::IsKeyDown(KEY_SPACE) || ::IsKeyDown(KEY_R)) p1 |= BTN_HEAT_RAGE; // Rage Art

    // Numpad support for P1 when in single player
    if (!m_twoPlayerMode) {
        if (::IsKeyDown(KEY_KP_4)) p1 |= BTN_LP;
        if (::IsKeyDown(KEY_KP_5)) p1 |= BTN_RP;
        if (::IsKeyDown(KEY_KP_1)) p1 |= BTN_LK;
        if (::IsKeyDown(KEY_KP_2)) p1 |= BTN_RK;
        if (::IsKeyDown(KEY_KP_6)) p1 |= (BTN_LP | BTN_RP);
        if (::IsKeyDown(KEY_KP_3)) p1 |= (BTN_LK | BTN_RK);
        if (::IsKeyDown(KEY_KP_7)) p1 |= BTN_THROW;
        if (::IsKeyDown(KEY_KP_ENTER)) p1 |= BTN_HEAT_RAGE;
    }

    // ==========================================
    // GAMEPAD SUPPORT FOR PLAYER 1 (Physical Xbox & All Available Gamepads)
    // ==========================================
    auto pollGamepadToPlayer1 = [&](int padIdx) {
        if (!::IsGamepadAvailable(padIdx)) return;

        // D-Pad Movement
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_FACE_UP))    p1 |= BTN_UP;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_FACE_DOWN))  p1 |= BTN_DOWN;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_FACE_LEFT))  p1 |= BTN_LEFT;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) p1 |= BTN_RIGHT;

        // Left Analog Stick Movement (with deadzone filtering)
        float stickX = ::GetGamepadAxisMovement(padIdx, GAMEPAD_AXIS_LEFT_X);
        float stickY = ::GetGamepadAxisMovement(padIdx, GAMEPAD_AXIS_LEFT_Y);
        if (stickX > 0.35f)  p1 |= BTN_RIGHT;
        if (stickX < -0.35f) p1 |= BTN_LEFT;
        if (stickY > 0.35f)  p1 |= BTN_DOWN;
        if (stickY < -0.35f) p1 |= BTN_UP;

        // Physical Xbox Controller 4-Face Buttons:
        // X (Square / West Face)    -> 1: Left Punch
        // Y (Triangle / North Face)  -> 2: Right Punch
        // A (Cross / South Face)     -> 3: Left Kick
        // B (Circle / East Face)     -> 4: Right Kick
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))  p1 |= BTN_LP;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_FACE_UP))    p1 |= BTN_RP;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  p1 |= BTN_LK;
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) p1 |= BTN_RK;

        // Bumpers & Triggers (Tekken Authentic Shortcuts)
        // RB (Right Bumper): 1 + 2 (Dual Strike / Demon Breath / Mach Punch / Tetsuzanko)
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p1 |= (BTN_LP | BTN_RP);

        // RT (Right Trigger): 3 + 4 (Dual Kick / Raiden Torpedo / Can-Can / Kangaroo Dropkick)
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_TRIGGER_2) ||
            ::GetGamepadAxisMovement(padIdx, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.25f) {
            p1 |= (BTN_LK | BTN_RK);
        }

        // LB (Left Bumper) or R3 (Right Stick Click): Rage Art / Heat Burst
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
            ::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_RIGHT_THUMB)) {
            p1 |= BTN_HEAT_RAGE;
        }

        // LT (Left Trigger) or L3 (Left Stick Click): Throw (1+3)
        if (::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
            ::GetGamepadAxisMovement(padIdx, GAMEPAD_AXIS_LEFT_TRIGGER) > 0.25f ||
            ::IsGamepadButtonDown(padIdx, GAMEPAD_BUTTON_LEFT_THUMB)) {
            p1 |= BTN_THROW;
        }
    };

    int primaryPad = GetPrimaryGamepadIndex();
    if (!m_twoPlayerMode) {
        // In 1P / VS CPU mode, check all connected gamepads so any plugged-in Xbox pad works instantly!
        for (int g = 0; g < 4; ++g) {
            pollGamepadToPlayer1(g);
        }
    } else {
        if (primaryPad >= 0) {
            pollGamepadToPlayer1(primaryPad);
        } else {
            pollGamepadToPlayer1(0);
        }
    }

    m_currentInputs[0] = p1;

    // ==========================================
    // POLL PLAYER 2 INPUTS (Keyboard & Gamepad)
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
    if (::IsKeyDown(KEY_KP_7)) p2 |= BTN_THROW;
    if (::IsKeyDown(KEY_KP_ENTER)) p2 |= BTN_HEAT_RAGE;

    // Gamepad 1 for Player 2 (if distinct from P1's pad in 2P mode)
    if (m_twoPlayerMode) {
        int p2Pad = (primaryPad == 0) ? 1 : 0;
        if (::IsGamepadAvailable(p2Pad)) {
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_FACE_UP))    p2 |= BTN_UP;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN))  p2 |= BTN_DOWN;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT))  p2 |= BTN_LEFT;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) p2 |= BTN_RIGHT;

            float stick2X = ::GetGamepadAxisMovement(p2Pad, GAMEPAD_AXIS_LEFT_X);
            float stick2Y = ::GetGamepadAxisMovement(p2Pad, GAMEPAD_AXIS_LEFT_Y);
            if (stick2X > 0.35f)  p2 |= BTN_RIGHT;
            if (stick2X < -0.35f) p2 |= BTN_LEFT;
            if (stick2Y > 0.35f)  p2 |= BTN_DOWN;
            if (stick2Y < -0.35f) p2 |= BTN_UP;

            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))  p2 |= BTN_LP;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_FACE_UP))    p2 |= BTN_RP;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  p2 |= BTN_LK;
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) p2 |= BTN_RK;

            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p2 |= (BTN_LP | BTN_RP);
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_TRIGGER_2) ||
                ::GetGamepadAxisMovement(p2Pad, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.25f) {
                p2 |= (BTN_LK | BTN_RK);
            }
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
                ::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_RIGHT_THUMB)) {
                p2 |= BTN_HEAT_RAGE;
            }
            if (::IsGamepadButtonDown(p2Pad, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
                ::GetGamepadAxisMovement(p2Pad, GAMEPAD_AXIS_LEFT_TRIGGER) > 0.25f) {
                p2 |= BTN_THROW;
            }
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
    int pad = GetPrimaryGamepadIndex();
    if (pad >= 0 && ::IsGamepadAvailable(pad)) {
        std::string padName = "🎮 Controller: " + GetGamepadDeviceName(0) + " [Connected & Active]";
        ::DrawText(padName.c_str(), 20, static_cast<int>(sh - 30), 16, Color{34, 197, 94, 255});
    } else {
        ::DrawText("🎮 Controller: Plug in Xbox controller or use Keyboard (WASD/Arrows + U/I/J/K or Z/X/C/V)", 20, static_cast<int>(sh - 30), 16, Color{160, 165, 180, 200});
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
