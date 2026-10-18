/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineSystemsTest
*/

#include "systems/EngineSystems.hpp"

#include "engine/components/Animation.hpp"
#include "engine/components/Collider.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/components/Lifetime.hpp"
#include "engine/components/Parallax.hpp"
#include "engine/components/Position.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Text.hpp"
#include "engine/components/Velocity.hpp"
#include "engine/ecs/Commands.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/render/RenderQueue.hpp"
#include "engine/scheduler/Scheduler.hpp"
#include "systems/RegisterEngineTypes.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <string>
#include <utility>
#include <vector>

namespace engine {

class Context {};

}

using namespace engine;
using Catch::Approx;

namespace {

TypeRegistry engineTypes() {
    TypeRegistry types;
    registerEngineTypes(types);
    return types;
}

struct World {
    TypeRegistry types = engineTypes();
    Registry registry{types};
    Commands commands{registry};
    EventBus events;
    std::vector<Collision> hits;
    Subscription sub = events.subscribe<Collision>([this](const Collision& c) { this->hits.push_back(c); });

    Entity at(float x, float y) {
        const Entity e = this->registry.create();
        this->registry.emplace<Position>(e, x, y);
        return e;
    }

    Entity box(float x, float y, float w, float h, uint32_t layer = 1, uint32_t mask = 0xFFFFFFFFu) {
        const Entity e = this->at(x, y);
        this->registry.emplace<Collider>(e, Collider{{w, h}, {}, layer, mask, false});
        return e;
    }

    std::vector<Collision> collide(float cell = 64) {
        this->hits.clear();
        collision(this->registry, this->events, cell);
        this->events.dispatch();
        return this->hits;
    }
};

}

TEST_CASE("movement uses the fixed delta and keeps the previous position", "[systems]") {
    World w;
    const Entity ship = w.at(10, 20);
    w.registry.emplace<Velocity>(ship, 60.f, -30.f);
    const Entity rock = w.at(5, 5);

    previousPosition(w.registry);
    movement(w.registry, 0.5);
    CHECK(w.registry.get<Position>(ship).x == Approx(40));
    CHECK(w.registry.get<Position>(ship).y == Approx(5));
    CHECK(w.registry.get<PreviousPosition>(ship).x == Approx(10));
    CHECK(w.registry.get<Position>(rock).x == Approx(5));

    previousPosition(w.registry);
    CHECK(w.registry.get<PreviousPosition>(ship).x == Approx(40));
}

TEST_CASE("children follow their parents, nested", "[systems]") {
    World w;
    const Entity ship = w.at(100, 50);
    const Entity force = w.at(0, 0);
    const Entity orb = w.at(0, 0);
    w.registry.setParent(force, ship, {26, 0});
    w.registry.setParent(orb, force, {0, -10});
    w.registry.emplace<Velocity>(ship, 10.f, 0.f);

    movement(w.registry, 1.0);
    hierarchy(w.registry);
    CHECK(w.registry.get<Position>(force).x == Approx(136));
    CHECK(w.registry.get<Position>(force).y == Approx(50));
    CHECK(w.registry.get<Position>(orb).x == Approx(136));
    CHECK(w.registry.get<Position>(orb).y == Approx(40));

    w.registry.setParent(force, Entity::null());
    movement(w.registry, 1.0);
    hierarchy(w.registry);
    CHECK(w.registry.get<Position>(force).x == Approx(136));
    CHECK(w.registry.get<Position>(orb).x == Approx(136));
}

TEST_CASE("parallax layers wrap horizontally", "[systems]") {
    World w;
    const Entity stars = w.at(-1300, 0);
    w.registry.emplace<Parallax>(stars, 0.2f, 1280.f);
    const Entity still = w.at(-1300, 0);
    w.registry.emplace<Parallax>(still, 0.2f, 0.f);
    parallax(w.registry);
    CHECK(w.registry.get<Position>(stars).x == Approx(-20));
    CHECK(w.registry.get<Position>(still).x == Approx(-1300));
    w.registry.get<Position>(stars).x = 30;
    parallax(w.registry);
    CHECK(w.registry.get<Position>(stars).x == Approx(-1250));
}

TEST_CASE("lifetime destroys through Commands once", "[systems]") {
    World w;
    const Entity shot = w.at(0, 0);
    w.registry.emplace<Lifetime>(shot, 0.25f);
    lifetime(w.registry, w.commands, 0.2);
    w.commands.apply();
    CHECK(w.registry.alive(shot));
    lifetime(w.registry, w.commands, 0.2);
    CHECK(w.registry.alive(shot));
    CHECK_FALSE(w.commands.empty());
    w.commands.apply();
    CHECK_FALSE(w.registry.alive(shot));
}

TEST_CASE("collisions follow the layer and mask rule both ways", "[systems]") {
    World w;
    const Entity player = w.box(0, 0, 10, 10, 0b001, 0b110);
    const Entity enemy = w.box(5, 5, 10, 10, 0b010, 0b101);
    const Entity friendShot = w.box(5, 0, 10, 10, 0b100, 0b010);
    w.box(100, 100, 10, 10);
    const auto hits = w.collide();
    REQUIRE(hits.size() == 2);
    CHECK(hits[0].a == player);
    CHECK(hits[0].b == enemy);
    CHECK(hits[1].a == enemy);
    CHECK(hits[1].b == friendShot);
    CHECK_FALSE(layersMatch(Collider{{}, {}, 0b001, 0b010, false}, Collider{{}, {}, 0b010, 0b100, false}));
}

TEST_CASE("touching boxes do not collide and large boxes are reported once", "[systems]") {
    World w;
    w.box(0, 0, 10, 10);
    w.box(10, 0, 10, 10);
    CHECK(w.collide().empty());

    World big;
    const Entity wall = big.box(0, 0, 500, 500);
    const Entity ship = big.box(250, 250, 10, 10);
    const auto hits = big.collide(64);
    REQUIRE(hits.size() == 1);
    CHECK(hits[0].a == wall);
    CHECK(hits[0].b == ship);
    CHECK(worldBox(Position{10, 20}, Collider{{4, 6}, {1, 2}, 1, 1, false}).x == Approx(11));
}

TEST_CASE("1000 colliders stay within budget", "[systems][perf]") {
    World w;
    for (int i = 0; i < 1000; ++i)
        w.box(static_cast<float>((i * 37) % 1280), static_cast<float>((i * 53) % 720), 16, 16);
    const auto start = std::chrono::steady_clock::now();
    const auto hits = w.collide(64);
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    CHECK(ms < 50);
    CHECK_FALSE(hits.empty());
}

TEST_CASE("animation advances frames, loops or stops", "[systems]") {
    World w;
    const Entity e = w.at(0, 0);
    w.registry.emplace<Sprite>(e);
    w.registry.emplace<Animation>(e, Animation{{{0, 0, 8, 8}, {8, 0, 8, 8}, {16, 0, 8, 8}}, 10.f, true, 0.f, 0});
    animation(w.registry, 0.05);
    CHECK(w.registry.get<Sprite>(e).frame.x == Approx(0));
    animation(w.registry, 0.1);
    CHECK(w.registry.get<Sprite>(e).frame.x == Approx(8));
    animation(w.registry, 0.2);
    CHECK(w.registry.get<Sprite>(e).frame.x == Approx(0));

    w.registry.get<Animation>(e).loop = false;
    animation(w.registry, 1.0);
    CHECK(w.registry.get<Sprite>(e).frame.x == Approx(16));
    CHECK(w.registry.get<Animation>(e).current == 2);
}

TEST_CASE("render systems fill the queue, sprites interpolated", "[systems]") {
    World w;
    RenderQueue queue;
    const Entity ship = w.at(0, 0);
    w.registry.emplace<Sprite>(ship, Sprite{{3}, {0, 0, 33, 17}, 30});
    previousPosition(w.registry);
    w.registry.get<Position>(ship).x = 100;
    const Entity hidden = w.at(0, 0);
    w.registry.emplace<Sprite>(hidden).visible = false;
    const Entity label = w.at(5, 6);
    w.registry.emplace<Text>(label, Text{"score", {}, 20});
    w.registry.emplace<Collider>(ship, Collider{{33, 17}, {}, 1, 1, false});

    renderSprites(w.registry, queue, 0.25);
    renderText(w.registry, queue);
    renderDebug(w.registry, queue);
    const RenderFrame frame = queue.frame();
    REQUIRE(frame.commands.size() == 3);
    CHECK(frame.commands[0].kind == DrawCmd::Kind::Sprite);
    CHECK(frame.commands[0].position.x == Approx(25));
    CHECK(frame.commands[0].layer == 30);
    CHECK(frame.commands[0].texture == TextureHandle{3});
    CHECK(frame.commands[1].kind == DrawCmd::Kind::Text);
    CHECK(frame.commands[2].kind == DrawCmd::Kind::RectOutline);
}

TEST_CASE("engine systems are registered with their names, stages and roles", "[systems]") {
    LogConfig config;
    config.file.clear();
    config.level = LogLevel::Error;
    Logger log(config);
    World w;
    Time time;
    time.fixedDelta = 0.5;
    RenderQueue queue;
    const TargetsOf targets = [&](Context&) {
        return SystemTargets{w.registry, w.commands, w.events, time, &queue, 64, false};
    };

    Scheduler client(Role::Client, log);
    registerEngineSystems(client, targets);
    REQUIRE(client.build());
    std::vector<std::string> names;
    for (const SystemStats& stats : client.stats())
        names.emplace_back(stats.name);
    CHECK(names == std::vector<std::string>{"engine.net.apply_snapshot", "engine.previous_position", "engine.movement",
                                            "engine.hierarchy", "engine.parallax", "engine.lifetime",
                                            "engine.animation", "engine.collision", "engine.net.send_input",
                                            "engine.net.interpolate", "engine.render.sprites", "engine.render.text",
                                            "engine.render.debug"});

    Scheduler server(Role::Server, log);
    registerEngineSystems(server, targets);
    REQUIRE(server.build());
    CHECK(server.stats().size() == 8);
    CHECK_FALSE(server.enabled("engine.render.sprites"));
    CHECK(server.enabled("engine.net.snapshot"));
    CHECK_FALSE(server.enabled("engine.net.interpolate"));

    const Entity ship = w.at(0, 0);
    w.registry.emplace<Velocity>(ship, 10.f, 0.f);
    Context ctx;
    server.run(Stage::FixedUpdate, ctx);
    CHECK(w.registry.get<Position>(ship).x == Approx(5));
    client.run(Stage::Render, ctx);
    CHECK(queue.size() == 0);
}
