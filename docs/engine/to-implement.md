# Engine — everything to implement

Status: **Locked** with the architecture (2026-10-09). This page is the **implementation checklist**:
every interface, class, struct, enum, macro, function, system and plugin class the engine team must
write, with its file, its task and the methods to code. Signatures and behaviour are in the
reference ([`reference/README.md`](reference/README.md)); tasks and owners are on the board
([`tasks.md`](tasks.md)). What a *game* uses from all this is in [`game-api.md`](game-api.md).

How to read the tables:

- **Kind**: `interface` (pure virtual, starts with `I`), `class`, `struct` (plain data, little or no
  logic), `enum`, `template`, `macro`, `function`, `system` (engine system registered in the scheduler),
  `plugin` (class living in a plugin library), `cmake`.
- **To implement**: every method that needs a body. A struct with only fields needs its header only.
- Paths are relative to `engine/`. `inc/` means `include/engine/`.
- 🔒 = engine only: called by the engine itself, never by game code.

Totals: **9 interfaces**, **~65 classes and structs** (6 private to the server layer), **20 engine components**, **20 engine systems**,
**5 Null Objects**, **7 plugin classes**, **8 macros**, **3 CMake functions**.

---

## 0 · Build — ENG-01, ENG-02, ENG-03

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| Engine project | cmake | `CMakeLists.txt`, `cmake/EngineOptions.cmake` | ENG-01 | `engine` target, C++23, warnings, `ENGINE_DYNAMIC_MODULES` option |
| `engine_add_plugin` | cmake | `cmake/EngineModules.cmake` | ENG-02 | SHARED/STATIC per option, output `<bin>/plugins`, hidden visibility, `ENGINE_BUILDING_PLUGIN` |
| `engine_add_game_module` | cmake | `cmake/EngineModules.cmake` | ENG-02 | same, output `<bin>/games` |
| `engine_add_launcher` | cmake | `cmake/EngineModules.cmake` | ENG-02 | builds `launcher/main.cpp` with `ENGINE_DEFAULT_CONFIG`, copies the config, links everything in static mode |
| Feature probe | file | `cmake/FeatureProbe.cpp` | ENG-03 | compiles every allowed C++23 feature (ADR 0017); `tl::expected` fallback switch |

## 1 · Core — ENG-04 to ENG-10 · [reference 01](reference/01-core.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `fnv1a64` | function | `inc/core/Hash.hpp` | ENG-04 | `constexpr`, compile-time usable |
| `Vec2` | struct | `inc/core/math/Vec2.hpp` | ENG-04 | `+ - *`, `dot`, `length`, `normalized`, `lerp` |
| `Rect` | struct | `inc/core/math/Rect.hpp` | ENG-04 | `contains(Vec2)`, `intersects(Rect)` |
| `Color` | struct | `inc/core/math/Color.hpp` | ENG-04 | `white()`, `black()`, `red()`… constants |
| `Errc` | enum | `inc/core/Error.hpp` | ENG-04 | 15 codes |
| `Error` | struct | `inc/core/Error.hpp`, `src/core/Error.cpp` | ENG-04 | `toString()` (`file:line: message`) |
| `Result<T>` | template | `inc/core/Error.hpp` | ENG-04 | alias `std::expected<T, Error>` (or `tl::expected`) |
| `BootError` | class | `inc/core/Error.hpp`, `src/core/Error.cpp` | ENG-04 | ctor, `exitCode()`, `error()` |
| `Time` | struct | `inc/core/Time.hpp` | ENG-05 | fields only |
| `Clock` | class | `inc/core/Time.hpp`, `src/core/Time.cpp` | ENG-05 | ctor, `restart()`, `elapsed()` (steady clock) |
| `Subscription` | class | `inc/core/Signal.hpp` | ENG-06 | move ctor/assign, dtor, `reset()`, `active()` |
| `Signal<void(Args...)>` | template | `inc/core/Signal.hpp` | ENG-06 | `connect()`, `emit()`, `size()`; safe disconnect during `emit` |
| `Role` | enum | `inc/EngineConfig.hpp` | ENG-07 | `Client`, `Server`, `Editor` |
| `EngineConfig` | struct | `inc/EngineConfig.hpp`, `src/core/EngineConfig.cpp` | ENG-07 | `load(file, argc, argv)`: YAML, 12 CLI options, unknown key → error with `file:line` |
| `Engine` | class | `inc/Engine.hpp`, `src/core/Engine.cpp` | ENG-08 | ctor, dtor, `run()`, `requestQuit()`; private `boot()`, `load()`, `loop()`, `shutdown()`; member order = shutdown order; exit codes 1–5 |
| `Context` | class | `inc/Context.hpp`, `src/core/Context.cpp` | ENG-09 | 17 accessors (`world` … `requestQuit`), private ctor, `friend Engine` |
| `main()` | function | `launcher/main.cpp` | ENG-10 | load config, SIGINT/SIGTERM + Windows console handler → `requestQuit`, `return engine.run()` |

## 2 · Modules and plugins — ENG-11 to ENG-15 · [reference 02](reference/02-modules.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `ENGINE_PLUGIN_API_VERSION`, `ENGINE_GAME_API_VERSION`, `ENGINE_EXPORT` | constants, macro | `inc/module/Export.hpp` | ENG-11 | values and per-OS export attribute |
| `ENGINE_GAME_MODULE(Class)` | macro | `inc/module/Export.hpp` | ENG-11 | dynamic: `create_game`, `destroy_game`, `game_api_version` (`extern "C"`, `noexcept`, try/catch); static: registrar |
| `ENGINE_PLUGIN(Class)` | macro | `inc/module/Export.hpp` | ENG-11 | same for `create_plugin`, `destroy_plugin`, `plugin_api_version` |
| `PluginKind` | enum | `inc/module/IPlugin.hpp` | ENG-11 | `Platform`, `Audio`, `Network`, `Scripting` |
| `PluginContext` | struct | `inc/module/IPlugin.hpp` | ENG-11 | references only |
| **`IPlugin`** | interface | `inc/module/IPlugin.hpp` | ENG-11 | declares `kind`, `name`, `apiVersion`, `init`, `shutdown` |
| **`IGameModule`** | interface | `inc/module/IGameModule.hpp` | ENG-11 | declares `registerTypes`, `registerSystems`, `firstScene`, `onStart`, `onShutdown` (implemented by games) |
| `DynamicLibrary` | class | `inc/module/DynamicLibrary.hpp`, `src/module/DynamicLibrary.cpp` | ENG-12 | `open`, `decorate`, move ops, dtor, `rawSymbol`, `symbol<Fn>`, `path`; `dlopen` / `LoadLibraryW` |
| `StaticModuleTable` | class | `inc/module/StaticModuleTable.hpp`, `src/module/StaticModuleTable.cpp` | ENG-13 | `addPlugin`, `addGame`, `findPlugin`, `findGame`; `PluginEntry`, `GameEntry` |
| `LoadedModule` | struct | `inc/module/ModuleManager.hpp` | ENG-14 | fields only |
| `ModuleManager` | class | `inc/module/ModuleManager.hpp`, `src/module/ModuleManager.cpp` | ENG-14 | ctor, dtor, `loadPlugin<I>`, `loadGame`, `unloadGame`, `unloadAll`, `loaded`; name `null` → Null Object |
| `NullPlatform` | class | `inc/module/NullPlugins.hpp`, `src/module/NullPlugins.cpp` | ENG-15 | every `IPlatform` method, no window |
| `NullRenderer` | class | same | ENG-15 | every `IRenderer` method; valid handles, nothing decoded |
| `NullAudio` | class | same | ENG-15 | every `IAudio` method; `play` returns 0 |
| `NullNetwork` | class | same | ENG-15 | every `INetwork` method; drops, receives nothing |
| `NullScripting` | class | same | ENG-15 | every `IScripting` method; `load` returns "scripting disabled" |

## 3 · ECS — ENG-16 to ENG-21 · [reference 03](reference/03-ecs.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `Entity` | struct | `inc/ecs/Entity.hpp` | ENG-16 | `null()`, `isNull()`, `==`, `std::hash<Entity>` |
| `EntityAllocator` | class | `inc/ecs/EntityAllocator.hpp`, `src/ecs/EntityAllocator.cpp` | ENG-16 | `create`, `release`, `alive`, `aliveCount` (free list, generations) |
| **`IStorage`** | interface | `inc/ecs/IStorage.hpp` | ENG-17 | declares `id`, `has`, `remove`, `size`, `clear`, `entities`, `getRaw` ×2, `emplaceRaw` |
| `SparseSet<T>` | template | `inc/ecs/SparseSet.hpp` | ENG-17 | ctor, `emplace`, `get` ×2, `tryGet`, `replace`, `has`, `remove` (swap-and-pop), `size`, `entities`, `components`, `clear`, raw overrides; pages of 4096 |
| `Registry` | class | `inc/ecs/Registry.hpp`, `src/ecs/Registry.cpp` | ENG-18 | entities (`create`, `destroy` recursive, `alive`, `aliveCount`, `clear`); typed (`emplace`, `get` ×2, `tryGet`, `has`, `replace`, `remove`); `view` ×2; raw (`storage`, `getRaw`, `emplaceRaw`, `removeRaw`, `componentsOf`); identity (`find(name)`, `find(Uuid)` indexed, `withTag`); hierarchy (`setParent`, `parent`, `children`) |
| `View<Ts...>`, `Exclude`, `exclude` | template | `inc/ecs/View.hpp` | ENG-19 | `iterator` (tuple, structured bindings), `begin`, `end`, `sizeHint`, `each`, `front`; iterate the smallest storage |
| `Commands` | class | `inc/ecs/Commands.hpp`, `src/ecs/Commands.cpp` | ENG-20 | `spawn` ×2, `destroy`, `add`, `set`, `remove`, `setParent`, `addRaw`, `empty`, `apply` 🔒 (recording order, skip dead entities) |
| `ComponentHooks` | struct | `inc/ecs/Registry.hpp` | ENG-21 | `onCreate`, `onReplace`, `onDestroy` signals; `Registry::hooks<T>()`, `hooks(id)`; fired by every path, `Commands` included |

## 4 · Reflection and serialization — ENG-22 to ENG-25 · [reference 04](reference/04-reflection.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `TypeCategory`, `TypeName<T>`, `HasTypeName`, `typeKey<T>()`, `ComponentId` | enum, template, concept | `inc/reflection/TypeKey.hpp` | ENG-22 | primary template: name, key, category |
| `ENGINE_COMPONENT`, `ENGINE_EVENT`, `ENGINE_MESSAGE`, `ENGINE_STRUCT`, `ENGINE_ENUM` | macro | `inc/reflection/TypeKey.hpp` | ENG-22 | specialise `TypeName` with the category; qualified name → last part; enum names list (ADR 0036) |
| `FieldKind`, `Transport` | enum | `inc/reflection/TypeInfo.hpp` | ENG-22 | 20 kinds (with `List`, `Struct`); `Unreliable`, `Reliable`, `Stream` |
| `FieldInfo`, `EnumInfo`, `WireContext` | struct | `inc/reflection/TypeInfo.hpp` | ENG-22 | fields only (`maxLength`, element kind); `WireContext`: `Entity` ↔ `NetworkId` functions |
| `TypeFlags` | enum | `inc/reflection/TypeInfo.hpp` | ENG-22 | `Replicated`, `EditorHidden`, `Tag`, `Transient` |
| `TypeInfo` | struct | `inc/reflection/TypeInfo.hpp` | ENG-22 | function pointers filled by the builders; category, transport |
| `FieldsBuilder<T>` + `ComponentBuilder`, `MessageBuilder`, `EventBuilder`, `StructBuilder`, `EnumBuilder` | template | `inc/reflection/TypeBuilder.hpp` | ENG-22 | `field` (deducing this, deduces `FieldKind` incl. vectors and structs), `maxLength`; `flags`, `yaml`, `binary` (components); `unreliable`, `reliable`, `stream`, `binary` (messages); generate construct/destroy/copy/makeStorage |
| `TypeRegistry` | class | `inc/reflection/TypeRegistry.hpp`, `src/reflection/TypeRegistry.cpp` | ENG-22 | `add<T>` (only method; builder by category), `id<T>`, `find`, `findByKey`, `info`, `all`, `messages`, `formatFingerprint`, `closestName`, `clearGameTypes`; `DuplicateName` check |
| `YamlNode`, `YamlWriter`, `LoadContext`, `SaveContext` | class | `inc/reflection/Yaml.hpp`, `src/reflection/Yaml.cpp` | ENG-23 | wrappers over yaml-cpp (kept out of game headers), one generic YAML codec per `FieldKind` (lists, structs, enums by name) |
| `ByteWriter` | class | `inc/reflection/ByteStream.hpp`, `src/reflection/ByteStream.cpp` | ENG-24 | ctor, `write` ×7, `writeBits`, `size`, `overflowed` |
| `ByteReader` | class | same | ENG-24 | ctor, `u8`, `u16`, `u32`, `u64`, `i32`, `f32`, `string`, `bits`, `remaining`, `failed` |
| Engine components | struct | `inc/components/*.hpp` | ENG-25 | `Position`, `PreviousPosition`, `Velocity`, `Sprite`, `Animation`, `Text`, `Collider`, `Lifetime`, `Parallax`, `Name`, `Uuid`, `Tags`, `Parent`, `Children`, `Script` (+ net components, block 10) |
| `registerEngineTypes` | function | `src/systems/RegisterEngineTypes.cpp` | ENG-25 | registers every engine component and event with fields and flags, before the game |

## 5 · Scheduler — ENG-26 · [reference 05](reference/05-scheduler.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `Stage` | enum | `inc/scheduler/Stage.hpp` | ENG-26 | `FixedUpdate`, `LateUpdate`, `Render` |
| `SystemFn`, `RoleMask` | alias, enum | `inc/scheduler/System.hpp` | ENG-26 | `move_only_function<void(Context&)>`; masks |
| `SystemBuilder` | class | `inc/scheduler/Scheduler.hpp` | ENG-26 | `after`, `before`, `roles`, `disabled`, `critical` |
| `SystemStats` | struct | same | ENG-26 | fields only |
| `Scheduler` | class | same, `src/scheduler/Scheduler.cpp` | ENG-26 | ctor, `add`, `build` 🔒 (topological sort, cycles), `run` 🔒 (timing, try/catch, failure policy per role), `setEnabled`, `enabled`, `stats`, `clearGameSystems` |

## 6 · Events — ENG-27 · [reference 06](reference/06-events.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `EventBus` | class | `inc/events/EventBus.hpp`, `src/events/EventBus.cpp` | ENG-27 | `subscribe<E>`, `publish<E>`, `emit<E>`, `publishFromAnyThread<E>` (mutex inbox), `dispatch` 🔒, `clearGameSubscriptions` |
| Engine events | struct | `inc/events/EngineEvents.hpp` | ENG-27 | `QuitRequested`, `WindowResized`, `WindowFocusChanged`, `Collision`, `SceneLoaded`, `SceneUnloading`, `PlayerJoined`, `PlayerLeft`, `JoinedServer`, `LeftServer`, `Caption`, `ConsoleCommand` |

## 7 · Input — ENG-28, ENG-29 · [reference 07](reference/07-input.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis` | enum | `inc/input/Keys.hpp`, `src/input/Keys.cpp` | ENG-28 | values + `keyName(Key)`, name ↔ enum for `input.yaml` |
| `RawInputEvent` | struct | `inc/input/RawInputEvent.hpp` | ENG-28 | fields only |
| `ActionId`, `action()` | struct, function | `inc/input/Action.hpp` | ENG-28 | `constexpr` hash |
| `AxisBinding`, `Binding` | struct, variant | `inc/input/InputMap.hpp` | ENG-29 | fields only |
| `InputOptions` | struct | same | ENG-29 | fields only |
| `ActionState` | class | `inc/input/ActionState.hpp`, `src/input/ActionState.cpp` | ENG-29 | `pressed`, `held`, `released`, `axis`, `toMask`, `fromMask` |
| `InputMap` | class | `inc/input/InputMap.hpp`, `src/input/InputMap.cpp` | ENG-29 | `load`, `save`, `bind`, `unbind`, `clear`, `bindings`, `boundTo`, `setOptions`, `options`, `feed` 🔒, `beginTick` 🔒 (edge latching, hold-to-toggle, auto-repeat, deadzone), `state`, `captureNextBinding` |

## 8 · Platform and rendering — ENG-30 to ENG-33, ENG-58 · [reference 08](reference/08-rendering.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `Handle<Tag>` + 5 aliases | template | `inc/render/Handles.hpp` | ENG-30 | `valid`, `==` |
| `WindowDesc` | struct | `inc/render/WindowDesc.hpp` | ENG-30 | fields only |
| `Feature`, `ColorFilter` | enum | `inc/render/IRenderer.hpp` | ENG-30 | values |
| `Camera2D` | struct | `inc/render/Camera2D.hpp`, `src/render/Camera2D.cpp` | ENG-30 | `screenToWorld`, `worldToScreen` (letterbox) |
| `DrawCmd` | struct | `inc/render/DrawCmd.hpp` | ENG-30 | fields only |
| `RenderFrame`, `RenderStats` | struct | `inc/render/RenderQueue.hpp`, `inc/render/IRenderer.hpp` | ENG-30 | fields only |
| `RenderQueue` | class | `inc/render/RenderQueue.hpp`, `src/render/RenderQueue.cpp` | ENG-30 | `sprite`, `text` (text arena), `rect`, `line`, `circle`, `push`, `setCamera`, `camera`, `setClearColor`, `frame` 🔒, `clear` 🔒, `size` |
| **`IRenderer`** | interface | `inc/render/IRenderer.hpp` | ENG-30 | declares `createTexture`, `destroyTexture`, `textureSize`, `createFont`, `destroyFont`, `submit`, `supports`, `setColorFilter`, `stats` |
| **`IPlatform`** | interface | `inc/render/IPlatform.hpp` | ENG-30 | declares `openWindow`, `closeWindow`, `windowOpen`, `pollEvents`, `renderer`, `setWindow`, `beginDebugUi`, `endDebugUi`, `setClipboard`, `clipboard` |
| `SfmlPlatform` | plugin | `plugins/platform_sfml/SfmlPlatform.*`, `SfmlKeys.cpp` | ENG-31 | every `IPlugin` + `IPlatform` method; SFML events → `RawInputEvent` → `InputMap::feed` |
| `SfmlRenderer` | plugin | `plugins/platform_sfml/SfmlRenderer.*` | ENG-31 | every `IRenderer` method; sort by layer, batch per texture with `sf::VertexArray` |
| ImGui / ImPlot hooks | plugin | `plugins/platform_sfml/SfmlImGui.cpp` | ENG-32 | `beginDebugUi`, `endDebugUi` with ImGui-SFML |
| Colour filters | plugin | `plugins/platform_sfml/ColorFilter.frag` | ENG-33 | 4 matrices + high contrast; UI scale |
| `RaylibPlatform`, `RaylibRenderer` | plugin | `plugins/platform_raylib/` | ENG-58 | same interfaces; polling → `RawInputEvent`; `supports(Render3D)` (Part 2) |

## 9 · Audio — ENG-34, ENG-35 · [reference 09](reference/09-audio.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `AudioBus`, `PlayParams`, `VoiceId` | enum, struct, alias | `inc/audio/IAudio.hpp` | ENG-34 | fields only |
| **`IAudio`** | interface | `inc/audio/IAudio.hpp` | ENG-34 | declares `createSound`, `destroySound`, `openMusic`, `closeMusic`, `play`, `stop`, `setVoiceVolume`, `playMusic`, `stopMusic`, `setBusVolume`, `busVolume`, `setListener`, `update` 🔒 |
| Caption wrapper | class | `src/audio/AudioCaptions.cpp` | ENG-34 | wraps `play`: publishes `Caption` when `PlayParams::caption` is set, for every backend |
| `MiniaudioAudio` | plugin | `plugins/audio_miniaudio/MiniaudioAudio.*` | ENG-35 | every `IPlugin` + `IAudio` method; one `ma_engine`, one group per bus, spatialisation |

## 10 · Network (engine side) — ENG-36 to ENG-39 · [reference 10](reference/10-network.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `PlayerId`, `ConnectionId`, `DisconnectReason` | alias, enum | `inc/net/NetTypes.hpp` | ENG-36 | values (`ServerFull`, `FormatMismatch` added) |
| `Endpoint` | struct | same | ENG-36 | `toString`, `resolve` |
| `Datagram`, `NetConfig`, `ConnectionStats` | struct | same | ENG-36 | fields only |
| **`INetwork`** | interface | `inc/net/INetwork.hpp` | ENG-36 | declares `openUdp`, `closeUdp`, `sendUdp`, `setReceiver`, `listenTcp`, `connectTcp`, `sendTcp`, `setTcpReceiver`, `closeTcp` |
| `AsioNetwork` | plugin | `plugins/network_asio/AsioNetwork.*` | ENG-37 | every `IPlugin` + `INetwork` method, **UDP and TCP from Part 1**; one `io_context` on its own thread |
| `NetworkSession` | class | `inc/net/NetworkSession.hpp`, `src/net/NetworkSession.cpp` | ENG-38, SRV-01, SRV-03 | core (ENG-38): ctor, `attach` 🔒, `poll` 🔒 (bounded inbox, validation, routing, heartbeats, timeouts), `sendRaw` 🔒, `isServer`, `connected`, `stats`. Game API: `startServer`, `stopServer`, `connectTo`, `disconnect`, `players`, `localPlayer`, `name`, `kick`, `ping` (SRV-01); `send<T>` ×2, `broadcast<T>`, `on<T>` ×2 (SRV-03). **Bytes: protocol RFC (Network part)** |
| Net components | struct | `inc/net/NetComponents.hpp` | ENG-39 | `NetworkId`, `NetPrefab`, `NetOwner`, `RemoteInput`, `NetInterpolation` + registration |
| `NetworkIdMap` | class | `inc/net/NetworkIdMap.hpp`, `src/net/NetworkIdMap.cpp` | ENG-39 | `allocate`, `bind`, `unbind`, `find`, `clear` |
| Net systems | system | `src/systems/NetSystems.cpp` | ENG-39 | `engine.net.assign_ids`, `snapshot`, `apply_snapshot`, `interpolate`, `send_input`, `receive_input` |

## Server layer (private classes) — SRV-01 to SRV-07 · [reference 10](reference/10-network.md#internal-structure-engine-only--enginesrcnet) · [server tasks](../server/tasks.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `PlayerTable` | class | `src/net/PlayerTable.hpp`, `.cpp` | SRV-01 | `add`, `remove`, `playerOf`, `connectionOf`, `name`, `players` |
| `Handshake` | class | `src/net/Handshake.hpp`, `.cpp` | SRV-02 | state machine: name, fingerprint, `ServerFull` / `FormatMismatch`, TCP ↔ UDP link |
| `MessageDispatcher` | class | `src/net/MessageDispatcher.hpp`, `.cpp` | SRV-03 | `idOf`, `typeOf`, `add`, `dispatch` (exceptions caught) |
| `Codec` | class | `src/reflection/Codec.hpp`, `.cpp` | SRV-04 | generic `encode` / `decode`: lists, structs, enums, `Entity` via `WireContext`, limits |
| `ReliableChannel` | class | `src/net/ReliableChannel.hpp`, `.cpp` | SRV-05 | `send`, `onAck`, `accept`, `due` |
| `StreamFramer` | class | `src/net/StreamFramer.hpp`, `.cpp` | SRV-06 | `feed`, `next`, size limit |
| Server tests | tests | `tests/net/players/`, `tests/net/messages/`, `tests/reflection/codec/` | SRV-07 | players, handshake, messages on three transports, codec fuzzing |

## Rooms (lobby, parties, rooms) — SRV-08 to SRV-15 · [reference 14](reference/14-rooms.md) · [server tasks](../server/tasks.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `PartyId`, `RoomId`, `RoomState`, `RoomSettings`, `Party`, `RoomInfo`, room events | types, events | `inc/room/RoomTypes.hpp`, `inc/room/RoomEvents.hpp` | SRV-08 | fields, `RoomFinished::resultAs<T>`, registration |
| `ServiceContext`, `LobbyContext` (+ `Context` base change) | class | `inc/ServiceContext.hpp`, `inc/LobbyContext.hpp`, `src/core/*Context.cpp` | SRV-09 | accessors; private constructors |
| `PartyTable`, `RoomDirectory` | private class | `src/room/` | SRV-10 | `createSolo`, `move`, `remove`, `setLeader`…; `add`, `remove`, `setState`, `enter`, `leave`, `roomOf` |
| `RoomRuntime`, `RoomMailbox` | private class | `src/room/` | SRV-11 | `start`, `tick`, mailboxes; system instances per room |
| `WorkerPool`, `IRoomHost`, `ThreadPoolRoomHost` | private class | `src/room/` | SRV-12 | `submit`; `start`, `stop`, `post`, `collect` |
| `Lobby`, `RoomHandle`, server lobby loop | class | `inc/room/Lobby.hpp`, `inc/room/RoomHandle.hpp`, `src/core/Engine.cpp` | SRV-13 | every `Lobby` method, `keep`, `reportFinished<T>`, module hooks |
| `PlayerRouting`, `RoomNetwork`, `ClientLobby` | class | `src/net/PlayerRouting.hpp`, `inc/room/RoomNetwork.hpp`, `inc/room/ClientLobby.hpp` | SRV-14 | routing, per-room replication, client requests |
| Room tests | tests | `tests/room/` | SRV-15 | isolation, crash, party and room flows (ThreadSanitizer) |

Also changed: `Registry::singleton<T>` (ENG-18), `Scheduler::add` per-room copies / `addFactory` / `instantiate` (ENG-26),
`IGameModule::onLobbyStart` / `onRoomStart` (ENG-11), `EngineConfig::rooms` (ENG-07).

## 11 · Scenes, prefabs, assets — ENG-40 to ENG-43 · [reference 11](reference/11-scenes-assets.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `AssetManager`, `AssetStats` | class, struct | `inc/assets/AssetManager.hpp`, `src/assets/AssetManager.cpp` | ENG-40 | ctor, `attach` 🔒, `texture`, `font`, `sound`, `music`, `script`, `pathOf`, `retain`, `release`, `collectUnused`, `reloadChanged`, `resolve` (rejects `../`), `stats`; fallbacks |
| `Prefab`, `ComponentData`, `PrefabChild` | struct | `inc/scene/Prefab.hpp` | ENG-41 | fields only |
| `PrefabLibrary` | class | `inc/scene/Prefab.hpp`, `src/scene/PrefabLibrary.cpp` | ENG-41 | ctor, `get` (cached), `findByKey`, `instantiate` (children, overrides), `clear` |
| `SceneLoader` | class | `inc/scene/SceneLoader.hpp`, `src/scene/SceneLoader.cpp` | ENG-42 | ctor, `load` (2 passes, `file:line`, no partial scene), `save` (diff to prefab, skip `Transient`) |
| `Scene` | class | `inc/scene/Scene.hpp` | ENG-43 | `name`, `path`, `registry` |
| `SceneManager` | class | `inc/scene/SceneManager.hpp`, `src/scene/SceneManager.cpp` | ENG-43 | ctor, `load` 🔒, `switchTo` (deferred), `markPersistent`, `save`, `world`, `activeName`, `prefabs`, `applyPending` 🔒, `clear` 🔒; bad file keeps the current scene |

## 12 · Services, tools, scripting — ENG-44 to ENG-49 · [reference 12](reference/12-services.md)

| Item | Kind | File | Task | To implement |
|---|---|---|---|---|
| `LogLevel`, `LogRecord`, `LogConfig` | enum, struct | `inc/services/Logger.hpp` | ENG-44 | fields only |
| **`ILogSink`** | interface | `inc/services/Logger.hpp` | ENG-44 | declares `write`, `flush` |
| `ConsoleSink`, `FileSink`, `ConsoleUiSink` | class | `src/services/Logger.cpp` | ENG-44 | `write` (colour, rotating file, developer console) |
| `Logger` | class | `inc/services/Logger.hpp`, `src/services/Logger.cpp` | ENG-44 | ctor, `trace`, `debug`, `info`, `warn`, `error`, `tagged`, `setLevel`, `addSink`, `flush`; thread-safe |
| `CVarFlags`, `CVarInfo` | enum, struct | `inc/services/CVars.hpp` | ENG-45 | values, fields |
| `CVar<T>` | template | same | ENG-45 | `value`, `set` (range check, `onChange`), `onChange` |
| `CVars` | class | same, `src/services/CVars.cpp` | ENG-45 | `define`, `set`, `get`, `find`, `forEach`, `loadArchive`, `saveArchive`; the engine CVars (`r_*`, `snd_*`, `in_deadzone`, `net_*`, `asset_hot_reload`, `phys_grid_cell`, `sim_tick_rate`) |
| `Console`, `CommandFn` | class, alias | `inc/services/Console.hpp`, `src/services/Console.cpp` | ENG-46 | ctor, `registerCommand`, `execute`, `print`, `history`, `output`, `complete`, `open`, `toggle`; 14 built-in commands, `Cheat` check |
| `Metrics`, `MetricsSnapshot` | class, struct | `inc/services/Metrics.hpp`, `src/services/Metrics.cpp` | ENG-47 | `beginFrame` 🔒, `endFrame` 🔒, `record`, `fps`, `frameMs`, `history` (ring of 300), `snapshot` |
| Debug overlay | system | `src/systems/DebugOverlaySystem.cpp` | ENG-47 | `engine.debug_overlay`: console, FPS graph, system timings, lagometer (ImGui, ImPlot) |
| `ScriptHost` | struct | `inc/script/IScripting.hpp` | ENG-48 | references only |
| **`IScripting`** | interface | `inc/script/IScripting.hpp` | ENG-48 | declares `bind`, `load`, `unload`, `reload`, `call`, `has` |
| Script systems | system | `src/script/ScriptSystems.cpp` | ENG-48 | `engine.script.lifecycle`, `engine.script.update`, `engine.script.collision`; error → script disabled |
| `LuaScripting` | plugin | `plugins/scripting_lua/LuaScripting.*`, `LuaApi.cpp`, `Sandbox.cpp` | ENG-49 | every `IPlugin` + `IScripting` method; sol2 API of guide 11; sandbox; instruction limit |

## 13 · Engine systems — ENG-50 to ENG-52 · [reference 13](reference/13-components-systems.md)

| System | Stage | File | Task |
|---|---|---|---|
| `registerEngineSystems` (registers all below with names and order) | — | `src/systems/RegisterEngineSystems.cpp` | ENG-50 |
| `engine.previous_position`, `engine.movement`, `engine.hierarchy`, `engine.parallax`, `engine.lifetime` | FixedUpdate | `src/systems/MovementSystems.cpp` | ENG-50 |
| `engine.collision` (uniform grid `phys_grid_cell`, AABB, layer/mask both ways, `Collision` event) | FixedUpdate | `src/systems/CollisionSystem.cpp` | ENG-51 |
| `engine.animation` | FixedUpdate | `src/systems/AnimationSystem.cpp` | ENG-52 |
| `engine.render.sprites` (interpolated with `alpha`), `engine.render.text`, `engine.render.debug` | Render | `src/systems/RenderSystems.cpp` | ENG-52 |
| `engine.net.*` (6) | see block 10 | `src/systems/NetSystems.cpp` | ENG-39 |
| `engine.script.*` (3) | FixedUpdate | `src/script/ScriptSystems.cpp` | ENG-48 |
| `engine.debug_overlay` | Render | `src/systems/DebugOverlaySystem.cpp` | ENG-47 |

## 14 · Tests, benchmark, tools — ENG-53 to ENG-59

| Item | Folder | Task | To implement |
|---|---|---|---|
| Core and module tests, test plugin, test game | `tests/core/`, `tests/module/`, `tests/fixtures/` | ENG-53 | load/unload, version mismatch, Null fallback |
| ECS and reflection tests | `tests/ecs/`, `tests/reflection/` | ENG-54 | generations, swap-and-pop, views, commands, hooks, YAML/binary round trips |
| Scheduler, events, input, scene, session tests | `tests/scheduler/` … `tests/net/` | ENG-55 | ordering and cycles, dispatch order, edges, two-pass loading, malformed datagrams |
| ECS storage benchmark | `benchmarks/ecs_storage/` | ENG-56 | sparse set vs sparse array vs archetypes |
| Headless smoke test | `tests/headless/` | ENG-57 | client with `null` plugins runs `--ticks 1000`, exit code 0 |
| Editor (Part 2, Track 1) | `tools/editor/` | ENG-59 | hierarchy, inspector, scene save |

---

## Order

Tasks are done by **wave**: a task's wave is one more than the highest wave of its dependencies, so
every task of a wave can be done in parallel once the earlier waves are done. The waves are computed
from the dependencies by `docs/tasks/generate.py` and shown in [`tasks.md`](tasks.md),
[`../server/tasks.md`](../server/tasks.md), [`../tasks/README.md`](../tasks/README.md) and the **Wave**
column of the Notion boards.

Start with wave 1 (ENG-01, ENG-04, ENG-05, ENG-06, ENG-16). The whole engine runs for the first time
at waves 10–11 (ENG-08 `Engine`, ENG-09 `Context`, ENG-10 launcher, ENG-57 headless test); until then
each piece is checked by its unit tests.
