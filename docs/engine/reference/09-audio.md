# 9 · Audio

Status: **Locked**. ADRs 0008, 0032.
Folder: `engine/include/engine/audio/`, `engine/plugins/audio_miniaudio/`.

## Role

Play sounds and music through one plugin for every platform backend, with separate volume buses
and visual captions for accessibility.

---

## `enum class AudioBus` and `struct PlayParams` — `audio/IAudio.hpp`

```cpp
enum class AudioBus : uint8_t { Master, Music, Effects, Interface };

struct PlayParams {
    float               volume = 1.f;          // multiplied by the bus volume and Master
    float               pitch = 1.f;
    float               pan = 0.f;             // -1 left … +1 right; ignored if position is set
    AudioBus            bus = AudioBus::Effects;
    std::optional<Vec2> position;              // world position: pan and attenuation from the listener
    bool                loop = false;
    std::string         caption;               // non-empty: publishes a Caption event (block 6)
};

using VoiceId = uint32_t;                      // 0 = not playing
```

## `class IAudio` — `audio/IAudio.hpp`

```cpp
class IAudio : public IPlugin {
public:
    virtual Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) = 0;
    virtual void   destroySound(SoundHandle) = 0;
    virtual Result<MusicHandle> openMusic(const std::filesystem::path& file) = 0; // streamed from disk
    virtual void   closeMusic(MusicHandle) = 0;

    virtual VoiceId play(SoundHandle s, const PlayParams& p = {}) = 0;   // fire and forget, or keep the id
    virtual void    stop(VoiceId v) = 0;
    virtual void    setVoiceVolume(VoiceId v, float volume) = 0;
    virtual void    playMusic(MusicHandle m, float fadeInSeconds = 0.f) = 0; // replaces the current track
    virtual void    stopMusic(float fadeOutSeconds = 0.f) = 0;

    virtual void    setBusVolume(AudioBus bus, float volume) = 0;        // 0..1, saved through CVars
    virtual float   busVolume(AudioBus bus) const = 0;
    virtual void    setListener(Vec2 position, float hearingRadius) = 0; // usually the camera center
    virtual void    update() = 0;                                        // engine only, once per frame
};
```

Captions are published by the engine wrapper around `play`, not by the plugin, so every backend
gets them.

## Plugin

| Plugin | Implements | Notes |
|---|---|---|
| `audio_miniaudio` | `MiniaudioAudio : IAudio` | one `ma_engine`, one sound group per bus, spatialisation by miniaudio |
