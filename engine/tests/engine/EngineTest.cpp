/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** EngineTest
*/

#include "engine/Engine.hpp"

#include "engine/EngineConfig.hpp"
#include "os/Os.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace engine;

namespace {

EngineConfig headless() {
    EngineConfig config;
    config.game = "smoke";
    config.platform = "null";
    config.audio = "null";
    config.network = "null";
    config.scripting = "null";
    config.root = os::executableDir() / "smoke";
    config.log.file.clear();
    config.log.color = false;
    config.log.level = LogLevel::Error;
    config.maxTicks = 1000;
    return config;
}

}

TEST_CASE("a headless client runs the game until it asks to quit", "[engine]") {
    Engine engine(headless());
    CHECK(engine.run() == 0);
    CHECK(engine.time().tick == 60);
    CHECK(engine.time().total > 0.99);
}

TEST_CASE("the tick limit stops the engine", "[engine]") {
    EngineConfig config = headless();
    config.maxTicks = 10;
    Engine engine(std::move(config));
    CHECK(engine.run() == 0);
    CHECK(engine.time().tick == 10);
}

TEST_CASE("a headless server runs the same world", "[engine]") {
    EngineConfig config = headless();
    config.role = Role::Server;
    config.platform = "sfml";
    config.audio = "miniaudio";
    config.net.port = 0;
    Engine engine(std::move(config));
    CHECK(engine.run() == 0);
    CHECK(engine.time().tick == 60);
}

TEST_CASE("each boot failure has its exit code", "[engine]") {
    EngineConfig plugin = headless();
    plugin.platform = "nothing";
    CHECK(Engine(std::move(plugin)).run() == 2);

    EngineConfig game = headless();
    game.game = "nothing";
    CHECK(Engine(std::move(game)).run() == 3);

    EngineConfig noGame = headless();
    noGame.game.clear();
    CHECK(Engine(std::move(noGame)).run() == 3);

    EngineConfig scene = headless();
    scene.sceneOverride = "scenes/missing.yaml";
    CHECK(Engine(std::move(scene)).run() == 4);
}

TEST_CASE("a system that throws ends the client with code 5", "[engine]") {
    EngineConfig config = headless();
    config.sceneOverride = "scenes/still.yaml";
    Engine engine(std::move(config));
    CHECK(engine.run() == 5);
    CHECK(engine.time().tick == 60);
}
