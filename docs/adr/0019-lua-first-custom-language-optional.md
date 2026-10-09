# ADR 0019: Scripting ships with Lua first, a custom language stays optional

- Status: Accepted
- Date: 2026-10-09
- Deciders: the R-Type team
- Discussion id: E-16

## Context

The library benchmark proposed a custom script language. Writing one means a lexer, parser, compiler or interpreter, a VM, bindings, error messages and documentation: a project of its own. Scripting is already a plugin behind `IScripting` (ADR 0002, ADR 0004), so the engine does not depend on a language.

## Decision

`scripting_lua` (Lua + sol2) is the first scripting plugin; scripts are sandboxed and can be reloaded without restarting. A custom language is optional: either a second scripting plugin, or a small domain-specific language for shot and wave patterns (BulletML style). It is never on the critical path of Track 3.

## Alternatives considered

- **Custom language only:** strong Track 1 argument, but blocks Track 3 behind a risky project.
- **Lua only, forever:** safe, but closes a Track 1 opportunity the team cares about.

## Consequences

- Track 3 content does not wait for the custom language.
- A second scripting plugin would demonstrate modularity the same way raylib does for rendering.
- Lua and sol2 become dependencies through the package manager.
