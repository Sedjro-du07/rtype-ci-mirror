# How real engines attach a game

Part of the technical and comparative study. Written during architecture round 4 (2026-10-08).

"The engine owns the game" hides four separate questions:

- **A.** Who owns the loop?
- **B.** Who owns `main()`?
- **C.** How is game code attached to the engine?
- **D.** How does the engine learn the game's types (reflection)?

## Survey

| Engine | A · Loop | B · main() | C · Game code | D · Reflection | Why |
|---|---|---|---|---|---|
| Quake 1 (1996) | engine | engine | VM bytecode (QuakeC, `progs.dat`) | VM | modding without engine source, portable, a VM cannot crash the machine |
| Quake 2 (1997) | engine | engine | native DLL via `GetGameAPI(game_import_t*)` | manual | speed of C; cost: downloaded mods are unsafe native code |
| Quake 3 (1999) | engine | engine | QVM bytecode (JIT) or DLL, split `qagame` / `cgame` / `ui` | manual | C comfort with VM safety; clean server/client split |
| Source (2004) | engine | engine (launcher) | `client.dll` + `server.dll` per game, versioned `CreateInterface` | manual macros (`DATADESC`, `SendTable`) | one engine, many games and mods |
| Unreal 4/5 | engine | engine (`Launch`, `GuardedMain`) | modules: DLLs in development, monolithic static in shipping | generated (UHT: `UCLASS`, `UPROPERTY`) | fast iteration and an editor in development, speed in shipping |
| Unity | engine | engine (`Player`) | C# assemblies + data | C# runtime | productivity, safety, portability (IL2CPP where JIT is banned) |
| Godot | engine | engine | scenes + GDScript, GDExtension `.so`, or static C++ module | runtime | data-driven; the editor is itself a Godot app |
| Bevy | engine | game | static plugins | `register_type` + `derive(Reflect)` | simplicity, no fragile Rust ABI |
| AAA in-house | engine | engine | static + scripts / visual scripting | varies | performance, console constraints |
| SFML, raylib | game | game | — | — | frameworks, not engines |

## What we take from it

1. **A is unanimous.** Every engine owns the loop. That is what separates an engine from a
   framework. → ADR 0003, ADR 0005.
2. **B is technically cosmetic**, but for clarity our engine ships the `main()` in a generic
   launcher, like Unity's Player. → ADR 0005.
3. **C is the real decision**, driven by modding and safety (VM, scripts), performance and consoles
   (static), iteration and editor (dynamic), many games on one engine (dynamic or data). We follow
   Unreal: design modules once, choose dynamic or static at build time. → ADR 0002, ADR 0009.
4. **Almost every engine has three layers**: engine, game logic, content for non-programmers.
   → ADR 0010.
5. **D is unavoidable** once there are scene files or an editor. In C++ without code generation it
   is explicit registration, as in Source and Bevy: our `TypeRegistry`.
6. **Quake 3 and Source split game code by side.** Our open question on the game module split
   comes from here.

## Server precedents

Quake keeps generic server code (`sv_*.c`) in the engine and rules in the game DLL. Source's
dedicated server is the engine without rendering plus `server.dll`. Unreal's dedicated server is
the engine compiled as a Server target, with rules in the game's `GameMode`. → ADR 0011.
