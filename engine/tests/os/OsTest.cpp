/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** OsTest
*/

#include "os/Os.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <atomic>
#include <csignal>
#include <cstddef>
#include <filesystem>
#include <set>

using namespace engine;

TEST_CASE("secureRandom gives 1000 different draws", "[os]") {
    std::set<std::array<std::byte, 16>> draws;
    for (int i = 0; i < 1000; ++i) {
        std::array<std::byte, 16> draw{};
        os::secureRandom(draw);
        draws.insert(draw);
    }
    CHECK(draws.size() == 1000);
    CHECK_FALSE(draws.contains(std::array<std::byte, 16>{}));
}

TEST_CASE("secureRandom fills a large buffer and accepts an empty one", "[os]") {
    std::array<std::byte, 4096> buffer{};
    os::secureRandom(buffer);
    CHECK(buffer != std::array<std::byte, 4096>{});
    os::secureRandom({});
}

TEST_CASE("userDataDir exists after the call", "[os]") {
    const auto dir = os::userDataDir("rtype-engine-test");
    CHECK(dir.is_absolute());
    CHECK(dir.filename() == "rtype-engine-test");
    CHECK(std::filesystem::is_directory(dir));
    std::filesystem::remove(dir);
}

TEST_CASE("executableDir contains the test binary", "[os]") {
    const auto dir = os::executableDir();
    CHECK(dir.is_absolute());
    CHECK((std::filesystem::exists(dir / "engine_os_tests") || std::filesystem::exists(dir / "engine_os_tests.exe")));
}

#if !defined(_WIN32)

namespace {
std::atomic<int> quitRequests{0};
}

TEST_CASE("onQuitSignal is called on SIGINT and SIGTERM", "[os]") {
    os::onQuitSignal([] { ++quitRequests; });
    std::raise(SIGINT);
    std::raise(SIGTERM);
    CHECK(quitRequests == 2);
}

#endif
