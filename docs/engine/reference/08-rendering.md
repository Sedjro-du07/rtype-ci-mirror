# 8 · Platform and rendering

Status: **Locked**. ADRs 0004, 0006, 0007, 0031.
Folder: `engine/include/engine/render/`, `engine/src/render/`, `engine/plugins/platform_sfml/`,
`engine/plugins/platform_raylib/`.

## Role

Open the window, collect raw input, and draw what systems describe as plain data. The engine never
draws itself and never sees an SFML or raylib type.

---

## Handles — `render/Handles.hpp`

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

## `struct WindowDesc` — `render/WindowDesc.hpp`

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

## `enum class Feature` and `ColorFilter` — `render/IRenderer.hpp`

```cpp
enum class Feature : uint8_t { Render3D, ColorFilter, Shaders };
enum class ColorFilter : uint8_t { None, Protanopia, Deuteranopia, Tritanopia, HighContrast }; // accessibility
```

## `struct Camera2D` — `render/Camera2D.hpp`

```cpp
struct Camera2D {
    Vec2  center{640, 360};  // world point at the middle of the screen
    float zoom = 1.f;
    float rotation = 0.f;
    Vec2  screenToWorld(Vec2 screen, const WindowDesc&) const;
    Vec2  worldToScreen(Vec2 world, const WindowDesc&) const;
};
```

`center` is drawn at the middle of the virtual resolution; `zoom` scales around it and `rotation` (degrees)
turns the view. The virtual resolution is then scaled by `min(width / virtualWidth, height / virtualHeight)`
and centred in the window (letterbox bars on the free sides). `screenToWorld` is the exact inverse.

## `struct DrawCmd` — `render/DrawCmd.hpp`

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

## `class RenderQueue` and `struct RenderFrame` — `render/RenderQueue.hpp`

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

## `class IRenderer` — `render/IRenderer.hpp`

```cpp
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
```

GPU memory belongs to the plugin; the `AssetManager` (block 11) owns the path → handle table.

## `class IPlatform` — `render/IPlatform.hpp`

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

## Plugins

| Plugin | Part | Implements | Notes |
|---|---|---|---|
| `platform_sfml` | 1 | `SfmlPlatform : IPlatform`, `SfmlRenderer : IRenderer` | `sf::RenderWindow`, batching through `sf::VertexArray` per texture and layer, ImGui-SFML, color filters through a fragment shader |
| `platform_raylib` | 2 | `RaylibPlatform`, `RaylibRenderer` | polling converted to `RawInputEvent`; `supports(Render3D)` true, 3D commands added then; rlImGui |
