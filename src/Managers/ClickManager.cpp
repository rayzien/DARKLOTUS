#include "DarkLotus/Managers/ClickManager.hpp"
#include "DarkLotus/Utils/Logger.hpp"

namespace DarkLotus {

ClickManager::ClickManager() = default;
ClickManager::~ClickManager() = default;

void ClickManager::Init() {
    m_currentPos = ::GetMousePosition();
    m_prevPos = m_currentPos;
    m_delta = Vector2{0.0f, 0.0f};
    Logger::Info("ClickManager", "Click and Raycast picking system initialized.");
}

void ClickManager::Update(const Camera3D& currentCamera) {
    m_prevPos = m_currentPos;
    m_currentPos = ::GetMousePosition();
    m_delta = Vector2{m_currentPos.x - m_prevPos.x, m_currentPos.y - m_prevPos.y};

    m_leftPressed = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    m_leftDown = ::IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    m_leftReleased = ::IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    m_rightPressed = ::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    m_rightDown = ::IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

    // Double click detection
    m_isDoubleClick = false;
    if (m_leftPressed) {
        double now = ::GetTime();
        if (now - m_lastClickTime < DOUBLE_CLICK_TIME) {
            m_isDoubleClick = true;
        }
        m_lastClickTime = now;
    }

    // Generate 3D ray into the scene
    m_worldRay = ::GetScreenToWorldRay(m_currentPos, currentCamera);
}

bool ClickManager::RaycastSphere(const Vector3& center, float radius, RayCollision* outCollision) const {
    RayCollision col = ::GetRayCollisionSphere(m_worldRay, center, radius);
    if (outCollision) *outCollision = col;
    return col.hit;
}

bool ClickManager::RaycastBox(const BoundingBox& box, RayCollision* outCollision) const {
    RayCollision col = ::GetRayCollisionBox(m_worldRay, box);
    if (outCollision) *outCollision = col;
    return col.hit;
}

bool ClickManager::RaycastGroundPlane(Vector3* outIntersection) const {
    // Plane at Y=0 with normal (0, 1, 0)
    RayCollision col = ::GetRayCollisionQuad(
        m_worldRay,
        Vector3{-1000.0f, 0.0f, -1000.0f},
        Vector3{-1000.0f, 0.0f,  1000.0f},
        Vector3{ 1000.0f, 0.0f,  1000.0f},
        Vector3{ 1000.0f, 0.0f, -1000.0f}
    );
    if (col.hit && outIntersection) {
        *outIntersection = col.point;
    }
    return col.hit;
}

} // namespace DarkLotus
