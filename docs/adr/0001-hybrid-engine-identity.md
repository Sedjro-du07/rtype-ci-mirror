# ADR 0001: The engine is a code-first ECS core with a Unity-like data layer

- Status: Accepted
- Date: 2026-10-08
- Deciders: the R-Type team
- Discussion id: E-01

## Context

The team wanted to know whether the engine should feel like Unity (editor first, data driven), like a framework (code first, Bevy or raylib style), or something in between. The kick-off slides already point at an in-house ECS, YAML scenes and prefabs, and an editor in Track 1. Unity's `MonoBehaviour` model puts logic inside components, which contradicts ECS.

## Decision

The engine is **hybrid**: a code-first ECS core (components are data, systems are functions), with a Unity-like data layer built on top: YAML scenes and prefabs, an inspector, an editor in Part 2. Logic never lives inside components.

## Alternatives considered

- **Unity-like, editor first:** the editor becomes a prerequisite and endangers the Part 1 deadline; copying `GameObject` + `MonoBehaviour` contradicts ECS.
- **Pure framework:** fastest to write, but gives game designers nothing and leaves Tracks 1 and 3 without a base.

## Consequences

- The editor and content tools can grow in Part 2 without rewriting the core.
- Everything a designer can edit must be reachable through data, which requires a type registry (ADR 0005).
