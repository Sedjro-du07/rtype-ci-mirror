/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EventBusTest
*/

#include "engine/events/EventBus.hpp"

#include "engine/events/EngineEvents.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace game {

struct BossDefeated {
    uint32_t score = 0;
};

struct Shot {
    int32_t id = 0;
};

}

ENGINE_EVENT(game::BossDefeated)
ENGINE_EVENT(game::Shot)

using namespace engine;

TEST_CASE("published events wait for dispatch and keep their order", "[events]") {
    EventBus bus;
    std::vector<std::string> seen;
    const Subscription a =
        bus.subscribe<game::Shot>([&](const game::Shot& e) { seen.push_back("shot " + std::to_string(e.id)); });
    const Subscription b = bus.subscribe<game::BossDefeated>(
        [&](const game::BossDefeated& e) { seen.push_back("boss " + std::to_string(e.score)); });

    bus.publish(game::Shot{1});
    bus.publish(game::BossDefeated{5000});
    bus.publish(game::Shot{2});
    CHECK(seen.empty());

    bus.dispatch();
    CHECK(seen == std::vector<std::string>{"shot 1", "boss 5000", "shot 2"});
    bus.dispatch();
    CHECK(seen.size() == 3);
}

TEST_CASE("emit delivers at once", "[events]") {
    EventBus bus;
    int calls = 0;
    const Subscription sub =
        bus.subscribe<Collision>([&](const Collision& e) { calls += static_cast<int>(e.a.index); });
    bus.emit(Collision{Entity{3, 0}, Entity{4, 0}});
    CHECK(calls == 3);
    bus.emit(QuitRequested{});
}

TEST_CASE("events published during dispatch wait for the next one", "[events]") {
    EventBus bus;
    std::vector<int32_t> seen;
    const Subscription sub = bus.subscribe<game::Shot>([&](const game::Shot& e) {
        seen.push_back(e.id);
        if (e.id < 3) bus.publish(game::Shot{e.id + 1});
    });
    bus.publish(game::Shot{1});
    bus.dispatch();
    CHECK(seen == std::vector<int32_t>{1});
    bus.dispatch();
    bus.dispatch();
    CHECK(seen == std::vector<int32_t>{1, 2, 3});
}

TEST_CASE("unsubscribing during dispatch", "[events]") {
    EventBus bus;
    int first = 0, second = 0;
    Subscription a, b;
    a = bus.subscribe<game::Shot>([&](const game::Shot&) {
        ++first;
        a.reset();
        b.reset();
    });
    b = bus.subscribe<game::Shot>([&](const game::Shot&) { ++second; });
    bus.publish(game::Shot{});
    bus.publish(game::Shot{});
    bus.dispatch();
    CHECK(first == 1);
    CHECK(second == 0);
}

TEST_CASE("events published from other threads arrive at dispatch", "[events]") {
    EventBus bus;
    std::vector<int32_t> seen;
    const Subscription sub = bus.subscribe<game::Shot>([&](const game::Shot& e) { seen.push_back(e.id); });

    constexpr int threads = 4, perThread = 1000;
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t)
        workers.emplace_back([&bus, t] {
            for (int i = 0; i < perThread; ++i)
                bus.publishFromAnyThread(game::Shot{t * perThread + i});
        });
    for (std::thread& worker : workers)
        worker.join();

    bus.dispatch();
    REQUIRE(seen.size() == threads * perThread);
    std::vector<int32_t> last(threads, -1);
    for (int32_t id : seen) {
        CHECK(id > last[id / perThread]);
        last[id / perThread] = id;
    }
}

TEST_CASE("clearGameSubscriptions removes the game listeners only", "[events]") {
    EventBus bus;
    int engineCalls = 0, gameCalls = 0;
    const Subscription engineSide = bus.subscribe<game::Shot>([&](const game::Shot&) { ++engineCalls; });
    bus.beginGameSubscriptions();
    const Subscription gameSide = bus.subscribe<game::Shot>([&](const game::Shot&) { ++gameCalls; });

    bus.emit(game::Shot{});
    CHECK(engineCalls == 1);
    CHECK(gameCalls == 1);

    bus.publish(game::Shot{});
    bus.clearGameSubscriptions();
    CHECK_FALSE(gameSide.active());
    CHECK(engineSide.active());
    bus.dispatch();
    bus.emit(game::Shot{});
    CHECK(engineCalls == 2);
    CHECK(gameCalls == 1);
}

TEST_CASE("a subscription can outlive its bus", "[events]") {
    Subscription sub;
    {
        EventBus bus;
        sub = bus.subscribe<SceneLoaded>([](const SceneLoaded&) {});
        CHECK(sub.active());
    }
    CHECK_FALSE(sub.active());
    sub.reset();
}

TEST_CASE("move-only listeners and events with strings", "[events]") {
    EventBus bus;
    auto total = std::make_unique<std::string>();
    std::string* out = total.get();
    const Subscription sub =
        bus.subscribe<Caption>([held = std::move(total), out](const Caption& c) { *out += c.text; });
    bus.publish(Caption{"explosion ", 1.f});
    bus.publish(Caption{"on the left", 1.f});
    bus.dispatch();
    CHECK(*out == "explosion on the left");
}
