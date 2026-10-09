# ADR 0035: Logger, CVars, console, metrics and the scripting interface

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 12

## Context

Every module needs logging and settings; Track 1 grades developer tools; Track 3 needs scripting (ADR 0019).

## Decision

`Logger` with levels, tagged child loggers and sinks (console, rotating file, developer console), thread-safe, injected. `CVars` typed with flags (Archive, Cheat, ReadOnly, ServerOnly), ranges and change signals, saved user settings. `Console` with commands, completion and built-ins. `Metrics` with series and ring buffers, shown by the `engine.debug_overlay` system (ImGui, ImPlot). `IScripting` binds a `ScriptHost` once, loads sandboxed scripts, calls hooks (`on_spawn`, `on_update`, `on_collision`, `on_destroy`) through engine systems, reloads them; Lua is accessed through reflection, so no per-type bindings.

## Alternatives considered

- **spdlog for logging:** one more dependency for features we need in a hundred lines.

## Consequences

- Tools are available in every game built on the engine.
- A script error disables that script only.
