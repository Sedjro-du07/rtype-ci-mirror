/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlKeys
*/

// engine/plugins/platform_sfml/include/SfmlKeys.hpp
// ENG-31 — SFML 3 events and codes to engine RawInputEvent (reference 07, 08)

#pragma once

#include "engine/input/Keys.hpp"
#include "engine/input/RawInputEvent.hpp"
#include "engine/render/WindowDesc.hpp"

#include <SFML/Window.hpp>

#include <optional>

namespace engine {

// Physical keys (scancodes): bindings follow the key position, whatever the keyboard layout (ADR 0030).
Key toKey(sf::Keyboard::Scancode code);
std::optional<MouseButton> toMouseButton(sf::Mouse::Button button);
// Xbox-style layout as SFML reports it on Linux and Windows: 0 A, 1 B, 2 X, 3 Y, 4 LB, 5 RB, 6 Back, 7 Start.
std::optional<GamepadButton> toGamepadButton(unsigned int sfmlButton);
std::optional<GamepadAxis> toGamepadAxis(sf::Joystick::Axis axis);
// Window pixels to virtual-resolution pixels, through the same letterbox as Camera2D.
Vec2 toVirtual(sf::Vector2i windowPixel, const WindowDesc& desc);
// One SFML event to one engine event; window events (close, resize, focus) and unmapped input give nullopt.
std::optional<RawInputEvent> toRawInputEvent(const sf::Event& event, const WindowDesc& desc);

}
