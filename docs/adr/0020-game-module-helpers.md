# ADR 0020: A macro and a CMake function plug a game into the engine

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-17

## Context

A loadable game must export `create_game`, `destroy_game` and an API version, with the right export attribute per OS, `noexcept`, and a `try/catch` so no exception leaves an `extern "C"` function. In static mode (ADR 0009) there is no `.so` and the game must register differently. Each mistake shows only at run time.

## Decision

The engine provides `ENGINE_GAME_MODULE(Class)`, which generates the three `extern "C"` functions in dynamic mode and a static registrar in static mode, so game code is identical in both modes. Plugins get the same with `ENGINE_PLUGIN(Class)`. The CMake function `engine_add_game_module(name sources...)` builds the module as `SHARED` or `STATIC` per `ENGINE_DYNAMIC_MODULES`, links the engine, outputs to `games/`, hides internal symbols and sets the mode definitions.

## Alternatives considered

- **Hand-written entry points in every game:** easy to get subtly wrong, and every game would carry `#ifdef` for both linking modes.
- **Hand-written CMake per game:** a rule change (output folder, visibility) would have to be fixed in every game.

## Consequences

- One place to fix entry-point and build rules.
- A macro hides code: its expansion is documented in the engine docs and kept short.
- Precedent: Unreal's `IMPLEMENT_PRIMARY_GAME_MODULE`.
