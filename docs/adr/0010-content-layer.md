# ADR 0010: Engine, game module and content are three layers

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-11

## Context

Track 3 asks that designers add levels and enemies without C++. Quake 3, Unity and Godot all separate engine, game logic and content.

## Decision

Three layers above the plugins: the engine (C++), the game module (C++ building blocks: components, systems), and content (YAML scenes and prefabs, plus scripts) that assembles those blocks.

## Alternatives considered

- **Everything in the game module:** every new level or enemy needs a recompile.

## Consequences

- Designers work in data and scripts only.
- The data format and script language are still to decide.
