// engine/plugins/platform_sfml/ColorFilter.frag
// ENG-33 — Colour-blindness filters and UI scale in platform_sfml
// Spec: docs/engine/reference/08-rendering.md · ADR 0031 · accessibility
// Details: docs/tasks/ENG-33.md
// Board: Notion › R-Type — Architecture › Engine — folder tree & tasks (self-assign)
// Status: not implemented yet. Claim the task on the board before editing this file.

#version 330 core
// Colour-blindness filter: protanopia, deuteranopia, tritanopia, high contrast.
