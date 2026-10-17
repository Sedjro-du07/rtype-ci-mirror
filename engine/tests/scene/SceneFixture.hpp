/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** SceneFixture
*/

#pragma once

#include "engine/assets/AssetManager.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/components/Hierarchy.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/reflection/TypeRegistry.hpp"
#include "engine/render/IRenderer.hpp"
#include "engine/script/IScripting.hpp"
#include "engine/services/Logger.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace scenetest {

using namespace engine;

enum class Kind : uint8_t { Grunt, Boss = 200 };

struct Stats {
    int32_t power = 1;
    std::string label;
};

struct Everything {
    bool flag = false;
    int8_t i8 = 0;
    uint8_t u8 = 0;
    int16_t i16 = 0;
    uint16_t u16 = 0;
    int32_t i32 = 0;
    uint32_t u32 = 0;
    int64_t i64 = 0;
    float f = 0;
    double d = 0;
    std::string text;
    Vec2 v;
    Rect r;
    Color c;
    Entity target;
    TextureHandle texture;
    Kind kind = Kind::Grunt;
    std::vector<int32_t> waves;
    std::vector<Stats> stats;
    std::vector<Kind> kinds;
    Stats main;
};

struct Position {
    float x = 0, y = 0;
};

struct Health {
    int32_t hp = 1;
    int32_t maxHp = 1;
};

struct Sprite {
    TextureHandle texture;
    int32_t layer = 0;
};

}

ENGINE_ENUM(scenetest::Kind, Grunt, Boss)
ENGINE_STRUCT(scenetest::Stats)
ENGINE_COMPONENT(scenetest::Everything)
ENGINE_COMPONENT(scenetest::Position)
ENGINE_COMPONENT(scenetest::Health)
ENGINE_COMPONENT(scenetest::Sprite)

namespace scenetest {

using namespace engine;

class FakeRenderer final : public IRenderer {
public:
    uint32_t next = 1;

    Result<TextureHandle> createTexture(std::span<const std::byte>, std::string_view) override {
        return TextureHandle{this->next++};
    }
    void destroyTexture(TextureHandle) override {}
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
    PluginKind kind() const override { return PluginKind::Audio; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<SoundHandle> createSound(std::span<const std::byte>, std::string_view) override { return SoundHandle{1}; }
    void destroySound(SoundHandle) override {}
    Result<MusicHandle> openMusic(const std::filesystem::path&) override { return MusicHandle{1}; }
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
    PluginKind kind() const override { return PluginKind::Scripting; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<void> bind(ScriptHost&) override { return {}; }
    Result<ScriptHandle> load(const std::filesystem::path&) override { return ScriptHandle{1}; }
    void unload(ScriptHandle) override {}
    Result<void> reload(ScriptHandle) override { return {}; }
    Result<void> call(ScriptHandle, std::string_view, Entity, Context&) override { return {}; }
    bool has(ScriptHandle, std::string_view) const override { return false; }
};

struct SceneFixture {
    std::filesystem::path root;
    Logger log{[] {
        LogConfig config;
        config.file.clear();
        config.color = false;
        config.level = LogLevel::Error;
        return config;
    }()};
    FakeRenderer renderer;
    FakeAudio audio;
    FakeScripting scripting;
    TypeRegistry types;
    AssetManager assets;

    explicit SceneFixture(const std::string& folder)
        : root(std::filesystem::temp_directory_path() / folder), assets(log, root) {
        std::filesystem::remove_all(this->root);
        std::filesystem::create_directories(this->root / "prefabs");
        std::filesystem::create_directories(this->root / "assets");
        this->write("assets/ship.png", "png");
        this->assets.attach(this->renderer, this->audio, this->scripting);
        this->types.add<Parent>().field("value", &Parent::value).field("offset", &Parent::offset);
        this->types.add<Children>().field("list", &Children::list);
        this->types.add<Kind>();
        this->types.add<Stats>().field("power", &Stats::power).field("label", &Stats::label);
        this->types.add<Everything>()
            .field("flag", &Everything::flag)
            .field("i8", &Everything::i8)
            .field("u8", &Everything::u8)
            .field("i16", &Everything::i16)
            .field("u16", &Everything::u16)
            .field("i32", &Everything::i32)
            .field("u32", &Everything::u32)
            .field("i64", &Everything::i64)
            .field("f", &Everything::f)
            .field("d", &Everything::d)
            .field("text", &Everything::text)
            .field("v", &Everything::v)
            .field("r", &Everything::r)
            .field("c", &Everything::c)
            .field("target", &Everything::target)
            .field("texture", &Everything::texture)
            .field("kind", &Everything::kind)
            .field("waves", &Everything::waves)
            .maxLength(4)
            .field("stats", &Everything::stats)
            .field("kinds", &Everything::kinds)
            .field("main", &Everything::main);
        this->types.add<Position>().field("x", &Position::x).field("y", &Position::y);
        this->types.add<Health>().field("hp", &Health::hp).field("maxHp", &Health::maxHp);
        this->types.add<Sprite>().field("texture", &Sprite::texture).field("layer", &Sprite::layer);
    }
    ~SceneFixture() { std::filesystem::remove_all(this->root); }

    void write(const std::string& path, const std::string& content) const {
        std::ofstream(this->root / path, std::ios::binary) << content;
    }
};

}
