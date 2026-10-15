/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** TimeTest
*/

#include "engine/core/Time.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

using namespace engine;

// Durations are only compared with each other or with zero: a CI machine can be slow at any moment.

TEST_CASE("Time defaults", "[core][time]") {
    constexpr Time t;
    static_assert(t.fixedDelta == 1.0 / 60.0);
    static_assert(t.tick == 0);
    static_assert(t.alpha == 0.0);
    static_assert(t.total == 0.0);
    static_assert(t.frameDelta == 0.0);
}

TEST_CASE("Clock::restart returns positive durations and starts again from zero", "[core][time]") {
    Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    const double first = clock.restart();
    CHECK(first > 0.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    const double second = clock.restart();
    CHECK(second > 0.0);

    // Right after a restart, the clock counts from zero again.
    CHECK(clock.elapsed() < second);
}

TEST_CASE("Clock::elapsed does not reset", "[core][time]") {
    Clock clock;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const double a = clock.elapsed();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const double b = clock.elapsed();
    CHECK(a > 0.0);
    CHECK(b > a);
    CHECK(clock.restart() >= b); // restart() saw everything elapsed() saw
}
