# ENG-61 · CI on GitHub Actions (GCC, Clang, MSVC; dynamic and static) and code style files

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · **Wave 2** · status and owner live on the board.

## Goal

Build and test every commit on Linux and Windows from the first week, with one code style (ADR 0002, 0045).

## Files

At the repository root (no skeleton: this task creates them):

- `.github/workflows/ci.yml`
- `.clang-format`
- `.clang-tidy`
- `.editorconfig`
- `.gitattributes`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `.github/workflows/ci.yml (repository root)`

```yaml
on: [push, pull_request]
jobs:
  build:   matrix: { linux-gcc-debug, linux-clang-debug, windows-msvc-debug, linux-gcc-static, windows-msvc-static }
           steps: checkout · setup vcpkg (binary cache) · cmake --preset $preset · cmake --build · ctest
  format:  clang-format --dry-run --Werror on changed *.hpp *.cpp
```

## Depends on

- [ENG-01](ENG-01.md) — Engine CMake project, vcpkg manifest, presets, options, exports and C++23 setup

## Unlocks

- [ENG-03](ENG-03.md) — C++23 feature probe compiled on GCC, Clang and MSVC in CI
- [ENG-57](ENG-57.md) — Headless smoke test in CI: client with Null plugins runs 1000 ticks

## Read first

- [ADR 0002](../adr/0002-dynamic-plugins-from-day-one.md)
- [ADR 0045](../adr/0045-tooling-vcpkg-presets-ci.md)

## To implement

- [ ] `.github/workflows/ci.yml`: matrix Ubuntu 24.04 (GCC 14, Clang 18) and Windows Server 2022 (MSVC); dynamic on all three, static on GCC and MSVC
- [ ] Steps: checkout, vcpkg with binary caching, `cmake --preset`, build, `ctest --output-on-failure`
- [ ] A format job: `clang-format --dry-run --Werror` on changed C++ files
- [ ] `.clang-format` (LLVM base, 4 spaces, 120 columns), `.clang-tidy` (bugprone, modernize, performance as warnings), `.editorconfig`, `.gitattributes` (LF, binaries marked)

## Done when

- A pull request shows five green build jobs and one green format job; a deliberate MSVC-only error turns the Windows job red

## Watch out

- Cache vcpkg packages or every run rebuilds SFML
- Branch protection on `engine` and `develop` requires the CI to pass (set on GitHub once the workflow exists)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
