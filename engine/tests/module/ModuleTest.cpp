/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** ModuleTest
*/

#include "engine/module/Export.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

using namespace engine;

#if defined(ENGINE_STATIC_MODULES)

namespace {

IGameModule* createGame() {
    const GameEntry* entry = StaticModuleTable::findGame("game_fixture");
    REQUIRE(entry != nullptr);
    return entry->create();
}

void destroyGame(IGameModule* game) {
    StaticModuleTable::findGame("game_fixture")->destroy(game);
}

IPlugin* createPlugin() {
    const PluginEntry* entry = StaticModuleTable::findPlugin("network_fixture");
    REQUIRE(entry != nullptr);
    return entry->create();
}

void destroyPlugin(IPlugin* plugin) {
    StaticModuleTable::findPlugin("network_fixture")->destroy(plugin);
}

uint32_t gameVersion() {
    return ENGINE_GAME_API_VERSION;
}

uint32_t pluginVersion() {
    return ENGINE_PLUGIN_API_VERSION;
}

}

TEST_CASE("unknown modules are not in the static table", "[module]") {
    CHECK(StaticModuleTable::findGame("nothing") == nullptr);
    CHECK(StaticModuleTable::findPlugin("platform_nothing") == nullptr);
}

#else

extern "C" IGameModule* create_game() noexcept;
extern "C" void destroy_game(IGameModule*) noexcept;
extern "C" uint32_t game_api_version() noexcept;
extern "C" IPlugin* create_plugin() noexcept;
extern "C" void destroy_plugin(IPlugin*) noexcept;
extern "C" uint32_t plugin_api_version() noexcept;

namespace {

IGameModule* createGame() {
    return create_game();
}

void destroyGame(IGameModule* game) {
    destroy_game(game);
}

IPlugin* createPlugin() {
    return create_plugin();
}

void destroyPlugin(IPlugin* plugin) {
    destroy_plugin(plugin);
}

uint32_t gameVersion() {
    return game_api_version();
}

uint32_t pluginVersion() {
    return plugin_api_version();
}

}

#endif

TEST_CASE("a game module is created, used and destroyed through its factory", "[module]") {
    IGameModule* game = createGame();
    REQUIRE(game != nullptr);
    CHECK(game->firstScene() == "scenes/fixture.scene.yaml");
    CHECK(gameVersion() == ENGINE_GAME_API_VERSION);
    destroyGame(game);
}

TEST_CASE("a plugin is created, used and destroyed through its factory", "[module]") {
    IPlugin* plugin = createPlugin();
    REQUIRE(plugin != nullptr);
    CHECK(plugin->kind() == PluginKind::Network);
    CHECK(plugin->name() == "fixture");
    CHECK(plugin->apiVersion() == ENGINE_PLUGIN_API_VERSION);
    CHECK(pluginVersion() == ENGINE_PLUGIN_API_VERSION);
    plugin->shutdown();
    destroyPlugin(plugin);
}
