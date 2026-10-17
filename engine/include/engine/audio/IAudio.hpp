/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** IAudio
*/

#pragma once

#include "engine/core/Error.hpp"
#include "engine/core/math/Vec2.hpp"
#include "engine/module/IPlugin.hpp"
#include "engine/render/Handles.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace engine {

enum class AudioBus : uint8_t { Master, Music, Effects, Interface };

struct PlayParams {
    float volume = 1.f;
    float pitch = 1.f;
    float pan = 0.f;
    AudioBus bus = AudioBus::Effects;
    std::optional<Vec2> position;
    bool loop = false;
    std::string caption;
};

using VoiceId = uint32_t;

class IAudio : public IPlugin {
public:
    virtual Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) = 0;
    virtual void destroySound(SoundHandle sound) = 0;
    virtual Result<MusicHandle> openMusic(const std::filesystem::path& file) = 0;
    virtual void closeMusic(MusicHandle music) = 0;

    virtual VoiceId play(SoundHandle sound, const PlayParams& params = {}) = 0;
    virtual void stop(VoiceId voice) = 0;
    virtual void setVoiceVolume(VoiceId voice, float volume) = 0;
    virtual void playMusic(MusicHandle music, float fadeInSeconds = 0.f) = 0;
    virtual void stopMusic(float fadeOutSeconds = 0.f) = 0;

    virtual void setBusVolume(AudioBus bus, float volume) = 0;
    virtual float busVolume(AudioBus bus) const = 0;
    virtual void setListener(Vec2 position, float hearingRadius) = 0;
    virtual void update() = 0;
};

}
