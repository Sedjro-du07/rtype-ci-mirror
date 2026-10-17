/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RawInputEvent
*/

#pragma once

#include "engine/core/math/Vec2.hpp"
#include "engine/input/Keys.hpp"

#include <cstdint>

namespace engine {

struct RawInputEvent {
    enum class Type : uint8_t {
        KeyDown,
        KeyUp,
        MouseDown,
        MouseUp,
        MouseMove,
        MouseWheel,
        GamepadDown,
        GamepadUp,
        GamepadAxis,
        GamepadConnected,
        GamepadDisconnected,
        TextEntered,
        TouchDown,
        TouchUp,
        TouchMove
    };

    Type type = Type::KeyDown;
    uint8_t device = 0;
    Key key = Key::Unknown;
    MouseButton mouse{};
    GamepadButton button{};
    engine::GamepadAxis axis{};
    float value = 0.f;
    Vec2 position;
    char32_t text = 0;
    uint8_t finger = 0;
};

}
