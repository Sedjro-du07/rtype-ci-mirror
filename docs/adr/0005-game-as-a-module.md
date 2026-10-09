# ADR 0005: The game is a module and the engine ships main()

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-06

## Context

Tracks 1 and 3 need a standalone engine, a second game, and an editor able to open any game. Real engines (Quake, Source, Unity, Unreal) own the loop and load the game as a module.

## Decision

The engine owns `main()` through a generic launcher (`engine/launcher/main.cpp`), built per executable with `engine_add_launcher(r-type_client CONFIG client.yaml)`. Games write no `main()` and no loop. A game is a module implementing `IGameModule` (`registerTypes`, `registerSystems(Role, Scheduler&)`, `firstScene`, optional `onStart` / `onShutdown`), exposed through `extern "C" create_game` / `destroy_game`. Interfaces are prefixed with `I`.

## Alternatives considered

- **Game links the engine and writes main():** simpler, but the editor cannot open a game and the engine cannot run alone.
- **Separate `registerServer` / `registerClient`:** a third role (Editor) would change every game.

## Consequences

- The engine needs a `TypeRegistry` so it can handle components it was compiled without.
- Component IDs come from the `TypeRegistry`, never from a static counter; no `typeid` keys and no singletons across modules.
- Pending: the `ENGINE_GAME_MODULE` macro and `engine_add_game_module` CMake function.
