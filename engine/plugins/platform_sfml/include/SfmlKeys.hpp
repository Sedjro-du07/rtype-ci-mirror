/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlKeys
*/

#pragma once

#include "engine/input/Keys.hpp"
#include "engine/input/RawInputEvent.hpp"
#include "engine/render/WindowDesc.hpp"

#include <SFML/Window.hpp>

#include <array>
#include <map>
#include <optional>
#include <vector>

namespace engine {

Key toKey(sf::Keyboard::Scancode code);
std::optional<MouseButton> toMouseButton(sf::Mouse::Button button);
std::optional<GamepadButton> toGamepadButton(unsigned int sfmlButton);
std::optional<GamepadAxis> toGamepadAxis(sf::Joystick::Axis axis);
Vec2 toVirtual(sf::Vector2i windowPixel, const WindowDesc& desc);
std::optional<RawInputEvent> toRawInputEvent(const sf::Event& event, const WindowDesc& desc);

struct DPadState {
    std::map<unsigned int, std::array<int, 2>> directions;
};

std::vector<RawInputEvent> dpadEvents(const sf::Event::JoystickMoved& moved, DPadState& state);
std::vector<RawInputEvent> toRawInputEvents(const sf::Event& event, const WindowDesc& desc, DPadState& dpad);

}
