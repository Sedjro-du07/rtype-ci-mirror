# ADR 0025: Plugins and games load through one ModuleManager

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 2

## Context

Backends and games are loaded at runtime (ADR 0002) or linked statically (ADR 0009), and objects must be created and destroyed by the module that owns their code.

## Decision

`IPlugin` (kind, name, apiVersion, init, shutdown) is the base of `IPlatform`, `IAudio`, `INetwork`, `IScripting`. `ModuleManager` resolves `<kind>_<name>` in `plugins/` and games in `games/` next to the executable, or in `StaticModuleTable` in static mode; it checks API versions, injects a `PluginContext` (logger, CVars, config, events) and unloads in reverse order. `DynamicLibrary` is the only OS-specific loader. Null Objects exist for every plugin kind. No hot reload of the game module in Part 1.

## Alternatives considered

- **One loader per plugin kind:** duplicated version checks and unload order.
- **Hot reload of the game module from Part 1:** requires state migration; kept as a Track 1 option.

## Consequences

- Every plugin follows the same lifecycle: load, version check, init, use, shutdown, destroy, close.
- Adding a plugin kind means one interface and one enum value.
