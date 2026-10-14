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
| two entities overlap but no `Collision` | layers and masks do not agree both ways | `(a.layer & b.mask)` and `(b.layer & a.mask)` must both be non-zero (page 18) |
| `Collision` received every tick for the same pair | normal: reported while they overlap | remove the collider or add a marker when handled (page 18) |
| entity slides across the screen after a respawn | `PreviousPosition` not reset | set both `Position` and `PreviousPosition` (page 16) |
| the Force stays behind when the ship moves | `Parent` missing or set with `world.setParent` inside a view | use `ctx.commands().setParent` in systems (page 20) |
| child destroyed with its parent unexpectedly | destruction cascades to children | detach before destroying the parent (page 20) |
| `world()` returns an empty world after `switchTo` | the switch happens at the end of the frame | read entities of the new scene from the next frame on (`SceneLoaded` event) |
| components lost after `switchTo` | the scene was replaced | `markPersistent` entities that must survive (page 10) |
| save file contains runtime values (timers, `PreviousPosition`) | components not flagged `Transient` | flag them at registration (page 21) |
| settings not kept between runs | CVar defined without `Archive` | add `engine::Archive` (page 21) |
| `script.lua:12: attempt to index a nil value` | component name typo or component absent | check with `self:has(name)`; names are the registered ones |
| script stopped with "instruction limit" | infinite or very long loop in a hook | split the work over several `on_update` calls |
| a server system disappears from `systems` timings | it threw and was disabled | read the log line naming it; fix and restart |
| clients never connect | wrong host or port, firewall, server not started | `net` on the server; check `server.yaml` and the client command line |
| `PlayerLeft{Timeout}` while the client runs | no packet for `net_timeout` seconds (debugger pause, frozen client) | expected; raise `net_timeout` only while debugging |
| works on Linux, crashes at load on Windows | plugin or game built with another compiler or runtime | rebuild everything with the same Visual Studio and `/MD` (page 24) |
| client gets `LeftServer{FormatMismatch}` | client and server built from different versions of the game types | rebuild both from the same commit |
| client gets `LeftServer{ServerFull}` | `maxClients` players already connected | raise `net.maxClients` in `server.yaml` or wait |
| a message callback never runs | type not registered, `on<T>` subscription not kept, or server not started | `types.add<T>()`, keep the `Subscription` in a member, call `startServer` |
| `Entity` field of a message is null | entity not replicated, or the message arrived before its spawn snapshot | give it a `NetPrefab`; check `isNull()` and ignore |
| `add<T>()` fails with "needs ENGINE_COMPONENT, ENGINE_EVENT…" | the type macro is missing | add the macro after the type, outside any namespace |
| static build: `unknown plugin 'platform_sfml'` | plugin not linked into the executable | `engine_add_launcher(... GAME ...)` links plugins only in static mode; rebuild with the option set |

