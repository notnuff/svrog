#pragma once

#include "../events/i_event.h"
#include "input_codes.h"

namespace nuff::engine::input {

struct KeyEvent : public events::Event<KeyEvent> {
    KeyCode   key;
    KeyAction action;
    KeyMod    mods;

    KeyEvent(KeyCode key, KeyAction action, KeyMod mods)
        : key(key), action(action), mods(mods) {}
};

struct MouseButtonEvent : public events::Event<MouseButtonEvent> {
    MouseButton button;
    KeyAction   action;
    double      x;
    double      y;
    KeyMod      mods;

    MouseButtonEvent(MouseButton button, KeyAction action,
                     double x, double y, KeyMod mods)
        : button(button), action(action), x(x), y(y), mods(mods) {}
};

struct MouseMoveEvent : public events::Event<MouseMoveEvent> {
    double x;
    double y;
    double dx;
    double dy;

    MouseMoveEvent(double x, double y, double dx, double dy)
        : x(x), y(y), dx(dx), dy(dy) {}
};

struct MouseScrollEvent : public events::Event<MouseScrollEvent> {
    double dx;
    double dy;

    MouseScrollEvent(double dx, double dy) : dx(dx), dy(dy) {}
};

} // namespace nuff::engine::input
