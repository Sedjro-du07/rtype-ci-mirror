/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** NullPluginsTest
*/

#include "engine/module/NullPlugins.hpp"

#include "engine/module/Export.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

using namespace engine;

TEST_CASE("the null renderer and platform give valid handles and draw nothing", "[module][null]") {
    NullPlatform platform;
    CHECK(platform.kind() == PluginKind::Platform);
    CHECK(platform.apiVersion() == ENGINE_PLUGIN_API_VERSION);
    CHECK_FALSE(platform.windowOpen());
    REQUIRE(platform.openWindow(WindowDesc{}));
    CHECK(platform.windowOpen());
    const std::vector<std::byte> bytes(4);
    const auto first = platform.renderer().createTexture(bytes, "a");
    const auto second = platform.renderer().createTexture(bytes, "b");
    REQUIRE(first);
    REQUIRE(second);
    CHECK(first->valid());
    CHECK(*first != *second);
    CHECK_FALSE(platform.renderer().supports(Feature::Shaders));
    platform.setClipboard("copied");
    CHECK(platform.clipboard() == "copied");
    platform.shutdown();
    CHECK_FALSE(platform.windowOpen());
}

TEST_CASE("null audio, network and scripting do nothing safely", "[module][null]") {
    NullAudio audio;
    const auto sound = audio.createSound({}, "shot");
    REQUIRE(sound);
    CHECK(sound->valid());
    CHECK(audio.play(*sound, {}) == 0);
    audio.setBusVolume(AudioBus::Music, 3.f);
    CHECK(audio.busVolume(AudioBus::Music) == 1.f);
    audio.setBusVolume(AudioBus::Music, 0.25f);
    CHECK(audio.busVolume(AudioBus::Music) == 0.25f);

    NullNetwork network;
    CHECK(network.openUdp(4242));
    network.sendUdp(Endpoint::ipv4(127, 0, 0, 1, 4242), {});
    const auto stream = network.connectTcp(Endpoint::ipv4(127, 0, 0, 1, 4242));
    REQUIRE_FALSE(stream);
    CHECK(stream.error().code == Errc::NetworkError);

    NullScripting scripting;
    const auto script = scripting.load("scripts/boss.lua");
    REQUIRE_FALSE(script);
    CHECK(script.error().code == Errc::ScriptError);
    CHECK_FALSE(scripting.has({}, "update"));

    for (PluginKind kind : {PluginKind::Platform, PluginKind::Audio, PluginKind::Network, PluginKind::Scripting}) {
        const auto plugin = makeNullPlugin(kind);
        CHECK(plugin->kind() == kind);
        CHECK(plugin->name() == "null");
    }
}
