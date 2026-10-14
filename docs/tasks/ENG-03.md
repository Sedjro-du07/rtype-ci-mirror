# ENG-03 · C++23 feature probe compiled on GCC, Clang and MSVC in CI

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **00 Build** · status and owner live on the board.

## Goal

Prove early that every C++23 feature on our allow-list compiles on GCC, Clang and MSVC.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `cmake/FeatureProbe.cpp`

## Depends on

- [ENG-01](ENG-01.md) — Engine CMake project, options and C++23 setup

## Unlocks

- no task waits for this one

## Read first

- [ADR 0017](../adr/0017-cpp23-with-allow-list.md)

## To implement

- [ ] `cmake/FeatureProbe.cpp` using each allowed feature: `std::expected`, `std::move_only_function`, `std::print`, deducing this, `std::to_underlying`, `std::unreachable`
- [ ] `try_compile` in CMake; on `std::expected` failure, switch `Result` to `tl::expected` with a define

## Done when

- CI job compiles the probe on the three compilers and prints which fallbacks are active

## Watch out

- Features outside the allow-list (ADR 0017) must not be added here silently: they need an ADR change

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
