/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AudioCaptions
*/

#pragma once

#include "engine/Export.hpp"
#include "engine/audio/IAudio.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string_view>

namespace engine {

class EventBus;

class ENGINE_API CaptionedAudio final : public IAudio {
private:
    IAudio& _backend;
    EventBus& _events;

public:
    CaptionedAudio(IAudio& backend, EventBus& events);

    PluginKind kind() const override;
    std::string_view name() const override;
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;

    Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) override;
    void destroySound(SoundHandle sound) override;
    Result<MusicHandle> openMusic(const std::filesystem::path& file) override;
    void closeMusic(MusicHandle music) override;
    VoiceId play(SoundHandle sound, const PlayParams& params) override;
    void stop(VoiceId voice) override;
    void setVoiceVolume(VoiceId voice, float volume) override;
    void playMusic(MusicHandle music, float fadeInSeconds) override;
    void stopMusic(float fadeOutSeconds) override;
    void setBusVolume(AudioBus bus, float volume) override;
    float busVolume(AudioBus bus) const override;
    void setListener(Vec2 position, float hearingRadius) override;
    void update() override;
};

}
