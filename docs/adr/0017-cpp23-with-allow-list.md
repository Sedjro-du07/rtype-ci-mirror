# ADR 0017: C++23 with an allow-list of features

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-14

## Context

Our designs already rely on C++20 (`std::span`, `std::format`, concepts, designated initializers, ranges). C++23 adds `std::expected`, which fits every expected failure in the engine: loading a plugin, parsing a YAML file, finding a component by name. Support for C++23 differs between GCC, Clang (libc++) and MSVC, and the game must build on Linux and Windows.

## Decision

`CMAKE_CXX_STANDARD 23`, with an allow-list documented in the coding conventions. Allowed: all of C++20 except modules; from C++23 `std::expected`, `std::move_only_function`, `std::print`, deducing this, `std::to_underlying`, `std::unreachable`. Anything else in C++23 (for example `std::flat_map`, `std::generator`) is forbidden until CI proves it on the three compilers. Minimum compiler versions are fixed after a feature-probe file compiles on GCC, Clang and MSVC; that file stays in CI. Fallback: `tl::expected` (same interface, from the package manager) if a target lacks `std::expected`.

## Alternatives considered

- **C++20 with `tl::expected` from the start:** safer for compatibility, but depends on a library for something the standard provides.
- **C++17:** our designs already use C++20 features.
- **C++20 modules:** tooling still fragile, and risky combined with runtime-loaded `.so` / `.dll`.

## Consequences

- Expected failures return `std::expected<T, Error>`; exceptions stay for unexpected failures and boot errors.
- CI images and clang-tidy / clang-format must be recent enough for C++23 syntax.
- The allow-list is part of the coding conventions.
