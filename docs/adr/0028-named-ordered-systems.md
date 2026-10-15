# ADR 0028: Systems are named and ordered by constraints inside fixed stages

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 5

## Context

Engine and game systems must interleave in a precise order (input, movement, hierarchy, collision, damage), and failures must not stop a server.

## Decision

Three stages (`FixedUpdate`, `LateUpdate`, `Render`). Systems have unique names `<module>.<system>` and declare `after` / `before` constraints, sorted topologically at boot (unknown name or cycle is a boot error). Options: roles, disabled, critical. Per-system timing feeds the profiler. Single-threaded. Exceptions follow ADR 0024: disabled on the server (critical ends the session cleanly), clean quit on the client.

## Alternatives considered

- **Registration order only:** fragile when engine and game add systems separately.
- **Parallel systems with a dependency graph:** complexity and nondeterminism not needed at this scale.

## Consequences

- Systems can be toggled and timed by name from the console.
- The engine's system names become a public API (reference page 13).
