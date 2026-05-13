#pragma once

#include <cstdint>
#include <type_traits>

namespace nuff::engine::input {

enum class KeyCode : std::uint16_t {
    Unknown = 0,

    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    Space, Escape, Enter, Tab, Backspace,

    LeftShift, RightShift,
    LeftCtrl,  RightCtrl,
    LeftAlt,   RightAlt,
    LeftSuper, RightSuper,

    Up, Down, Left, Right,

    Count
};

enum class MouseButton : std::uint8_t {
    Unknown = 0,
    Left,
    Right,
    Middle,
    X1,
    X2,

    Count
};

enum class KeyAction : std::uint8_t {
    Press,
    Release,
    Repeat
};

enum class KeyMod : std::uint32_t {
    None  = 0,
    Shift = 1u << 0,
    Ctrl  = 1u << 1,
    Alt   = 1u << 2,
    Super = 1u << 3
};

constexpr KeyMod operator|(KeyMod a, KeyMod b) {
    using U = std::underlying_type_t<KeyMod>;
    return static_cast<KeyMod>(static_cast<U>(a) | static_cast<U>(b));
}

constexpr KeyMod operator&(KeyMod a, KeyMod b) {
    using U = std::underlying_type_t<KeyMod>;
    return static_cast<KeyMod>(static_cast<U>(a) & static_cast<U>(b));
}

constexpr KeyMod& operator|=(KeyMod& a, KeyMod b) { a = a | b; return a; }
constexpr KeyMod& operator&=(KeyMod& a, KeyMod b) { a = a & b; return a; }

constexpr bool any(KeyMod m) {
    return static_cast<std::underlying_type_t<KeyMod>>(m) != 0;
}

constexpr bool hasFlag(KeyMod value, KeyMod flag) {
    return any(value & flag);
}

} // namespace nuff::engine::input
