/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** InputTest
*/

#include "engine/input/Action.hpp"
#include "engine/input/ActionState.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/input/Keys.hpp"
#include "engine/input/RawInputEvent.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

using namespace engine;
using Catch::Approx;

namespace {

constexpr ActionId Fire = action("Fire");
constexpr ActionId MoveUp = action("MoveUp");
constexpr double Tick = 1.0 / 60.0;

static_assert(action("Fire") == Fire);
static_assert(action("Fire") != MoveUp);

RawInputEvent key(Key k, bool down) {
    RawInputEvent e;
    e.type = down ? RawInputEvent::Type::KeyDown : RawInputEvent::Type::KeyUp;
    e.key = k;
    return e;
}

RawInputEvent stick(GamepadAxis axis, float value) {
    RawInputEvent e;
    e.type = RawInputEvent::Type::GamepadAxis;
    e.axis = axis;
    e.value = value;
    return e;
}

struct TempFile {
    std::filesystem::path path;

    explicit TempFile(const std::string& name, const std::string& content)
        : path(std::filesystem::temp_directory_path() / name) {
        std::ofstream(this->path, std::ios::binary) << content;
    }
    ~TempFile() { std::filesystem::remove(this->path); }
};

}

TEST_CASE("every device name parses back to its value", "[input]") {
    for (uint16_t i = 0; i < static_cast<uint16_t>(Key::Unknown); ++i) {
        const auto k = static_cast<Key>(i);
        CHECK(keyFromName(keyName(k)) == k);
    }
    CHECK_FALSE(keyFromName("Unknown"));
    CHECK_FALSE(keyFromName("Foo"));
    CHECK(keyName(Key::Space) == "Space");
    for (uint8_t i = 0; i < 3; ++i)
        CHECK(mouseButtonFromName(mouseButtonName(static_cast<MouseButton>(i))) == static_cast<MouseButton>(i));
    for (uint8_t i = 0; i < 12; ++i)
        CHECK(gamepadButtonFromName(gamepadButtonName(static_cast<GamepadButton>(i))) == static_cast<GamepadButton>(i));
    for (uint8_t i = 0; i < 6; ++i)
        CHECK(gamepadAxisFromName(gamepadAxisName(static_cast<GamepadAxis>(i))) == static_cast<GamepadAxis>(i));
    for (const char* text : {"W", "Mouse:Left", "Gamepad:South", "Axis:LeftY-", "Axis:RightX+"}) {
        const auto binding = parseBinding(text);
        REQUIRE(binding);
        CHECK(bindingName(*binding) == text);
    }
    CHECK_FALSE(parseBinding("Axis:LeftY"));
    CHECK_FALSE(parseBinding("Gamepad:Nope"));
}

TEST_CASE("a tap between two ticks is seen once", "[input]") {
    InputMap map;
    map.bind(Fire, Key::Space);
    map.feed(key(Key::Space, true));
    map.feed(key(Key::Space, false));
    map.beginTick(Tick);
    CHECK(map.state().pressed(Fire));
    CHECK(map.state().released(Fire));
    CHECK_FALSE(map.state().held(Fire));
    map.beginTick(Tick);
    CHECK_FALSE(map.state().pressed(Fire));
    CHECK_FALSE(map.state().released(Fire));
}

TEST_CASE("a held key is pressed once, held, then released", "[input]") {
    InputMap map;
    map.bind(MoveUp, Key::Up);
    map.bind(MoveUp, Key::W);
    map.feed(key(Key::Up, true));
    map.beginTick(Tick);
    CHECK(map.state().pressed(MoveUp));
    CHECK(map.state().held(MoveUp));
    CHECK(map.state().axis(MoveUp) == 1.f);
    map.feed(key(Key::W, true));
    map.feed(key(Key::Up, false));
    map.beginTick(Tick);
    CHECK_FALSE(map.state().pressed(MoveUp));
    CHECK(map.state().held(MoveUp));
    map.feed(key(Key::W, false));
    map.beginTick(Tick);
    CHECK(map.state().released(MoveUp));
    CHECK_FALSE(map.state().held(MoveUp));
}

TEST_CASE("hold to toggle and auto-repeat", "[input]") {
    InputMap map;
    map.bind(Fire, Key::Space);
    map.setOptions(InputOptions{{Fire}, {}, 8.f, 0.2f, false});
    map.feed(key(Key::Space, true));
    map.feed(key(Key::Space, false));
    map.beginTick(Tick);
    CHECK(map.state().pressed(Fire));
    for (int i = 0; i < 10; ++i)
        map.beginTick(Tick);
    CHECK(map.state().held(Fire));
    map.feed(key(Key::Space, true));
    map.feed(key(Key::Space, false));
    map.beginTick(Tick);
    CHECK(map.state().released(Fire));
    CHECK_FALSE(map.state().held(Fire));

    map.setOptions(InputOptions{{}, {Fire}, 8.f, 0.2f, false});
    map.feed(key(Key::Space, true));
    int presses = 0;
    for (int i = 0; i < 60; ++i) {
        map.beginTick(Tick);
        if (map.state().pressed(Fire)) ++presses;
    }
    CHECK(presses == 8);
}

TEST_CASE("gamepad axes respect the deadzone and threshold", "[input]") {
    InputMap map;
    map.bind(MoveUp, AxisBinding{GamepadAxis::LeftY, -1.f, 0.5f});
    map.feed(stick(GamepadAxis::LeftY, -0.1f));
    map.beginTick(Tick);
    CHECK(map.state().axis(MoveUp) == 0.f);
    map.feed(stick(GamepadAxis::LeftY, -0.3f));
    map.beginTick(Tick);
    CHECK(map.state().axis(MoveUp) == Approx(0.3f));
    CHECK_FALSE(map.state().held(MoveUp));
    map.feed(stick(GamepadAxis::LeftY, -0.8f));
    map.beginTick(Tick);
    CHECK(map.state().held(MoveUp));
    CHECK(map.state().pressed(MoveUp));
    map.feed(stick(GamepadAxis::LeftY, 0.8f));
    map.beginTick(Tick);
    CHECK(map.state().released(MoveUp));
}

TEST_CASE("input.yaml loads, the one-handed set replaces bindings, and saves back", "[input]") {
    TempFile file("rtype-input-test.yaml", "actions:\n"
                                           "  MoveUp: [Up, W, \"Gamepad:DPadUp\", \"Axis:LeftY-\"]\n"
                                           "  Fire: [Space, \"Gamepad:South\"]\n"
                                           "one_handed:\n"
                                           "  Fire: [J]\n"
                                           "options:\n"
                                           "  holdToToggle: [Fire]\n"
                                           "  autoRepeatHz: 10\n"
                                           "  deadzone: 0.25\n"
                                           "  oneHanded: true\n");
    InputMap map;
    REQUIRE(map.load(file.path));
    CHECK(map.bindings(MoveUp).size() == 4);
    REQUIRE(map.bindings(Fire).size() == 1);
    CHECK(map.bindings(Fire)[0] == Binding{Key::J});
    CHECK(map.options().autoRepeatHz == 10.f);
    CHECK(map.options().holdToToggle.size() == 1);
    CHECK(map.boundTo(Key::W) == MoveUp);
    CHECK_FALSE(map.boundTo(Key::Space));

    TempFile saved("rtype-input-saved.yaml", "");
    REQUIRE(map.save(saved.path));
    InputMap again;
    REQUIRE(again.load(saved.path));
    CHECK(again.bindings(MoveUp).size() == 4);
    CHECK(again.bindings(Fire)[0] == Binding{Key::J});
    CHECK(again.options().deadzone == Approx(0.25f));
    again.setOptions(InputOptions{});
    CHECK(again.bindings(Fire).size() == 2);
}

TEST_CASE("input.yaml errors name the line", "[input]") {
    TempFile typo("rtype-input-typo.yaml", "actions:\n  Fire: [Space, \"Gamepad:Southh\"]\n");
    InputMap map;
    const auto bad = map.load(typo.path);
    REQUIRE_FALSE(bad);
    CHECK(bad.error().line == 2);
    CHECK(bad.error().message.find("unknown binding") != std::string::npos);
    TempFile key("rtype-input-key.yaml", "action:\n  Fire: [Space]\n");
    CHECK_FALSE(map.load(key.path));
}

TEST_CASE("the next key pressed becomes a binding", "[input]") {
    InputMap map;
    map.bind(Fire, Key::Space);
    std::optional<Binding> captured;
    map.captureNextBinding([&](Binding b) { captured = b; });
    map.feed(key(Key::F, true));
    REQUIRE(captured);
    CHECK(*captured == Binding{Key::F});
    map.feed(key(Key::F, false));
    map.feed(key(Key::Space, true));
    map.beginTick(Tick);
    CHECK(map.state().pressed(Fire));
}

TEST_CASE("actions travel as a bit mask", "[input]") {
    InputMap map;
    map.bind(Fire, Key::Space);
    map.bind(MoveUp, Key::Up);
    map.feed(key(Key::Up, true));
    map.beginTick(Tick);
    const std::array<ActionId, 2> order{Fire, MoveUp};
    const uint64_t mask = map.state().toMask(order);
    CHECK(mask == 0b10);
    const ActionState remote = ActionState::fromMask(0b11, mask, order);
    CHECK(remote.held(Fire));
    CHECK(remote.pressed(Fire));
    CHECK(remote.held(MoveUp));
    CHECK_FALSE(remote.pressed(MoveUp));
    CHECK(ActionState::fromMask(0, 0b11, order).released(MoveUp));
}
