/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CommandsTest
*/

#include "engine/ecs/Commands.hpp"

#include "TestWorld.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <new>
#include <string>
#include <vector>

using namespace engine;
using namespace world;

TEST_CASE("spawn inside a view loop", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    for (int i = 0; i < 3; ++i)
        registry.emplace<Health>(registry.create(), 10);

    int visited = 0;
    for (auto [e, hp] : registry.view<Health>()) {
        ++visited;
        const Entity spawned = cmd.spawn();
        CHECK(registry.alive(spawned));
        cmd.add(spawned, Health{hp.hp + 1});
    }
    CHECK(visited == 3);
    CHECK(registry.view<Health>().sizeHint() == 3);

    cmd.apply();
    CHECK(registry.view<Health>().sizeHint() == 6);
    CHECK(cmd.empty());
}

TEST_CASE("destroying inside a loop skips nobody", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    const std::vector<int32_t> hps{30, 0, 50, 0};
    for (int32_t hp : hps)
        registry.emplace<Health>(registry.create(), hp);

    for (auto [e, hp] : registry.view<Health>())
        if (hp.hp <= 0) cmd.destroy(e);
    cmd.apply();

    CHECK(registry.aliveCount() == 2);
    for (auto [e, hp] : registry.view<Health>())
        CHECK(hp.hp > 0);
}

TEST_CASE("destroy then add on the same entity", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    const Entity e = registry.create();
    cmd.destroy(e);
    cmd.add(e, Health{5});
    cmd.set(e, Position{1, 1});
    cmd.remove<Health>(e);
    cmd.apply();
    CHECK_FALSE(registry.alive(e));
    CHECK(registry.view<Health>().sizeHint() == 0);

    const Entity reused = registry.create();
    CHECK(reused.index == e.index);
    CHECK_FALSE(registry.has<Health>(reused));
}

TEST_CASE("commands apply in recording order", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    const Entity a = registry.create();
    const Entity b = registry.create();

    cmd.set(a, Health{1});
    cmd.remove<Health>(a);
    cmd.remove<Health>(b);
    cmd.set(b, Health{2});
    cmd.set(b, Health{3});
    cmd.add(b, Health{4});
    cmd.apply();

    CHECK_FALSE(registry.has<Health>(a));
    CHECK(registry.get<Health>(b).hp == 3);
}

TEST_CASE("hooks fire at apply, not when recording", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    int created = 0, replaced = 0, destroyed = 0;
    ComponentHooks& hooks = registry.hooks<Health>();
    const Subscription a = hooks.onCreate.connect([&](Registry&, Entity) { ++created; });
    const Subscription b = hooks.onReplace.connect([&](Registry&, Entity) { ++replaced; });
    const Subscription c = hooks.onDestroy.connect([&](Registry&, Entity) { ++destroyed; });

    const Entity e = cmd.spawn();
    cmd.set(e, Health{1});
    cmd.set(e, Health{2});
    cmd.destroy(e);
    CHECK(created == 0);
    cmd.apply();
    CHECK(created == 1);
    CHECK(replaced == 1);
    CHECK(destroyed == 1);
}

TEST_CASE("commands recorded during apply wait for the next apply", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    const Subscription sub =
        registry.hooks<Health>().onCreate.connect([&](Registry&, Entity e) { cmd.add(e, Position{7, 7}); });

    const Entity e = cmd.spawn();
    cmd.add(e, Health{});
    cmd.apply();
    CHECK(registry.has<Health>(e));
    CHECK_FALSE(registry.has<Position>(e));
    CHECK_FALSE(cmd.empty());
    cmd.apply();
    CHECK(registry.get<Position>(e).x == 7.f);
}

TEST_CASE("setParent through commands", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    Commands cmd(registry);
    const Entity boss = cmd.spawn();
    const Entity turret = cmd.spawn();
    cmd.setParent(turret, boss, Vec2{5, 0});
    cmd.apply();
    CHECK(registry.parent(turret) == boss);

    cmd.setParent(turret, Entity::null());
    cmd.apply();
    CHECK(registry.parent(turret).isNull());

    cmd.setParent(turret, boss);
    cmd.destroy(boss);
    cmd.apply();
    CHECK_FALSE(registry.alive(turret));
}

namespace {

std::vector<std::byte> constructedName(const TypeInfo& info, const std::string& text) {
    std::vector<std::byte> bytes(info.size);
    info.construct(bytes.data());
    std::launder(reinterpret_cast<Name*>(bytes.data()))->value = text;
    return bytes;
}

}

TEST_CASE("addRaw copies a constructed object and destroys the buffer", "[ecs][commands]") {
    const TypeRegistry types = makeTypes();
    Registry registry(types);
    const TypeInfo& name = *types.find("Name");
    {
        Commands cmd(registry);
        const Entity e = cmd.spawn();
        cmd.addRaw(e, name.id, constructedName(name, "a name long enough to allocate on the heap"));
        cmd.apply();
        CHECK(registry.get<Name>(e).value == "a name long enough to allocate on the heap");
        CHECK(registry.find("a name long enough to allocate on the heap") == e);
    }
    {
        Commands cmd(registry);
        cmd.addRaw(cmd.spawn(), name.id, constructedName(name, "never applied, still freed by the buffer"));
    }
}
