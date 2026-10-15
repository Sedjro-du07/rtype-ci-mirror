# ENG-01 · Engine CMake project, vcpkg manifest, presets, options, exports and C++23 setup

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · **Wave 1** · status and owner live on the board.

## Goal

Create the `engine` CMake project every other task builds into, with vcpkg, presets, exports, the C++23 setup and the build options.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `CMakeLists.txt`
- `cmake/EngineOptions.cmake`

At the repository root (no skeleton: this task creates them):

- `CMakeLists.txt`
- `vcpkg.json`
- `CMakePresets.json`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25)
project(rtype_engine LANGUAGES CXX)
include(cmake/EngineOptions.cmake)
include(cmake/EngineModules.cmake)

add_library(engine SHARED|STATIC <sources of src/**>)        # SHARED when ENGINE_DYNAMIC_MODULES (ADR 0044)
set_target_properties(engine PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
include(GenerateExportHeader)
generate_export_header(engine BASE_NAME ENGINE EXPORT_MACRO_NAME ENGINE_API
                       EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/engine/Export.hpp)
target_include_directories(engine PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include)
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
function(engine_set_msvc_options target)    # /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc /utf-8
```

### `/CMakeLists.txt (repository root)`

```cmake
cmake_minimum_required(VERSION 3.25)
project(rtype LANGUAGES CXX)
add_subdirectory(engine)
# add_subdirectory(rtype)      # the game, later
```

### `/vcpkg.json (repository root)`

```json
{
  "name": "rtype", "version": "0.1.0",
  "builtin-baseline": "<pinned vcpkg commit>",
  "dependencies": [ "sfml", "yaml-cpp", "asio", "lua", "sol2", "miniaudio",
                    "imgui", "imgui-sfml", "implot", "catch2", "tl-expected" ],
  "overrides": [ { "name": "lua", "version": "5.4.8" } ]
}
```

### `/CMakePresets.json (repository root)`

```json
configurePresets: base (hidden: toolchain $env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake, binaryDir build/${presetName})
  linux-gcc-debug, linux-gcc-release, linux-clang-debug, linux-clang-release,
  windows-msvc-debug, windows-msvc-release,
  linux-gcc-static, windows-msvc-static        (ENGINE_DYNAMIC_MODULES=OFF)
buildPresets and testPresets with the same names
```

## Depends on

- nothing: can start now

## Unlocks

- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher
- [ENG-03](ENG-03.md) — C++23 feature probe compiled on GCC, Clang and MSVC in CI
- [ENG-61](ENG-61.md) — CI on GitHub Actions (GCC, Clang, MSVC; dynamic and static) and code style files

## Read first

- [ADR 0009](../adr/0009-dual-linking-mode.md)
- [ADR 0017](../adr/0017-cpp23-with-allow-list.md)
- [ADR 0044](../adr/0044-platforms-and-portability.md)
- [ADR 0045](../adr/0045-tooling-vcpkg-presets-ci.md)

## To implement

- [ ] Root `CMakeLists.txt`: `project(rtype)`, `add_subdirectory(engine)` (the game is added later); the engine still configures alone from `engine/`
- [ ] Root `vcpkg.json` (manifest mode, pinned `builtin-baseline`): sfml, yaml-cpp, asio, lua (override 5.4), sol2, miniaudio, imgui, imgui-sfml, implot, catch2, tl-expected (ADR 0045)
- [ ] Root `CMakePresets.json`: `linux-gcc`, `linux-clang`, `windows-msvc` × `debug` / `release`, plus `-static` variants; toolchain `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`; binary dir `build/<preset>`
- [ ] `engine/CMakeLists.txt`: `engine` library (SHARED when `ENGINE_DYNAMIC_MODULES`, else STATIC), `include/` public, `src/` private
- [ ] `generate_export_header` → `engine/Export.hpp` with `ENGINE_API`; hidden visibility everywhere (`CXX_VISIBILITY_PRESET hidden`, `VISIBILITY_INLINES_HIDDEN`) (ADR 0044)
- [ ] `cmake/EngineOptions.cmake`: `ENGINE_DYNAMIC_MODULES` (ON), warnings as errors, sanitizers, tests, benchmarks
- [ ] C++23 (`cxx_std_23`); GCC/Clang `-Wall -Wextra -Wpedantic`; MSVC `/W4 /permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc /utf-8`; position-independent code
- [ ] Add `tests/` and `benchmarks/` as optional sub-directories

## Done when

- `cmake --preset linux-gcc-debug && cmake --build --preset linux-gcc-debug` and the `windows-msvc-debug` equivalent succeed with an empty engine
- The `-static` presets build too; options appear in `cmake -LH`

## Watch out

- No global `include_directories` or `add_definitions`: everything on targets
- Every public non-template class or function of `include/engine/` gets `ENGINE_API`; a forgotten one fails on Linux too thanks to hidden visibility
- Third-party types must never leak into public headers (reference README)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
