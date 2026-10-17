/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SchedulerTest
*/

#include "engine/scheduler/Scheduler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace engine {

class Context {
public:
    std::vector<std::string> calls;
};

}

using namespace engine;

namespace {

Logger quietLog() {
    LogConfig config;
    config.file.clear();
    config.color = false;
    config.level = LogLevel::Error;
    return Logger(config);
}

auto record(std::string name) {
    return [name = std::move(name)](Context& ctx) { ctx.calls.push_back(name); };
}

std::vector<std::string> namesOf(const Scheduler& scheduler, Stage stage) {
    std::vector<std::string> names;
    for (const SystemStats& stats : scheduler.stats())
        if (stats.stage == stage) names.emplace_back(stats.name);
    return names;
}

}

TEST_CASE("systems follow after and before, ties keep registration order", "[scheduler]") {
    Logger log = quietLog();
    Scheduler scheduler(Role::Client, log);
    scheduler.add(Stage::FixedUpdate, "game.ai", record("ai")).after("engine.movement");
    scheduler.add(Stage::FixedUpdate, "engine.collision", record("collision"));
    scheduler.add(Stage::FixedUpdate, "engine.movement", record("movement")).before("engine.collision");
    scheduler.add(Stage::FixedUpdate, "game.input", record("input")).before("engine.movement");
    scheduler.add(Stage::Render, "engine.render.sprites", record("sprites"));
    REQUIRE(scheduler.build());

    CHECK(namesOf(scheduler, Stage::FixedUpdate) ==
          std::vector<std::string>{"game.input", "engine.movement", "game.ai", "engine.collision"});
    Context ctx;
    scheduler.run(Stage::FixedUpdate, ctx);
    CHECK(ctx.calls == std::vector<std::string>{"input", "movement", "ai", "collision"});
    scheduler.run(Stage::Render, ctx);
    CHECK(ctx.calls.back() == "sprites");
}

TEST_CASE("an unknown name, a cycle or a duplicate fail the build", "[scheduler]") {
    Logger log = quietLog();
    {
        Scheduler scheduler(Role::Client, log);
        scheduler.add(Stage::FixedUpdate, "game.ai", record("ai")).after("engine.movment");
        const auto built = scheduler.build();
        REQUIRE_FALSE(built);
        CHECK(built.error().code == Errc::ScheduleCycle);
        CHECK(built.error().message == "system 'game.ai' is ordered after 'engine.movment', which is not registered");
    }
    {
        Scheduler scheduler(Role::Client, log);
        scheduler.add(Stage::FixedUpdate, "a", record("a")).after("c");
        scheduler.add(Stage::FixedUpdate, "b", record("b")).after("a");
        scheduler.add(Stage::FixedUpdate, "c", record("c")).after("b");
        scheduler.add(Stage::FixedUpdate, "d", record("d"));
        const auto built = scheduler.build();
        REQUIRE_FALSE(built);
        CHECK(built.error().message == "systems of FixedUpdate form a cycle: a -> b -> c -> a");
    }
    {
        Scheduler scheduler(Role::Client, log);
        scheduler.add(Stage::FixedUpdate, "a", record("a")).after("b");
        scheduler.add(Stage::Render, "b", record("b"));
        const auto built = scheduler.build();
        REQUIRE_FALSE(built);
        CHECK(built.error().message.find("stages are ordered by the loop") != std::string::npos);
    }
    {
        Scheduler scheduler(Role::Client, log);
        scheduler.add(Stage::FixedUpdate, "a", record("a"));
        scheduler.add(Stage::LateUpdate, "a", record("a"));
        const auto built = scheduler.build();
        REQUIRE_FALSE(built);
        CHECK(built.error().code == Errc::DuplicateName);
    }
}

TEST_CASE("roles filter systems and their constraints", "[scheduler]") {
    Logger log = quietLog();
    Scheduler server(Role::Server, log);
    server.add(Stage::FixedUpdate, "engine.net.apply_snapshot", record("snapshot")).roles(ClientRole);
    server.add(Stage::FixedUpdate, "game.ai", record("ai")).after("engine.net.apply_snapshot");
    server.add(Stage::Render, "engine.render.sprites", record("sprites")).roles(ClientRole | EditorRole);
    REQUIRE(server.build());
    CHECK(namesOf(server, Stage::FixedUpdate) == std::vector<std::string>{"game.ai"});
    CHECK(namesOf(server, Stage::Render).empty());
    CHECK_FALSE(server.enabled("engine.render.sprites"));
}

TEST_CASE("disabled systems are skipped until enabled", "[scheduler]") {
    Logger log = quietLog();
    Scheduler scheduler(Role::Client, log);
    scheduler.add(Stage::FixedUpdate, "engine.debug", record("debug")).disabled();
    REQUIRE(scheduler.build());
    Context ctx;
    CHECK_FALSE(scheduler.enabled("engine.debug"));
    scheduler.run(Stage::FixedUpdate, ctx);
    CHECK(ctx.calls.empty());
    scheduler.setEnabled("engine.debug", true);
    scheduler.run(Stage::FixedUpdate, ctx);
    CHECK(ctx.calls.size() == 1);
    CHECK(scheduler.stats()[0].lastMs >= 0);
}

TEST_CASE("failure policy depends on the role", "[scheduler]") {
    Logger log = quietLog();
    const auto boom = [](Context&) { throw std::runtime_error("boom"); };
    SECTION("the server disables the system and continues") {
        Scheduler scheduler(Role::Server, log);
        scheduler.add(Stage::FixedUpdate, "game.broken", boom);
        scheduler.add(Stage::FixedUpdate, "game.ok", record("ok"));
        REQUIRE(scheduler.build());
        Context ctx;
        scheduler.run(Stage::FixedUpdate, ctx);
        scheduler.run(Stage::FixedUpdate, ctx);
        CHECK(ctx.calls == std::vector<std::string>{"ok", "ok"});
        CHECK(scheduler.stats()[0].faulted);
        CHECK(scheduler.stats()[0].lastError == "boom");
        CHECK_FALSE(scheduler.quitRequested());
    }
    SECTION("a critical system ends the server session") {
        Scheduler scheduler(Role::Server, log);
        scheduler.add(Stage::FixedUpdate, "game.rules", boom).critical();
        REQUIRE(scheduler.build());
        Context ctx;
        scheduler.run(Stage::FixedUpdate, ctx);
        CHECK(scheduler.quitRequested());
        CHECK(scheduler.exitCode() == 5);
    }
    SECTION("the client quits with code 5") {
        Scheduler scheduler(Role::Client, log);
        scheduler.add(Stage::FixedUpdate, "game.broken", [](Context&) { throw 42; });
        scheduler.add(Stage::FixedUpdate, "game.after", record("after"));
        REQUIRE(scheduler.build());
        Context ctx;
        scheduler.run(Stage::FixedUpdate, ctx);
        CHECK(scheduler.quitRequested());
        CHECK(scheduler.exitCode() == 5);
        CHECK(scheduler.stats()[0].lastError == "unknown exception");
        CHECK(ctx.calls.empty());
    }
}

TEST_CASE("each room gets its own copy of a system", "[scheduler]") {
    Logger log = quietLog();
    Scheduler scheduler(Role::Server, log);
    struct Counter {
        int count = 0;
        void operator()(Context& ctx) { ctx.calls.push_back(std::to_string(++this->count)); }
    };
    scheduler.add(Stage::FixedUpdate, "game.counter", Counter{});
    scheduler.addFactory(Stage::FixedUpdate, "game.made", [] { return SystemFn(record("made")); });
    REQUIRE(scheduler.build());
    auto roomA = scheduler.instantiate(Stage::FixedUpdate);
    auto roomB = scheduler.instantiate(Stage::FixedUpdate);
    REQUIRE(roomA.size() == 2);
    Context a;
    Context b;
    roomA[0](a);
    roomA[0](a);
    roomB[0](b);
    roomB[1](b);
    CHECK(a.calls == std::vector<std::string>{"1", "2"});
    CHECK(b.calls == std::vector<std::string>{"1", "made"});
}

TEST_CASE("game systems are removed before the module unloads", "[scheduler]") {
    Logger log = quietLog();
    Scheduler scheduler(Role::Client, log);
    scheduler.add(Stage::FixedUpdate, "engine.movement", record("movement"));
    scheduler.beginGameSystems();
    scheduler.add(Stage::FixedUpdate, "game.ai", record("ai")).after("engine.movement");
    REQUIRE(scheduler.build());
    CHECK(scheduler.stats().size() == 2);
    scheduler.clearGameSystems();
    CHECK(scheduler.stats().empty());
    REQUIRE(scheduler.build());
    CHECK(namesOf(scheduler, Stage::FixedUpdate) == std::vector<std::string>{"engine.movement"});
}
