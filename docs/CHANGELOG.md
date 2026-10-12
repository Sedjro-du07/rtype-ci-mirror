# Changelog

All notable changes to this project are documented here, in first person,
under the date they landed. Each entry names its author when it is not the
person writing the section.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
