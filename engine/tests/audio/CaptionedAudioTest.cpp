/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CaptionedAudioTest
*/

#include "audio/AudioCaptions.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

using namespace engine;

namespace {

class FakeAudio final : public IAudio {
public:
    std::vector<SoundHandle> played;
    float music = 1.f;

    PluginKind kind() const override { return PluginKind::Audio; }
    std::string_view name() const override { return "fake"; }
    uint32_t apiVersion() const override { return 1; }
    Result<void> init(const PluginContext&) override { return {}; }
    void shutdown() noexcept override {}
    Result<SoundHandle> createSound(std::span<const std::byte>, std::string_view) override { return SoundHandle{1}; }
    void destroySound(SoundHandle) override {}
    Result<MusicHandle> openMusic(const std::filesystem::path&) override { return MusicHandle{1}; }
    void closeMusic(MusicHandle) override {}
    VoiceId play(SoundHandle sound, const PlayParams&) override {
        this->played.push_back(sound);
        return static_cast<VoiceId>(this->played.size());
    }
    void stop(VoiceId) override {}
    void setVoiceVolume(VoiceId, float) override {}
    void playMusic(MusicHandle, float) override {}
    void stopMusic(float) override {}
    void setBusVolume(AudioBus, float volume) override { this->music = volume; }
    float busVolume(AudioBus) const override { return this->music; }
    void setListener(Vec2, float) override {}
    void update() override {}
};

}

TEST_CASE("a sound with a caption publishes a Caption event", "[audio]") {
    FakeAudio backend;
    EventBus events;
    CaptionedAudio audio(backend, events);
    std::vector<std::string> captions;
    const Subscription sub = events.subscribe<Caption>([&](const Caption& c) { captions.push_back(c.text); });

    PlayParams params;
    params.caption = "laser shot";
    CHECK(audio.play(SoundHandle{7}, params) == 1);
    CHECK(audio.play(SoundHandle{8}, {}) == 2);
    events.dispatch();

    CHECK(captions == std::vector<std::string>{"laser shot"});
    CHECK(backend.played == std::vector<SoundHandle>{SoundHandle{7}, SoundHandle{8}});
}

TEST_CASE("the caption wrapper forwards everything else to the plugin", "[audio]") {
    FakeAudio backend;
    EventBus events;
    CaptionedAudio audio(backend, events);
    IAudio& api = audio;
    CHECK(api.kind() == PluginKind::Audio);
    CHECK(api.name() == "fake");
    CHECK(api.createSound({}, "boom")->valid());
    api.setBusVolume(AudioBus::Music, 2.f);
    CHECK(api.busVolume(AudioBus::Music) == 1.f);
    api.setBusVolume(AudioBus::Music, 0.25f);
    CHECK(backend.music == 0.25f);
}
