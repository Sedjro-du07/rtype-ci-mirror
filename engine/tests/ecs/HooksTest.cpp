/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** HooksTest
*/

#include "TestWorld.hpp"

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace engine;
using namespace world;

TEST_CASE("each hook fires once per change", "[ecs][hooks]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    int created = 0, replaced = 0, destroyed = 0;
    ComponentHooks& hooks = registry.hooks<Health>();
    const Subscription a = hooks.onCreate.connect([&](Registry&, Entity) { ++created; });
    const Subscription b = hooks.onReplace.connect([&](Registry&, Entity) { ++replaced; });
    const Subscription c = hooks.onDestroy.connect([&](Registry&, Entity) { ++destroyed; });

    const Entity e = registry.create();
    registry.emplace<Health>(e);
    registry.replace<Health>(e, Health{5});
    registry.remove<Health>(e);
    CHECK(created == 1);
    CHECK(replaced == 1);
    CHECK(destroyed == 1);

    const Health source{7};
    registry.emplaceRaw(e, types.id<Health>(), &source);
    registry.removeRaw(e, types.id<Health>());
    CHECK(created == 2);
    CHECK(destroyed == 2);
}

TEST_CASE("onDestroy still sees the component, also through destroy and clear", "[ecs][hooks]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    std::vector<int32_t> lastHp;
    const Subscription sub = registry.hooks<Health>().onDestroy.connect(
        [&](Registry& world, Entity e) { lastHp.push_back(world.get<Health>(e).hp); });

    const Entity a = registry.create();
    registry.emplace<Health>(a, 30);
    registry.destroy(a);

    const Entity b = registry.create();
    registry.emplace<Health>(b, 40);
    registry.clear();

    CHECK(lastHp == std::vector<int32_t>{30, 40});
}

TEST_CASE("hooks by ComponentId and an unsubscribed listener", "[ecs][hooks]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    int created = 0;
    Subscription sub = registry.hooks(types.id<Position>()).onCreate.connect([&](Registry&, Entity) { ++created; });
    registry.emplace<Position>(registry.create());
    sub.reset();
    registry.emplace<Position>(registry.create());
    CHECK(created == 1);
}
