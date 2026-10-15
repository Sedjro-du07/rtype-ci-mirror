# 04 · Systems and scheduling

A system is a callable taking `engine::Context&`. It is where game logic lives.

## Write

```cpp
void bydosAiSystem(engine::Context& ctx) {
    const float dt = static_cast<float>(ctx.time().fixedDelta);
    for (auto [e, pos, ai] : ctx.world().view<engine::Position, BydosAI>()) {
        ai.timer -= dt;
        if (ai.timer <= 0.f) {
            ctx.commands().spawn("prefabs/bydos_shot.yaml", {pos.x - 16, pos.y});
            ai.timer = ai.shootCooldown;
        }
    }
}
```

Systems with state are functors or lambdas capturing it:

```cpp
struct WaveSpawner {
    float timer = 0.f;
    void operator()(engine::Context& ctx) { /* … */ }
};
```

## Register

```cpp
void MyGameModule::registerSystems(engine::Role role, engine::Scheduler& s) {
    using engine::Stage;
    s.add(Stage::FixedUpdate, "rtype.player_control", playerControlSystem)
        .before("engine.movement");
    s.add(Stage::FixedUpdate, "rtype.damage", damageSystem)
        .after("engine.collision")
        .roles(engine::ServerRole);
    s.add(Stage::FixedUpdate, "rtype.wave_spawner", WaveSpawner{})
        .roles(engine::ServerRole)
        .critical();
    s.add(Stage::Render, "rtype.hud", hudSystem)
        .roles(engine::ClientRole);
}
```

- Name every system `"<game>.<system>"`. Names are used for ordering, the console and the profiler.
- `roles()` is optional; you can also test `role` with an `if`.

## Stages

| Stage | Runs | Use for |
|---|---|---|
| `FixedUpdate` | N times per frame, every `1/tickRate` | anything that changes the simulation: input, movement, AI, collisions, damage, spawning |
| `LateUpdate` | once per frame after the simulation | sending network data, bookkeeping |
| `Render` | once per frame, client and editor | filling the render queue, HUD |

Always use `ctx.time().fixedDelta` in `FixedUpdate`; never count frames.

## Ordering

Inside a stage, order follows `after` / `before` against any system name, engine systems included
(list in reference page 13). An unknown name or a cycle stops the boot with exit code 3 and the
names involved.

## When a system throws

| Role | Result |
|---|---|
| Server | the system is logged and disabled, the game continues; `critical()` systems end the session cleanly |
| Client | the error is logged and shown, the client quits with code 5 |

In the console: `systems` lists systems with timings and status; `system disable rtype.hud` turns one off.

## Rules

- Change component values directly; change structure (spawn, destroy, add, remove) through `ctx.commands()`.
- Do not keep `Context`, component references or views between calls.
- Do not block: no file I/O or sleeping in a system. Use `onStart` or the asset manager.
