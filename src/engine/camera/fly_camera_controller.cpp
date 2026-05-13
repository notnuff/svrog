#include "fly_camera_controller.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../components/transform_component.h"
#include "../entities/entity.h"
#include "../events/event_bus.h"
#include "../events/event_dispatcher.h"
#include "../input/input_events.h"
#include "../input/input_system.h"

namespace nuff::engine {

void FlyCameraController::init() {}

void FlyCameraController::render() {}

void FlyCameraController::bindInput(input::InputSystem& input) {
    unbindInput();
    auto& bus = input.events();
    subscribeTo<input::KeyEvent>(bus);
    subscribeTo<input::MouseButtonEvent>(bus);
    subscribeTo<input::MouseMoveEvent>(bus);
}

void FlyCameraController::unbindInput() {
    unsubscribeAll();
}

void FlyCameraController::onEvent(const events::IEvent& event) {
    events::EventDispatcher d(event);

    d.dispatch<input::KeyEvent>([this](const input::KeyEvent& ke) {
        if (ke.key == input::KeyCode::Unknown) return;
        const auto idx = static_cast<std::size_t>(ke.key);
        if (idx >= m_keyDown.size()) return;
        if (ke.action == input::KeyAction::Press)   m_keyDown[idx] = true;
        if (ke.action == input::KeyAction::Release) m_keyDown[idx] = false;
    });

    d.dispatch<input::MouseButtonEvent>([this](const input::MouseButtonEvent& mb) {
        if (mb.button != input::MouseButton::Left) return;
        if (mb.action == input::KeyAction::Press)   m_leftMouseDown = true;
        if (mb.action == input::KeyAction::Release) m_leftMouseDown = false;
    });

    d.dispatch<input::MouseMoveEvent>([this](const input::MouseMoveEvent& mm) {
        if (!m_leftMouseDown) return;
        m_accumDx += mm.dx;
        m_accumDy += mm.dy;
    });
}

void FlyCameraController::update(float deltaTime) {
    if (!m_owner) return;
    auto* xf = m_owner->getComponent<TransformComponent>();
    if (!xf) return;

    auto keyAxis = [this](input::KeyCode pos, input::KeyCode neg) -> float {
        const float p = m_keyDown[static_cast<std::size_t>(pos)] ? 1.0f : 0.0f;
        const float n = m_keyDown[static_cast<std::size_t>(neg)] ? 1.0f : 0.0f;
        return p - n;
    };

    const float fwdAxis   = keyAxis(input::KeyCode::W, input::KeyCode::S);
    const float rightAxis = keyAxis(input::KeyCode::D, input::KeyCode::A);
    const float upAxis    = keyAxis(input::KeyCode::E, input::KeyCode::Q);

    const bool boost = m_keyDown[static_cast<std::size_t>(input::KeyCode::LeftShift)]
                    || m_keyDown[static_cast<std::size_t>(input::KeyCode::RightShift)];
    const float speed = m_moveSpeed * (boost ? m_boostMultiplier : 1.0f);

    if (fwdAxis != 0.0f || rightAxis != 0.0f || upAxis != 0.0f) {
        const glm::vec3 fwd   = xf->forward();
        const glm::vec3 right = xf->right();
        const glm::vec3 worldUp{0.0f, 0.0f, 1.0f};

        glm::vec3 delta = fwd * fwdAxis + right * rightAxis + worldUp * upAxis;
        if (glm::dot(delta, delta) > 0.0f) {
            delta = glm::normalize(delta) * speed * deltaTime;
            xf->setPosition(xf->position() + delta);
        }
    }

    if (m_leftMouseDown && (m_accumDx != 0.0 || m_accumDy != 0.0)) {
        const float yaw   = -static_cast<float>(m_accumDx) * m_lookSensitivity;
        const float pitch = -static_cast<float>(m_accumDy) * m_lookSensitivity;
        m_accumDx = 0.0;
        m_accumDy = 0.0;

        const glm::vec3 worldUp{0.0f, 0.0f, 1.0f};
        const glm::quat qYaw   = glm::angleAxis(yaw,   worldUp);
        const glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        xf->setRotation(glm::normalize(qYaw * xf->rotation() * qPitch));
    }
}

void FlyCameraController::setMoveSpeed(float unitsPerSecond) { m_moveSpeed = unitsPerSecond; }
float FlyCameraController::moveSpeed() const { return m_moveSpeed; }

void FlyCameraController::setLookSensitivity(float radiansPerPixel) { m_lookSensitivity = radiansPerPixel; }
float FlyCameraController::lookSensitivity() const { return m_lookSensitivity; }

void FlyCameraController::setBoostMultiplier(float multiplier) { m_boostMultiplier = multiplier; }
float FlyCameraController::boostMultiplier() const { return m_boostMultiplier; }

} // namespace nuff::engine
