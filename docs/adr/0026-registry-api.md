# ADR 0026: Registry API, views, commands and hooks

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 3

## Context

ADRs 0012 to 0016 fixed the storage; game code, the scene loader, the editor, scripts and replication need one consistent API on top.

## Decision

`Registry` offers typed access (`emplace`, `get`, `tryGet`, `has`, `replace`, `remove`), type-erased access by `ComponentId` (`getRaw`, `emplaceRaw`, `componentsOf`), queries (`view<Ts...>()`, `exclude<Xs...>`), identity (`find(name)`, `find(uuid)`, `withTag`) and hierarchy (`setParent`, `parent`, `children`). `View` iterates the smallest storage. `Commands` records spawn, prefab spawn, destroy (recursive), add, set, remove. `ComponentHooks` (onCreate, onReplace, onDestroy) are `Signal`s with RAII `Subscription`s.

## Alternatives considered

- **Separate APIs for game code and tools:** two ways to do the same thing, divergence.

## Consequences

- Tools and scripts reuse the type-erased path; game code uses the typed one.
- `Signal` lives in `core/` so the ECS does not depend on the events block.
