# ENG-30 · Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

The rendering API as plain data: the engine never draws itself.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/render/Handles.hpp`
- `include/engine/render/WindowDesc.hpp`
- `include/engine/render/Camera2D.hpp`
- `include/engine/render/DrawCmd.hpp`
- `include/engine/render/RenderQueue.hpp`
- `include/engine/render/IRenderer.hpp`
- `include/engine/render/IPlatform.hpp`
- `src/render/RenderQueue.cpp`
- `src/render/Camera2D.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/render/Handles.hpp`

```cpp
template <class Tag> struct Handle {
    uint32_t id = 0;                       // 0 = invalid
    constexpr bool valid() const noexcept;   // id != 0
    friend constexpr bool operator==(Handle, Handle) = default;
};
using TextureHandle = Handle<struct TextureTag>;
using FontHandle    = Handle<struct FontTag>;
using SoundHandle   = Handle<struct SoundTag>;   // block 9
using MusicHandle   = Handle<struct MusicTag>;   // block 9
using ScriptHandle  = Handle<struct ScriptTag>;  // block 12
```

### `include/engine/render/WindowDesc.hpp`

```cpp
struct WindowDesc {
    std::string title = "Engine";
    uint32_t    width = 1280, height = 720;          // physical window size
    uint32_t    virtualWidth = 1280, virtualHeight = 720; // logical resolution used by the game; letterboxed
    bool        fullscreen = false;
    bool        vsync = true;
    float       uiScale = 1.f;                       // accessibility: scales text and HUD
};
```

### `include/engine/render/IRenderer.hpp`

```cpp
enum class Feature : uint8_t { Render3D, ColorFilter, Shaders };
enum class ColorFilter : uint8_t { None, Protanopia, Deuteranopia, Tritanopia, HighContrast }; // accessibility

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) = 0;
    virtual void   destroyTexture(TextureHandle) = 0;
    virtual Vec2   textureSize(TextureHandle) const = 0;
    virtual Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) = 0;
    virtual void   destroyFont(FontHandle) = 0;
    virtual void   submit(const RenderFrame& frame) = 0;   // sort by layer, batch by texture, draw, present
    virtual bool   supports(Feature f) const = 0;
    virtual void   setColorFilter(ColorFilter f) = 0;      // no-op if !supports(ColorFilter)
    virtual RenderStats stats() const = 0;                 // commands, batches, draw calls of the last frame
};

struct RenderStats { uint32_t commands = 0, batches = 0, drawCalls = 0; double submitMs = 0; };
```

### `include/engine/render/Camera2D.hpp`

```cpp
struct Camera2D {
    Vec2  center{640, 360};  // world point at the middle of the screen
    float zoom = 1.f;
    float rotation = 0.f;
    Vec2  screenToWorld(Vec2 screen, const WindowDesc&) const;
    Vec2  worldToScreen(Vec2 world, const WindowDesc&) const;
};
```

### `include/engine/render/DrawCmd.hpp`

```cpp
struct DrawCmd {
    enum class Kind : uint8_t { Sprite, Text, Rect, RectOutline, Line, Circle };
    Kind          kind;
    int16_t       layer = 0;          // sort key: lower is drawn first
    bool          screenSpace = false;// true: ignores the camera (HUD)
    TextureHandle texture;            // Sprite
    Rect          source;             // Sprite: region of the texture
    Vec2          position;           // world or screen position (origin applied)
    Vec2          size;               // Rect, RectOutline; Circle: size.x = radius
    Vec2          end;                // Line
    Vec2          scale{1, 1};
    Vec2          origin;             // pivot, in pixels of the sprite
    float         rotation = 0.f;     // degrees
    Color         tint;
    FontHandle    font;               // Text
    uint16_t      textSize = 16;
    uint32_t      textOffset = 0, textLength = 0; // into the queue's text arena
};
```

### `include/engine/render/RenderQueue.hpp`

```cpp
struct RenderFrame {
    Camera2D                camera;
    Color                   clearColor;
    std::span<const DrawCmd> commands;
    std::string_view        textArena;
};

class RenderQueue {
public:
    void sprite(TextureHandle t, Rect source, Vec2 position, int16_t layer,
                Vec2 scale = {1, 1}, float rotation = 0, Color tint = {}, Vec2 origin = {});
    void text(std::string_view s, FontHandle f, uint16_t size, Vec2 position, int16_t layer,
              Color color = {}, bool screenSpace = true);
    void rect(Rect r, Color c, int16_t layer, bool filled = true, bool screenSpace = false);
    void line(Vec2 a, Vec2 b, Color c, int16_t layer);
    void circle(Vec2 center, float radius, Color c, int16_t layer);
    void push(const DrawCmd& cmd);              // raw form
    void setCamera(const Camera2D& c); const Camera2D& camera() const;
    void setClearColor(Color c);
    RenderFrame frame() const;                  // engine only: view handed to IRenderer::submit
    void clear();                               // engine only: keeps capacity
    size_t size() const;
};
```

### `include/engine/render/IPlatform.hpp`

```cpp
class IPlatform : public IPlugin {
public:
    virtual Result<void> openWindow(const WindowDesc& desc) = 0;
    virtual void   closeWindow() = 0;
    virtual bool   windowOpen() const = 0;
    virtual void   pollEvents(InputMap& input, EventBus& events) = 0; // raw input → InputMap::feed;
                                                                      // close/resize/focus → EventBus
    virtual IRenderer& renderer() = 0;
    virtual void   setWindow(const WindowDesc& desc) = 0;   // title, size, fullscreen, vsync at runtime
    virtual void   beginDebugUi() = 0;                      // Dear ImGui new frame (console, metrics)
    virtual void   endDebugUi() = 0;                        // ImGui render, after submit()
    virtual void   setClipboard(std::string_view) = 0;
    virtual std::string clipboard() const = 0;
};
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-25](ENG-25.md) — Engine components and their registration
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload
- [ENG-52](ENG-52.md) — Render systems: sprites (interpolated), text, animation, debug colliders

## Read first

- [ADR 0006](../adr/0006-command-list-renderer.md)
- [ADR 0031](../adr/0031-render-api.md)
- [reference 08](../engine/reference/08-rendering.md)

## To implement

- [ ] Handles, WindowDesc, Feature, ColorFilter, Camera2D (screen ↔ world with letterbox)
- [ ] DrawCmd, RenderFrame, RenderStats
- [ ] RenderQueue (text arena)
- [ ] IRenderer, IPlatform declarations

## Done when

- Tests: Camera2D conversions round trip; RenderQueue keeps capacity after clear

## Watch out

- No SFML or raylib type in these headers

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
