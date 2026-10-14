# ENG-01 · Engine CMake project, options and C++23 setup

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · status and owner live on the board.

## Goal

Create the `engine` CMake project every other task builds into, with the C++23 setup and the build options.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `CMakeLists.txt`
- `cmake/EngineOptions.cmake`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25)
project(rtype_engine LANGUAGES CXX)
include(cmake/EngineOptions.cmake)
include(cmake/EngineModules.cmake)

add_library(engine STATIC|SHARED <sources of src/**>)
target_include_directories(engine PUBLIC include)
target_compile_features(engine PUBLIC cxx_std_23)
target_link_libraries(engine PUBLIC <third-party targets>)   # yaml-cpp stays PRIVATE
add_subdirectory(plugins/<each plugin>)
if(ENGINE_BUILD_TESTS)       add_subdirectory(tests)       endif()
if(ENGINE_BUILD_BENCHMARKS)  add_subdirectory(benchmarks)  endif()
```

### `cmake/EngineOptions.cmake`

```cmake
option(ENGINE_DYNAMIC_MODULES   "Plugins and games as runtime-loaded libraries" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors"                      OFF)
option(ENGINE_SANITIZERS         "Build with address and undefined sanitizers"   OFF)
option(ENGINE_BUILD_TESTS        "Build engine/tests"                            ON)
option(ENGINE_BUILD_BENCHMARKS   "Build engine/benchmarks"                       OFF)

function(engine_set_warnings target)        # -Wall -Wextra -Wpedantic / /W4 (+ errors if enabled)
```

## Depends on

- nothing: can start now

## Unlocks

- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher
- [ENG-03](ENG-03.md) — C++23 feature probe compiled on GCC, Clang and MSVC in CI

## Read first

- [ADR 0009](../adr/0009-dual-linking-mode.md)
- [ADR 0017](../adr/0017-cpp23-with-allow-list.md)

## To implement

- [ ] `engine/CMakeLists.txt`: project, `engine` library target, `include/` as public include dir, `src/` sources by glob or list
- [ ] `cmake/EngineOptions.cmake`: `ENGINE_DYNAMIC_MODULES` (ON), warnings as errors option, sanitizers option
- [ ] C++23 required (`cxx_std_23`), `-Wall -Wextra -Wpedantic` / `/W4`, position-independent code
- [ ] Package manager hook (Conan or vcpkg, chosen in the Tooling part): leave one clearly marked place for it
- [ ] Add `tests/` and `benchmarks/` as optional sub-directories

## Done when

- `cmake -B build && cmake --build build` succeeds on Linux and Windows with an empty engine
- Options appear in `cmake -LH`

## Watch out

- No global `include_directories` or `add_definitions`: everything on targets
- Third-party types must never leak into public headers (reference README)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
