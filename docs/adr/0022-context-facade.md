# ADR 0022: Game code reaches the engine only through Context

- Status: Accepted, amended by [ADR 0039](0039-room-api.md) (three context types)
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 1

## Context

Systems and game hooks need the world, commands, scenes, events, actions, assets, rendering, audio, time and logging. Giving them `Engine&` lets game code break the engine and couples it to every header; singletons are duplicated per `.so`.

## Decision

A non-owning `Context`, built only by `Engine` (private constructor, `friend class Engine`), passed to every system and hook. It exposes `world()` (active scene registry, a method because scenes change), `commands()`, `scenes()`, `events()`, `input()` and `time()` (read-only), `assets()`, `render()`, `audio()`, `log()`, `role()` and `requestQuit()`. References, never pointers. Used only on the loop thread. Network access is added in the network block.

## Alternatives considered

- **`Engine&` given to the game:** the game could unload its own module or reorder the lifecycle.
- **Global service locator / singletons:** duplicated per module, hidden dependencies.
- **Each dependency passed separately:** signatures grow with every need.
- **Bevy-style typed system parameters:** heavy metaprogramming for six weeks.

## Consequences

- Facade and Dependency Injection in one object.
- Changing `Engine` internals does not break games.
