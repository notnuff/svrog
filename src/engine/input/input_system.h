#pragma once

#include <QtCore/qtclasshelpermacros.h>

#include <array>
#include <cstddef>

#include "../events/event_bus.h"
#include "input_codes.h"

namespace nuff::engine::input {

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    Q_DISABLE_COPY_MOVE(InputSystem)

    void postKey(KeyCode key, KeyAction action, KeyMod mods);
    void postMouseButton(MouseButton button, KeyAction action,
                         double x, double y, KeyMod mods);
    void postMouseMove(double x, double y);
    void postScroll(double dx, double dy);

    bool isKeyDown(KeyCode key) const;
    bool isMouseButtonDown(MouseButton button) const;

    double mouseX() const { return m_mouseX; }
    double mouseY() const { return m_mouseY; }

    events::EventBus& events() { return m_bus; }

private:
    events::EventBus m_bus;

    std::array<bool, static_cast<std::size_t>(KeyCode::Count)>     m_keyDown{};
    std::array<bool, static_cast<std::size_t>(MouseButton::Count)> m_mouseDown{};

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    bool   m_hasMousePos = false;
};

} // namespace nuff::engine::input
