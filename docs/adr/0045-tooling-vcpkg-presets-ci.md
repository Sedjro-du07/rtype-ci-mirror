# ADR 0045: Tooling: vcpkg, CMake presets, CI on Linux and Windows, installable engine

- Status: Accepted
- Date: 2026-10-15
- Deciders: the R-Type team (vcpkg chosen by the team; the rest delegated to the architect)
- Discussion id: T-1 (tooling)

## Context

The subject requires a package manager and a cross-platform build. ADR 0002 promised CI on Linux and
Windows from week 1 but no task created it. The guide shows `find_package(rtype_engine)`, which needs
an installed, exported engine. Formatting and line endings were never fixed.

## Decision

1. **vcpkg in manifest mode.** One `vcpkg.json` at the repository root (engine and game), with a
   pinned `builtin-baseline`. Dependencies: sfml, yaml-cpp, asio, lua (pinned to 5.4 by an override, for
   sol2), sol2, miniaudio, imgui, imgui-sfml, implot, catch2, tl-expected; raylib arrives with ENG-58. Triplets `x64-linux` and
   `x64-windows` (dynamic CRT `/MD`, matching ADR 0002). vcpkg is found through `VCPKG_ROOT`.
2. **CMake presets.** `CMakePresets.json` at the root is the only supported way to configure:
   `linux-gcc`, `linux-clang`, `windows-msvc`, each in `-debug` and `-release`, plus `-static`
   variants (`ENGINE_DYNAMIC_MODULES=OFF`). The same names work in a terminal, VS Code, CLion and
   Visual Studio. Build folders go to `build/<preset>/`.
3. **Root project.** A root `CMakeLists.txt` adds `engine/` and, later, the game; the engine still
   builds alone from `engine/` (slide 14).
4. **CI on GitHub Actions.** On every push and pull request: Ubuntu 24.04 with GCC 14 and Clang 18,
   Windows Server 2022 with MSVC; dynamic mode on all three, static mode on GCC and MSVC; steps
   configure (preset), build, `ctest`, the C++23 probe (ENG-03), the headless smoke test (ENG-57),
   loading every plugin. vcpkg binary caching keeps builds fast. A format check job runs
   `clang-format --dry-run`.
5. **Code style files.** `.clang-format` (based on LLVM, 4 spaces, 120 columns), `.clang-tidy`
   (bugprone, modernize, performance; warnings, not errors), `.editorconfig`, `.gitattributes`
   (LF, binary assets marked binary).
6. **Installable engine.** `cmake --install` produces `include/`, the libraries, the plugins and
   `rtype_engineConfig.cmake`, so another project uses `find_package(rtype_engine)` and the CMake
   functions of ENG-02. CPack builds a `.tar.gz` (Linux) and a `.zip` (Windows) of the game as
   described in guide 24.
7. **Tests:** Catch2 v3, registered with `ctest`.

## Alternatives considered

- **Conan:** needs Python and profiles per compiler; vcpkg integrates through one toolchain file.
- **CPM / FetchContent only:** everything rebuilt from source on each clean build; no binary cache.
- **No presets:** every member configures differently; Windows paths and options drift.

## Consequences

- ENG-01 creates `vcpkg.json`, `CMakePresets.json` and the root `CMakeLists.txt`.
- New tasks: ENG-61 (CI and code style files), ENG-62 (install, export, packaging).
- Guides 01 and 24 give the exact commands (`cmake --preset linux-gcc-debug`).
