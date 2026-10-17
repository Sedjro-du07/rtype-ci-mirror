/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** AudioCaptions
*/

#include "AudioCaptions.hpp"

#include "engine/events/EngineEvents.hpp"
#include "engine/events/EventBus.hpp"

#include <algorithm>

namespace engine {

CaptionedAudio::CaptionedAudio(IAudio& backend, EventBus& events) : _backend(backend), _events(events) {}

PluginKind CaptionedAudio::kind() const {
    return this->_backend.kind();
}

std::string_view CaptionedAudio::name() const {
    return this->_backend.name();
}

uint32_t CaptionedAudio::apiVersion() const {
    return this->_backend.apiVersion();
}

Result<void> CaptionedAudio::init(const PluginContext& ctx) {
    return this->_backend.init(ctx);
}

void CaptionedAudio::shutdown() noexcept {
    this->_backend.shutdown();
}

Result<SoundHandle> CaptionedAudio::createSound(std::span<const std::byte> encoded, std::string_view debugName) {
    return this->_backend.createSound(encoded, debugName);
}

void CaptionedAudio::destroySound(SoundHandle sound) {
    this->_backend.destroySound(sound);
}

Result<MusicHandle> CaptionedAudio::openMusic(const std::filesystem::path& file) {
    return this->_backend.openMusic(file);
}

void CaptionedAudio::closeMusic(MusicHandle music) {
    this->_backend.closeMusic(music);
}

VoiceId CaptionedAudio::play(SoundHandle sound, const PlayParams& params) {
    if (!params.caption.empty()) this->_events.publish(Caption{params.caption, 2.f});
    return this->_backend.play(sound, params);
}

void CaptionedAudio::stop(VoiceId voice) {
    this->_backend.stop(voice);
}

void CaptionedAudio::setVoiceVolume(VoiceId voice, float volume) {
    this->_backend.setVoiceVolume(voice, volume);
}

void CaptionedAudio::playMusic(MusicHandle music, float fadeInSeconds) {
    this->_backend.playMusic(music, fadeInSeconds);
}

void CaptionedAudio::stopMusic(float fadeOutSeconds) {
    this->_backend.stopMusic(fadeOutSeconds);
}

void CaptionedAudio::setBusVolume(AudioBus bus, float volume) {
    this->_backend.setBusVolume(bus, std::clamp(volume, 0.f, 1.f));
}

float CaptionedAudio::busVolume(AudioBus bus) const {
    return this->_backend.busVolume(bus);
}

void CaptionedAudio::setListener(Vec2 position, float hearingRadius) {
    this->_backend.setListener(position, hearingRadius);
}

void CaptionedAudio::update() {
    this->_backend.update();
}

}
