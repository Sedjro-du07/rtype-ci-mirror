# Engine classes and interfaces

Status: **Locked** (2026-10-09). Every type below is specified, with all its methods, in the
architecture reference: [`reference/README.md`](reference/README.md).

Naming: interfaces start with `I`, concrete classes have no prefix. Language: C++23 with the
allow-list of ADR 0017. Online views: [class reference](https://claude.ai/artifact/QJCdiQcPYn15WT9iu8eF7B),
[inventory](https://claude.ai/artifact/FGAmyXhSoPrJycMsLXsoUA).

## Interfaces

| Interface | Role | Implemented by | Reference |
|---|---|---|---|
| `IGameModule` | contract between a game and the engine | each game | [02](reference/02-modules.md) |
| `IPlugin` | base of every plugin | every plugin | [02](reference/02-modules.md) |
| `IPlatform` | window, raw input, renderer access, debug UI | `platform_sfml`, `platform_raylib`, `NullPlatform` | [08](reference/08-rendering.md) |
| `IRenderer` | GPU resources and frame submission | `SfmlRenderer`, `RaylibRenderer`, `NullRenderer` | [08](reference/08-rendering.md) |
| `IAudio` | sounds, music, buses, listener | `audio_miniaudio`, `NullAudio` | [09](reference/09-audio.md) |
| `INetwork` | byte transport on an I/O thread | `network_asio`, `NullNetwork` | [10](reference/10-network.md) |
| `IScripting` | script loading and hooks | `scripting_lua`, `NullScripting` | [12](reference/12-services.md) |
| `IStorage` | type-erased component storage | `SparseSet<T>` | [03](reference/03-ecs.md) |
| `ILogSink` | log output | console, file, developer console sinks | [12](reference/12-services.md) |

## Classes and types by block

| Block | Types | Reference |
|---|---|---|
| 1 Core | `Engine`, `Context`, `EngineConfig`, `Role`, `Time`, `Clock`, `Error`, `Result`, `BootError`, `Vec2`, `Rect`, `Color`, `Signal`, `Subscription`, launcher `main()` | [01](reference/01-core.md) |
| 2 Modules | `PluginKind`, `PluginContext`, `DynamicLibrary`, `StaticModuleTable`, `ModuleManager`, `LoadedModule`, Null Objects, `ENGINE_PLUGIN`, `ENGINE_GAME_MODULE`, CMake functions | [02](reference/02-modules.md) |
| 3 ECS | `Entity`, `EntityAllocator`, `SparseSet<T>`, `Registry`, `View<Ts...>`, `Exclude`, `Commands`, `ComponentHooks` | [03](reference/03-ecs.md) |
| 4 Reflection | `TypeName`, `ENGINE_COMPONENT`, `ENGINE_EVENT`, `ENGINE_ENUM`, `FieldKind`, `FieldInfo`, `TypeFlags`, `TypeInfo`, `TypeBuilder<T>`, `TypeRegistry`, `ByteWriter`, `ByteReader` | [04](reference/04-reflection.md) |
| 5 Scheduler | `Stage`, `SystemFn`, `RoleMask`, `SystemBuilder`, `SystemStats`, `Scheduler` | [05](reference/05-scheduler.md) |
| 6 Events | `EventBus`, engine events | [06](reference/06-events.md) |
| 7 Input | `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis`, `RawInputEvent`, `ActionId`, `Binding`, `InputOptions`, `ActionState`, `InputMap` | [07](reference/07-input.md) |
| 8 Rendering | `Handle<Tag>`, `WindowDesc`, `Feature`, `ColorFilter`, `Camera2D`, `DrawCmd`, `RenderFrame`, `RenderQueue` | [08](reference/08-rendering.md) |
| 9 Audio | `AudioBus`, `PlayParams`, `VoiceId` | [09](reference/09-audio.md) |
| 10 Network | `ConnectionId`, `Endpoint`, `Datagram`, `Channel`, `DisconnectReason`, `NetConfig`, `ConnectionStats`, `NetworkSession`, `NetworkIdMap`, net components | [10](reference/10-network.md) |
| 11 Scenes and assets | `AssetManager`, `Prefab`, `PrefabLibrary`, `SceneLoader`, `Scene`, `SceneManager` | [11](reference/11-scenes-assets.md) |
| 12 Services | `Logger`, `LogLevel`, `LogConfig`, `CVar<T>`, `CVars`, `Console`, `Metrics`, `ScriptHost` | [12](reference/12-services.md) |
| Built-ins | 20 engine components, 18 engine systems | [13](reference/13-components-systems.md) |
