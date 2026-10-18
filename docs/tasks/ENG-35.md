# ENG-35 · audio_miniaudio plugin

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **09 Audio** · **Wave 6** · status and owner live on the board.

## Goal

One audio plugin for all platforms, on miniaudio.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/audio_miniaudio/CMakeLists.txt`
- `plugins/audio_miniaudio/include/MiniaudioAudio.hpp`
- `plugins/audio_miniaudio/MiniaudioAudio.cpp`
- `plugins/audio_miniaudio/MiniaudioImplementation.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/audio_miniaudio/include/MiniaudioAudio.hpp`

```cpp
class MiniaudioAudio final : public IAudio {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "miniaudio"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;      // ma_engine_init, one ma_sound_group per bus
    void shutdown() noexcept override;
    Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) override;
    void   destroySound(SoundHandle) override;
    Result<MusicHandle> openMusic(const std::filesystem::path& file) override;
    void   closeMusic(MusicHandle) override;
    VoiceId play(SoundHandle s, const PlayParams& p) override;
    void    stop(VoiceId v) override;
    void    setVoiceVolume(VoiceId v, float volume) override;
    void    playMusic(MusicHandle m, float fadeInSeconds) override;
    void    stopMusic(float fadeOutSeconds) override;
    void    setBusVolume(AudioBus bus, float volume) override;
    float   busVolume(AudioBus bus) const override;
    void    setListener(Vec2 position, float hearingRadius) override;
    void    update() override;                                 // recycles finished voices
private:
    ma_engine                                   engine_;
    std::array<ma_sound_group, 4>               buses_;
    std::vector<ma_decoder>                     sounds_;        // decoded once, index = handle id - 1
    std::unordered_map<VoiceId, ma_sound>       voices_;
};
ENGINE_PLUGIN(MiniaudioAudio)
```

## Depends on

- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- no task waits for this one

## Read first

- [ADR 0008](../adr/0008-shared-audio-plugin.md)
- [ADR 0032](../adr/0032-audio-buses-captions.md)

## To implement

- [ ] One `ma_engine`, one sound group per bus
- [ ] Sounds from memory, streamed music, positional sound, fades

## Done when

- Effects and music play; bus volumes change live; no device → NullAudio with a warning

## Watch out

- miniaudio callbacks run on its own thread

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
