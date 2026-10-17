/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RenderTest
*/

#include "engine/render/Camera2D.hpp"
#include "engine/render/Handles.hpp"
#include "engine/render/IPlatform.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/render/WindowDesc.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace engine;

static_assert(!TextureHandle{}.valid());
static_assert(TextureHandle{3}.valid());
static_assert(TextureHandle{3} == TextureHandle{3});
static_assert(!std::is_same_v<TextureHandle, FontHandle>);

namespace {

void checkNear(Vec2 a, Vec2 b) {
    CHECK(a.x == Catch::Approx(b.x).margin(1e-3));
    CHECK(a.y == Catch::Approx(b.y).margin(1e-3));
}

WindowDesc window(uint32_t width, uint32_t height) {
    WindowDesc desc;
    desc.width = width;
    desc.height = height;
    return desc;
}

}

TEST_CASE("the camera centre is drawn at the middle of the window", "[render][camera]") {
    const Camera2D camera;
    checkNear(camera.worldToScreen(camera.center, window(1280, 720)), {640, 360});
    checkNear(camera.worldToScreen(camera.center, window(1920, 1080)), {960, 540});
}

TEST_CASE("letterboxing keeps the virtual resolution inside the window", "[render][camera]") {
    const Camera2D camera;
    checkNear(camera.worldToScreen({0, 0}, window(1280, 1000)), {0, 140});
    checkNear(camera.worldToScreen({1280, 720}, window(1280, 1000)), {1280, 860});
    checkNear(camera.worldToScreen({0, 0}, window(2000, 720)), {360, 0});
    checkNear(camera.worldToScreen({0, 0}, window(1920, 1080)), {0, 0});
    checkNear(camera.worldToScreen({1280, 720}, window(1920, 1080)), {1920, 1080});
}

TEST_CASE("screen to world and back is the identity", "[render][camera]") {
    Camera2D camera;
    camera.center = {300, -50};
    camera.zoom = 2.5f;
    camera.rotation = 30.f;
    for (const WindowDesc& desc : {window(1280, 720), window(1920, 1080), window(800, 1200), window(3000, 600)}) {
        for (const Vec2 world : {Vec2{0, 0}, Vec2{300, -50}, Vec2{-120, 470}, Vec2{1000, 1000}}) {
            checkNear(camera.screenToWorld(camera.worldToScreen(world, desc), desc), world);
        }
        checkNear(camera.worldToScreen(camera.screenToWorld({17, 42}, desc), desc), {17, 42});
    }
}

TEST_CASE("zoom brings world points closer to the centre on screen", "[render][camera]") {
    Camera2D camera;
    const WindowDesc desc = window(1280, 720);
    checkNear(camera.worldToScreen({740, 360}, desc), {740, 360});
    camera.zoom = 2.f;
    checkNear(camera.worldToScreen({740, 360}, desc), {840, 360});
}

TEST_CASE("the queue records every kind of command", "[render][queue]") {
    RenderQueue queue;
    queue.sprite(TextureHandle{1}, Rect{0, 0, 32, 16}, {100, 200}, 10, {2, 2}, 45.f, Color::red(), {16, 8});
    queue.text("SCORE", FontHandle{2}, 24, {10, 10}, 100);
    queue.text("P1", FontHandle{2}, 16, {10, 40}, 100, Color::blue(), false);
    queue.rect(Rect{1, 2, 3, 4}, Color::green(), 5);
    queue.rect(Rect{1, 2, 3, 4}, Color::green(), 5, false, true);
    queue.line({0, 0}, {10, 10}, Color::white(), 1);
    queue.circle({50, 50}, 7.f, Color::white(), 2);
    REQUIRE(queue.size() == 7);

    const RenderFrame frame = queue.frame();
    const auto& cmds = frame.commands;
    CHECK(cmds[0].kind == DrawCmd::Kind::Sprite);
    CHECK(cmds[0].texture == TextureHandle{1});
    CHECK(cmds[0].source == Rect{0, 0, 32, 16});
    CHECK(cmds[0].scale == Vec2{2, 2});
    CHECK(cmds[0].origin == Vec2{16, 8});
    CHECK(cmds[0].layer == 10);
    CHECK(frame.textArena.substr(cmds[1].textOffset, cmds[1].textLength) == "SCORE");
    CHECK(frame.textArena.substr(cmds[2].textOffset, cmds[2].textLength) == "P1");
    CHECK(cmds[1].screenSpace);
    CHECK_FALSE(cmds[2].screenSpace);
    CHECK(cmds[3].kind == DrawCmd::Kind::Rect);
    CHECK(cmds[3].size == Vec2{3, 4});
    CHECK(cmds[4].kind == DrawCmd::Kind::RectOutline);
    CHECK(cmds[4].screenSpace);
    CHECK(cmds[5].end == Vec2{10, 10});
    CHECK(cmds[6].size.x == 7.f);
}

TEST_CASE("the queue keeps its camera, clear colour and capacity", "[render][queue]") {
    RenderQueue queue;
    Camera2D camera;
    camera.center = {1, 2};
    queue.setCamera(camera);
    queue.setClearColor(Color::blue());
    for (int i = 0; i < 100; ++i)
        queue.text("a long enough text to need its own storage", FontHandle{1}, 16, {}, 0);
    const DrawCmd* commands = queue.frame().commands.data();
    const char* text = queue.frame().textArena.data();

    queue.clear();
    CHECK(queue.size() == 0);
    CHECK(queue.frame().textArena.empty());
    for (int i = 0; i < 100; ++i)
        queue.text("a long enough text to need its own storage", FontHandle{1}, 16, {}, 0);
    CHECK(queue.frame().commands.data() == commands);
    CHECK(queue.frame().textArena.data() == text);
    CHECK(queue.camera().center == Vec2{1, 2});
    CHECK(queue.frame().clearColor == Color::blue());
}

namespace {

class FakeRenderer final : public IRenderer {
public:
    Result<TextureHandle> createTexture(std::span<const std::byte>, std::string_view) override {
        return TextureHandle{1};
    }
    void destroyTexture(TextureHandle) override {}
    Vec2 textureSize(TextureHandle) const override { return {32, 32}; }
    Result<FontHandle> createFont(std::span<const std::byte>, std::string_view) override { return FontHandle{1}; }
    void destroyFont(FontHandle) override {}
    void submit(const RenderFrame& frame) override { this->submitted = frame.commands.size(); }
    bool supports(Feature) const override { return false; }
    void setColorFilter(ColorFilter) override {}
    RenderStats stats() const override { return {}; }

    size_t submitted = 0;
};

}

TEST_CASE("a renderer receives the frame of a queue", "[render]") {
    FakeRenderer renderer;
    RenderQueue queue;
    queue.circle({0, 0}, 1, Color::white(), 0);
    IRenderer& api = renderer;
    api.submit(queue.frame());
    CHECK(renderer.submitted == 1);
    CHECK(api.createTexture({}, "ship")->valid());
    static_assert(std::is_abstract_v<IPlatform>);
}
