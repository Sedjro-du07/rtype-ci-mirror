/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EntityTest
*/

#include "engine/ecs/Entity.hpp"

#include "engine/ecs/EntityAllocator.hpp"

#include <catch2/catch_test_macros.hpp>

#include <unordered_set>

using namespace engine;

static_assert(Entity::null().isNull());
static_assert(Entity{}.isNull());
static_assert(!Entity{0, 0}.isNull());
static_assert(Entity{3, 1} == Entity{3, 1});
static_assert(Entity{3, 1} != Entity{3, 2});

TEST_CASE("create gives new slots in order", "[ecs][entity]") {
    EntityAllocator entities;
    const Entity a = entities.create();
    const Entity b = entities.create();
    CHECK(a == Entity{0, 0});
    CHECK(b == Entity{1, 0});
    CHECK(entities.alive(a));
    CHECK(entities.alive(b));
    CHECK(entities.aliveCount() == 2);
}

TEST_CASE("a reused slot gets a new generation", "[ecs][entity]") {
    EntityAllocator entities;
    const Entity first = entities.create();
    entities.release(first);
    const Entity second = entities.create();
    CHECK(second.index == first.index);
    CHECK(second.generation == first.generation + 1);
}

TEST_CASE("a stale handle is not alive", "[ecs][entity]") {
    EntityAllocator entities;
    const Entity bullet = entities.create();
    entities.release(bullet);
    CHECK_FALSE(entities.alive(bullet));
    const Entity enemy = entities.create();
    CHECK_FALSE(entities.alive(bullet));
    CHECK(entities.alive(enemy));
}

TEST_CASE("null and unknown handles are never alive", "[ecs][entity]") {
    EntityAllocator entities;
    CHECK_FALSE(entities.alive(Entity::null()));
    CHECK_FALSE(entities.alive(Entity{5, 0}));
    entities.create();
    CHECK_FALSE(entities.alive(Entity::null()));
    CHECK_FALSE(entities.alive(Entity{0, 1}));
}

TEST_CASE("releasing twice or releasing a dead handle changes nothing", "[ecs][entity]") {
    EntityAllocator entities;
    const Entity a = entities.create();
    entities.release(a);
    entities.release(a);
    entities.release(Entity::null());
    CHECK(entities.aliveCount() == 0);

    const Entity b = entities.create();
    const Entity c = entities.create();
    CHECK(b.index != c.index);
    CHECK(entities.aliveCount() == 2);
}

TEST_CASE("aliveCount follows create and release", "[ecs][entity]") {
    EntityAllocator entities;
    Entity handles[10];
    for (Entity& e : handles)
        e = entities.create();
    CHECK(entities.aliveCount() == 10);
    for (int i = 0; i < 10; i += 2)
        entities.release(handles[i]);
    CHECK(entities.aliveCount() == 5);
    for (int i = 0; i < 5; ++i)
        entities.create();
    CHECK(entities.aliveCount() == 10);
}

TEST_CASE("Entity works as a key of unordered containers", "[ecs][entity]") {
    std::unordered_set<Entity> set;
    set.insert({7, 2});
    set.insert({7, 3});
    set.insert({8, 2});
    set.insert({7, 2});
    CHECK(set.size() == 3);
    CHECK(set.contains({7, 3}));
    CHECK(std::hash<Entity>{}({7, 2}) == 0x0000000200000007ull);
}
