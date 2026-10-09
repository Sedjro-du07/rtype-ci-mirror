# ADR 0009: Modules are designed once and linked dynamically or statically

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-10

## Context

Unreal builds modules as DLLs in development and links them monolithically for shipping. The subject lists three forms of modularity, and dynamic loading on Windows is the main delivery risk.

## Decision

Every module (plugins and game) has the same interface and entry point. The CMake option `ENGINE_DYNAMIC_MODULES` builds them as runtime `.so` / `.dll` (development, Track 1 demo, editor) or links them statically into one binary through a small registration table (safety net, release).

## Alternatives considered

- **Dynamic only:** no fallback if Windows loading fails before a defense.

## Consequences

- Compile-time, link-time and runtime modularity are all demonstrable.
- About fifty lines for the static registration table.
