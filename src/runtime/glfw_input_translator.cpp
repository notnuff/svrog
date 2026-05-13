#include "glfw_input_translator.h"

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace nuff::runtime::input {

using engine::input::KeyCode;
using engine::input::MouseButton;
using engine::input::KeyAction;
using engine::input::KeyMod;

KeyCode translateKey(int glfwKey) {
    if (glfwKey >= GLFW_KEY_A && glfwKey <= GLFW_KEY_Z) {
        const int offset = glfwKey - GLFW_KEY_A;
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + offset);
    }
    if (glfwKey >= GLFW_KEY_0 && glfwKey <= GLFW_KEY_9) {
        const int offset = glfwKey - GLFW_KEY_0;
        return static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + offset);
    }

    switch (glfwKey) {
        case GLFW_KEY_SPACE:     return KeyCode::Space;
        case GLFW_KEY_ESCAPE:    return KeyCode::Escape;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER:  return KeyCode::Enter;
        case GLFW_KEY_TAB:       return KeyCode::Tab;
        case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;

        case GLFW_KEY_LEFT_SHIFT:   return KeyCode::LeftShift;
        case GLFW_KEY_RIGHT_SHIFT:  return KeyCode::RightShift;
        case GLFW_KEY_LEFT_CONTROL: return KeyCode::LeftCtrl;
        case GLFW_KEY_RIGHT_CONTROL:return KeyCode::RightCtrl;
        case GLFW_KEY_LEFT_ALT:     return KeyCode::LeftAlt;
        case GLFW_KEY_RIGHT_ALT:    return KeyCode::RightAlt;
        case GLFW_KEY_LEFT_SUPER:   return KeyCode::LeftSuper;
        case GLFW_KEY_RIGHT_SUPER:  return KeyCode::RightSuper;

        case GLFW_KEY_UP:    return KeyCode::Up;
        case GLFW_KEY_DOWN:  return KeyCode::Down;
        case GLFW_KEY_LEFT:  return KeyCode::Left;
        case GLFW_KEY_RIGHT: return KeyCode::Right;
    }
    return KeyCode::Unknown;
}

MouseButton translateMouseButton(int glfwButton) {
    switch (glfwButton) {
        case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:      return MouseButton::X1;
        case GLFW_MOUSE_BUTTON_5:      return MouseButton::X2;
        default:                       return MouseButton::Unknown;
    }
}

KeyAction translateAction(int glfwAction) {
    switch (glfwAction) {
        case GLFW_PRESS:   return KeyAction::Press;
        case GLFW_RELEASE: return KeyAction::Release;
        case GLFW_REPEAT:  return KeyAction::Repeat;
        default:           return KeyAction::Release;
    }
}

KeyMod translateModifiers(int glfwMods) {
    KeyMod result = KeyMod::None;
    if (glfwMods & GLFW_MOD_SHIFT)   result |= KeyMod::Shift;
    if (glfwMods & GLFW_MOD_CONTROL) result |= KeyMod::Ctrl;
    if (glfwMods & GLFW_MOD_ALT)     result |= KeyMod::Alt;
    if (glfwMods & GLFW_MOD_SUPER)   result |= KeyMod::Super;
    return result;
}

} // namespace nuff::runtime::input
