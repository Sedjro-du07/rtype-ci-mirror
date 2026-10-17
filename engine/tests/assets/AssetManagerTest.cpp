/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AssetManagerTest
*/

#include "engine/assets/AssetManager.hpp"

#include "engine/audio/IAudio.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/script/IScripting.hpp"
#include "engine/services/Logger.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace engine;
namespace fs = std::filesystem;

namespace {

class FakeRenderer final : public IRenderer {
public:
    uint32_t next = 1;
    std::vector<std::string> created;
    std::set<uint32_t> destroyed;
    std::vector<std::byte> lastBytes;

    Result<TextureHandle> createTexture(std::span<const std::byte> bytes, std::string_view name) override {
        if (bytes.empty()) return fail(Errc::AssetLoadFailed, "empty image");
        this->lastBytes.assign(bytes.begin(), bytes.end());
        this->created.emplace_back(name);
        return TextureHandle{this->next++};
    }
    void destroyTexture(TextureHandle t) override { this->destroyed.insert(t.id); }
    Vec2 textureSize(TextureHandle) const override { return {}; }
    Result<FontHandle> createFont(std::span<const std::byte>, std::string_view) override {
        return FontHandle{this->next++};
    }
    void destroyFont(FontHandle) override {}
    void submit(const RenderFrame&) override {}
    bool supports(Feature) const override { return false; }
    void setColorFilter(ColorFilter) override {}
    RenderStats stats() const override { return {}; }
};

class FakeAudio final : public IAudio {
public:
    std::vector<fs::path> musics;

    PluginKind kind() const override { return PluginKind::Audio; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<SoundHandle> createSound(std::span<const std::byte>, std::string_view) override { return SoundHandle{9}; }
    void destroySound(SoundHandle) override {}
    Result<MusicHandle> openMusic(const fs::path& file) override {
        this->musics.push_back(file);
        return MusicHandle{4};
    }
    void closeMusic(MusicHandle) override {}
    VoiceId play(SoundHandle, const PlayParams&) override { return 0; }
    void stop(VoiceId) override {}
    void setVoiceVolume(VoiceId, float) override {}
    void playMusic(MusicHandle, float) override {}
    void stopMusic(float) override {}
    void setBusVolume(AudioBus, float) override {}
    float busVolume(AudioBus) const override { return 1; }
    void setListener(Vec2, float) override {}
    void update() override {}
};

class FakeScripting final : public IScripting {
public:
    int reloads = 0;

    PluginKind kind() const override { return PluginKind::Scripting; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<void> bind(ScriptHost&) override { return {}; }
    Result<ScriptHandle> load(const fs::path&) override { return ScriptHandle{5}; }
    void unload(ScriptHandle) override {}
    Result<void> reload(ScriptHandle) override {
        ++this->reloads;
        return {};
    }
    Result<void> call(ScriptHandle, std::string_view, Entity, Context&) override { return {}; }
    bool has(ScriptHandle, std::string_view) const override { return false; }
};

class CountingSink final : public ILogSink {
public:
    int errors = 0;
    void write(const LogRecord& record) override {
        if (record.level == LogLevel::Error) ++this->errors;
    }
};

struct Fixture {
    fs::path root = fs::temp_directory_path() / "rtype-assets-test";
    Logger log{[] {
        LogConfig config;
        config.file.clear();
        config.color = false;
        config.level = LogLevel::Warn;
        return config;
    }()};
    std::shared_ptr<CountingSink> sink = std::make_shared<CountingSink>();
    FakeRenderer renderer;
    FakeAudio audio;
    FakeScripting scripting;
    AssetManager assets{log, root};

    Fixture() {
        fs::remove_all(this->root);
        fs::create_directories(this->root / "sprites");
        fs::create_directories(this->root / "scripts");
        this->write("sprites/ship.png", "png bytes");
        this->write("sprites/empty.png", "");
        this->write("sounds.ogg", "ogg bytes");
        this->write("music.ogg", "ogg bytes");
        this->write("font.ttf", "ttf bytes");
        this->write("scripts/boss.lua", "return {}");
        this->log.addSink(this->sink);
        this->assets.attach(this->renderer, this->audio, this->scripting);
    }
    ~Fixture() { fs::remove_all(this->root); }

    void write(const std::string& path, const std::string& content) {
        std::ofstream(this->root / path, std::ios::binary) << content;
    }
};

}

TEST_CASE("a texture is loaded once and found again by path", "[assets]") {
    Fixture f;
    const TextureHandle ship = f.assets.texture("sprites/ship.png");
    CHECK(ship.valid());
    CHECK(f.assets.texture("sprites/ship.png") == ship);
    CHECK(f.renderer.created.size() == 1);
    CHECK(f.assets.pathOf(ship) == "sprites/ship.png");
    CHECK(f.assets.stats().textures == 1);
    CHECK(f.assets.stats().bytes == 9);
}

TEST_CASE("a missing texture gives the checkerboard and logs once", "[assets]") {
    Fixture f;
    const TextureHandle first = f.assets.texture("sprites/nope.png");
    const TextureHandle second = f.assets.texture("sprites/nope.png");
    const TextureHandle broken = f.assets.texture("sprites/empty.png");
    CHECK(first.valid());
    CHECK(first == second);
    CHECK(broken == first);
    CHECK(f.sink->errors == 2);
    REQUIRE(f.renderer.lastBytes.size() >= 8);
    CHECK(f.renderer.lastBytes[1] == std::byte{'P'});
    CHECK(f.assets.stats().failed == 2);
}

TEST_CASE("paths stay inside the game root and match the case on disk", "[assets]") {
    Fixture f;
    CHECK(f.assets.resolve("sprites/ship.png") == f.root / "sprites" / "ship.png");
    CHECK(f.assets.resolve("../outside.png").empty());
    CHECK(f.assets.resolve("sprites/../../outside.png").empty());
    CHECK(f.assets.resolve((f.root / "sprites/ship.png").string()).empty());
    CHECK(f.assets.resolve("sprites/Ship.png").empty());
    CHECK(f.assets.resolve("Sprites/ship.png").empty());
    CHECK_FALSE(f.assets.resolve("sprites/not-there.png").empty());
}

TEST_CASE("sounds, music, fonts and scripts", "[assets]") {
    Fixture f;
    CHECK(f.assets.sound("sounds.ogg") == SoundHandle{9});
    CHECK_FALSE(f.assets.sound("missing.ogg").valid());
    CHECK(f.assets.music("music.ogg") == MusicHandle{4});
    REQUIRE(f.audio.musics.size() == 1);
    CHECK(f.audio.musics[0] == f.root / "music.ogg");
    CHECK(f.assets.font("font.ttf").valid());
    CHECK_FALSE(f.assets.font("missing.ttf").valid());
    CHECK(f.assets.script("scripts/boss.lua") == ScriptHandle{5});
    CHECK(f.assets.pathOf(ScriptHandle{5}) == "scripts/boss.lua");
}

TEST_CASE("collectUnused frees what nobody retains", "[assets]") {
    Fixture f;
    const TextureHandle kept = f.assets.texture("sprites/ship.png");
    f.write("sprites/bullet.png", "png");
    const TextureHandle dropped = f.assets.texture("sprites/bullet.png");
    const TextureHandle missing = f.assets.texture("sprites/nope.png");
    f.assets.retain(kept);
    f.assets.retain(dropped);
    f.assets.release(dropped);

    f.assets.collectUnused();
    CHECK(f.renderer.destroyed.contains(dropped.id));
    CHECK_FALSE(f.renderer.destroyed.contains(kept.id));
    CHECK_FALSE(f.renderer.destroyed.contains(missing.id));
    CHECK(f.assets.pathOf(dropped).empty());
    CHECK(f.assets.texture("sprites/ship.png") == kept);
}

TEST_CASE("changed scripts are reloaded", "[assets]") {
    Fixture f;
    f.assets.script("scripts/boss.lua");
    f.assets.reloadChanged();
    CHECK(f.scripting.reloads == 0);
    const auto file = f.root / "scripts/boss.lua";
    fs::last_write_time(file, fs::last_write_time(file) + std::chrono::seconds(5));
    f.assets.reloadChanged();
    CHECK(f.scripting.reloads == 1);
    f.assets.reloadChanged();
    CHECK(f.scripting.reloads == 1);
}
