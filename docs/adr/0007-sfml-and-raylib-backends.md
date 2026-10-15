# ADR 0007: SFML for 2D and raylib for optional 3D, as swappable platform plugins

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-08

## Context

The team wants both SFML and raylib. Running both in one process creates two windows and two GL contexts.

## Decision

Two `platform` plugins, one per process, chosen in the configuration: `platform_sfml` (2D) and `platform_raylib` (2D plus optional 3D). SFML ships for Part 1, raylib arrives early in Part 2. The interface is 2D first with a 3D extension point. The engine owns its vocabulary (`Key`, `Color`, `Rect`, `Vec2`, handles); the raylib plugin turns polling into raw events.

## Alternatives considered

- **Both libraries in one process:** conflicting windows, contexts and event loops.

## Consequences

- Swapping the backend in a config file is the strongest decoupling demo.
- Every new platform feature is written twice.
