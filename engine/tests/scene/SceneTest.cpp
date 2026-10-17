/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneTest
*/

#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/scene/SceneLoader.hpp"
#include "engine/scene/SceneManager.hpp"
#include "scene/SceneFixture.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace scenetest {

struct Aim {
    Entity target;
};

}

ENGINE_COMPONENT(scenetest::Aim)

using namespace scenetest;

namespace {

struct LoaderFixture : SceneFixture {
    PrefabLibrary prefabs{types, assets};
    SceneLoader loader{types, assets, prefabs};

    explicit LoaderFixture(const std::string& folder) : SceneFixture(folder) {
        this->types.add<Aim>().field("target", &Aim::target);
        std::filesystem::create_directories(this->root / "scenes");
        this->write("prefabs/ship.yaml", "Health: { hp: 3, maxHp: 3 }\n"
                                         "Sprite: { texture: assets/ship.png, layer: 30 }\n"
                                         "children:\n"
                                         "  - prefab: prefabs/force.yaml\n"
                                         "    Parent: { offset: { x: 26, y: 0 } }\n");
        this->write("prefabs/force.yaml", "Health: { hp: 99, maxHp: 99 }\n");
        this->write("scenes/level1.yaml", "name: Level 1\n"
                                          "entities:\n"
                                          "  - uuid: 0a\n"
                                          "    Aim: { target: 0b }\n"
                                          "  - uuid: 0b\n"
                                          "    prefab: prefabs/ship.yaml\n"
                                          "    Position: { x: 40, y: 90 }\n"
                                          "    Health: { hp: 1 }\n"
                                          "  - uuid: 0c\n"
                                          "    Health: { hp: 7 }\n"
                                          "    Parent: { value: 0b, offset: { x: 5, y: 0 } }\n");
        this->write("scenes/level2.yaml", "name: Level 2\n"
                                          "entities:\n"
                                          "  - Health: { hp: 42 }\n");
    }

    std::string errorOf(const std::string& text) {
        this->write("scenes/broken.yaml", text);
        const auto scene = this->loader.load("scenes/broken.yaml");
        REQUIRE_FALSE(scene);
        return scene.error().toString();
    }

    std::string read(const std::string& path) const {
        std::ifstream in(this->root / path, std::ios::binary);
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }
};

Entity withHealth(Registry& world, int32_t hp) {
    for (Entity e : world.entities())
        if (const auto* health = world.tryGet<Health>(e); health && health->hp == hp) return e;
    return Entity::null();
}

}

TEST_CASE("a scene links entities by uuid in any order", "[scene]") {
    LoaderFixture f("rtype-scene-load");
    const auto loaded = f.loader.load("scenes/level1.yaml");
    REQUIRE(loaded);
    Scene& scene = **loaded;
    Registry& world = scene.registry();
    CHECK(scene.name() == "Level 1");
    CHECK(scene.path() == "scenes/level1.yaml");

    const Entity ship = world.find(Uuid{0, 0x0b});
    const Entity turret = world.find(Uuid{0, 0x0a});
    REQUIRE_FALSE(ship.isNull());
    REQUIRE_FALSE(turret.isNull());
    CHECK(world.get<Aim>(turret).target == ship);
    CHECK(world.get<Health>(ship).hp == 1);
    CHECK(world.get<Health>(ship).maxHp == 3);
    CHECK(world.get<Position>(ship).x == 40);
    CHECK(world.get<Position>(ship).y == 90);
    CHECK(scene.prefabOf(ship) == "prefabs/ship.yaml");

    const auto children = world.children(ship);
    REQUIRE(children.size() == 2);
    const Entity force = children[0];
    CHECK(world.get<Health>(force).hp == 99);
    CHECK(world.get<Position>(force).x == 66);
    CHECK(scene.generated(force));
    const Entity wing = world.find(Uuid{0, 0x0c});
    CHECK(children[1] == wing);
    CHECK(world.get<Parent>(wing).value == ship);
    CHECK(world.get<Parent>(wing).offset == Vec2{5, 0});
}

TEST_CASE("scene errors name the line and leave nothing behind", "[scene]") {
    LoaderFixture f("rtype-scene-errors");
    CHECK(f.errorOf("entities:\n  - Helth: { hp: 1 }\n") ==
          "scenes/broken.yaml:2: unknown component 'Helth' (did you mean 'Health'?)");
    CHECK(f.errorOf("entities:\n  - prefab: prefabs/nope.yaml\n") ==
          "scenes/broken.yaml:2: unknown prefab 'prefabs/nope.yaml'");
    CHECK(f.errorOf("entities:\n  - Aim: { target: ff }\n") ==
          "scenes/broken.yaml:2: Aim refers to uuid '00000000000000ff', which no entity of this scene has");
    CHECK(f.errorOf("entities:\n  - uuid: 01\n  - uuid: 1\n") ==
          "scenes/broken.yaml:3: uuid '0000000000000001' is used twice");
    CHECK(f.errorOf("entities:\n  - uuid: 1\n    Parent: { value: 2 }\n  - uuid: 2\n    Parent: { value: 1 }\n")
              .find("loop") != std::string::npos);
    CHECK(f.errorOf("levels: []\n").find("unknown scene key 'levels'") != std::string::npos);
    CHECK_FALSE(f.loader.load("scenes/missing.yaml"));
}

TEST_CASE("saving writes only what differs from the prefab", "[scene]") {
    LoaderFixture f("rtype-scene-save");
    auto loaded = f.loader.load("scenes/level1.yaml");
    REQUIRE(loaded);
    Scene& scene = **loaded;
    scene.registry().get<Health>(scene.registry().find(Uuid{0, 0x0c})).hp = 8;
    REQUIRE(f.loader.save(scene, f.root / "scenes/saved.yaml"));

    const std::string text = f.read("scenes/saved.yaml");
    CHECK(text.find("name: Level 1") != std::string::npos);
    CHECK(text.find("prefab: prefabs/ship.yaml") != std::string::npos);
    CHECK(text.find("Sprite") == std::string::npos);
    CHECK(text.find("hp: 99") == std::string::npos);
    CHECK(std::ranges::count(text, '\n') > 5);
    size_t uuids = 0;
    for (size_t at = text.find("uuid:"); at != std::string::npos; at = text.find("uuid:", at + 1))
        ++uuids;
    CHECK(uuids == 3);

    auto again = f.loader.load("scenes/saved.yaml");
    REQUIRE(again);
    Registry& world = (*again)->registry();
    const Entity ship = world.find(Uuid{0, 0x0b});
    CHECK(world.get<Health>(ship).hp == 1);
    CHECK(world.get<Health>(ship).maxHp == 3);
    CHECK(world.get<Sprite>(ship).layer == 30);
    CHECK(world.get<Aim>(world.find(Uuid{0, 0x0a})).target == ship);
    CHECK(world.get<Health>(world.find(Uuid{0, 0x0c})).hp == 8);
    CHECK(world.children(ship).size() == 2);
}

TEST_CASE("switching scenes waits for the end of the frame and keeps persistent entities", "[scene]") {
    LoaderFixture f("rtype-scene-switch");
    EventBus events;
    SceneManager scenes(f.types, f.assets, events, f.log);
    std::vector<std::string> seen;
    const Subscription unloading =
        events.subscribe<SceneUnloading>([&](const SceneUnloading& e) { seen.push_back("unloading " + e.name); });
    const Subscription loaded =
        events.subscribe<SceneLoaded>([&](const SceneLoaded& e) { seen.push_back("loaded " + e.name); });

    REQUIRE(scenes.load("scenes/level1.yaml"));
    CHECK(scenes.activeName() == "Level 1");
    const Entity ship = scenes.world().find(Uuid{0, 0x0b});
    scenes.markPersistent(ship);

    scenes.switchTo("scenes/level2.yaml");
    CHECK(scenes.hasPending());
    CHECK(scenes.activeName() == "Level 1");
    scenes.applyPending();
    CHECK_FALSE(scenes.hasPending());
    CHECK(scenes.activeName() == "Level 2");
    CHECK(seen == std::vector<std::string>{"loaded Level 1", "unloading Level 1", "loaded Level 2"});

    Registry& world = scenes.world();
    CHECK_FALSE(withHealth(world, 42).isNull());
    const Entity kept = world.find(Uuid{0, 0x0b});
    REQUIRE_FALSE(kept.isNull());
    CHECK(world.get<Health>(kept).hp == 1);
    const auto children = world.children(kept);
    REQUIRE(children.size() == 2);
    CHECK(world.get<Parent>(children[0]).value == kept);
    CHECK(withHealth(world, 7) == children[1]);
    CHECK(world.find(Uuid{0, 0x0a}).isNull());

    scenes.switchTo("scenes/missing.yaml");
    scenes.applyPending();
    CHECK(scenes.activeName() == "Level 2");
    CHECK(seen.size() == 3);

    scenes.clear();
    CHECK(scenes.activeName().empty());
    CHECK(scenes.world().entities().empty());
}
