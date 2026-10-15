# ADR 0004: Four plugin kinds: platform, audio, network, scripting

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-05

## Context

A plugin boundary must follow what varies independently, what shares an OS resource, which process needs it, and who owns which thread. With SFML, the window, the GL context and the event source are one object.

## Decision

Four plugin kinds: `platform` (window, rendering, raw input events, ImGui backend), `audio`, `network`, `scripting`. Raw input is turned into actions by the core `InputMap`, so rebinding and accessibility are written once for every backend. The server loads only `network` (and `scripting`).

## Alternatives considered

- **One plugin per concern (graphics and input separate):** forces two plugins to talk directly, since the SFML window owns the events.
- **One big SFML plugin:** no way to mix backends, weak modularity demo.

## Consequences

- Precedents: Quake 2 renderer DLLs, Ogre render systems; Unreal splits further because it owns its rendering layer.
- Input actions, not keys, are what game code reads.
