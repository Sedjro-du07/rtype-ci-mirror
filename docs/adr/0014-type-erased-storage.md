# ADR 0014: Component storages are type-erased and indexed by the TypeRegistry

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: ECS-3

## Context

Game components are defined in the game module, which the engine was compiled without.

## Decision

The registry holds `std::vector<std::unique_ptr<IStorage>>` indexed by the `ComponentId` the `TypeRegistry` assigns. The registry is cleared before a game module is unloaded, because the storages' code lives in that module.

## Alternatives considered

- **Static template counters for IDs:** each module would get its own counter.

## Consequences

- Unload order matters and is enforced by `Engine`'s member order.
