/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** PrefabTest
*/

#include "engine/scene/Prefab.hpp"

#include "engine/ecs/Commands.hpp"
#include "scene/SceneFixture.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace scenetest;

namespace {

struct PrefabFixture : SceneFixture {
    PrefabLibrary prefabs{types, assets};
    Registry world{types};

    explicit PrefabFixture(const std::string& folder) : SceneFixture(folder) {
        this->write("prefabs/ship.yaml", "Health: { hp: 3, maxHp: 3 }\n"
                                         "Sprite: { texture: assets/ship.png, layer: 30 }\n"
                                         "children:\n"
                                         "  - prefab: prefabs/force.yaml\n"
                                         "    Parent: { offset: { x: 26, y: 0 } }\n"
                                         "  - Health: { hp: 9 }\n"
                                         "    Parent: { offset: { x: -10, y: 4 } }\n");
        this->write("prefabs/force.yaml", "Health: { hp: 99, maxHp: 99 }\n");
    }

    const Prefab& get(const std::string& path) {
        const auto found = this->prefabs.get(path);
        REQUIRE(found);
        return **found;
    }

    std::string errorOf(const std::string& path) {
        const auto found = this->prefabs.get(path);
        REQUIRE_FALSE(found);
        return found.error().toString();
    }
};

}

TEST_CASE("a prefab is parsed once and found by its key", "[prefab]") {
    PrefabFixture f("rtype-prefab-cache");
    const Prefab& ship = f.get("prefabs/ship.yaml");
    CHECK(&f.get("prefabs/ship.yaml") == &ship);
    CHECK(ship.key == prefabKey("prefabs/ship.yaml"));
    CHECK(f.prefabs.findByKey(ship.key) == &ship);
    CHECK(f.prefabs.findByKey(ship.key + 1) == nullptr);
    CHECK(ship.components.size() == 2);
    REQUIRE(ship.children.size() == 2);
    CHECK(ship.children[0].resolved == &f.get("prefabs/force.yaml"));
    CHECK(ship.children[0].offset == Vec2{26, 0});
    const uint32_t key = ship.key;
    f.prefabs.clear();
    CHECK(f.prefabs.findByKey(key) == nullptr);
}

TEST_CASE("instantiate creates the entity, its position and its children", "[prefab]") {
    PrefabFixture f("rtype-prefab-instantiate");
    const Entity ship = f.prefabs.instantiate(f.world, f.get("prefabs/ship.yaml"), {100, 50});
    CHECK(f.world.get<Health>(ship).hp == 3);
    CHECK(f.world.get<Sprite>(ship).layer == 30);
    CHECK(f.assets.pathOf(f.world.get<Sprite>(ship).texture) == "assets/ship.png");
    CHECK(f.world.get<Position>(ship).x == 100);
    CHECK(f.world.get<Position>(ship).y == 50);

    const auto children = f.world.children(ship);
    REQUIRE(children.size() == 2);
    const Entity force = children[0];
    CHECK(f.world.get<Health>(force).hp == 99);
    CHECK(f.world.get<Parent>(force).value == ship);
    CHECK(f.world.get<Parent>(force).offset == Vec2{26, 0});
    CHECK(f.world.get<Position>(force).x == 126);
    const Entity inline_ = children[1];
    CHECK(f.world.get<Health>(inline_).hp == 9);
    CHECK(f.world.get<Health>(inline_).maxHp == 1);
    CHECK(f.world.get<Position>(inline_).y == 54);

    f.world.destroy(ship);
    CHECK(f.world.aliveCount() == 0);
}

TEST_CASE("overrides keep the prefab values they do not write", "[prefab]") {
    PrefabFixture f("rtype-prefab-overrides");
    const Prefab& force = f.get("prefabs/force.yaml");
    const auto node = YamlNode::parse("{ hp: 1 }", "scene.yaml");
    REQUIRE(node);
    LoadContext ctx{f.types, &f.assets, {}};
    auto partial = ComponentData::parse(f.types.info(f.types.id<Health>()), *node, ctx);
    REQUIRE(partial);

    const ComponentData fromYaml[] = {*partial};
    const Entity a = f.prefabs.instantiate(f.world, force, {}, fromYaml);
    CHECK(f.world.get<Health>(a).hp == 1);
    CHECK(f.world.get<Health>(a).maxHp == 99);

    const ComponentData fromCode[] = {ComponentData::of(f.types, Health{5, 5}),
                                      ComponentData::of(f.types, Sprite{{}, 7})};
    const Entity b = f.prefabs.instantiate(f.world, force, {}, fromCode);
    CHECK(f.world.get<Health>(b).hp == 5);
    CHECK(f.world.get<Health>(b).maxHp == 5);
    CHECK(f.world.get<Sprite>(b).layer == 7);
}

TEST_CASE("prefab errors name the file and the line", "[prefab]") {
    PrefabFixture f("rtype-prefab-errors");
    f.write("prefabs/typo.yaml", "Health: { hp: 1 }\nHelth: { hp: 2 }\n");
    f.write("prefabs/bad-field.yaml", "Health: { hp: three }\n");
    f.write("prefabs/loop.yaml", "children:\n  - prefab: prefabs/loop.yaml\n");
    f.write("prefabs/missing-child.yaml", "children:\n  - prefab: prefabs/nope.yaml\n");
    CHECK(f.errorOf("prefabs/typo.yaml") == "prefabs/typo.yaml:2: unknown component 'Helth' (did you mean 'Health'?)");
    CHECK(f.errorOf("prefabs/bad-field.yaml").starts_with("prefabs/bad-field.yaml:1: field 'hp' of Health expects"));
    CHECK(f.errorOf("prefabs/loop.yaml").find("includes itself") != std::string::npos);
    CHECK(f.errorOf("prefabs/missing-child.yaml") == "unknown prefab 'prefabs/nope.yaml'");
    CHECK(f.errorOf("../outside.yaml") == "unknown prefab '../outside.yaml'");
    CHECK(f.prefabs.get("prefabs/force.yaml"));
}

TEST_CASE("Commands spawn a prefab when applied", "[prefab]") {
    PrefabFixture f("rtype-prefab-commands");
    Commands commands(f.world, &f.prefabs);
    const Entity e = commands.spawn("prefabs/force.yaml", {3, 4});
    REQUIRE_FALSE(e.isNull());
    CHECK(f.world.alive(e));
    CHECK_FALSE(f.world.has<Health>(e));
    commands.apply();
    CHECK(f.world.get<Health>(e).hp == 99);
    CHECK(f.world.get<Position>(e).x == 3);
    CHECK(commands.spawn("prefabs/nope.yaml", {}).isNull());
    Commands without(f.world);
    CHECK(without.spawn("prefabs/force.yaml", {}).isNull());
}
