# Tasks — details

One page per task: goal, files, dependencies, what to implement, how to know it is done. Claiming happens on the Notion boards; this folder is where you read what a task means. Generated from `tasks.py` and `details.py` by `generate.py`: edit those, not the pages.

**Wave** = the order in which tasks can be done: a task's wave is one more than the highest wave of its dependencies, so every task of a wave can be done in parallel once the earlier waves are done. Take your next task from the lowest open wave.

## Engine tasks

Board: [Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7) · table: [`docs/engine/tasks.md`](../engine/tasks.md)

| Wave | ID | Phase | Task | Depends on |
|---|---|---|---|---|
| 1 | [ENG-01](ENG-01.md) | 00 Build | Engine CMake project, vcpkg manifest, presets, options, exports and C++23 setup | — |
| 1 | [ENG-04](ENG-04.md) | 01 Core | Core types: hash, Vec2/Rect/Color, Error, Result, BootError | — |
| 1 | [ENG-05](ENG-05.md) | 01 Core | Time and Clock | — |
| 1 | [ENG-06](ENG-06.md) | 01 Core | Signal and RAII Subscription | — |
| 1 | [ENG-16](ENG-16.md) | 03 ECS | Entity and EntityAllocator (generations, free list) | — |
| 2 | [ENG-02](ENG-02.md) | 00 Build | CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher | [ENG-01](ENG-01.md) |
| 2 | [ENG-11](ENG-11.md) | 02 Modules | IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE | [ENG-04](ENG-04.md) |
| 2 | [ENG-12](ENG-12.md) | 02 Modules | DynamicLibrary for Linux and Windows | [ENG-04](ENG-04.md) |
| 2 | [ENG-17](ENG-17.md) | 03 ECS | IStorage and paged SparseSet<T> | [ENG-16](ENG-16.md) |
| 2 | [ENG-24](ENG-24.md) | 04 Reflection | ByteWriter / ByteReader: bounds-checked, bit packing | [ENG-04](ENG-04.md) |
| 2 | [ENG-28](ENG-28.md) | 07 Input | Keys, RawInputEvent, ActionId | [ENG-04](ENG-04.md) |
| 2 | [ENG-44](ENG-44.md) | 12 Services | Logger and sinks (console, file, developer console) | [ENG-04](ENG-04.md) |
| 2 | [ENG-60](ENG-60.md) | 01 Core | OS layer: secure random, user data folder, executable folder, quit signals | [ENG-04](ENG-04.md) |
| 2 | [ENG-61](ENG-61.md) | 00 Build | CI on GitHub Actions (GCC, Clang, MSVC; dynamic and static) and code style files | [ENG-01](ENG-01.md) |
| 3 | [ENG-03](ENG-03.md) | 00 Build | C++23 feature probe compiled on GCC, Clang and MSVC in CI | [ENG-01](ENG-01.md), [ENG-61](ENG-61.md) |
| 3 | [ENG-13](ENG-13.md) | 02 Modules | StaticModuleTable for ENGINE_DYNAMIC_MODULES=OFF | [ENG-11](ENG-11.md) |
| 3 | [ENG-22](ENG-22.md) | 04 Reflection | TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum) | [ENG-17](ENG-17.md), [ENG-04](ENG-04.md) |
| 3 | [ENG-26](ENG-26.md) | 05 Scheduler | Scheduler: stages, names, after/before, roles, stats, failure policy | [ENG-04](ENG-04.md), [ENG-44](ENG-44.md) |
| 3 | [ENG-30](ENG-30.md) | 08 Rendering | Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform | [ENG-04](ENG-04.md), [ENG-11](ENG-11.md) |
| 3 | [ENG-36](ENG-36.md) | 10 Network | INetwork and network types | [ENG-11](ENG-11.md) |
| 3 | [ENG-56](ENG-56.md) | 14 Tests & bench | ECS storage benchmark: sparse set vs sparse array vs archetypes | [ENG-17](ENG-17.md) |
| 3 | [ENG-62](ENG-62.md) | 00 Build | Installable engine: install, export, find_package(rtype_engine), CPack packages | [ENG-02](ENG-02.md) |
| 4 | [ENG-14](ENG-14.md) | 02 Modules | ModuleManager: load by kind and name, version check, reverse unload | [ENG-12](ENG-12.md), [ENG-13](ENG-13.md), [ENG-11](ENG-11.md), [ENG-44](ENG-44.md) |
| 4 | [ENG-18](ENG-18.md) | 03 ECS | Registry: typed and type-erased access, identity, hierarchy | [ENG-17](ENG-17.md), [ENG-22](ENG-22.md), [ENG-06](ENG-06.md) |
| 4 | [ENG-23](ENG-23.md) | 04 Reflection | YAML wrappers and field codecs | [ENG-22](ENG-22.md) |
| 4 | [ENG-25](ENG-25.md) | 04 Reflection | Engine components and their registration | [ENG-22](ENG-22.md), [ENG-30](ENG-30.md) |
| 4 | [ENG-27](ENG-27.md) | 06 Events | EventBus: queued, immediate, thread inbox; engine events | [ENG-06](ENG-06.md), [ENG-22](ENG-22.md) |
| 4 | [ENG-37](ENG-37.md) | 10 Network | network_asio plugin: UDP and TCP on an I/O thread | [ENG-36](ENG-36.md), [ENG-02](ENG-02.md) |
| 5 | [ENG-07](ENG-07.md) | 01 Core | EngineConfig: YAML file and command-line overrides | [ENG-04](ENG-04.md), [ENG-23](ENG-23.md) |
| 5 | [ENG-19](ENG-19.md) | 03 ECS | View<Ts...> and exclude<Xs...> | [ENG-18](ENG-18.md) |
| 5 | [ENG-20](ENG-20.md) | 03 ECS | Commands buffer (spawn, prefab spawn, destroy, add, set, remove) | [ENG-18](ENG-18.md) |
| 5 | [ENG-21](ENG-21.md) | 03 ECS | Component hooks onCreate / onReplace / onDestroy | [ENG-18](ENG-18.md), [ENG-06](ENG-06.md) |
| 5 | [ENG-29](ENG-29.md) | 07 Input | InputMap and ActionState: input.yaml, rebinding, accessibility options, masks | [ENG-28](ENG-28.md), [ENG-23](ENG-23.md) |
| 5 | [ENG-34](ENG-34.md) | 09 Audio | IAudio, PlayParams, buses, captions | [ENG-11](ENG-11.md), [ENG-27](ENG-27.md) |
| 5 | [ENG-38](ENG-38.md) | 10 Network | NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts | [ENG-36](ENG-36.md), [ENG-27](ENG-27.md), [ENG-24](ENG-24.md) |
| 5 | [ENG-45](ENG-45.md) | 12 Services | CVars: typed variables, flags, ranges, archive | [ENG-06](ENG-06.md), [ENG-23](ENG-23.md), [ENG-60](ENG-60.md) |
| 5 | [ENG-48](ENG-48.md) | 12 Services | IScripting, Script component, script systems | [ENG-18](ENG-18.md), [ENG-27](ENG-27.md), [ENG-11](ENG-11.md) |
| 5 | [ENG-51](ENG-51.md) | 13 Engine systems | Collision system: uniform grid, AABB, layers, Collision events | [ENG-25](ENG-25.md), [ENG-27](ENG-27.md) |
| 5 | [ENG-52](ENG-52.md) | 13 Engine systems | Render systems: sprites (interpolated), text, animation, debug colliders | [ENG-25](ENG-25.md), [ENG-30](ENG-30.md) |
| 5 | [ENG-53](ENG-53.md) | 14 Tests & bench | Unit tests: core and modules, with a test plugin and a test game | [ENG-14](ENG-14.md) |
| 6 | [ENG-15](ENG-15.md) | 02 Modules | Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting | [ENG-11](ENG-11.md), [ENG-30](ENG-30.md), [ENG-34](ENG-34.md), [ENG-36](ENG-36.md), [ENG-48](ENG-48.md) |
| 6 | [ENG-31](ENG-31.md) | 08 Rendering | platform_sfml plugin: window, events to InputMap, batching renderer | [ENG-30](ENG-30.md), [ENG-29](ENG-29.md), [ENG-11](ENG-11.md), [ENG-02](ENG-02.md) |
| 6 | [ENG-35](ENG-35.md) | 09 Audio | audio_miniaudio plugin | [ENG-34](ENG-34.md), [ENG-02](ENG-02.md) |
| 6 | [ENG-39](ENG-39.md) | 10 Network | Replication: NetworkIdMap, net components, snapshot and input systems | [ENG-38](ENG-38.md), [ENG-18](ENG-18.md), [ENG-22](ENG-22.md), [ENG-29](ENG-29.md), [SRV-04](SRV-04.md) |
| 6 | [ENG-40](ENG-40.md) | 11 Scenes & assets | AssetManager: handles, cache, refcount, fallback, hot reload | [ENG-30](ENG-30.md), [ENG-34](ENG-34.md), [ENG-48](ENG-48.md) |
| 6 | [ENG-46](ENG-46.md) | 12 Services | Console and built-in commands | [ENG-45](ENG-45.md), [ENG-27](ENG-27.md) |
| 6 | [ENG-49](ENG-49.md) | 12 Services | scripting_lua plugin: sol2 API, sandbox, reload | [ENG-48](ENG-48.md), [ENG-02](ENG-02.md) |
| 6 | [ENG-50](ENG-50.md) | 13 Engine systems | Systems: previous_position, movement, hierarchy, parallax, lifetime | [ENG-25](ENG-25.md), [ENG-26](ENG-26.md), [ENG-20](ENG-20.md) |
| 6 | [ENG-54](ENG-54.md) | 14 Tests & bench | Unit tests: ECS, reflection, commands, hooks | [ENG-18](ENG-18.md), [ENG-20](ENG-20.md), [ENG-21](ENG-21.md), [ENG-22](ENG-22.md) |
| 7 | [ENG-32](ENG-32.md) | 08 Rendering | Dear ImGui and ImPlot in platform_sfml (debug UI hooks) | [ENG-31](ENG-31.md) |
| 7 | [ENG-33](ENG-33.md) | 08 Rendering | Colour-blindness filters and UI scale in platform_sfml | [ENG-31](ENG-31.md) |
| 7 | [ENG-41](ENG-41.md) | 11 Scenes & assets | PrefabLibrary and instantiate | [ENG-23](ENG-23.md), [ENG-18](ENG-18.md), [ENG-40](ENG-40.md) |
| 7 | [ENG-58](ENG-58.md) | 15 Part 2 | platform_raylib plugin: 2D, then optional 3D | [ENG-31](ENG-31.md) |
| 8 | [ENG-42](ENG-42.md) | 11 Scenes & assets | SceneLoader: two passes, errors with file:line, save | [ENG-41](ENG-41.md) |
| 8 | [ENG-47](ENG-47.md) | 12 Services | Metrics and engine.debug_overlay (ImGui, ImPlot) | [ENG-32](ENG-32.md), [ENG-26](ENG-26.md) |
| 9 | [ENG-43](ENG-43.md) | 11 Scenes & assets | Scene and SceneManager: switch, persistent entities | [ENG-42](ENG-42.md), [ENG-27](ENG-27.md) |
| 10 | [ENG-08](ENG-08.md) | 01 Core | Engine: lifecycle, member order, fixed-step loop, error policy | [ENG-07](ENG-07.md), [ENG-14](ENG-14.md), [ENG-26](ENG-26.md), [ENG-43](ENG-43.md), [ENG-27](ENG-27.md), [ENG-29](ENG-29.md), [ENG-30](ENG-30.md), [ENG-38](ENG-38.md), [ENG-46](ENG-46.md) |
| 10 | [ENG-55](ENG-55.md) | 14 Tests & bench | Unit tests: scheduler, events, input, scenes, session | [ENG-26](ENG-26.md), [ENG-27](ENG-27.md), [ENG-29](ENG-29.md), [ENG-43](ENG-43.md), [ENG-38](ENG-38.md) |
| 10 | [ENG-59](ENG-59.md) | 15 Part 2 | Editor (Track 1): hierarchy, inspector, scene save | [ENG-43](ENG-43.md), [ENG-32](ENG-32.md) |
| 11 | [ENG-09](ENG-09.md) | 01 Core | Context facade | [ENG-08](ENG-08.md), [SRV-09](SRV-09.md) |
| 11 | [ENG-10](ENG-10.md) | 01 Core | Generic launcher main() and signal handling | [ENG-08](ENG-08.md), [ENG-02](ENG-02.md), [ENG-60](ENG-60.md) |
| 11 | [ENG-57](ENG-57.md) | 14 Tests & bench | Headless smoke test in CI: client with Null plugins runs 1000 ticks | [ENG-08](ENG-08.md), [ENG-15](ENG-15.md), [ENG-61](ENG-61.md) |

## Server tasks

Board: [Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d) · table: [`docs/server/tasks.md`](../server/tasks.md)

| Wave | ID | Phase | Task | Depends on |
|---|---|---|---|---|
| 1 | [SRV-19](SRV-19.md) | 04 Tests | Wireshark dissector for MSEP (Lua) | — |
| 4 | [SRV-04](SRV-04.md) | 02 Messages | Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits | [ENG-22](ENG-22.md), [ENG-24](ENG-24.md) |
| 5 | [SRV-06](SRV-06.md) | 03 Transports | StreamFramer: TCP message framing with size limit | [ENG-37](ENG-37.md) |
| 5 | [SRV-08](SRV-08.md) | 05 Rooms | Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events | [ENG-22](ENG-22.md), [ENG-27](ENG-27.md), [ENG-36](ENG-36.md) |
| 6 | [SRV-01](SRV-01.md) | 01 Players | Players: PlayerTable, startServer / connectTo, player events, names, kick, ping | [ENG-38](ENG-38.md) |
| 6 | [SRV-05](SRV-05.md) | 03 Transports | ReliableChannel: packet acks, RTT and loss, reliable ordered messages | [ENG-38](ENG-38.md) |
| 6 | [SRV-10](SRV-10.md) | 05 Rooms | PartyTable and RoomDirectory: parties, leaders, room records, maxRooms | [SRV-08](SRV-08.md) |
| 7 | [SRV-02](SRV-02.md) | 01 Players | Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits | [SRV-01](SRV-01.md), [SRV-06](SRV-06.md), [ENG-22](ENG-22.md), [ENG-60](ENG-60.md) |
| 7 | [SRV-03](SRV-03.md) | 02 Messages | Typed messages: MessageDispatcher, send / broadcast / on<T> | [SRV-01](SRV-01.md), [SRV-04](SRV-04.md), [SRV-05](SRV-05.md), [SRV-06](SRV-06.md) |
| 8 | [SRV-07](SRV-07.md) | 04 Tests | Tests: players, handshake, messages, codec, malformed input | [SRV-01](SRV-01.md), [SRV-02](SRV-02.md), [SRV-03](SRV-03.md), [SRV-04](SRV-04.md), [SRV-05](SRV-05.md), [SRV-06](SRV-06.md) |
| 8 | [SRV-16](SRV-16.md) | 01 Players | Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink | [SRV-02](SRV-02.md), [SRV-05](SRV-05.md) |
| 9 | [SRV-09](SRV-09.md) | 05 Rooms | Contexts: ServiceContext, LobbyContext, one Context per room | [SRV-08](SRV-08.md), [ENG-40](ENG-40.md), [ENG-44](ENG-44.md), [ENG-45](ENG-45.md), [ENG-47](ENG-47.md) |
| 12 | [SRV-11](SRV-11.md) | 05 Rooms | RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages | [ENG-09](ENG-09.md), [ENG-18](ENG-18.md), [ENG-20](ENG-20.md), [ENG-26](ENG-26.md), [ENG-27](ENG-27.md), [ENG-39](ENG-39.md), [ENG-43](ENG-43.md), [SRV-09](SRV-09.md) |
| 13 | [SRV-12](SRV-12.md) | 05 Rooms | WorkerPool, IRoomHost and ThreadPoolRoomHost: rooms in parallel on a thread pool | [SRV-11](SRV-11.md) |
| 14 | [SRV-13](SRV-13.md) | 05 Rooms | Lobby and RoomHandle: the server game API and the server loop | [SRV-10](SRV-10.md), [SRV-12](SRV-12.md), [SRV-03](SRV-03.md) |
| 15 | [SRV-14](SRV-14.md) | 05 Rooms | Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset | [SRV-13](SRV-13.md), [ENG-39](ENG-39.md) |
| 16 | [SRV-15](SRV-15.md) | 04 Tests | Tests: parallel rooms, isolation, crash in a room, party and room flows | [SRV-13](SRV-13.md), [SRV-14](SRV-14.md) |
| 16 | [SRV-17](SRV-17.md) | 03 Transports | Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize | [ENG-39](ENG-39.md), [SRV-05](SRV-05.md), [SRV-14](SRV-14.md) |
| 17 | [SRV-18](SRV-18.md) | 04 Tests | Fuzz tests: UDP packets, TCP frames, handshake, snapshot and input decoding | [SRV-07](SRV-07.md), [SRV-17](SRV-17.md) |
| 17 | [SRV-20](SRV-20.md) | 04 Tests | Network measurements: snapshot sizes, bandwidth under netem (5 KB/s), reliable latency | [SRV-16](SRV-16.md), [SRV-17](SRV-17.md) |
