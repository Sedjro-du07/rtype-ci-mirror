# Changelog

All notable changes to this project are documented here, in first person,
under the date they landed. Each entry names its author when it is not the
person writing the section.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
