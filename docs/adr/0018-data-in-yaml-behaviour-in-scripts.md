# ADR 0018: Data lives in YAML, behaviour lives in scripts

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-15

## Context

The team asked why not use Lua instead of YAML. Data (what exists, where, with which values) and behaviour (what happens, when) have different needs: an editor must load and save scenes; level files may be shared; the configuration is read before any plugin, including the scripting plugin.

## Decision

YAML for data: engine configuration, scenes, prefabs, enemy statistics. A script language for behaviour: wave patterns, boss phases, special AI, procedural spawning at runtime. A prefab points to its script (`Script: { file: scripts/boss.lua }`).

## Alternatives considered

- **Lua for everything (as LÖVE or Factorio do):** an editor cannot write back a loop it only sees the result of; level files become executable code; the configuration would need Lua in the core before plugins load.

## Consequences

- The editor can round-trip scenes and prefabs.
- Level files carry no code and are safe to share.
- Parse errors report `file:line` with the component name.
