#include "DarkLotus/Managers/ButtonManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

ButtonManager::ButtonManager() = default;
ButtonManager::~ButtonManager() = default;

void ButtonManager::Init() {
    m_buttons.clear();
    m_inputHistory[0].reserve(BUFFER_SIZE);
    m_inputHistory[1].reserve(BUFFER_SIZE);
    Logger::Info("ButtonManager", "Button and Input buffering system initialized.");
}

void ButtonManager::RegisterButton(const UIButton& button) {
    m_buttons[button.id] = button;
}

void ButtonManager::ClearButtons() {
    m_buttons.clear();
}

bool ButtonManager::IsButtonClicked(const std::string& id) const {
    return m_lastClickedButton == id;
}

void ButtonManager::Update(float /*deltaTime*/, int currentFrame) {
    m_lastClickedButton = "";
    Vector2 mousePos = ::GetMousePosition();
    bool mouseClicked = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Update UI buttons
    for (auto& [id, btn] : m_buttons) {
        if (!btn.enabled) {
            btn.state = ButtonVisualState::DISABLED;
            continue;
        }

        if (::CheckCollisionPointRec(mousePos, btn.bounds)) {
            if (::IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                btn.state = ButtonVisualState::PRESSED;
            } else {
                btn.state = ButtonVisualState::HOVERED;
            }

            if (mouseClicked) {
                m_lastClickedButton = id;
                if (btn.onClick) {
                    btn.onClick();
                }
            }
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

    // Movement
    if (::IsKeyDown(KEY_W)) p1 |= BTN_UP;
    if (::IsKeyDown(KEY_S)) p1 |= BTN_DOWN;
    if (::IsKeyDown(KEY_A)) p1 |= BTN_LEFT;
    if (::IsKeyDown(KEY_D)) p1 |= BTN_RIGHT;

    // Tekken 4-buttons: 1=LP (U), 2=RP (I), 3=LK (J), 4=RK (K)
    if (::IsKeyDown(KEY_U)) p1 |= BTN_LP;
    if (::IsKeyDown(KEY_I)) p1 |= BTN_RP;
    if (::IsKeyDown(KEY_J)) p1 |= BTN_LK;
    if (::IsKeyDown(KEY_K)) p1 |= BTN_RK;

    // Shortcuts
    if (::IsKeyDown(KEY_O)) p1 |= (BTN_LP | BTN_RP); // 1+2
    if (::IsKeyDown(KEY_L)) p1 |= (BTN_LK | BTN_RK); // 3+4
    if (::IsKeyDown(KEY_SPACE)) p1 |= BTN_HEAT_RAGE; // Rage Art / Heat Burst

    // Gamepad 0 support
    if (::IsGamepadAvailable(0)) {
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) p1 |= BTN_UP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) p1 |= BTN_DOWN;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) p1 |= BTN_LEFT;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) p1 |= BTN_RIGHT;

        // Stick analog threshold
        float stickX = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float stickY = ::GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (stickX > 0.5f) p1 |= BTN_RIGHT;
        if (stickX < -0.5f) p1 |= BTN_LEFT;
        if (stickY > 0.5f) p1 |= BTN_DOWN;
        if (stickY < -0.5f) p1 |= BTN_UP;

        // Face buttons (X=LP, Y=RP, A=LK, B=RK)
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) p1 |= BTN_LP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))   p1 |= BTN_RP;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) p1 |= BTN_LK;
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))p1 |= BTN_RK;

        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p1 |= (BTN_LP | BTN_RP);
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) p1 |= (BTN_LK | BTN_RK);
        if (::IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1))  p1 |= BTN_HEAT_RAGE;
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

        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) p2 |= BTN_LP;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_UP))   p2 |= BTN_RP;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) p2 |= BTN_LK;
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))p2 |= BTN_RK;

        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) p2 |= (BTN_LP | BTN_RP);
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) p2 |= (BTN_LK | BTN_RK);
        if (::IsGamepadButtonDown(1, GAMEPAD_BUTTON_LEFT_TRIGGER_1))  p2 |= BTN_HEAT_RAGE;
    }

    m_currentInputs[1] = p2;

    // Record into input history
    for (int p = 0; p < 2; ++p) {
        if (m_inputHistory[p].size() >= BUFFER_SIZE) {
            m_inputHistory[p].erase(m_inputHistory[p].begin());
        }
        m_inputHistory[p].push_back({m_currentInputs[p], currentFrame});
    }
}

void ButtonManager::DrawUI() {
    for (const auto& [id, btn] : m_buttons) {
        Color bgColor = btn.normalColor;
        Color borderColor = Color{80, 80, 90, 255};

        if (btn.state == ButtonVisualState::HOVERED) {
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
        ::DrawRectangleLinesEx(btn.bounds, 2.0f, borderColor);

        // Center text
        int fontSize = 20;
        int textWidth = ::MeasureText(btn.label.c_str(), fontSize);
        int textX = static_cast<int>(btn.bounds.x + (btn.bounds.width - textWidth) / 2.0f);
        int textY = static_cast<int>(btn.bounds.y + (btn.bounds.height - fontSize) / 2.0f);

        ::DrawText(btn.label.c_str(), textX, textY, fontSize, btn.textColor);
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

    // Scan backwards from the newest input within windowFrames
    int seqIdx = static_cast<int>(sequence.size()) - 1;
    for (int i = static_cast<int>(history.size()) - 1; i >= 0; --i) {
        if (currentFrame - history[i].frame > windowFrames) {
            break;
        }

        if ((history[i].buttons & sequence[seqIdx]) == sequence[seqIdx]) {
            seqIdx--;
            if (seqIdx < 0) {
                return true; // Entire sequence matched!
            }
        }
    }
    return false;
}

} // namespace DarkLotus
