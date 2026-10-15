# Changelog

All notable changes to this project are documented here, in first person,
under the date they landed. Each entry names its author when it is not the
person writing the section.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2026-10-15] — engine reflection: ENG-22

### Added

- `TypeKey` (`TypeCategory`, `TypeName<T>`, `HasTypeName`, `typeKey<T>()`, the five type macros; qualified
  names keep their last part), `TypeInfo` (`FieldKind`, `FieldInfo`, `EnumInfo`, `TypeFlags`, `Transport`,
  `WireContext`), the builders (`FieldsBuilder<T>` with deducing this, component, event, message, struct and
  enum builders, `FieldKindOf<M>` for field kinds) and `TypeRegistry` (`add<T>()` by category, lookups,
  `formatFingerprint`, `closestName`, `beginGameTypes` / `clearGameTypes`, `DuplicateName` boot error).
- 7 tests and 3 compile-fail tests (type without macro, option of another category, unsupported field type).

### Changed

- `field()`, `maxLength()`, `quantize()` take `this Self&& self`: with `Self&` the chain did not compile on
  the temporary returned by `add<T>()`. Reference 04 updated, with `beginGameTypes()` (needed by
  `clearGameTypes`) and the `FieldKindOf` / `enumInfoOf` helpers.

## [2026-10-15] — engine ECS: ENG-16, ENG-17, ENG-56

### Added

- ECS storage benchmark (ENG-56, `engine/benchmarks/ecs_storage/`, built with `ENGINE_BUILD_BENCHMARKS=ON`):
  sparse set vs sparse array vs a minimal archetype store, 10 000 entities; Google Benchmark added to
  `vcpkg.json`. Results and reading in the new comparative study, `docs/study/01-ecs-storage.md`.
- `IStorage` (with `ComponentId`) and the paged `SparseSet<T>` (pages of 4096, packed entities and
  components, swap-and-pop remove, raw access for type-erased callers); `IStorage.hpp` written by Sedjro;
  9 tests.
- `Entity` (index + generation, `null()`, `isNull()`, `==`, `std::hash`) and `EntityAllocator` (free list,
  generation bumped on release, `alive`, `aliveCount`), written by Sedjro; 7 tests in `engine_ecs_tests`.

### Changed

- Every `.inl` starts with `#pragma once`, includes its own header and the standard headers it uses, so an
  editor can open it alone (`Hash.inl`, `Signal.inl`, `Entity.inl`).

## [2026-10-15] — engine core: ENG-04, ENG-05, ENG-06, ENG-60

### Added

- `Time` (fixed step, tick, alpha, simulated and frame time) and `Clock` on `steady_clock` (ENG-05);
  3 tests.
- `Signal<void(Args...)>` and RAII `Subscription` (ENG-06): shared state so a Subscription can
  outlive its Signal; connect, disconnect and emit are safe from inside a listener; 11 tests.
- OS layer `engine::os` (ENG-60, private `src/os/`): `secureRandom` (getrandom / BCryptGenRandom),
  `userDataDir` (XDG_DATA_HOME or ~/.local/share / APPDATA), `executableDir` (/proc/self/exe /
  GetModuleFileNameW), `onQuitSignal` (SIGINT, SIGTERM / console control handler); `bcrypt` linked on
  Windows; 5 tests (the signal test runs on Linux only).
- Core types: `fnv1a64` (compile time), `Vec2`, `Rect`, `Color`, `Errc`, `Error`, `Result<T>`
  (`std::expected` or `tl::expected`), `fail()`, `BootError`.
- First test setup: `engine_add_test` and Catch2 in `engine/tests/CMakeLists.txt`; 7 tests for ENG-04.
- Epitech header at the top of every file (rule in `docs/engine/folder-tree.md`): ENG-04 files, and
  every skeleton through `generate.py` (C++ `/* ** */`, CMake `##`, Lua `--`).
- The same header on the build files already merged: the CMake files, `FeatureProbe.cpp`, the CI
  workflow, `.clang-format`, `.clang-tidy`, `.editorconfig`, `.gitattributes`, `.gitignore`. The two
  JSON files (`CMakePresets.json`, `vcpkg.json`) cannot hold comments and keep none.

### Changed

- Code style: a `.hpp` holds declarations only; bodies go to the `.cpp`, templates and `constexpr`
  functions to a `.inl` included at the end of the header. `Vec2`, `Rect`, `Color` (now `.cpp`, no longer
  `constexpr`), `fail()`, `BootError`, `Subscription` and the non-template part of `Signal`
  (`SignalBase`, `SignalEmission`, `SignalSlot` in `Signal.cpp`) follow it.
- Code style: no comments in code besides the Epitech header; `private:` before `public:`
  (`docs/engine/folder-tree.md`). Core files and tests rewritten accordingly; `.clang-format` no longer
  adds `// namespace` comments (`FixNamespaceComments: false`).
- GCC/Clang: `-Wno-missing-field-initializers` (aggregates are filled partially on purpose).
  MSVC: `/wd4251 /wd4275` (standard types in exported classes; same runtime everywhere).

## [2026-10-15] — engine build (ENG-01, ENG-61)

### Added

- Root `CMakeLists.txt`, `vcpkg.json` (pinned baseline; Lua held at 5.4 for sol2) and
  `CMakePresets.json`; the engine library with its generated `ENGINE_API` header (ENG-01).
- CI workflow and code style files (ENG-61). GitHub refuses to start the jobs for now: the
  organisation has no Actions budget.
- Guide 29 · Every command: installing the tools, presets and options, running, console, shipping,
  style, documentation scripts, Git workflow.
- ENG-02: `engine_add_plugin`, `engine_add_game_module`, `engine_add_launcher` (new `ROLE` argument to
  pick the game build linked in static mode). Game targets are `game_<name>_<role>`, so launchers may
  be called `<name>_client`; files keep `lib<name>_<role>`. Tested in both modes: modules land in
  `bin/plugins` and `bin/games`, static registrars run.
- ENG-03: `cmake/EngineFeatures.cmake` probes the six allowed C++23 features. GCC 13 lacks deducing
  this, so GCC 14 is now the confirmed minimum and the GCC presets call `g++-14`; Clang 18 uses the
  `tl::expected` fallback. C++20 module scanning is turned off (it needs `clang-scan-deps`).
- ENG-62: `cmake --install` gives a usable engine (`find_package(rtype_engine)` tested from an outside
  project), a game installed against it ships the engine library, and `engine_enable_packaging`
  makes `cpack --preset linux-gcc-release` produce `r-type-0.1.0-Linux.tar.gz`. Package presets added.

### Fixed

- Static presets produced a shared engine (preset inheritance order).
- Guide 02 used `find_package(engine)`; it is `find_package(rtype_engine)`. Guide 02 and reference 01
  still named one game module per game; ADR 0040 builds two.

## [2026-10-15] — multiplatform and tooling

### Added

- ADR 0044: Linux and Windows built and tested; macOS best effort; Android and iOS designed for
  (static mode as the mobile build, touch and suspend events reserved, read-only game root).
  `libengine` is shared in dynamic mode with an `ENGINE_API` export header and hidden visibility on
  every platform; operating-system calls live in `engine/src/os/`; portable coding rules.
- ADR 0045: vcpkg in manifest mode (chosen by the team), `CMakePresets.json`, a root `CMakeLists.txt`,
  CI on GitHub Actions (GCC, Clang, MSVC; dynamic and static; format check), code style files, an
  installable engine (`find_package(rtype_engine)`) and CPack archives; Catch2 for tests.
- Tasks ENG-60 (OS layer), ENG-61 (CI and code style), ENG-62 (install and packaging); tasks can now
  own files at the repository root (`root_files`).

### Changed

- ENG-01 creates the vcpkg manifest, the presets, the root project and the export header; ENG-03 and
  ENG-57 depend on the CI; ENG-10, ENG-45 and SRV-02 use the OS layer; ENG-27, ENG-28 reserve the
  mobile events; ENG-40 checks path case. References 01, 02, 06, 07, 11, 12 and guides 01, 24, 27
  updated; guide 01 now names the two game modules of ADR 0040.

## [2026-10-14] — protocol renamed MSEP

### Changed

- The protocol is now called MSEP (Magic Shifter Engine Protocol) instead of RTEP, in the RFC, the
  references, the tasks and the architecture pages. The Wireshark dissector is `tools/wireshark/msep.lua`.
  Bytes are unchanged.

### Fixed

- `generate.py` wrote `//` comments in Lua skeletons; Lua files now get `--`.

## [2026-10-14] — network part done: RTEP v1.0

### Added

- The team delegated the remaining choices to my recommendations after D1. `docs/network/04-decisions.md`
  now holds D1–D36, answering the 37 questions of step 2 (frame, session, reliability, replication,
  lobby and rooms).
- `docs/network/rfc.md`: RTEP v1.0, the protocol RFC. Part I (engine protocol): conventions, 16-byte
  UDP header, TCP frames, field encoding, handshake and admission, presence, reconnection, packet acks
  and ordered reliable messages, lobby and room messages, replication (send until acknowledged,
  budget, priority, relevance, quantisation), errors, security, why TCP, constants, and byte-exact
  examples. Part II: R-Type's messages (provisional until the Game part).
- `docs/network/06-scenarios.md`: 40 scenarios walked through the RFC; four precisions added to it.
- `docs/network/07-prototypes.md`: Wireshark dissector, fuzz tests and measurements, as tasks.
- ADR 0043: what the game sees of protocol v1 (new reasons and messages, admission API, reconnection
  events and settings, online players, refused requests, room reasons, optimisation settings).
- Tasks SRV-16 (reconnection), SRV-17 (replication v1), SRV-18 (fuzz), SRV-19 (Wireshark), SRV-20
  (measurements), with skeletons.

### Changed

- References 04, 06, 10, 13, 14, `game-api.md`, guides 12, 15, 27, 28 describe the v1 API.
- Tasks ENG-22, ENG-38, ENG-39, SRV-01 to SRV-07, SRV-13, SRV-14 point to RFC sections and gain the
  matching work. Step 8: version 1 frozen.

## [2026-10-14] — network part, step 4, round 1: byte order

### Added

- `docs/network/04-decisions.md`, D1: big-endian everywhere on the wire, floats as their IEEE 754
  bit pattern. Reference 04, guide 27 and ENG-24 updated.

## [2026-10-14] — network part, step 3: layers

### Added

- `docs/network/03-layers.md`: transport, frame, session, reliability, engine messages (lobby,
  replication), game messages; each layer's role, its step 4 questions and the code already planned
  for it. Engine and game messages share one typed mechanism with separate id ranges; reliability is
  UDP only. SRV-03 and SRV-05 note it.

## [2026-10-14] — network part, step 2 complete: reconnection

### Added

- `docs/network/02-exchanges.md`, group 7 (reconnection): short cut, UDP address change, return
  within the grace period, grace expired, client or server restart; questions 32 to 37. Step 2 is
  complete: seven groups, 37 questions for step 4.

## [2026-10-14] — network part, step 2: end of game

### Added

- `docs/network/02-exchanges.md`, group 6 (end of game): normal end, failed room, shutdown during
  games, lobby ↔ room channel reserved for Part 2; questions 28 to 31.

## [2026-10-14] — network part, step 2: in game

### Added

- `docs/network/02-exchanges.md`, group 5 (in game): input and snapshot loop, spawn and destroy,
  reliable, unreliable and stream game messages, loss, delay and duplicates; questions 20 to 27.

## [2026-10-14] — network part, step 2: entering and leaving a room

### Added

- `docs/network/02-exchanges.md`, group 4 (rooms): starting, joining, creation refused, leaving,
  disconnection during a game, room in another process (Part 2); questions 16 to 19 (TCP/UDP
  ordering at entry, loading time, late packets of the previous room, content of `EnteredRoom`).

## [2026-10-14] — network part, step 2: lobby and party

### Added

- `docs/network/02-exchanges.md`, group 3 (lobby and party): arrival, invitation, leaving, member
  disconnecting, room list, game messages in the lobby; inventory and questions 12 to 15 (online
  players list, member names, refused requests, invitations during a game).

## [2026-10-14] — network part, step 2: presence

### Added

- `docs/network/02-exchanges.md`, group 2 (presence): heartbeat, voluntary leave, crash, lost
  machine, kick, server shutdown, garbage in a loop; inventory and questions 7 to 11 for step 4.

## [2026-10-14] — network part, step 2: connection

### Added

- `docs/network/02-exchanges.md`, group 1 (connection): our handshake Hello → Challenge → UdpLink →
  Accepted or Refused, on top of TCP's, its failure paths, the inventory table and six questions for
  step 4.
- ADR 0042: the engine checks only what it needs to decode (protocol version, format fingerprint),
  automatically; the game decides admission (version, password, bans…) with its own reason, through
  an extension point designed in step 4. Amends ADR 0037; reference 10, guides 12 and 27, SRV-02 updated.

## [2026-10-14] — network part, step 1: scope and who optimizes

### Added

- `docs/network/`: the Network part, step by step. Step 1 (scope) recorded in `01-scope.md`: what is
  already decided, the subject's network requirements read from the kick-off slides, the budgets
  (with the subject's 5 KB/s test link and duplicates), the threats, encryption out of scope.
- ADR 0041: the engine is used as a product, without its code. It gives the means (protocol fields,
  settings, extension points); the game team writes its prediction and reconciliation and tunes its
  bandwidth. Engine interpolation stays as a default a game can disable.

### Changed

- ADR 0033 amended by 0041; reference 10, guides 12 and 27 updated.

## [2026-10-14] — network part, step 1: server capacity

### Fixed

- `net.maxClients` still defaulted to 4, a value from before rooms: with 8 rooms allowed, the fifth
  player was refused with `ServerFull`. It keeps its meaning (connections to the whole server, lobby
  included) and now defaults to 32 (`rooms.maxRooms` 8 × 4 players). ADR 0038, reference 10 and
  guides 02 and 12 updated. `maxRooms` 8 stays a default to adjust once the prototype is measured.
- `docs/tasks/generate.py` added the "headers are private" line to folder READMEs again on every run
  (up to 23 copies in `src/reflection/`); it now writes it once.

## [2026-10-14] — server part, round 4: one game code, two builds

### Added

- E-12 decided and recorded as ADR 0040: a game is organised in `shared/`, `server/` and `client/` and
  `engine_add_game_module` builds `lib<name>_server` and `lib<name>_client`; the engine still loads one
  game module (the one of its role). The engine gets the same split: `net/` (both sides), `server/`
  (server features) and `client/` (client features), with an inclusion rule for game code.
- `ServerLoop` (`src/server/`): the server's main-thread loop leaves `Engine.cpp`.

### Changed

- Room files moved from `room/` to `net/` (`RoomTypes`, `RoomEvents`, `RoomNetwork`), `server/`
  (`Lobby`, `RoomHandle`, `LobbyContext` and the private classes) and `client/` (`ClientLobby`).
  Tasks SRV-08 to SRV-15 keep their content with the new paths.
- Robustness of the server (disconnection during a game, reconnection) moves to the Network part: the
  engine protocol provides the mechanism (tokens, grace period, events), the game the policy.

## [2026-10-14] — server part, round 3: room API

### Added

- We designed the room API and I recorded it as ADR 0039: lobby (server level), parties (a group that
  stays together, every player always in one, a player alone is a party of 1) and rooms (one game);
  `Lobby`, `RoomHandle`, `RoomNetwork`, `ClientLobby` and their events; client requests reach the game
  as lobby events and the game answers; starting a game is a game message.
- Reference 14 (lobby, parties and rooms) with the private classes (`PartyTable`, `RoomDirectory`,
  `RoomRuntime`, `RoomMailbox`, `IRoomHost`, `ThreadPoolRoomHost`, `WorkerPool`, `PlayerRouting`) and
  guide page 28. Server tasks SRV-08 to SRV-15 (waves 5 to 16) with their declarations.

### Changed

- ADR 0039 amends ADR 0022: three context types (`ServiceContext`, `Context` per room, `LobbyContext`).
- `IGameModule` gains `onLobbyStart` and `onRoomStart`; `onShutdown` receives a `ServiceContext`.
- `Registry::singleton<T>()` holds the state of one game; systems are instantiated once per room
  (`Scheduler::add` copies, `addFactory`); `EngineConfig` gets a `rooms` section.

## [2026-10-14] — server part, round 2: rooms

### Added

- We decided how several groups play at once on one server, and I recorded it as ADR 0038: the room
  is the unit of play (its own world, systems run, events, tick, players); the engine provides the
  room mechanics and the game the rules; rooms run in one process on a pool of worker threads in
  Part 1 (`ThreadPoolRoomHost`), and a one-process-per-room host (`ProcessRoomHost`) stays possible
  for bigger games without changing game code, thanks to four rules (state in the room, creation
  through the engine, messages only, redirection reserved in the protocol).
- Guide page 27 lists what bigger games would still need inside one match: parallel systems,
  interest management, world streaming.

### Not yet

- The room API, files and tasks come with the next Server round.

## [2026-10-14] — task waves

### Added

- Every task has a **wave**, computed from its dependencies: wave 1 has no dependency, a task's wave
  is one more than the highest wave of what it depends on. Tasks of one wave can be done in parallel.
  The task tables (repo and Notion) are sorted by wave, and each task page shows its wave. The
  hand-written "suggested order" of `to-implement.md` is replaced by the waves.

## [2026-10-14] — task details, server board, generated tree

### Added

- I added one detail page per task in `docs/tasks/` (66 pages: goal, files, dependencies, what it
  unlocks, what to read, a checklist of what to implement, how to know it is done, traps). The task
  tables link every ID to its page, and the Notion task pages carry the same content.
- I created the server task board (`docs/server/tasks.md` and Notion **Server — folder tree & tasks**)
  with SRV-01 to SRV-07, derived from ADR 0037: players, handshake, typed messages, generic codec,
  reliable UDP, TCP framing, tests.
- New private classes and their skeletons in `engine/src/net/` (`PlayerTable`, `Handshake`,
  `MessageDispatcher`, `ReliableChannel`, `StreamFramer`) and `engine/src/reflection/Codec`, documented
  in reference 10 ("Internal structure"). Private headers live next to their sources, outside `include/`.
- `docs/tasks/tasks.py` and `docs/tasks/details.py` are now the single source of the tasks;
  `python3 docs/tasks/generate.py` regenerates the tables, detail pages, folder tree, folder READMEs and
  missing skeletons. Every skeleton header now points to its detail page.
- Every task page now has a **Classes and methods** section: the declarations the task must write
  (classes, methods, functions, systems, tests), without their implementation. They come from the
  reference automatically, plus `docs/tasks/api.py` for what the reference describes only in prose
  (plugins, systems, components, events, private server classes, tests).
- The documentation viewer opens links between pages inside the viewer (they used to 404) and keeps
  the current page in the address (`index.html#tasks/SRV-03.md`).

### Changed

- ENG-38 is now the session core only (connections, inbox, validation, heartbeat, timeouts); the
  game-facing layers moved to SRV-01 and SRV-03. ENG-37 implements TCP from Part 1. ENG-39 depends on
  the generic codec (SRV-04).

## [2026-10-14] — server part, round 1: players and typed messages

### Added

- We opened the Server part with how the game talks to the network, and I recorded it as ADR 0037:
  the server is part of the engine and the game never touches sockets or bytes. It opens the server
  or connects explicitly (`startServer`, `stopServer`, `connectTo` with a player name, `disconnect`),
  sees players (`PlayerId`, `PlayerJoined`, `PlayerLeft`, `JoinedServer`, `LeftServer`) and sends
  typed messages (`send`, `broadcast`, `on<T>` callbacks run at the start of the frame).
- Messages are structs declared with `ENGINE_MESSAGE` and serialized by the engine from their fields,
  with default limits (text 256, lists 64) and `.maxLength(n)`. Each message chooses
  `unreliable()` (UDP), `reliable()` (UDP with acknowledgements, default) or `stream()` (TCP, for the
  lobby and text chat). `Entity` fields travel as `NetworkId`. A format fingerprint compared at the
  handshake refuses a client built from other game types.
- `ENGINE_STRUCT` for small records used inside other types, and list fields (`std::vector`).

### Changed

- ADR 0036 amends ADR 0027: `types.add<T>()` is the only registration method; the macro after the
  type gives its kind and `add` returns the matching builder, so wrong options do not compile.
  `addEvent` and `addEnum` are gone; `TypeBuilder` became `FieldsBuilder` and five builders.
- ADR 0037 amends ADR 0033: `host()` / `connect()` / `onMessage()` with raw bytes and
  `ConnectionId` are engine-internal now; `ClientConnected`, `ClientDisconnected`,
  `ConnectedToServer` and `DisconnectedFromServer` became the four player events; `NetOwner` holds a
  `PlayerId`; TCP is implemented from Part 1.
- The reference (04, 06, 10), the guide (03, 06, 12, 15), `game-api.md`, `to-implement.md`, the
  class summary and the titles of tasks ENG-22 and ENG-38 follow these decisions.

## [2026-10-12] — implementation list and game API

### Added

- I added `engine/to-implement.md`: every interface, class, macro, system and plugin class the engine
  team must code, with its file, its task and the methods that need a body, plus a suggested order.
- I added `engine/game-api.md`: everything a game may use from the engine (CMake, macros,
  `IGameModule`, `Context`, ECS, events, input, scenes, assets, drawing, sound, network, services,
  Lua), and what game code must never touch.

## [2026-10-12] — engine guide completed

### Added

- I added twelve guide pages: time and the game loop, the Context, collisions, game module and game
  state, hierarchy and identity, saving, roles, testing, building and shipping, an accessibility
  checklist, best practices, and what is not in the engine yet.
- The guide index is now grouped by topic, and troubleshooting covers collisions, scenes, saving,
  scripts, networking and Windows builds.

### Changed

- Writing those pages showed gaps in the reference, which I filled without changing any decision:
  the collision rule (layers and masks must match both ways, detection only, `trigger` for sensors,
  `phys_grid_cell`), `Commands::setParent` for attaching inside systems, the `null` plugin name for
  headless runs, the `--scene` and `--ticks` options, and the automatic load and save of `Archive`
  settings in `user/settings.yaml`.

### Known gap

- The engine has no UI widgets (buttons, focus, menu navigation without a mouse). It is listed in
  guide page 27 and needs its own design and ADR.

## [2026-10-09] — engine file skeletons

### Added

- I created every file of the engine tree as a skeleton (141 files: headers, sources, CMake, shader).
  Each one starts with a comment naming its task, its spec page and its ADR, then an empty
  `namespace engine`. Nothing is implemented: implementing is the task's work, claimed on the board.
- On the Notion board, the **File** field of every task now lists all of its files, relative to `engine/`.

## [2026-10-09] — engine architecture complete

### Added

- We validated every remaining engine block and I recorded them as ADRs 0025 to 0035 (modules,
  registry API, reflection, scheduler, events, input, rendering, audio, engine networking, scenes and
  assets, services and scripting). ADR 0033 also settles E-03: entities are identified on the wire by
  a `NetworkId`, never by their local handle.
- I wrote the architecture reference, one page per block, with every class, interface and method
  (`engine/reference/`), and the usage guide for game developers (`engine/guide/`, 15 pages).
- I created the engine folder tree (`engine/`, 58 folders with a README listing the files to write and
  their task) and `engine/folder-tree.md`, then derived 59 tasks from it (`engine/tasks.md` and the
  Notion board, self-assign).

### Changed

- Systems are now registered with a unique name (`s.add(Stage::FixedUpdate, "pong.bounce", fn)`) so
  they can be ordered, toggled and profiled; the Pong tutorial follows the final API.
- Components are named with `ENGINE_COMPONENT(Type)` and registered with `t.add<Type>()`; actions are
  read with `action("MoveUp")`. The tutorial and guide use these forms.
- `Signal` moved to `core/` so the ECS does not depend on the events block; `Engine.hpp`,
  `Context.hpp` and `EngineConfig.hpp` sit at the top of `include/engine/`.
- Every engine status in the documentation is now Locked.

## [2026-10-09] — block 1 locked

### Added

- We locked block 1 and I recorded it as ADRs 0021 to 0024: `Engine` as composition root with
  member order as shutdown order, `Context` as the only door from game code, Null Objects for absent
  services, and the error-handling policy (exit codes per boot stage, systems disabled instead of
  crashing the server, signals turned into a clean quit).

## [2026-10-09] — language, data and tooling decisions

### Added

- We locked four decisions and I recorded them as ADRs 0017 to 0020: C++23 with an allow-list of
  features and `tl::expected` as fallback; data in YAML and behaviour in scripts; Lua as the first
  scripting plugin with our own language kept optional; `ENGINE_GAME_MODULE` / `ENGINE_PLUGIN` and
  `engine_add_game_module` to plug games and plugins into the engine.

### Changed

- The custom script language from the library benchmark is no longer on the Track 3 critical path.
  It becomes an optional second `IScripting` plugin, so Lua content does not wait for it.

## [2026-10-09] — engine architecture, rounds 4 to 8

### Added

- I wrote the engine documentation set: `engine/architecture.md` (layers, who loads what,
  lifecycle, game loop, error handling), `engine/classes.md` (every class and interface with its
  status), `engine/making-a-game.md` (a Pong built on the engine, step by step),
  `engine/real-engines-study.md` (how Quake, Source, Unreal, Unity, Godot and Bevy attach a game)
  and `design-patterns.md`.
- I recorded the sixteen accepted decisions as ADRs 0001 to 0016 in `adr/`, with an index that
  maps each one to its discussion id (E-xx, ECS-x).
- I added the documentation viewer (`index.html`, `app.js`) and `docs/index.json`, the same setup
  as our other projects.
- I published three HTML companions under `docs/architecture/`: the architecture snapshot, the
  class reference and the engine inventory. They share one navigation bar so the reader moves
  between them without coming back to the docs, and `engine/architecture.md` lists their links.

### Changed

- `GameModule` is now `IGameModule`: every abstract interface starts with `I`.
- `registerServer` / `registerClient` became one `registerSystems(Role, Scheduler&)`, so an Editor
  role does not change every game.
- The engine now ships `main()` in a generic launcher built with `engine_add_launcher`. I had first
  written the launcher as if each game wrote it, which contradicted "the engine owns `main()`".
- I narrowed the exception rule: no exception may leave an `extern "C"` function; exceptions across
  C++ interfaces built with the same toolchain are allowed. The first version forbade every
  cross-module exception, which would have made the server's per-system `try/catch` impossible.

## [2026-10-08] — engine architecture, rounds 1 to 3

### Added

- I opened the `architecture-task-distribution` branch and the Notion decision log.
- We locked the engine identity (hybrid), dynamic plugins from day one, plugin boundaries, the game
  as a module, and the SFML / raylib backend split.

## [2026-10-07] — library benchmark

### Added

- **(y4n1s)** I wrote `benchmark-graphics-rtype.md`, the comparison of graphics libraries that led
  to SFML 3, with SDL3 as the documented alternative.
