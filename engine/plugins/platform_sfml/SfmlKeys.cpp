/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlKeys
*/

#include "SfmlKeys.hpp"

#include <algorithm>

namespace engine {

Key toKey(sf::Keyboard::Scancode code) {
    using S = sf::Keyboard::Scancode;
    const int c = static_cast<int>(code);
    if (c >= static_cast<int>(S::A) && c <= static_cast<int>(S::Z))
        return static_cast<Key>(static_cast<int>(Key::A) + (c - static_cast<int>(S::A)));
    if (c >= static_cast<int>(S::F1) && c <= static_cast<int>(S::F12))
        return static_cast<Key>(static_cast<int>(Key::F1) + (c - static_cast<int>(S::F1)));
    switch (code) {
    case S::Num0:
        return Key::Num0;
    case S::Num1:
        return Key::Num1;
    case S::Num2:
        return Key::Num2;
    case S::Num3:
        return Key::Num3;
    case S::Num4:
        return Key::Num4;
    case S::Num5:
        return Key::Num5;
    case S::Num6:
        return Key::Num6;
    case S::Num7:
        return Key::Num7;
    case S::Num8:
        return Key::Num8;
    case S::Num9:
        return Key::Num9;
    case S::Up:
        return Key::Up;
    case S::Down:
        return Key::Down;
    case S::Left:
        return Key::Left;
    case S::Right:
        return Key::Right;
    case S::Space:
        return Key::Space;
    case S::Enter:
        return Key::Enter;
    case S::NumpadEnter:
        return Key::Enter;
    case S::Escape:
        return Key::Escape;
    case S::LShift:
        return Key::LShift;
    case S::RShift:
        return Key::RShift;
    case S::LControl:
        return Key::LCtrl;
    case S::RControl:
        return Key::RCtrl;
    case S::LAlt:
        return Key::LAlt;
    case S::RAlt:
        return Key::RAlt;
    case S::Tab:
        return Key::Tab;
    case S::Backspace:
        return Key::Backspace;
    default:
        return Key::Unknown;
    }
}

std::optional<MouseButton> toMouseButton(sf::Mouse::Button button) {
    switch (button) {
    case sf::Mouse::Button::Left:
        return MouseButton::Left;
    case sf::Mouse::Button::Right:
        return MouseButton::Right;
    case sf::Mouse::Button::Middle:
        return MouseButton::Middle;
    default:
        return std::nullopt;
    }
}

std::optional<GamepadButton> toGamepadButton(unsigned int sfmlButton) {
    switch (sfmlButton) {
    case 0:
        return GamepadButton::South;
    case 1:
        return GamepadButton::East;
    case 2:
        return GamepadButton::West;
    case 3:
        return GamepadButton::North;
    case 4:
        return GamepadButton::LeftShoulder;
    case 5:
        return GamepadButton::RightShoulder;
    case 6:
        return GamepadButton::Select;
    case 7:
        return GamepadButton::Start;
    default:
        return std::nullopt;
    }
}

std::optional<GamepadAxis> toGamepadAxis(sf::Joystick::Axis axis) {
    switch (axis) {
    case sf::Joystick::Axis::X:
        return GamepadAxis::LeftX;
    case sf::Joystick::Axis::Y:
        return GamepadAxis::LeftY;
    case sf::Joystick::Axis::U:
        return GamepadAxis::RightX;
    case sf::Joystick::Axis::V:
        return GamepadAxis::RightY;
    case sf::Joystick::Axis::Z:
        return GamepadAxis::LeftTrigger;
    case sf::Joystick::Axis::R:
        return GamepadAxis::RightTrigger;
    default:
        return std::nullopt;
    }
}

Vec2 toVirtual(sf::Vector2i windowPixel, const WindowDesc& desc) {
    const float vw = static_cast<float>(std::max(1u, desc.virtualWidth));
    const float vh = static_cast<float>(std::max(1u, desc.virtualHeight));
    const float w = static_cast<float>(std::max(1u, desc.width));
    const float h = static_cast<float>(std::max(1u, desc.height));
    const float scale = std::min(w / vw, h / vh);
    const Vec2 offset{(w - vw * scale) / 2.f, (h - vh * scale) / 2.f};
    return Vec2{(static_cast<float>(windowPixel.x) - offset.x) / scale,
                (static_cast<float>(windowPixel.y) - offset.y) / scale};
}

std::optional<RawInputEvent> toRawInputEvent(const sf::Event& event, const WindowDesc& desc) {
    using T = RawInputEvent::Type;
    RawInputEvent out;
    if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
        out.type = T::KeyDown;
        out.key = toKey(e->scancode);
        if (out.key == Key::Unknown) return std::nullopt;
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::KeyReleased>()) {
        out.type = T::KeyUp;
        out.key = toKey(e->scancode);
        if (out.key == Key::Unknown) return std::nullopt;
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::TextEntered>()) {
        if (e->unicode < 0x20 || e->unicode == 0x7F) return std::nullopt;
        out.type = T::TextEntered;
        out.text = e->unicode;
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        const auto button = toMouseButton(e->button);
        if (!button) return std::nullopt;
        out.type = T::MouseDown;
        out.mouse = *button;
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::MouseButtonReleased>()) {
        const auto button = toMouseButton(e->button);
        if (!button) return std::nullopt;
        out.type = T::MouseUp;
        out.mouse = *button;
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
        out.type = T::MouseMove;
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (e->wheel != sf::Mouse::Wheel::Vertical) return std::nullopt;
        out.type = T::MouseWheel;
        out.value = e->delta;
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::JoystickButtonPressed>()) {
        const auto button = toGamepadButton(e->button);
        if (!button) return std::nullopt;
        out.type = T::GamepadDown;
        out.device = static_cast<uint8_t>(e->joystickId);
        out.button = *button;
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::JoystickButtonReleased>()) {
        const auto button = toGamepadButton(e->button);
        if (!button) return std::nullopt;
        out.type = T::GamepadUp;
        out.device = static_cast<uint8_t>(e->joystickId);
        out.button = *button;
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::JoystickMoved>()) {
        const auto axis = toGamepadAxis(e->axis);
        if (!axis) return std::nullopt;
        out.type = T::GamepadAxis;
        out.device = static_cast<uint8_t>(e->joystickId);
        out.axis = *axis;
        const bool trigger = *axis == GamepadAxis::LeftTrigger || *axis == GamepadAxis::RightTrigger;
        out.value = trigger ? (e->position + 100.f) / 200.f : e->position / 100.f;
        out.value = std::clamp(out.value, trigger ? 0.f : -1.f, 1.f);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::JoystickConnected>()) {
        out.type = T::GamepadConnected;
        out.device = static_cast<uint8_t>(e->joystickId);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::JoystickDisconnected>()) {
        out.type = T::GamepadDisconnected;
        out.device = static_cast<uint8_t>(e->joystickId);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::TouchBegan>()) {
        out.type = T::TouchDown;
        out.finger = static_cast<uint8_t>(e->finger);
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::TouchMoved>()) {
        out.type = T::TouchMove;
        out.finger = static_cast<uint8_t>(e->finger);
        out.position = toVirtual(e->position, desc);
        return out;
    }
    if (const auto* e = event.getIf<sf::Event::TouchEnded>()) {
        out.type = T::TouchUp;
        out.finger = static_cast<uint8_t>(e->finger);
        out.position = toVirtual(e->position, desc);
        return out;
    }
    return std::nullopt;
}

namespace {

constexpr float DPadThreshold = 50.f;

RawInputEvent dpadEvent(unsigned int joystick, bool horizontal, int direction, bool down) {
    RawInputEvent out;
    out.type = down ? RawInputEvent::Type::GamepadDown : RawInputEvent::Type::GamepadUp;
    out.device = static_cast<uint8_t>(joystick);
    if (horizontal)
        out.button = direction > 0 ? GamepadButton::DPadRight : GamepadButton::DPadLeft;
    else
        out.button = direction > 0 ? GamepadButton::DPadUp : GamepadButton::DPadDown;
    return out;
}

int directionOf(float position) {
    if (position > DPadThreshold) return 1;
    if (position < -DPadThreshold) return -1;
    return 0;
}

}

std::vector<RawInputEvent> dpadEvents(const sf::Event::JoystickMoved& moved, DPadState& state) {
    const bool horizontal = moved.axis == sf::Joystick::Axis::PovX;
    if (!horizontal && moved.axis != sf::Joystick::Axis::PovY) return {};
    const int direction = directionOf(moved.position);
    int& previous = state.directions[moved.joystickId][horizontal ? 0 : 1];
    if (direction == previous) return {};
    std::vector<RawInputEvent> out;
    if (previous != 0) out.push_back(dpadEvent(moved.joystickId, horizontal, previous, false));
    if (direction != 0) out.push_back(dpadEvent(moved.joystickId, horizontal, direction, true));
    previous = direction;
    return out;
}

std::vector<RawInputEvent> toRawInputEvents(const sf::Event& event, const WindowDesc& desc, DPadState& dpad) {
    if (const auto* moved = event.getIf<sf::Event::JoystickMoved>()) {
        auto pressed = dpadEvents(*moved, dpad);
        if (!pressed.empty()) return pressed;
    }
    if (const auto* gone = event.getIf<sf::Event::JoystickDisconnected>()) dpad.directions.erase(gone->joystickId);
    if (auto single = toRawInputEvent(event, desc)) return {*single};
    return {};
}

}
