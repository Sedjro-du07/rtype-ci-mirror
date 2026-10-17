/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** Keys
*/

#include "engine/input/Keys.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

namespace engine {

namespace {

constexpr std::array<std::string_view, 64> KeyNames{
    "A",      "B",     "C",     "D",    "E",    "F",    "G",         "H",     "I",      "J",      "K",
    "L",      "M",     "N",     "O",    "P",    "Q",    "R",         "S",     "T",      "U",      "V",
    "W",      "X",     "Y",     "Z",    "Num0", "Num1", "Num2",      "Num3",  "Num4",   "Num5",   "Num6",
    "Num7",   "Num8",  "Num9",  "Up",   "Down", "Left", "Right",     "Space", "Enter",  "Escape", "LShift",
    "RShift", "LCtrl", "RCtrl", "LAlt", "RAlt", "Tab",  "Backspace", "F1",    "F2",     "F3",     "F4",
    "F5",     "F6",    "F7",    "F8",   "F9",   "F10",  "F11",       "F12",   "Unknown"};
constexpr std::array<std::string_view, 3> MouseNames{"Left", "Right", "Middle"};
constexpr std::array<std::string_view, 12> PadNames{"South",        "East",          "West",     "North",
                                                    "LeftShoulder", "RightShoulder", "Start",    "Select",
                                                    "DPadUp",       "DPadDown",      "DPadLeft", "DPadRight"};
constexpr std::array<std::string_view, 6> AxisNames{"LeftX",  "LeftY",       "RightX",
                                                    "RightY", "LeftTrigger", "RightTrigger"};

template <class E, size_t N>
std::string_view nameIn(const std::array<std::string_view, N>& names, E value) {
    const auto index = static_cast<size_t>(value);
    return index < N ? names[index] : std::string_view{};
}

template <class E, size_t N>
std::optional<E> findIn(const std::array<std::string_view, N>& names, std::string_view name) {
    const auto it = std::ranges::find(names, name);
    if (it == names.end()) return std::nullopt;
    return static_cast<E>(it - names.begin());
}

}

std::string_view keyName(Key key) {
    return nameIn(KeyNames, key);
}

std::optional<Key> keyFromName(std::string_view name) {
    if (name == "Unknown") return std::nullopt;
    return findIn<Key>(KeyNames, name);
}

std::string_view mouseButtonName(MouseButton button) {
    return nameIn(MouseNames, button);
}

std::optional<MouseButton> mouseButtonFromName(std::string_view name) {
    return findIn<MouseButton>(MouseNames, name);
}

std::string_view gamepadButtonName(GamepadButton button) {
    return nameIn(PadNames, button);
}

std::optional<GamepadButton> gamepadButtonFromName(std::string_view name) {
    return findIn<GamepadButton>(PadNames, name);
}

std::string_view gamepadAxisName(GamepadAxis axis) {
    return nameIn(AxisNames, axis);
}

std::optional<GamepadAxis> gamepadAxisFromName(std::string_view name) {
    return findIn<GamepadAxis>(AxisNames, name);
}

}
