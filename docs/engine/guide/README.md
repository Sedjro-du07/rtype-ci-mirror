# Using the engine

Status: **Locked** with the architecture (2026-10-09). This guide is for people who **make games**
with the engine. How the engine is built inside is in the reference: [`../reference/README.md`](../reference/README.md).

Start with the tutorial, then use the topic pages as needed.

| Page | You will learn |
|---|---|
| [Tutorial: make a Pong](../making-a-game.md) | the whole path, from an empty folder to a running game |
| [01 · Getting started](01-getting-started.md) | prerequisites, building, running, who edits what |
| [02 · Project and configuration](02-project-and-config.md) | CMake functions, every config key, command line, exit codes |
| [03 · Components](03-components.md) | defining and registering data, field types, flags |
| [04 · Systems and scheduling](04-systems.md) | writing logic, stages, ordering, roles, failures |
| [05 · Entities, views and commands](05-entities.md) | querying and changing the world safely, hierarchy, hooks |
| [06 · Events](06-events.md) | publishing and listening, engine events |
| [07 · Input](07-input.md) | actions, `input.yaml`, rebinding, accessibility options |
| [08 · Rendering](08-rendering.md) | sprites, layers, camera, text, animation, parallax, debug drawing |
| [09 · Audio](09-audio.md) | sounds, music, buses, positional sound, captions |
| [10 · Scenes, prefabs and assets](10-scenes-prefabs-assets.md) | YAML formats, switching scenes, asset paths |
| [11 · Scripting with Lua](11-scripting-lua.md) | the Lua API, hooks, sandbox, hot reload |
| [12 · Networking](12-networking.md) | making entities replicated, connections, player input, messages |
| [13 · Console, CVars and metrics](13-console-cvars-metrics.md) | developer tools and settings |
| [14 · Writing a plugin](14-writing-a-plugin.md) | adding a backend (renderer, audio, network, scripting) |
| [15 · Troubleshooting](15-troubleshooting.md) | error messages, exit codes, common mistakes |

## The three kinds of people who use the engine

| Who | Works in | Recompiles? |
|---|---|---|
| Gameplay developer | C++ in the game module: components, systems, the module class | the game module only |
| Game designer | YAML scenes and prefabs, Lua scripts, assets, `input.yaml` | never |
| Engine developer | the engine core and plugins | the engine or one plugin |
