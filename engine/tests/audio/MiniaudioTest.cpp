/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** MiniaudioTest
*/

#include "MiniaudioAudio.hpp"
#include "engine/EngineConfig.hpp"
#include "engine/events/EventBus.hpp"
#include "engine/services/CVars.hpp"
#include "engine/services/Logger.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <vector>

using namespace engine;

namespace {

constexpr uint32_t Rate = 48000;

void put16(std::vector<std::byte>& out, uint16_t v) {
    out.push_back(static_cast<std::byte>(v & 0xFF));
    out.push_back(static_cast<std::byte>(v >> 8));
}

void put32(std::vector<std::byte>& out, uint32_t v) {
    put16(out, static_cast<uint16_t>(v & 0xFFFF));
    put16(out, static_cast<uint16_t>(v >> 16));
}

void putTag(std::vector<std::byte>& out, const char (&tag)[5]) {
    for (int i = 0; i < 4; ++i)
        out.push_back(static_cast<std::byte>(tag[i]));
}

std::vector<std::byte> beep(uint32_t frames) {
    std::vector<std::byte> out;
    putTag(out, "RIFF");
    put32(out, 36 + frames * 2);
    putTag(out, "WAVE");
    putTag(out, "fmt ");
    put32(out, 16);
    put16(out, 1);
    put16(out, 1);
    put32(out, Rate);
    put32(out, Rate * 2);
    put16(out, 2);
    put16(out, 16);
    putTag(out, "data");
    put32(out, frames * 2);
    for (uint32_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i) / Rate;
        const auto sample = static_cast<int16_t>(8000 * std::sin(2 * std::numbers::pi * 440 * t));
        put16(out, static_cast<uint16_t>(sample));
    }
    return out;
}

struct Audio {
    Logger log{[] {
        LogConfig settings;
        settings.file.clear();
        settings.color = false;
        settings.level = LogLevel::Error;
        return settings;
    }()};
    CVars cvars;
    EngineConfig config;
    EventBus events;
    MiniaudioAudio audio{false};

    Audio() { REQUIRE(this->audio.init(PluginContext{this->log, this->cvars, this->config, this->events})); }
};

}

TEST_CASE("sounds decode from memory and refuse bad bytes", "[audio][miniaudio]") {
    Audio a;
    CHECK(a.audio.name() == "miniaudio");
    CHECK(a.audio.kind() == PluginKind::Audio);
    CHECK_FALSE(a.audio.silent());
    const auto shot = a.audio.createSound(beep(Rate / 10), "shot");
    REQUIRE(shot);
    CHECK(shot->valid());
    const std::vector<std::byte> junk(64, std::byte{7});
    const auto bad = a.audio.createSound(junk, "junk");
    REQUIRE_FALSE(bad);
    CHECK(bad.error().code == Errc::AssetLoadFailed);
    CHECK_FALSE(a.audio.createSound({}, "empty"));
}

TEST_CASE("a played sound ends and its voice is released", "[audio][miniaudio]") {
    Audio a;
    const auto shot = a.audio.createSound(beep(Rate / 10), "shot");
    REQUIRE(shot);
    const VoiceId voice = a.audio.play(*shot, PlayParams{});
    CHECK(voice != 0);
    CHECK(a.audio.activeVoices() == 1);
    a.audio.update();
    CHECK(a.audio.activeVoices() == 1);
    a.audio.advance(Rate / 5);
    a.audio.update();
    CHECK(a.audio.activeVoices() == 0);
    CHECK(a.audio.play(SoundHandle{999}, PlayParams{}) == 0);
}

TEST_CASE("looping voices keep playing until stopped", "[audio][miniaudio]") {
    Audio a;
    const auto hum = a.audio.createSound(beep(Rate / 20), "hum");
    REQUIRE(hum);
    PlayParams params;
    params.loop = true;
    params.bus = AudioBus::Interface;
    params.pan = -0.5f;
    const VoiceId voice = a.audio.play(*hum, params);
    a.audio.advance(Rate / 2);
    a.audio.update();
    CHECK(a.audio.activeVoices() == 1);
    a.audio.setVoiceVolume(voice, 0.3f);
    a.audio.stop(voice);
    CHECK(a.audio.activeVoices() == 0);
    a.audio.stop(voice);
}

TEST_CASE("positional voices and destroying a sound stop its voices", "[audio][miniaudio]") {
    Audio a;
    const auto boom = a.audio.createSound(beep(Rate), "boom");
    REQUIRE(boom);
    a.audio.setListener({640, 360}, 800);
    PlayParams params;
    params.position = Vec2{900, 360};
    a.audio.play(*boom, params);
    a.audio.play(*boom, PlayParams{});
    a.audio.advance(256);
    CHECK(a.audio.activeVoices() == 2);
    a.audio.destroySound(*boom);
    CHECK(a.audio.activeVoices() == 0);
    CHECK(a.audio.play(*boom, PlayParams{}) == 0);
}

TEST_CASE("bus volumes are clamped and read back", "[audio][miniaudio]") {
    Audio a;
    CHECK(a.audio.busVolume(AudioBus::Music) == 1.f);
    a.audio.setBusVolume(AudioBus::Music, 0.25f);
    CHECK(a.audio.busVolume(AudioBus::Music) == 0.25f);
    a.audio.setBusVolume(AudioBus::Effects, 3.f);
    CHECK(a.audio.busVolume(AudioBus::Effects) == 1.f);
    a.audio.setBusVolume(AudioBus::Master, -1.f);
    CHECK(a.audio.busVolume(AudioBus::Master) == 0.f);
}

TEST_CASE("music streams from a file, fades and stops", "[audio][miniaudio]") {
    Audio a;
    const auto file = std::filesystem::temp_directory_path() / "rtype-music-test.wav";
    {
        const auto bytes = beep(Rate);
        std::ofstream(file, std::ios::binary)
            .write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    const auto music = a.audio.openMusic(file);
    REQUIRE(music);
    CHECK_FALSE(a.audio.openMusic(file.parent_path() / "rtype-missing-music.wav"));
    a.audio.playMusic(*music, 0.5f);
    a.audio.advance(Rate / 10);
    CHECK(a.audio.musicPlaying());
    a.audio.stopMusic(0.f);
    CHECK_FALSE(a.audio.musicPlaying());
    a.audio.playMusic(*music, 0.f);
    CHECK(a.audio.musicPlaying());
    a.audio.closeMusic(*music);
    CHECK_FALSE(a.audio.musicPlaying());
    a.audio.shutdown();
    std::filesystem::remove(file);
}
