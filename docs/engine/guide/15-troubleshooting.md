# 15 · Troubleshooting

| You see | Cause | Fix |
|---|---|---|
| exit code 1, `config.yaml:7: unknown key 'tickrate'` | typo or unknown key in the config | keys are case-sensitive: `tickRate` |
| exit code 2, `platform_sfml: cannot load plugins/libplatform_sfml.so` | plugin not built or not next to the executable | rebuild; check `build/bin/plugins/` |
| exit code 2, `platform_sfml: API version 3, engine expects 4` | plugin built against another engine version | rebuild the plugin with the current engine |
| exit code 3, `games/librtype.so: symbol create_game not found` | `ENGINE_GAME_MODULE(...)` missing | add it at the end of the module `.cpp` |
| exit code 3, `schedule cycle: rtype.a → rtype.b → rtype.a` | contradictory `after` / `before` | remove one constraint |
| exit code 3, `unknown system 'engine.movment' in after()` | typo in a system name | `systems` in the console lists names |
| exit code 4, `scenes/level1.yaml:14: unknown component 'Helth'` | typo, or type not registered | fix the name or add it in `registerTypes` |
| exit code 5 | a client system threw | read the log line above it: system name and message |
| `[warn] audio_miniaudio: no output device, using NullAudio` | no sound card | expected; the game runs silently |
| checkerboard texture | missing or unreadable image | path is relative to the game folder; check the log |
| component values reset or wrong entity changed | a pointer or reference to a component was kept | keep the `Entity` and look it up again |
| crash in a loop over a view | `world.emplace/remove/destroy` during iteration | use `ctx.commands()` |
| game runs faster on one PC | movement not using `fixedDelta` or logic in `Render` | move logic to `FixedUpdate`, multiply by `fixedDelta` |
| two loggers / settings not shared between modules | a global or static singleton in game code | use the services from `Context` |
| `get<T>()` aborts on first use | `T` not registered | add it in `registerTypes` |
| client freezes then disconnects | server stopped answering; timeout after `net_timeout` | check the server log; raise `net_timeout` only for debugging |
