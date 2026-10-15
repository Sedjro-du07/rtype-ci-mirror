# ENG-62 · Installable engine: install, export, find_package(rtype_engine), CPack packages

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · **Wave 3** · status and owner live on the board.

## Goal

Make the engine usable from another project, and the game shippable as an archive (ADR 0045, guide 24).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `cmake/EngineInstall.cmake`
- `cmake/rtype_engineConfig.cmake.in`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `cmake/EngineInstall.cmake`

```cmake
engine_install_engine()      # engine + headers + Export.hpp + plugins (export set rtype_engineTargets),
                             # CMake helpers and launcher main.cpp into lib/cmake/rtype_engine,
                             # rtype_engineConfig.cmake + version file (SameMinorVersion)
engine_enable_packaging(NAME <name> [VENDOR <vendor>])   # CPack: TGZ on Linux, ZIP on Windows
```

### `cmake/rtype_engineConfig.cmake.in`

```cmake
@PACKAGE_INIT@
find_dependency(tl-expected) when the engine was built with the fallback
include(rtype_engineTargets.cmake)                     # rtype_engine::engine, rtype_engine::<plugins>
set(ENGINE_DYNAMIC_MODULES <engine's mode>); ENGINE_LAUNCHER_SOURCE; ENGINE_PLUGIN_TARGETS
include(EngineOptions) include(EngineModules) include(EngineInstall)
```

## Depends on

- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- no task waits for this one

## Read first

- [ADR 0045](../adr/0045-tooling-vcpkg-presets-ci.md)
- [guide 24](../engine/guide/24-building-and-shipping.md)

## To implement

- [ ] `cmake/EngineInstall.cmake`: `install(TARGETS engine … EXPORT rtype_engineTargets)`, public headers, the generated `Export.hpp`, the CMake functions of ENG-02, plugins
- [ ] `rtype_engineConfig.cmake.in` + `write_basic_package_version_file`: `find_package(rtype_engine)` gives `rtype_engine::engine` and the functions
- [ ] CPack: `.tar.gz` on Linux, `.zip` on Windows, laid out as guide 24 (executables, `plugins/`, `games/`, content, runtime DLLs)

## Done when

- A tiny project outside the repo builds against an installed engine with `find_package(rtype_engine)` on Linux and Windows
- `cpack` produces an archive that runs on a clean machine

## Watch out

- Install the dependencies' runtime DLLs on Windows (vcpkg applocal or `install(RUNTIME_DEPENDENCY_SET)`)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
