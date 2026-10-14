# ENG-34 · IAudio, PlayParams, buses, captions

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **09 Audio** · **Wave 5** · status and owner live on the board.

## Goal

The audio interface, buses and captions for every backend.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/audio/IAudio.hpp`
- `src/audio/AudioCaptions.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/audio/IAudio.hpp`

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

### `src/audio/AudioCaptions.cpp`

```cpp
// Decorator placed between Context::audio() and the plugin: every backend gets captions.
class CaptionedAudio final : public IAudio {
public:
    CaptionedAudio(IAudio& backend, EventBus& events);
    VoiceId play(SoundHandle s, const PlayParams& p) override;   // publishes Caption{p.caption} if not empty, then forwards
    // every other IAudio / IPlugin method forwards to backend_
private:
    IAudio&   backend_;
    EventBus& events_;
};
```

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-35](ENG-35.md) — audio_miniaudio plugin
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload

## Read first

- [ADR 0032](../adr/0032-audio-buses-captions.md)
- [reference 09](../engine/reference/09-audio.md)

## To implement

- [ ] `AudioBus`, `PlayParams`, `VoiceId`, `IAudio` declarations
- [ ] Caption wrapper publishing `Caption` events

## Done when

- Test with NullAudio: a play with a caption publishes Caption

## Watch out

- Captions come from the wrapper, not the plugin

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
