/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ViewTest
*/

#include "TestWorld.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace engine;
using namespace world;

TEST_CASE("a view walks the entities having every component", "[ecs][view]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    std::vector<Entity> moving;
    for (int i = 0; i < 10; ++i) {
        const Entity e = registry.create();
        registry.emplace<Position>(e, static_cast<float>(i), 0.f);
        if (i % 2 == 0) {
            registry.emplace<Velocity>(e, 1.f, 2.f);
            moving.push_back(e);
        }
    }

    auto view = registry.view<Position, Velocity>();
    CHECK(view.sizeHint() == 5);
    std::vector<Entity> seen;
    for (auto [e, position, velocity] : view) {
        position.x += velocity.x;
        seen.push_back(e);
    }
    CHECK(seen.size() == moving.size());
    for (Entity e : moving)
        CHECK(registry.get<Position>(e).x == static_cast<float>(e.index) + 1.f);
}

TEST_CASE("exclude skips entities having a component", "[ecs][view]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity a = registry.create();
    const Entity b = registry.create();
    registry.emplace<Position>(a);
    registry.emplace<Position>(b);
    registry.emplace<Frozen>(b);

    std::vector<Entity> seen;
    registry.view<Position>(exclude<Frozen>).each([&](Entity e, Position&) { seen.push_back(e); });
    CHECK(seen == std::vector<Entity>{a});

    std::vector<Entity> all;
    registry.view<Position>(exclude<Velocity>).each([&](Entity e, Position&) { all.push_back(e); });
    CHECK(all.size() == 2);
}

TEST_CASE("empty views and front", "[ecs][view]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    CHECK(registry.view<Health>().front().isNull());
    CHECK(registry.view<Health>().begin() == registry.view<Health>().end());

    const Entity e = registry.create();
    registry.emplace<Health>(e);
    CHECK(registry.view<Health>().front() == e);
    CHECK(registry.view<Health, Velocity>().front().isNull());
}

TEST_CASE("a view drives on its smallest storage", "[ecs][view]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    for (int i = 0; i < 100; ++i)
        registry.emplace<Position>(registry.create());
    const Entity rare = registry.create();
    registry.emplace<Position>(rare);
    registry.emplace<Health>(rare);
    CHECK(registry.view<Position, Health>().sizeHint() == 1);
    CHECK(registry.view<Position, Health>().front() == rare);
}
