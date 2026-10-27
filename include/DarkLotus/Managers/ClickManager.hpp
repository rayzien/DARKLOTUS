#pragma once

#include "DarkLotus/Core/Common.hpp"

namespace DarkLotus {

struct ClickEvent {
    Vector2 screenPosition;
    Ray worldRay;
    int mouseButton; // MOUSE_BUTTON_LEFT, MOUSE_BUTTON_RIGHT
    bool isDown;
    bool isPressed;
    bool isReleased;
    bool isDoubleClick;
};

class ClickManager {
public:
    ClickManager();
    ~ClickManager();

    void Init();
    void Update(const Camera3D& currentCamera);

    Vector2 GetMousePosition() const { return m_currentPos; }
    Vector2 GetMouseDelta() const { return m_delta; }
    Ray GetWorldRay() const { return m_worldRay; }

    bool IsLeftClicked() const { return m_leftPressed; }
    bool IsLeftDown() const { return m_leftDown; }
    bool IsLeftReleased() const { return m_leftReleased; }
    bool IsLeftDoubleClick() const { return m_isDoubleClick; }

    bool IsRightClicked() const { return m_rightPressed; }
    bool IsRightDown() const { return m_rightDown; }

    // 3D picking helpers
    bool RaycastSphere(const Vector3& center, float radius, RayCollision* outCollision = nullptr) const;
    bool RaycastBox(const BoundingBox& box, RayCollision* outCollision = nullptr) const;
    bool RaycastGroundPlane(Vector3* outIntersection = nullptr) const;

private:
    Vector2 m_currentPos{0.0f, 0.0f};
    Vector2 m_prevPos{0.0f, 0.0f};
    Vector2 m_delta{0.0f, 0.0f};
    Ray m_worldRay{};

    bool m_leftPressed{false};
    bool m_leftDown{false};
    bool m_leftReleased{false};
    bool m_rightPressed{false};
    bool m_rightDown{false};

    double m_lastClickTime{0.0};
    bool m_isDoubleClick{false};
    static constexpr double DOUBLE_CLICK_TIME = 0.25; // 250ms threshold
};

} // namespace DarkLotus
