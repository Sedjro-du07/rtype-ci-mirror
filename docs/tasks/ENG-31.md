# ENG-31 · platform_sfml plugin: window, events to InputMap, batching renderer

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · status and owner live on the board.

## Goal

The Part 1 window, input and renderer on SFML 3.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_sfml/CMakeLists.txt`
- `plugins/platform_sfml/SfmlPlatform.hpp`
- `plugins/platform_sfml/SfmlPlatform.cpp`
- `plugins/platform_sfml/SfmlRenderer.hpp`
- `plugins/platform_sfml/SfmlRenderer.cpp`
- `plugins/platform_sfml/SfmlKeys.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/platform_sfml/SfmlPlatform.hpp`

```cpp
class SfmlPlatform final : public IPlatform {
public:
    // IPlugin
    PluginKind kind() const override;                 // Platform
    std::string_view name() const override;           // "sfml"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    // IPlatform
    Result<void> openWindow(const WindowDesc& desc) override;
    void closeWindow() override;
    bool windowOpen() const override;
    void pollEvents(InputMap& input, EventBus& events) override;
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override;                     // ENG-32
    void endDebugUi() override;                       // ENG-32
    void setClipboard(std::string_view text) override;
    std::string clipboard() const override;
private:
    sf::RenderWindow window_;
    SfmlRenderer     renderer_;
    WindowDesc       desc_;
};
ENGINE_PLUGIN(SfmlPlatform)
```

### `plugins/platform_sfml/SfmlRenderer.hpp`

```cpp
class SfmlRenderer final : public IRenderer {
public:
    explicit SfmlRenderer(sf::RenderWindow& window);
    Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) override;
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;
    Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame& frame) override;   // sort by layer, batch by texture, draw, present
    bool   supports(Feature f) const override;
    void   setColorFilter(ColorFilter f) override;      // ENG-33
    RenderStats stats() const override;
private:
    void applyView(const Camera2D& camera);             // virtual resolution + letterbox
    void flushBatch();                                  // one sf::VertexArray draw
    sf::RenderWindow&                          window_;
    std::vector<std::unique_ptr<sf::Texture>>  textures_;   // index = handle id - 1
    std::vector<std::unique_ptr<sf::Font>>     fonts_;
    sf::VertexArray                            batch_;
    RenderStats                                stats_;
};
```

### `plugins/platform_sfml/SfmlKeys.cpp`

```cpp
Key           toKey(sf::Keyboard::Scancode code);
MouseButton   toMouseButton(sf::Mouse::Button button);
GamepadButton toGamepadButton(unsigned int sfmlButton);
GamepadAxis   toGamepadAxis(sf::Joystick::Axis axis);
std::optional<RawInputEvent> toRawInputEvent(const sf::Event& event, const WindowDesc& desc);
```

### `plugins/platform_sfml/CMakeLists.txt`

```cmake
engine_add_plugin(platform_sfml SOURCES SfmlPlatform.cpp SfmlRenderer.cpp SfmlKeys.cpp SfmlImGui.cpp
                  DEPENDS SFML::Graphics ImGui-SFML::ImGui-SFML implot)
```

## Depends on

- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- [ENG-32](ENG-32.md) — Dear ImGui and ImPlot in platform_sfml (debug UI hooks)
- [ENG-33](ENG-33.md) — Colour-blindness filters and UI scale in platform_sfml
- [ENG-58](ENG-58.md) — platform_raylib plugin: 2D, then optional 3D

## Read first

- [ADR 0007](../adr/0007-sfml-and-raylib-backends.md)
- [ADR 0031](../adr/0031-render-api.md)

## To implement

- [ ] `SfmlPlatform`: window, events → RawInputEvent → InputMap::feed, window events → EventBus
- [ ] `SfmlRenderer`: textures, fonts, sort by layer, batch by texture with VertexArray, letterbox
- [ ] SFML key mapping

## Done when

- A sprite, a text and shapes appear; resizing keeps the virtual resolution; input reaches actions

## Watch out

- One draw call per texture and layer, not per sprite

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
