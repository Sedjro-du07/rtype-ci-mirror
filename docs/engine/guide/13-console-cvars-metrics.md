# 13 · Console, CVars and metrics

## Console

Open with F1 (action `Console`). Tab completes, up/down browses history.

| Command | Does |
|---|---|
| `help [command]` | list commands or show one |
| `set <cvar> <value>`, `get <cvar>`, `cvars [prefix]` | console variables |
| `systems` | systems with stage, timing and status |
| `system enable\|disable <name>` | toggle a system |
| `entities [component]` | count or list entities |
| `inspect <name>` | components of an entity |
| `scene <path>` | switch scene |
| `modules` | loaded plugins and game module |
| `assets` | loaded assets and memory |
| `net` | connections and statistics |
| `bind <action> <key>` | rebind |
| `quit` | quit cleanly |

## Your own commands and variables

```cpp
void MyGameModule::onStart(engine::Context& ctx) {
    godMode_ = &ctx.cvars().define("rtype_god", false, "Players take no damage", engine::Cheat);
    ctx.console().registerCommand("spawn_wave", "spawn_wave <n>: spawns wave n now",
        [this](auto args, engine::Console& c) { /* … */ c.print("wave spawned"); });
}
```

`Cheat` variables are refused on servers unless `sv_cheats 1`. `Archive` variables are saved in
the user settings file.

## Engine CVars

| CVar | Default | Meaning |
|---|---|---|
| `r_vsync` | 1 | vertical sync |
| `r_ui_scale` | 1.0 | text and HUD scale |
| `r_color_filter` | none | colour-blindness filter |
| `r_debug_colliders` | 0 | draw collider outlines |
| `snd_master`, `snd_music`, `snd_effects`, `snd_interface` | 1.0 | bus volumes |
| `in_deadzone` | 0.2 | gamepad dead zone |
| `net_snapshot_rate` | 20 | snapshots per second (server) |
| `net_timeout` | 5.0 | seconds before a silent peer is disconnected |
| `asset_hot_reload` | 0 | reload changed files |
| `sim_tick_rate` | (read-only) | fixed steps per second |
| `sv_cheats` | 0 | allow cheat variables on a server |

## Metrics overlay

F2 toggles the overlay: FPS and frame-time graph, time per system, entity count, draw calls, and
the network lagometer (RTT, loss, bandwidth). Record your own series:

```cpp
ctx.metrics().record("rtype.enemies", static_cast<double>(enemyCount));
```
