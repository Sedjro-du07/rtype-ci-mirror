# ENG-58 · platform_raylib plugin: 2D, then optional 3D

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **15 Part 2** · **Wave 7** · status and owner live on the board.

## Goal

The raylib backend, then optional 3D (Part 2).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_raylib/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/platform_raylib/RaylibPlatform.hpp`

```cpp
class RaylibPlatform final : public IPlatform {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "raylib"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override; void shutdown() noexcept override;
    Result<void> openWindow(const WindowDesc& desc) override;   // InitWindow
    void closeWindow() override;
    bool windowOpen() const override;                           // !WindowShouldClose()
    void pollEvents(InputMap& input, EventBus& events) override;// raylib polling (IsKeyPressed…) → RawInputEvent
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override; void endDebugUi() override;   // rlImGui
    void setClipboard(std::string_view) override; std::string clipboard() const override;
private:
    RaylibRenderer renderer_;
    std::array<bool, 512> previousKeys_{};                      // to turn polling into edges
};
ENGINE_PLUGIN(RaylibPlatform)
```

### `plugins/platform_raylib/RaylibRenderer.hpp`

```cpp
class RaylibRenderer final : public IRenderer {
public:
    Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) override;
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;
    Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame& frame) override;           // BeginDrawing / DrawTexturePro / EndDrawing
    bool   supports(Feature f) const override;                  // Render3D: true
    void   setColorFilter(ColorFilter f) override;
    RenderStats stats() const override;
private:
    std::vector<Texture2D> textures_;
    std::vector<Font>      fonts_;
};
```

## Depends on

- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer

## Unlocks

- no task waits for this one

## Read first

- [ADR 0007](../adr/0007-sfml-and-raylib-backends.md)

## To implement

- [ ] RaylibPlatform / RaylibRenderer for 2D
- [ ] Polling converted to RawInputEvent
- [ ] `supports(Render3D)` and 3D commands later

## Done when

- The game runs unchanged with `--platform raylib`

## Watch out

- No raylib type outside the plugin

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
