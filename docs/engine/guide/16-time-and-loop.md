# 16 · Time and the game loop

The engine owns the loop. Your code runs inside it, at fixed points. Understanding when gives you
correct, machine-independent games.

## One frame

```
POLL          devices → actions, network → messages
FIXED UPDATE  0, 1 or several steps of exactly 1/tickRate seconds
                 your FixedUpdate systems → commands applied → events delivered
LATE UPDATE   once: network send, bookkeeping
RENDER        once: draw, with positions interpolated between the last two steps
```

A fast PC renders many frames per step; a slow one runs two steps in one frame to catch up. The
simulation itself always advances in identical steps, on every machine and on the server.

## `ctx.time()`

| Field | Meaning | Use it for |
|---|---|---|
| `fixedDelta` | seconds per step (1/60 by default) | **every** movement, timer, cooldown in `FixedUpdate` |
| `tick` | number of steps since start | deterministic timers, "every 30 ticks" |
| `total` | simulated seconds since start | wave schedules, animations of the HUD |
| `alpha` | 0..1 position between the last two steps | custom interpolation in `Render` |
| `frameDelta` | real seconds of the last frame | UI effects only, never the simulation |

```cpp
// correct: same result at any frame rate
pos.x += vel.dx * static_cast<float>(ctx.time().fixedDelta);
gun.cooldown -= static_cast<float>(ctx.time().fixedDelta);

// wrong: speed depends on the machine
pos.x += 2.f;                              // "2 pixels per call"
pos.x += vel.dx * ctx.time().frameDelta;   // frame time in the simulation
```

## Timers

```cpp
struct Spawner { float every = 2.f; float timer = 0.f; };
ENGINE_COMPONENT(Spawner)

void spawnerSystem(engine::Context& ctx) {
    const float dt = static_cast<float>(ctx.time().fixedDelta);
    for (auto [e, pos, sp] : ctx.world().view<engine::Position, Spawner>()) {
        sp.timer += dt;
        while (sp.timer >= sp.every) { sp.timer -= sp.every; ctx.commands().spawn("prefabs/bydos.yaml", {pos.x, pos.y}); }
    }
}
```

`while` instead of `if` keeps the rhythm exact when one step covers more than one period.

## Interpolation

`engine.previous_position` copies `Position` into `PreviousPosition` at the start of each step;
`engine.render.sprites` draws at `lerp(previous, current, alpha)`. You get smooth motion for free.

**Teleporting** (respawn, wrap around): set both, or the sprite slides across the screen for one frame.

```cpp
ctx.commands().set(e, engine::Position{40, 90});
ctx.commands().set(e, engine::PreviousPosition{40, 90});
```

## Tick rate

`tickRate` in the config (default 60). Client and server must use the same value. Higher is more
precise and costs more CPU; 60 suits a shoot'em up. It is read-only at runtime (`sim_tick_rate`).

## Pausing

The engine never stops the loop. A game pauses by skipping its own simulation: keep a `Paused` flag
in the game state (page 19) and return early from gameplay systems. During development you can also
freeze one system from the console (`system disable rtype.wave_spawner`). On a networked game only the
server decides a pause.
