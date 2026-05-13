#include "input_system.h"

#include "input_events.h"

namespace nuff::engine::input {

InputSystem::InputSystem() = default;
InputSystem::~InputSystem() = default;

void InputSystem::postKey(KeyCode key, KeyAction action, KeyMod mods) {
    if (key != KeyCode::Unknown) {
        const auto idx = static_cast<std::size_t>(key);
        if (idx < m_keyDown.size()) {
            if (action == KeyAction::Press)   m_keyDown[idx] = true;
            if (action == KeyAction::Release) m_keyDown[idx] = false;
        }
    }
    m_bus.sendEvent(KeyEvent{key, action, mods});
}

void InputSystem::postMouseButton(MouseButton button, KeyAction action,
                                  double x, double y, KeyMod mods) {
    if (button != MouseButton::Unknown) {
        const auto idx = static_cast<std::size_t>(button);
        if (idx < m_mouseDown.size()) {
            if (action == KeyAction::Press)   m_mouseDown[idx] = true;
            if (action == KeyAction::Release) m_mouseDown[idx] = false;
        }
    }
    m_mouseX = x;
    m_mouseY = y;
    m_hasMousePos = true;
    m_bus.sendEvent(MouseButtonEvent{button, action, x, y, mods});
}

void InputSystem::postMouseMove(double x, double y) {
    double dx = 0.0;
    double dy = 0.0;
    if (m_hasMousePos) {
        dx = x - m_mouseX;
        dy = y - m_mouseY;
    }
    m_mouseX = x;
    m_mouseY = y;
    m_hasMousePos = true;
    m_bus.sendEvent(MouseMoveEvent{x, y, dx, dy});
}

void InputSystem::postScroll(double dx, double dy) {
    m_bus.sendEvent(MouseScrollEvent{dx, dy});
}

bool InputSystem::isKeyDown(KeyCode key) const {
    const auto idx = static_cast<std::size_t>(key);
    if (idx >= m_keyDown.size()) return false;
    return m_keyDown[idx];
}

bool InputSystem::isMouseButtonDown(MouseButton button) const {
    const auto idx = static_cast<std::size_t>(button);
    if (idx >= m_mouseDown.size()) return false;
    return m_mouseDown[idx];
}

} // namespace nuff::engine::input
