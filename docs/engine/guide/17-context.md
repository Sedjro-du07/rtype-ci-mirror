# 17 · The Context

`engine::Context` is everything game code can reach. Systems receive it as their only parameter;
`onStart` and `onShutdown` receive it too. You never see the `Engine` itself.

## What it gives you

| Call | Gives | Typical use |
|---|---|---|
| `ctx.world()` | the active scene's `Registry` | views, `get`, `tryGet`, `find` (page 05) |
| `ctx.commands()` | the command buffer | spawn, destroy, add, remove (page 05) |
| `ctx.scenes()` | the `SceneManager` | `switchTo`, `markPersistent`, `save` (pages 10, 21) |
| `ctx.events()` | the `EventBus` | publish, subscribe (page 06) |
| `ctx.input()` | the local player's `ActionState` | `pressed`, `held`, `axis` (page 07) |
| `ctx.inputMap()` | bindings and options | options menu, rebinding (page 07) |
| `ctx.assets()` | the `AssetManager` | textures, sounds, music, fonts, scripts (page 10) |
| `ctx.render()` | the `RenderQueue` | HUD, custom drawing, camera (page 08) |
| `ctx.audio()` | `IAudio` | sounds, music, volumes (page 09) |
| `ctx.network()` | the `NetworkSession` | connect, messages, stats (page 12) |
| `ctx.time()` | `Time` | `fixedDelta`, `tick`, `alpha` (page 16) |
| `ctx.log()` | the `Logger` | `ctx.log().warn("wave {} empty", n)` |
| `ctx.cvars()` | console variables | define and read settings (page 13) |
| `ctx.console()` | the developer console | register commands (page 13) |
| `ctx.metrics()` | metrics | record series for the overlay (page 13) |
| `ctx.role()` | `Client`, `Server` or `Editor` | role-dependent behaviour (page 22) |
| `ctx.requestQuit()` | — | the Quit button of a menu |

## Rules

- **Use it during the call, never store it.** Do not keep `Context&` in a component, a static or a
  member that outlives the call. It is valid only while the engine is calling you.
- **Re-read `world()` after a scene switch.** It returns the registry of the scene active now.
- **Loop thread only.** Never pass it, or anything obtained from it, to another thread.
- **Absent services are safe.** On the server, `audio()` and `render()` do nothing instead of
  crashing, so a shared system can call them (page 22).
- **No globals.** Everything a system needs comes from `ctx` or from state it owns (page 19).
