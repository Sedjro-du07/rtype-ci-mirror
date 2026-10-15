/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SparseSetTest
*/

#include "engine/ecs/SparseSet.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace engine;

namespace {

struct Health {
    int value = 0;
};

struct Velocity {
    float x = 0, y = 0;
};

struct Name {
    std::string text;
};

struct Locked {
    std::unique_ptr<int> key;
};

}

TEST_CASE("emplace, has and get across pages", "[ecs][sparse]") {
    SparseSet<Health> set(2);
    const Entity a{5, 0}, b{4100, 0}, c{100000, 3};
    set.emplace(a, 10);
    set.emplace(b, 20);
    set.emplace(c, 30);

    CHECK(set.id() == 2);
    CHECK(set.size() == 3);
    CHECK(set.has(a));
    CHECK(set.has(b));
    CHECK(set.has(c));
    CHECK(set.get(a).value == 10);
    CHECK(set.get(b).value == 20);
    CHECK(set.get(c).value == 30);
    CHECK_FALSE(set.has(Entity{6, 0}));
    CHECK_FALSE(set.has(Entity{8192, 0}));
    CHECK_FALSE(set.has(Entity::null()));
}

TEST_CASE("a stale generation is not found", "[ecs][sparse]") {
    SparseSet<Health> set(0);
    set.emplace(Entity{3, 1}, 40);
    CHECK_FALSE(set.has(Entity{3, 0}));
    CHECK_FALSE(set.has(Entity{3, 2}));
    CHECK(set.tryGet(Entity{3, 0}) == nullptr);
    set.remove(Entity{3, 0});
    CHECK(set.size() == 1);
}

TEST_CASE("emplace builds the component from its arguments", "[ecs][sparse]") {
    SparseSet<Velocity> set(0);
    Velocity& v = set.emplace(Entity{0, 0}, 3.f, 4.f);
    CHECK(v.x == 3.f);
    CHECK(v.y == 4.f);
    v.x = 9.f;
    CHECK(set.get(Entity{0, 0}).x == 9.f);
}

TEST_CASE("remove swaps with the last and keeps every other component", "[ecs][sparse]") {
    SparseSet<Health> set(0);
    const Entity e12{12, 0}, e3{3, 0}, e742{742, 0}, e50{50, 0};
    set.emplace(e12, 100);
    set.emplace(e3, 40);
    set.emplace(e742, 10);
    set.emplace(e50, 70);

    set.remove(e3);
    CHECK(set.size() == 3);
    CHECK_FALSE(set.has(e3));
    CHECK(set.get(e12).value == 100);
    CHECK(set.get(e742).value == 10);
    CHECK(set.get(e50).value == 70);
    CHECK(set.entities()[1] == e50);

    set.remove(e50);
    set.remove(e50);
    set.remove(Entity{9, 0});
    CHECK(set.size() == 2);
    CHECK(set.get(e12).value == 100);
    CHECK(set.get(e742).value == 10);
}

TEST_CASE("entities and components are in the same order", "[ecs][sparse]") {
    SparseSet<Health> set(0);
    for (uint32_t i = 0; i < 10; ++i)
        set.emplace(Entity{i * 1000, 0}, static_cast<int>(i));
    set.remove(Entity{0, 0});
    set.remove(Entity{5000, 0});

    const auto entities = set.entities();
    const auto components = set.components();
    REQUIRE(entities.size() == components.size());
    for (size_t i = 0; i < entities.size(); ++i)
        CHECK(components[i].value == static_cast<int>(entities[i].index / 1000));
}

TEST_CASE("tryGet and replace", "[ecs][sparse]") {
    SparseSet<Name> set(0);
    const Entity e{1, 0};
    CHECK(set.tryGet(e) == nullptr);
    set.emplace(e, "ship");
    REQUIRE(set.tryGet(e) != nullptr);
    CHECK(set.tryGet(e)->text == "ship");
    set.replace(e, Name{"boss"});
    CHECK(set.get(e).text == "boss");
}

TEST_CASE("clear removes everything", "[ecs][sparse]") {
    SparseSet<Health> set(0);
    set.emplace(Entity{1, 0}, 1);
    set.emplace(Entity{9000, 0}, 2);
    set.clear();
    CHECK(set.size() == 0);
    CHECK_FALSE(set.has(Entity{1, 0}));
    CHECK_FALSE(set.has(Entity{9000, 0}));
    set.emplace(Entity{1, 0}, 3);
    CHECK(set.get(Entity{1, 0}).value == 3);
}

TEST_CASE("raw access through IStorage", "[ecs][sparse]") {
    std::unique_ptr<IStorage> storage = std::make_unique<SparseSet<Name>>(7);
    const Entity e{4, 0};
    const Name source{"enemy"};

    void* created = storage->emplaceRaw(e, &source);
    REQUIRE(created != nullptr);
    CHECK(static_cast<Name*>(created)->text == "enemy");
    CHECK(storage->id() == 7);
    CHECK(storage->has(e));
    CHECK(storage->size() == 1);
    CHECK(storage->entities()[0] == e);
    CHECK(static_cast<Name*>(storage->getRaw(e))->text == "enemy");

    const IStorage& readOnly = *storage;
    CHECK(static_cast<const Name*>(readOnly.getRaw(e))->text == "enemy");
    CHECK(readOnly.getRaw(Entity{5, 0}) == nullptr);

    storage->remove(e);
    CHECK_FALSE(storage->has(e));
    CHECK(storage->getRaw(e) == nullptr);
}

TEST_CASE("move-only components are stored", "[ecs][sparse]") {
    SparseSet<Locked> set(0);
    set.emplace(Entity{0, 0}, std::make_unique<int>(1));
    set.emplace(Entity{1, 0}, std::make_unique<int>(2));
    set.remove(Entity{0, 0});
    CHECK(*set.get(Entity{1, 0}).key == 2);
}
