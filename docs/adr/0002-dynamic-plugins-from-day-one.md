# ADR 0002: Backends are dynamic plugins from the first week

- Status: Accepted
- Date: 2026-10-08
- Deciders: the R-Type team
- Discussion id: E-02

## Context

The subject grades modularity and lists compile-time, link-time and runtime forms. The team chose runtime plugins from day 1, against a static-first proposal, as in their earlier Arcade project. Windows (`LoadLibrary`, exports, C ABI) is the main risk.

## Decision

Backends are `.so` / `.dll` plugins loaded at runtime, under these rules: one CMake `SHARED` target per plugin built into `plugins/`; `extern "C"` create and destroy functions, memory freed by the module that allocated it; an API version checked at load, a mismatch refused with a clear error; no exception leaves an `extern "C"` function; one compiler and one C++ runtime for everything; OS-specific loading code only in `DynamicLibrary`; CI builds and loads every plugin on Linux and Windows from week 1.

## Alternatives considered

- **Static first, dynamic in Part 2:** less risk early, but the team prefers proving modularity from the start.
- **Static with a code-generated factory:** one binary, simplest build, weakest modularity demo.

## Consequences

- Strongest Track 1 argument from the first defense.
- Windows pitfalls are paid early; CI must cover them.
- Template statics, `typeid` keys and singletons become cross-module hazards (see ADR 0005).
