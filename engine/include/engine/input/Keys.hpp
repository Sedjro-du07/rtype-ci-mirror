/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Keys
*/

#pragma once

#include "engine/Export.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace engine {

enum class Key : uint16_t {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Escape,
    LShift,
    RShift,
    LCtrl,
    RCtrl,
    LAlt,
    RAlt,
    Tab,
    Backspace,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Unknown
};

enum class MouseButton : uint8_t { Left, Right, Middle };

enum class GamepadButton : uint8_t {
    South,
    East,
    West,
    North,
    LeftShoulder,
    RightShoulder,
    Start,
    Select,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight
};

enum class GamepadAxis : uint8_t { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };

ENGINE_API std::string_view keyName(Key key);
ENGINE_API std::optional<Key> keyFromName(std::string_view name);
ENGINE_API std::string_view mouseButtonName(MouseButton button);
ENGINE_API std::optional<MouseButton> mouseButtonFromName(std::string_view name);
ENGINE_API std::string_view gamepadButtonName(GamepadButton button);
ENGINE_API std::optional<GamepadButton> gamepadButtonFromName(std::string_view name);
ENGINE_API std::string_view gamepadAxisName(GamepadAxis axis);
ENGINE_API std::optional<GamepadAxis> gamepadAxisFromName(std::string_view name);

}
