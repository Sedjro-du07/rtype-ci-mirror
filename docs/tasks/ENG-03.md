# ENG-03 · C++23 feature probe compiled on GCC, Clang and MSVC in CI

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · **Wave 3** · status and owner live on the board.

## Goal

Prove early that every C++23 feature on our allow-list compiles on GCC, Clang and MSVC.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `cmake/FeatureProbe.cpp`
- `cmake/EngineFeatures.cmake`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `cmake/FeatureProbe.cpp`

```cpp
// One function per allowed feature (ADR 0017); compiled with try_compile, never linked.
std::expected<int, int>            probeExpected();
std::move_only_function<void()>    probeMoveOnlyFunction();
void                               probePrint();                 // std::print
struct ProbeDeducingThis { template <class Self> auto& get(this Self& self); };
int                                probeToUnderlying();          // std::to_underlying
[[noreturn]] void                  probeUnreachable();           // std::unreachable
```

### `cmake/EngineFeatures.cmake`

```cmake
engine_probe_cxx23(<target>)   # try_compile FeatureProbe.cpp with -DPROBE_<FEATURE> for each feature
# EXPECTED missing → ENGINE_USE_TL_EXPECTED + tl::expected; PRINT missing → ENGINE_NO_STD_PRINT;
# MOVE_ONLY_FUNCTION, DEDUCING_THIS, TO_UNDERLYING, UNREACHABLE missing → configure error
```

## Depends on

- [ENG-01](ENG-01.md) — Engine CMake project, vcpkg manifest, presets, options, exports and C++23 setup
- [ENG-61](ENG-61.md) — CI on GitHub Actions (GCC, Clang, MSVC; dynamic and static) and code style files

## Unlocks

- no task waits for this one

## Read first

- [ADR 0017](../adr/0017-cpp23-with-allow-list.md)

## To implement

- [ ] `cmake/FeatureProbe.cpp` using each allowed feature: `std::expected`, `std::move_only_function`, `std::print`, deducing this, `std::to_underlying`, `std::unreachable`
- [ ] `cmake/EngineFeatures.cmake`: `engine_probe_cxx23(target)`, one `try_compile` per feature (module scanning off); summary line at configure
- [ ] `std::expected` missing → `ENGINE_USE_TL_EXPECTED` + `tl::expected`; `std::print` missing → `ENGINE_NO_STD_PRINT`; the other four missing → configure error naming the supported compilers

## Done when

- CI job compiles the probe on the three compilers and prints which fallbacks are active

## Watch out

- Features outside the allow-list (ADR 0017) must not be added here silently: they need an ADR change

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
