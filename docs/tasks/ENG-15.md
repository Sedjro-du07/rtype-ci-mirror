# ENG-15 · Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **02 Modules** · status and owner live on the board.

## Goal

Do-nothing implementations used on the server, in CI and when a device is missing (ADR 0023).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/module/NullPlugins.hpp`
- `src/module/NullPlugins.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/module/NullPlugins.hpp`

```cpp
class NullRenderer final : public IRenderer {
public:
    Result<TextureHandle> createTexture(std::span<const std::byte>, std::string_view) override;  // valid handle, no decode
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;                                             // {0, 0}
    Result<FontHandle> createFont(std::span<const std::byte>, std::string_view) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame&) override;                                                   // ignores
    bool   supports(Feature) const override;                                                      // false
    void   setColorFilter(ColorFilter) override;
    RenderStats stats() const override;
};

class NullPlatform final : public IPlatform {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void> openWindow(const WindowDesc&) override;  void closeWindow() override;
    bool windowOpen() const override;                     // true until requestQuit
    void pollEvents(InputMap&, EventBus&) override;       // produces nothing
    IRenderer& renderer() override;                       // a NullRenderer member
    void setWindow(const WindowDesc&) override;
    void beginDebugUi() override; void endDebugUi() override;
    void setClipboard(std::string_view) override; std::string clipboard() const override;
private:
    NullRenderer renderer_;
};

class NullAudio final : public IAudio {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<SoundHandle> createSound(std::span<const std::byte>, std::string_view) override;  // valid handle
    void   destroySound(SoundHandle) override;
    Result<MusicHandle> openMusic(const std::filesystem::path&) override;
    void   closeMusic(MusicHandle) override;
    VoiceId play(SoundHandle, const PlayParams&) override;           // returns 0
    void    stop(VoiceId) override;
    void    setVoiceVolume(VoiceId, float) override;
    void    playMusic(MusicHandle, float) override;
    void    stopMusic(float) override;
    void    setBusVolume(AudioBus, float) override;                  // remembered, so busVolume() is consistent
    float   busVolume(AudioBus) const override;
    void    setListener(Vec2, float) override;
    void    update() override;
private:
    std::array<float, 4> volumes_{1, 1, 1, 1};
};

class NullNetwork final : public INetwork {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void> openUdp(uint16_t) override;                         // succeeds, nothing bound
    void closeUdp() override;
    void sendUdp(const Endpoint&, std::span<const std::byte>) override;   // drops
    void setReceiver(std::move_only_function<void(Datagram&&)>) override; // never called
    Result<void> listenTcp(uint16_t) override;
    Result<uint32_t> connectTcp(const Endpoint&) override;           // NetworkError "offline"
    void sendTcp(uint32_t, std::span<const std::byte>) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)>) override;
    void closeTcp(uint32_t) override;
};

class NullScripting final : public IScripting {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void>         bind(ScriptHost&) override;                 // succeeds
    Result<ScriptHandle> load(const std::filesystem::path&) override;   // ScriptError "scripting disabled"
    void                 unload(ScriptHandle) override;
    Result<void>         reload(ScriptHandle) override;
    Result<void>         call(ScriptHandle, std::string_view, Entity, Context&) override;
    bool                 has(ScriptHandle, std::string_view) const override;   // false
};

std::unique_ptr<IPlugin> makeNullPlugin(PluginKind kind);   // used by ModuleManager for the name "null"
```

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-36](ENG-36.md) — INetwork and network types
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems

## Unlocks

- [ENG-57](ENG-57.md) — Headless smoke test in CI: client with Null plugins runs 1000 ticks

## Read first

- [ADR 0023](../adr/0023-null-object-services.md)

## To implement

- [ ] NullPlatform, NullRenderer (valid handles, no decoding), NullAudio, NullNetwork, NullScripting

## Done when

- Headless smoke test (ENG-57) runs with all Null Objects

## Watch out

- Each Null Object in use is logged once at boot

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
