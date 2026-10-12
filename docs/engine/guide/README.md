# Using the engine

Status: **Locked** with the architecture (2026-10-09), completed 2026-10-12. This guide is for people
who **make games** with the engine. How the engine is built inside is in the reference:
[`../reference/README.md`](../reference/README.md).

Start with the tutorial and *Getting started*, then read the topic you need. Everything a game may
call, on one page: [`../game-api.md`](../game-api.md).

## Start here

| Page | You will learn |
|---|---|
| [Tutorial: make a Pong](../making-a-game.md) | the whole path, from an empty folder to a running game |
| [01 · Getting started](01-getting-started.md) | prerequisites, building, running, who edits what |
| [02 · Project and configuration](02-project-and-config.md) | CMake functions, every config key, command line, exit codes |
| [19 · Game module and game state](19-game-module-and-state.md) | the module class in full, where game state lives, code layout |
| [17 · The Context](17-context.md) | everything game code can reach, and the rules |
| [16 · Time and the game loop](16-time-and-loop.md) | fixed steps, `fixedDelta`, timers, interpolation, pausing |

## The world

| Page | You will learn |
|---|---|
| [03 · Components](03-components.md) | defining and registering data, field types, flags |
| [04 · Systems and scheduling](04-systems.md) | writing logic, stages, ordering, roles, failures |
| [05 · Entities, views and commands](05-entities.md) | querying and changing the world safely, hooks, `Lifetime` |
| [20 · Hierarchy and identity](20-hierarchy-and-identity.md) | names, uuids, tags, parent and children (the Force) |
| [06 · Events](06-events.md) | publishing and listening, engine events |
| [18 · Collisions](18-collisions.md) | colliders, layers and masks, reacting, triggers, walls |

## Player, screen, sound

| Page | You will learn |
|---|---|
| [07 · Input](07-input.md) | actions, `input.yaml`, rebinding, accessibility options |
| [08 · Rendering](08-rendering.md) | sprites, layers, camera, text, animation, parallax, debug drawing |
| [09 · Audio](09-audio.md) | sounds, music, buses, positional sound, captions |
| [25 · Accessibility checklist](25-accessibility.md) | what the engine gives, what each game must do |

## Content and data

| Page | You will learn |
|---|---|
| [10 · Scenes, prefabs and assets](10-scenes-prefabs-assets.md) | YAML formats, switching scenes, asset paths |
| [11 · Scripting with Lua](11-scripting-lua.md) | the Lua API, hooks, sandbox, hot reload |
| [21 · Saving](21-saving.md) | settings, key bindings, save files, game files |

## Network and roles

| Page | You will learn |
|---|---|
| [22 · Roles: client, server, editor](22-roles.md) | what each role loads and runs, which systems go where |
| [12 · Networking](12-networking.md) | replicated entities, connections, player input, messages |

## Tools, quality, shipping

| Page | You will learn |
|---|---|
| [13 · Console, CVars and metrics](13-console-cvars-metrics.md) | developer tools and settings |
| [23 · Testing your game](23-testing.md) | pure functions, headless runs, scene validation in CI |
| [24 · Building and shipping](24-building-and-shipping.md) | dynamic and static builds, what to ship, defense checklist |
| [26 · Best practices](26-best-practices.md) | naming, data vs logic, events, safety, performance |
| [14 · Writing a plugin](14-writing-a-plugin.md) | adding a backend (renderer, audio, network, scripting) |
| [15 · Troubleshooting](15-troubleshooting.md) | error messages, exit codes, common mistakes |
| [27 · What is not in the engine yet](27-not-yet-in-the-engine.md) | missing features and where they will come from |

## The three kinds of people who use the engine

| Who | Works in | Recompiles? |
|---|---|---|
| Gameplay developer | C++ in the game module: components, systems, the module class | the game module only |
| Game designer | YAML scenes and prefabs, Lua scripts, assets, `input.yaml` | never |
| Engine developer | the engine core and plugins | the engine or one plugin |
