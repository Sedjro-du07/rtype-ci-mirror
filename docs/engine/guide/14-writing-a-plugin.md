# 14 · Writing a plugin

A plugin implements one engine interface: `IPlatform`, `IAudio`, `INetwork` or `IScripting`.
Example: an SDL3 platform backend.

## 1. Folder and CMake

```
engine/plugins/platform_sdl/
├── CMakeLists.txt
├── SdlPlatform.hpp / .cpp     IPlatform
└── SdlRenderer.hpp / .cpp     IRenderer
```

```cmake
engine_add_plugin(platform_sdl
    SOURCES SdlPlatform.cpp SdlRenderer.cpp
    DEPENDS SDL3::SDL3)
```

The name must be `<kind>_<name>`: the config line `platform: sdl` loads `platform_sdl`.

## 2. The class

```cpp
class SdlPlatform final : public engine::IPlatform {
public:
    engine::PluginKind kind() const override { return engine::PluginKind::Platform; }
    std::string_view name() const override { return "sdl"; }
    uint32_t apiVersion() const override { return engine::ENGINE_PLUGIN_API_VERSION; }
    engine::Result<void> init(const engine::PluginContext& ctx) override;
    void shutdown() noexcept override;

    engine::Result<void> openWindow(const engine::WindowDesc&) override;
    void pollEvents(engine::InputMap& input, engine::EventBus& events) override;
    engine::IRenderer& renderer() override;
    /* … every other IPlatform method … */
};
ENGINE_PLUGIN(SdlPlatform)
```

## 3. Rules

- **No library type in any engine header.** Translate everything: SDL keys → `engine::Key`,
  SDL events → `engine::RawInputEvent`, textures → `engine::TextureHandle`.
- **Never throw out of the plugin.** Return `Result` errors; `ENGINE_PLUGIN` catches anything else at the entry point.
- **Free what you allocate.** Objects you create are destroyed by you (`destroy_plugin`, `shutdown`).
- **Respect thread rules.** Rendering and window calls on the loop thread; `INetwork` receivers on your I/O thread only.
- **Same compiler and runtime** as the engine (dynamic mode).
- Log through the `PluginContext` logger; define your CVars through `PluginContext::cvars`.

## 4. Test

Run the engine tests with `--platform sdl`, then the client: the same game must look and behave
the same. A mismatched `apiVersion` is refused with a clear error, never a crash.
