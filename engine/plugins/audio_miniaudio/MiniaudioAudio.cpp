/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** MiniaudioAudio
*/

#include "MiniaudioAudio.hpp"

#include "engine/module/Export.hpp"
#include "engine/services/Logger.hpp"

#include <algorithm>
#include <format>
#include <string>
#include <utility>

namespace engine {

namespace {

constexpr ma_uint32 ClipFlags = MA_SOUND_FLAG_DECODE;
constexpr ma_uint32 TrackFlags = MA_SOUND_FLAG_STREAM;

size_t indexOf(AudioBus bus) {
    return static_cast<size_t>(bus);
}

float clampVolume(float volume) {
    return std::clamp(volume, 0.f, 1.f);
}

ma_uint64 milliseconds(float seconds) {
    return static_cast<ma_uint64>(std::max(0.f, seconds) * 1000.f);
}

ma_result openFile(ma_engine* engine, const std::filesystem::path& file, ma_uint32 flags, ma_sound_group* group,
                   ma_sound* sound) {
#if defined(_WIN32)
    return ma_sound_init_from_file_w(engine, file.wstring().c_str(), flags, group, nullptr, sound);
#else
    return ma_sound_init_from_file(engine, file.string().c_str(), flags, group, nullptr, sound);
#endif
}

}

MiniaudioAudio::MiniaudioAudio(bool withDevice) : _withDevice(withDevice) {}

MiniaudioAudio::~MiniaudioAudio() {
    this->shutdown();
}

PluginKind MiniaudioAudio::kind() const {
    return PluginKind::Audio;
}

std::string_view MiniaudioAudio::name() const {
    return "miniaudio";
}

uint32_t MiniaudioAudio::apiVersion() const {
    return ENGINE_PLUGIN_API_VERSION;
}

Result<void> MiniaudioAudio::start(bool withDevice) {
    ma_engine_config config = ma_engine_config_init();
    config.noDevice = withDevice ? MA_FALSE : MA_TRUE;
    if (!withDevice) {
        config.channels = 2;
        config.sampleRate = 48000;
    }
    auto engine = std::make_unique<ma_engine>();
    if (ma_engine_init(&config, engine.get()) != MA_SUCCESS)
        return fail(Errc::PluginNotFound, "miniaudio could not start");
    this->_engine = std::move(engine);
    for (auto& bus : this->_buses) {
        auto made = std::make_unique<ma_sound_group>();
        ma_sound_group* parent = this->_buses[0] ? this->_buses[0].get() : nullptr;
        ma_sound_group_init(this->_engine.get(), 0, parent, made.get());
        bus = std::move(made);
    }
    return {};
}

Result<void> MiniaudioAudio::init(const PluginContext& ctx) {
    this->_log = &ctx.log;
    if (this->_engine) return {};
    if (this->start(this->_withDevice)) return {};
    this->_log->warn("no audio device: sound is disabled");
    this->_silent = true;
    return this->start(false);
}

void MiniaudioAudio::shutdown() noexcept {
    if (!this->_engine) return;
    for (auto& [id, voice] : this->_voices)
        this->release(voice);
    this->_voices.clear();
    for (auto& [id, track] : this->_tracks)
        ma_sound_uninit(track.sound.get());
    this->_tracks.clear();
    for (auto it = this->_buses.rbegin(); it != this->_buses.rend(); ++it)
        if (*it) ma_sound_group_uninit(it->get());
    for (auto& [id, clip] : this->_clips)
        ma_resource_manager_unregister_data(ma_engine_get_resource_manager(this->_engine.get()), clip.name.c_str());
    this->_clips.clear();
    ma_engine_uninit(this->_engine.get());
    this->_engine.reset();
    for (auto& bus : this->_buses)
        bus.reset();
    this->_music = 0;
}

ma_sound_group* MiniaudioAudio::group(AudioBus bus) {
    return this->_buses[indexOf(bus)].get();
}

Result<SoundHandle> MiniaudioAudio::createSound(std::span<const std::byte> encoded, std::string_view debugName) {
    if (!this->_engine) return fail(Errc::AssetLoadFailed, "audio is not initialised");
    ma_decoder probe;
    if (encoded.empty() || ma_decoder_init_memory(encoded.data(), encoded.size(), nullptr, &probe) != MA_SUCCESS)
        return fail(Errc::AssetLoadFailed, std::format("cannot decode sound '{}'", debugName));
    ma_decoder_uninit(&probe);
    const uint32_t id = this->_nextClip++;
    Clip clip{std::format("engine-sound-{}", id), {encoded.begin(), encoded.end()}};
    ma_resource_manager_register_encoded_data(ma_engine_get_resource_manager(this->_engine.get()), clip.name.c_str(),
                                              clip.bytes.data(), clip.bytes.size());
    this->_clips.emplace(id, std::move(clip));
    return SoundHandle{id};
}

void MiniaudioAudio::destroySound(SoundHandle sound) {
    const auto it = this->_clips.find(sound.id);
    if (it == this->_clips.end()) return;
    this->releaseVoicesOf(sound.id);
    ma_resource_manager_unregister_data(ma_engine_get_resource_manager(this->_engine.get()), it->second.name.c_str());
    this->_clips.erase(it);
}

Result<MusicHandle> MiniaudioAudio::openMusic(const std::filesystem::path& file) {
    if (!this->_engine) return fail(Errc::AssetLoadFailed, "audio is not initialised");
    auto sound = std::make_unique<ma_sound>();
    if (openFile(this->_engine.get(), file, TrackFlags, this->group(AudioBus::Music), sound.get()) != MA_SUCCESS)
        return fail(Errc::AssetLoadFailed, std::format("cannot open music '{}'", file.generic_string()));
    ma_sound_set_looping(sound.get(), MA_TRUE);
    const uint32_t id = this->_nextTrack++;
    this->_tracks.emplace(id, Track{std::move(sound)});
    return MusicHandle{id};
}

void MiniaudioAudio::closeMusic(MusicHandle music) {
    const auto it = this->_tracks.find(music.id);
    if (it == this->_tracks.end()) return;
    ma_sound_uninit(it->second.sound.get());
    this->_tracks.erase(it);
    if (this->_music == music.id) this->_music = 0;
}

void MiniaudioAudio::configure(ma_sound& sound, const PlayParams& params) {
    ma_sound_set_volume(&sound, clampVolume(params.volume));
    ma_sound_set_pitch(&sound, std::max(0.01f, params.pitch));
    ma_sound_set_looping(&sound, params.loop ? MA_TRUE : MA_FALSE);
    if (!params.position) {
        ma_sound_set_spatialization_enabled(&sound, MA_FALSE);
        ma_sound_set_pan(&sound, std::clamp(params.pan, -1.f, 1.f));
        return;
    }
    ma_sound_set_spatialization_enabled(&sound, MA_TRUE);
    ma_sound_set_attenuation_model(&sound, ma_attenuation_model_linear);
    ma_sound_set_min_distance(&sound, 1.f);
    ma_sound_set_max_distance(&sound, this->_hearingRadius);
    ma_sound_set_position(&sound, params.position->x, params.position->y, 0.f);
}

VoiceId MiniaudioAudio::play(SoundHandle sound, const PlayParams& params) {
    const auto clip = this->_clips.find(sound.id);
    if (this->_silent || clip == this->_clips.end()) return 0;
    auto voice = std::make_unique<ma_sound>();
    if (ma_sound_init_from_file(this->_engine.get(), clip->second.name.c_str(), ClipFlags, this->group(params.bus),
                                nullptr, voice.get()) != MA_SUCCESS)
        return 0;
    this->configure(*voice, params);
    ma_sound_start(voice.get());
    const VoiceId id = this->_nextVoice++;
    this->_voices.emplace(id, Voice{std::move(voice), sound.id});
    return id;
}

void MiniaudioAudio::release(Voice& voice) {
    if (!voice.sound) return;
    ma_sound_uninit(voice.sound.get());
    voice.sound.reset();
}

void MiniaudioAudio::releaseVoicesOf(uint32_t clip) {
    std::erase_if(this->_voices, [&](auto& entry) {
        if (entry.second.clip != clip) return false;
        this->release(entry.second);
        return true;
    });
}

void MiniaudioAudio::stop(VoiceId voice) {
    const auto it = this->_voices.find(voice);
    if (it == this->_voices.end()) return;
    this->release(it->second);
    this->_voices.erase(it);
}

void MiniaudioAudio::setVoiceVolume(VoiceId voice, float volume) {
    const auto it = this->_voices.find(voice);
    if (it != this->_voices.end()) ma_sound_set_volume(it->second.sound.get(), clampVolume(volume));
}

void MiniaudioAudio::playMusic(MusicHandle music, float fadeInSeconds) {
    const auto it = this->_tracks.find(music.id);
    if (this->_silent || it == this->_tracks.end()) return;
    if (this->_music != 0 && this->_music != music.id) this->stopMusic(0.f);
    ma_sound* sound = it->second.sound.get();
    ma_sound_reset_stop_time_and_fade(sound);
    ma_sound_seek_to_pcm_frame(sound, 0);
    ma_sound_set_fade_in_milliseconds(sound, fadeInSeconds > 0.f ? 0.f : 1.f, 1.f, milliseconds(fadeInSeconds));
    ma_sound_start(sound);
    this->_music = music.id;
}

void MiniaudioAudio::stopMusic(float fadeOutSeconds) {
    const auto it = this->_tracks.find(this->_music);
    this->_music = 0;
    if (it == this->_tracks.end()) return;
    if (fadeOutSeconds > 0.f)
        ma_sound_stop_with_fade_in_milliseconds(it->second.sound.get(), milliseconds(fadeOutSeconds));
    else
        ma_sound_stop(it->second.sound.get());
}

void MiniaudioAudio::setBusVolume(AudioBus bus, float volume) {
    const size_t index = indexOf(bus);
    if (index >= this->_volumes.size()) return;
    this->_volumes[index] = clampVolume(volume);
    if (this->_buses[index]) ma_sound_group_set_volume(this->_buses[index].get(), this->_volumes[index]);
}

float MiniaudioAudio::busVolume(AudioBus bus) const {
    const size_t index = indexOf(bus);
    return index < this->_volumes.size() ? this->_volumes[index] : 1.f;
}

void MiniaudioAudio::setListener(Vec2 position, float hearingRadius) {
    if (hearingRadius > 0.f) this->_hearingRadius = hearingRadius;
    if (this->_engine) ma_engine_listener_set_position(this->_engine.get(), 0, position.x, position.y, 0.f);
}

void MiniaudioAudio::update() {
    std::erase_if(this->_voices, [&](auto& entry) {
        ma_sound* sound = entry.second.sound.get();
        if (ma_sound_is_looping(sound) || !ma_sound_at_end(sound)) return false;
        this->release(entry.second);
        return true;
    });
}

bool MiniaudioAudio::silent() const {
    return this->_silent;
}

size_t MiniaudioAudio::activeVoices() const {
    return this->_voices.size();
}

bool MiniaudioAudio::musicPlaying() const {
    const auto it = this->_tracks.find(this->_music);
    return it != this->_tracks.end() && ma_sound_is_playing(it->second.sound.get());
}

void MiniaudioAudio::advance(uint32_t frames) {
    if (!this->_engine || this->_withDevice) return;
    std::vector<float> buffer(static_cast<size_t>(frames) * ma_engine_get_channels(this->_engine.get()));
    ma_engine_read_pcm_frames(this->_engine.get(), buffer.data(), frames, nullptr);
}

}

ENGINE_PLUGIN(engine::MiniaudioAudio)
