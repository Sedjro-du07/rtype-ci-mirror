/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** RegistryTest
*/

#include "TestWorld.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

using namespace engine;
using namespace world;

TEST_CASE("entities are created, destroyed and counted", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity a = registry.create();
    const Entity b = registry.create();
    CHECK(registry.alive(a));
    CHECK(registry.aliveCount() == 2);
    registry.destroy(a);
    CHECK_FALSE(registry.alive(a));
    CHECK(registry.alive(b));
    CHECK(registry.aliveCount() == 1);
    registry.destroy(a);
    CHECK(registry.aliveCount() == 1);
}

TEST_CASE("typed component access", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity ship = registry.create();

    Position& p = registry.emplace<Position>(ship, 10.f, 20.f);
    CHECK(p.x == 10.f);
    CHECK(registry.has<Position>(ship));
    CHECK_FALSE(registry.has<Velocity>(ship));
    CHECK(registry.tryGet<Velocity>(ship) == nullptr);

    registry.get<Position>(ship).x = 15.f;
    CHECK(registry.get<Position>(ship).x == 15.f);
    const Registry& readOnly = registry;
    CHECK(readOnly.get<Position>(ship).y == 20.f);

    registry.replace<Position>(ship, Position{1.f, 2.f});
    CHECK(registry.get<Position>(ship).y == 2.f);

    registry.remove<Position>(ship);
    CHECK_FALSE(registry.has<Position>(ship));
    registry.remove<Position>(ship);
}

TEST_CASE("destroy removes every component", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity e = registry.create();
    registry.emplace<Position>(e);
    registry.emplace<Health>(e, 50);
    registry.destroy(e);
    const Entity reused = registry.create();
    CHECK(reused.index == e.index);
    CHECK_FALSE(registry.has<Position>(reused));
    CHECK_FALSE(registry.has<Health>(reused));
}

TEST_CASE("raw access by ComponentId", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity e = registry.create();
    const ComponentId health = types.id<Health>();

    CHECK(registry.storage(health) == nullptr);
    const Health source{42};
    void* created = registry.emplaceRaw(e, health, &source);
    CHECK(static_cast<Health*>(created)->hp == 42);
    CHECK(registry.get<Health>(e).hp == 42);
    CHECK(registry.storage(health) != nullptr);
    CHECK(static_cast<Health*>(registry.getRaw(e, health))->hp == 42);

    registry.emplace<Position>(e);
    std::vector<ComponentId> ids = registry.componentsOf(e);
    std::ranges::sort(ids);
    std::vector<ComponentId> expected{types.id<Position>(), health};
    std::ranges::sort(expected);
    CHECK(ids == expected);

    registry.removeRaw(e, health);
    CHECK_FALSE(registry.has<Health>(e));
    CHECK(registry.getRaw(e, health) == nullptr);
}

TEST_CASE("find by name and uuid, and text tags", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity boss = registry.create();
    const Entity grunt = registry.create();
    registry.emplace<Name>(boss, "Boss");
    registry.emplace<Name>(grunt, "Grunt");
    CHECK(registry.find("Boss") == boss);
    CHECK(registry.find("Nobody").isNull());

    const Uuid id = Uuid::generate();
    registry.emplace<Uuid>(boss, id);
    CHECK(registry.find(id) == boss);
    const Uuid other = Uuid::generate();
    registry.replace<Uuid>(boss, other);
    CHECK(registry.find(id).isNull());
    CHECK(registry.find(other) == boss);
    registry.destroy(boss);
    CHECK(registry.find(other).isNull());

    registry.emplace<Tags>(grunt, Tags{{fnv1a64("enemy"), fnv1a64("small")}});
    CHECK(registry.withTag("enemy") == std::vector<Entity>{grunt});
    CHECK(registry.withTag("boss").empty());
}

TEST_CASE("hierarchy and recursive destroy", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const Entity boss = registry.create();
    const Entity turret = registry.create();
    const Entity barrel = registry.create();
    const Entity other = registry.create();

    registry.setParent(turret, boss, Vec2{10, 0});
    registry.setParent(barrel, turret);
    CHECK(registry.parent(turret) == boss);
    CHECK(registry.get<Parent>(turret).offset == Vec2{10, 0});
    REQUIRE(registry.children(boss).size() == 1);
    CHECK(registry.children(boss)[0] == turret);

    registry.setParent(turret, other);
    CHECK(registry.children(boss).empty());
    CHECK(registry.children(other)[0] == turret);
    registry.setParent(turret, boss);

    registry.destroy(boss);
    CHECK_FALSE(registry.alive(boss));
    CHECK_FALSE(registry.alive(turret));
    CHECK_FALSE(registry.alive(barrel));
    CHECK(registry.alive(other));
    CHECK(registry.children(other).empty());

    const Entity child = registry.create();
    registry.setParent(child, other);
    registry.destroy(child);
    CHECK(registry.children(other).empty());

    const Entity orphan = registry.create();
    registry.setParent(orphan, other);
    registry.setParent(orphan, Entity::null());
    CHECK(registry.parent(orphan).isNull());
    CHECK(registry.children(other).empty());
}

TEST_CASE("world singletons live on one hidden entity", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    CHECK(registry.trySingleton<Score>() == nullptr);
    registry.singleton<Score>().value = 10;
    registry.singleton<Score>().value += 5;
    REQUIRE(registry.trySingleton<Score>() != nullptr);
    CHECK(registry.trySingleton<Score>()->value == 15);
    CHECK(registry.aliveCount() == 1);
}

TEST_CASE("clear destroys everything", "[ecs][registry]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    for (int i = 0; i < 10; ++i)
        registry.emplace<Position>(registry.create());
    registry.singleton<Score>();
    registry.clear();
    CHECK(registry.aliveCount() == 0);
    CHECK(registry.view<Position>().sizeHint() == 0);
    CHECK(registry.trySingleton<Score>() == nullptr);
}
