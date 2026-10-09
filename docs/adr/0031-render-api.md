# ADR 0031: Rendering API: handles, draw commands, camera and accessibility filters

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: Block 8

## Context

ADR 0006 fixed the command-list principle; the exact API must serve sprites, text, HUD, debug drawing and accessibility on two backends.

## Decision

Typed `Handle<Tag>` for textures, fonts, sounds, music, scripts. `DrawCmd` kinds: Sprite, Text, Rect, RectOutline, Line, Circle, with layer, screen-space flag and a text arena. `RenderQueue` helpers plus `Camera2D`. `IRenderer` creates and destroys GPU resources, submits a `RenderFrame`, reports stats and capabilities (`Render3D`, `ColorFilter`, `Shaders`), applies colour-blindness filters. `IPlatform` adds window control, event polling into `InputMap` and `EventBus`, Dear ImGui frames and clipboard. A virtual resolution is letterboxed.

## Alternatives considered

- **Exposing backend sprites or textures to games:** breaks backend independence.

## Consequences

- Both backends implement the same small surface.
- The editor and debug overlay use the same ImGui hooks.
