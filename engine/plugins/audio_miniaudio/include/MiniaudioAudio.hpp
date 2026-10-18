/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** MiniaudioAudio
*/

#pragma once

#include "engine/audio/IAudio.hpp"

#include <miniaudio.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

class Logger;

class MiniaudioAudio final : public IAudio {
private:
    struct Clip {
        std::string name;
        std::vector<std::byte> bytes;
    };

    struct Voice {
        std::unique_ptr<ma_sound> sound;
        uint32_t clip = 0;
    };

    struct Track {
        std::unique_ptr<ma_sound> sound;
    };

    bool _withDevice;
    bool _silent = false;
    std::unique_ptr<ma_engine> _engine;
    std::array<std::unique_ptr<ma_sound_group>, 4> _buses;
    std::array<float, 4> _volumes{1.f, 1.f, 1.f, 1.f};
    std::unordered_map<uint32_t, Clip> _clips;
    std::unordered_map<VoiceId, Voice> _voices;
    std::unordered_map<uint32_t, Track> _tracks;
    uint32_t _nextClip = 1;
    uint32_t _nextTrack = 1;
    VoiceId _nextVoice = 1;
    uint32_t _music = 0;
    float _hearingRadius = 1000.f;
    Logger* _log = nullptr;

    Result<void> start(bool withDevice);
    ma_sound_group* group(AudioBus bus);
    void configure(ma_sound& sound, const PlayParams& params);
    void release(Voice& voice);
    void releaseVoicesOf(uint32_t clip);

public:
    explicit MiniaudioAudio(bool withDevice = true);
    MiniaudioAudio(const MiniaudioAudio&) = delete;
    MiniaudioAudio& operator=(const MiniaudioAudio&) = delete;
    ~MiniaudioAudio() override;

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

    bool silent() const;
    size_t activeVoices() const;
    bool musicPlaying() const;
    void advance(uint32_t frames);
};

}
