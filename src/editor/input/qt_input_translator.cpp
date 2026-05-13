#include "qt_input_translator.h"

namespace nuff::editor::input {

using engine::input::KeyCode;
using engine::input::MouseButton;
using engine::input::KeyMod;

KeyCode translateKey(int qtKey) {
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
        const int offset = qtKey - Qt::Key_A;
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + offset);
    }
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
        const int offset = qtKey - Qt::Key_0;
        return static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + offset);
    }

    switch (qtKey) {
        case Qt::Key_Space:     return KeyCode::Space;
        case Qt::Key_Escape:    return KeyCode::Escape;
        case Qt::Key_Return:
        case Qt::Key_Enter:     return KeyCode::Enter;
        case Qt::Key_Tab:       return KeyCode::Tab;
        case Qt::Key_Backspace: return KeyCode::Backspace;

        case Qt::Key_Shift:     return KeyCode::LeftShift;
        case Qt::Key_Control:   return KeyCode::LeftCtrl;
        case Qt::Key_Alt:       return KeyCode::LeftAlt;
        case Qt::Key_Meta:      return KeyCode::LeftSuper;

        case Qt::Key_Up:    return KeyCode::Up;
        case Qt::Key_Down:  return KeyCode::Down;
        case Qt::Key_Left:  return KeyCode::Left;
        case Qt::Key_Right: return KeyCode::Right;
    }
    return KeyCode::Unknown;
}

MouseButton translateMouseButton(Qt::MouseButton qtButton) {
    switch (qtButton) {
        case Qt::LeftButton:    return MouseButton::Left;
        case Qt::RightButton:   return MouseButton::Right;
        case Qt::MiddleButton:  return MouseButton::Middle;
        case Qt::XButton1:      return MouseButton::X1;
        case Qt::XButton2:      return MouseButton::X2;
        default:                return MouseButton::Unknown;
    }
}

KeyMod translateModifiers(Qt::KeyboardModifiers qtMods) {
    KeyMod result = KeyMod::None;
    if (qtMods & Qt::ShiftModifier)   result |= KeyMod::Shift;
    if (qtMods & Qt::ControlModifier) result |= KeyMod::Ctrl;
    if (qtMods & Qt::AltModifier)     result |= KeyMod::Alt;
    if (qtMods & Qt::MetaModifier)    result |= KeyMod::Super;
    return result;
}

} // namespace nuff::editor::input
