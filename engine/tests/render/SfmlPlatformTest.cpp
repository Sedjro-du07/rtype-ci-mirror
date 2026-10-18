/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SfmlPlatformTest
*/

#include "SfmlKeys.hpp"
#include "SfmlRenderer.hpp"
#include "engine/render/RenderQueue.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <vector>

using namespace engine;

namespace {

WindowDesc wideWindow() {
    WindowDesc desc;
    desc.width = 1600;
    desc.height = 720;
    desc.virtualWidth = 1280;
    desc.virtualHeight = 720;
    return desc;
}

std::vector<std::byte> png(sf::Color color, unsigned size = 8) {
    const sf::Image image({size, size}, color);
    const std::optional<std::vector<std::uint8_t>> bytes = image.saveToMemory("png");
    REQUIRE(bytes.has_value());
    std::vector<std::byte> out(bytes->size());
    std::memcpy(out.data(), bytes->data(), bytes->size());
    return out;
}

bool canRender() {
#if defined(__linux__)
    if (std::getenv("DISPLAY") == nullptr && std::getenv("WAYLAND_DISPLAY") == nullptr) return false;
#endif
    static const bool available = [] {
        sf::RenderTexture probe;
        return probe.resize({1, 1});
    }();
    return available;
}

struct Target {
    sf::RenderTexture texture;
    SfmlRenderer renderer{texture, [this] { this->texture.display(); }};

    explicit Target(const WindowDesc& desc) {
        if (!this->texture.resize({desc.width, desc.height}))
            SKIP("this OpenGL context cannot create the render target");
        this->renderer.setWindowDesc(desc);
    }

    sf::Color pixel(unsigned x, unsigned y) const { return this->texture.getTexture().copyToImage().getPixel({x, y}); }
};

}

TEST_CASE("SFML scancodes map to engine keys by position", "[platform_sfml]") {
    using S = sf::Keyboard::Scancode;
    CHECK(toKey(S::A) == Key::A);
    CHECK(toKey(S::Z) == Key::Z);
    CHECK(toKey(S::Num0) == Key::Num0);
    CHECK(toKey(S::Num9) == Key::Num9);
    CHECK(toKey(S::F1) == Key::F1);
    CHECK(toKey(S::F12) == Key::F12);
    CHECK(toKey(S::Up) == Key::Up);
    CHECK(toKey(S::LControl) == Key::LCtrl);
    CHECK(toKey(S::NumpadEnter) == Key::Enter);
    CHECK(toKey(S::Insert) == Key::Unknown);
    CHECK(toMouseButton(sf::Mouse::Button::Right) == MouseButton::Right);
    CHECK_FALSE(toMouseButton(sf::Mouse::Button::Extra1).has_value());
    CHECK(toGamepadButton(0) == GamepadButton::South);
    CHECK(toGamepadButton(7) == GamepadButton::Start);
    CHECK_FALSE(toGamepadButton(20).has_value());
    CHECK(toGamepadAxis(sf::Joystick::Axis::V) == GamepadAxis::RightY);
}

TEST_CASE("SFML events become RawInputEvent; window events do not", "[platform_sfml]") {
    const WindowDesc desc = wideWindow();
    sf::Event::KeyPressed key{};
    key.code = sf::Keyboard::Key::Space;
    key.scancode = sf::Keyboard::Scancode::Space;
    const auto down = toRawInputEvent(sf::Event(key), desc);
    REQUIRE(down.has_value());
    CHECK(down->type == RawInputEvent::Type::KeyDown);
    CHECK(down->key == Key::Space);

    CHECK_FALSE(toRawInputEvent(sf::Event(sf::Event::Closed{}), desc).has_value());
    CHECK_FALSE(toRawInputEvent(sf::Event(sf::Event::TextEntered{U'\b'}), desc).has_value());
    const auto text = toRawInputEvent(sf::Event(sf::Event::TextEntered{U'é'}), desc);
    REQUIRE(text.has_value());
    CHECK(text->text == U'é');

    const auto click =
        toRawInputEvent(sf::Event(sf::Event::MouseButtonPressed{sf::Mouse::Button::Left, {160, 0}}), desc);
    REQUIRE(click.has_value());
    CHECK(click->position.x == 0.f);
    CHECK(click->position.y == 0.f);
    const auto move = toRawInputEvent(sf::Event(sf::Event::MouseMoved{{1440, 720}}), desc);
    REQUIRE(move.has_value());
    CHECK(move->position.x == 1280.f);
    CHECK(move->position.y == 720.f);

    const auto stick = toRawInputEvent(sf::Event(sf::Event::JoystickMoved{1, sf::Joystick::Axis::X, -50.f}), desc);
    REQUIRE(stick.has_value());
    CHECK(stick->device == 1);
    CHECK(stick->value == -0.5f);
    const auto trigger = toRawInputEvent(sf::Event(sf::Event::JoystickMoved{0, sf::Joystick::Axis::Z, -100.f}), desc);
    REQUIRE(trigger.has_value());
    CHECK(trigger->value == 0.f);
}

TEST_CASE("the renderer letterboxes the virtual resolution", "[platform_sfml][gl]") {
    if (!canRender()) SKIP("no OpenGL context: run under xvfb-run on Linux");
    Target target(wideWindow());
    const auto red = target.renderer.createTexture(png(sf::Color::Red), "red");
    REQUIRE(red.has_value());
    CHECK(target.renderer.textureSize(*red).x == 8.f);

    RenderQueue queue;
    queue.setClearColor(Color::blue());
    Camera2D camera;
    queue.setCamera(camera);
    queue.sprite(*red, {}, {0.f, 0.f}, 0, {4.f, 4.f});
    target.renderer.submit(queue.frame());

    CHECK(target.pixel(10, 360) == sf::Color::Black);
    CHECK(target.pixel(1590, 360) == sf::Color::Black);
    CHECK(target.pixel(162, 2) == sf::Color::Red);
    CHECK(target.pixel(800, 700) == sf::Color::Blue);
}

TEST_CASE("one draw call per texture and layer", "[platform_sfml][gl]") {
    if (!canRender()) SKIP("no OpenGL context: run under xvfb-run on Linux");
    Target target(wideWindow());
    const TextureHandle a = *target.renderer.createTexture(png(sf::Color::Red), "a");
    const TextureHandle b = *target.renderer.createTexture(png(sf::Color::Green), "b");

    RenderQueue queue;
    for (int i = 0; i < 100; ++i)
        queue.sprite(a, {}, {static_cast<float>(i), 0.f}, 0);
    target.renderer.submit(queue.frame());
    CHECK(target.renderer.stats().commands == 100);
    CHECK(target.renderer.stats().drawCalls == 1);

    queue.clear();
    for (int i = 0; i < 50; ++i) {
        queue.sprite(i % 2 == 0 ? a : b, {}, {static_cast<float>(i), 0.f}, 0);
        queue.sprite(i % 2 == 0 ? a : b, {}, {static_cast<float>(i), 20.f}, 1);
    }
    target.renderer.submit(queue.frame());
    CHECK(target.renderer.stats().drawCalls == 4);

    queue.clear();
    queue.rect({0, 0, 10, 10}, Color::white(), 0);
    queue.circle({50, 50}, 5, Color::white(), 0);
    queue.line({0, 0}, {100, 100}, Color::white(), 0);
    queue.rect({0, 0, 10, 10}, Color::white(), 0, false);
    target.renderer.submit(queue.frame());
    CHECK(target.renderer.stats().drawCalls == 1);
}

TEST_CASE("bad input never crashes the renderer", "[platform_sfml][gl]") {
    if (!canRender()) SKIP("no OpenGL context: run under xvfb-run on Linux");
    Target target(wideWindow());
    CHECK_FALSE(target.renderer.createTexture({}, "empty").has_value());
    const std::vector<std::byte> junk(64, std::byte{0x42});
    CHECK_FALSE(target.renderer.createTexture(junk, "junk").has_value());
    CHECK_FALSE(target.renderer.createFont(junk, "junk").has_value());

    const TextureHandle t = *target.renderer.createTexture(png(sf::Color::Red), "t");
    target.renderer.destroyTexture(t);
    target.renderer.destroyTexture(TextureHandle{99});
    RenderQueue queue;
    queue.sprite(t, {}, {0, 0}, 0);
    queue.text("hello", FontHandle{7}, 16, {0, 0}, 0);
    target.renderer.submit(queue.frame());
    CHECK(target.renderer.stats().drawCalls == 0);

    const TextureHandle reused = *target.renderer.createTexture(png(sf::Color::Green), "reused");
    CHECK(reused.id == t.id);
}

TEST_CASE("text is drawn from the queue's text arena", "[platform_sfml][gl]") {
    if (!canRender()) SKIP("no OpenGL context: run under xvfb-run on Linux");
    const std::filesystem::path path = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    if (!std::filesystem::exists(path)) SKIP("no test font");
    std::ifstream file(path, std::ios::binary);
    const std::vector<char> raw{std::istreambuf_iterator<char>(file), {}};
    std::vector<std::byte> bytes(raw.size());
    std::memcpy(bytes.data(), raw.data(), raw.size());

    Target target(wideWindow());
    const auto font = target.renderer.createFont(bytes, "DejaVuSans");
    REQUIRE(font.has_value());
    RenderQueue queue;
    queue.text("Pulsar ███", *font, 48, {0, 0}, 10, Color::white());
    target.renderer.submit(queue.frame());
    CHECK(target.renderer.stats().drawCalls == 1);
    const sf::Image image = target.texture.getTexture().copyToImage();
    int lit = 0;
    for (unsigned x = 160; x < 600; ++x)
        for (unsigned y = 0; y < 70; ++y)
            if (image.getPixel({x, y}).r > 128) ++lit;
    CHECK(lit > 200);
}

TEST_CASE("the d-pad hat becomes d-pad button presses and releases", "[platform_sfml]") {
    DPadState state;
    const auto moved = [](sf::Joystick::Axis axis, float position) {
        return sf::Event::JoystickMoved{0, axis, position};
    };
    auto events = dpadEvents(moved(sf::Joystick::Axis::PovX, 100.f), state);
    REQUIRE(events.size() == 1);
    CHECK(events[0].type == RawInputEvent::Type::GamepadDown);
    CHECK(events[0].button == GamepadButton::DPadRight);

    events = dpadEvents(moved(sf::Joystick::Axis::PovX, -100.f), state);
    REQUIRE(events.size() == 2);
    CHECK(events[0].type == RawInputEvent::Type::GamepadUp);
    CHECK(events[0].button == GamepadButton::DPadRight);
    CHECK(events[1].type == RawInputEvent::Type::GamepadDown);
    CHECK(events[1].button == GamepadButton::DPadLeft);

    CHECK(dpadEvents(moved(sf::Joystick::Axis::PovX, -90.f), state).empty());
    events = dpadEvents(moved(sf::Joystick::Axis::PovX, 0.f), state);
    REQUIRE(events.size() == 1);
    CHECK(events[0].type == RawInputEvent::Type::GamepadUp);

    events = dpadEvents(moved(sf::Joystick::Axis::PovY, 100.f), state);
    REQUIRE(events.size() == 1);
    CHECK(events[0].button == GamepadButton::DPadUp);
    CHECK(dpadEvents(moved(sf::Joystick::Axis::X, 100.f), state).empty());

    WindowDesc desc;
    const sf::Event stick = sf::Event::JoystickMoved{0, sf::Joystick::Axis::X, 50.f};
    const auto raw = toRawInputEvents(stick, desc, state);
    REQUIRE(raw.size() == 1);
    CHECK(raw[0].type == RawInputEvent::Type::GamepadAxis);
}
