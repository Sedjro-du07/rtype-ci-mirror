# What a game uses from the engine

Status: **Locked** with the architecture (2026-10-09). This page lists **everything a game may
use** from the engine: types, methods, macros, components, systems, events, CVars, CMake functions
and the Lua API. If it is not on this page, game code must not touch it. Everything else is engine
internals ([`to-implement.md`](to-implement.md)). Explanations and examples are in the guide
([`guide/README.md`](guide/README.md)); full signatures in the reference ([`reference/README.md`](reference/README.md)).

All types are in namespace `engine`. Headers are under `engine/include/engine/`.

---

## 1 · Plugging the game in

### CMake — guide [02](guide/02-project-and-config.md), [24](guide/24-building-and-shipping.md)

| Function | Use |
|---|---|
| `engine_add_game_module(<name> SOURCES … [DEPENDS …])` | builds the game as `games/lib<name>.so` (or static) |
| `engine_add_launcher(<exe> CONFIG <file.yaml> [GAME <name>])` | builds the executable (`r-type_client`, `r-type_server`); the engine's `main()` |
| `ENGINE_DYNAMIC_MODULES` | option: `ON` (default) or `OFF` for one static executable |

### Macros

| Macro | Header | Use |
|---|---|---|
| `ENGINE_GAME_MODULE(Class)` | `module/Export.hpp` | once, at the end of the module `.cpp` |
| `ENGINE_COMPONENT(Type)` | `reflection/TypeKey.hpp` | after each component struct |
| `ENGINE_EVENT(Type)` | `reflection/TypeKey.hpp` | after each event struct (inside the process) |
| `ENGINE_MESSAGE(Type)` | `reflection/TypeKey.hpp` | after each network message struct |
| `ENGINE_STRUCT(Type)` | `reflection/TypeKey.hpp` | after each struct used only as a field of another type |
| `ENGINE_ENUM(Type, values…)` | `reflection/TypeKey.hpp` | after each enum used in a field |

Type macros go right after the type, outside any namespace; the macro says **what kind** of type it
is, and `types.add<T>()` uses it (ADR 0036).

### `IGameModule` — the class the game implements · `module/IGameModule.hpp` · guide [19](guide/19-game-module-and-state.md)

| Method | Must | Called |
|---|---|---|
| `void registerTypes(TypeRegistry&)` | yes | once at boot, after engine types |
| `void registerSystems(Role, Scheduler&)` | yes | once at load, after engine systems |
| `std::string_view firstScene() const` | yes | at load (unless `--scene`) |
| `void onStart(Context&)` | no | client: once, after the first scene, before the loop |
| `void onLobbyStart(LobbyContext&)` | no | server: once, lobby subscriptions and `on<T>` |
| `void onRoomStart(Context&)` | no | server: once per room, room subscriptions with `ctx.room().keep(...)` |
| `void onShutdown(ServiceContext&)` | no | once, before the module is unloaded |

### Config — `config.yaml` and command line · guide [02](guide/02-project-and-config.md)

Keys of `EngineConfig`: `role`, `game`, `platform`, `audio`, `network`, `scripting`, `tickRate`, `root`,
`window` (`WindowDesc`), `net` (`NetConfig`), `log` (`LogConfig`), `cvars`.
Options: `--config`, `--role`, `--platform`, `--audio`, `--network`, `--scripting`, `--port`, `--host`,
`--log`, `--scene`, `--ticks`, `--set name=value`. Plugin name `null` = no backend.
Exit codes: 0 ok, 1 config, 2 plugin, 3 game module or schedule, 4 scene, 5 client system error.

---

## 2 · Registration (in `registerTypes` and `registerSystems`)

### `TypeRegistry` · `reflection/TypeRegistry.hpp` · guide [03](guide/03-components.md)

| Method | Use |
|---|---|
| `auto add<T>()` | the only registration method, for every kind; returns the builder of the kind of `T` |
| `ComponentId id<T>() const`, `const TypeInfo* find(name) const` | rarely: generic code, tools |

### Builders · `reflection/TypeBuilder.hpp`

| Builder (returned by `add<T>()` for…) | Methods |
|---|---|
| all except enums | `field(name, &T::member, tooltip = {})`: one per field read from YAML, shown, saved or sent; `maxLength(n)`: limit of the text or list field just declared (default 256 / 64); `quantize(min, max)`: the float or `Vec2` field just declared travels as 16 bits |
| `ComponentBuilder` (`ENGINE_COMPONENT`) | `flags(f)`: `Replicated`, `Transient`, `EditorHidden`, `Tag`; `yaml(read, write)`; `binary(encode, decode)` |
| `MessageBuilder` (`ENGINE_MESSAGE`) | `unreliable()` (UDP), `reliable()` (UDP + acks, default), `stream()` (TCP); `binary(encode, decode)` |
| `EventBuilder`, `StructBuilder` | fields only |
| `EnumBuilder` | nothing |

Supported field types (`FieldKind`): `bool`, `int32_t`, `uint32_t`, `int64_t`, `float`, `double`,
`std::string`, `Vec2`, `Rect`, `Color`, `Entity`, `TextureHandle`, `SoundHandle`, `MusicHandle`,
`FontHandle`, `ScriptHandle`, `ENGINE_ENUM` enums, `ENGINE_STRUCT` structs, `std::vector` of these.

### `Scheduler` · `scheduler/Scheduler.hpp` · guide [04](guide/04-systems.md)

| Item | Use |
|---|---|
| `SystemBuilder add(Stage, std::string name, SystemFn fn)` | register a system named `"<game>.<system>"` |
| `SystemBuilder::after(name)` / `before(name)` | order within the stage (engine names: section 6) |
| `SystemBuilder::roles(mask)` | `ClientRole`, `ServerRole`, `EditorRole`, `AllRoles` (enum `RoleMask`) |
| `SystemBuilder::disabled()` | starts off |
| `SystemBuilder::critical()` | on the server, a fault ends the session |
| `Stage` | `FixedUpdate` (logic), `LateUpdate` (after simulation), `Render` (drawing) |
| `SystemFn` | any callable `void(Context&)`: function, lambda, functor with state |
| `Role` | `Client`, `Server`, `Editor` (argument of `registerSystems`) |

---

## 3 · At runtime: the `Context` · `Context.hpp` · guide [17](guide/17-context.md), [28](guide/28-lobby-parties-rooms.md)

Three context types (ADR 0039): `Context` (game systems: one per room on the server, the game on the
client), `LobbyContext` (server lobby code: no world), `ServiceContext` (their common base: `log`,
read-only `cvars`, `assets`, `metrics`, `role`). On the server, `ctx.world()`, `events()`, `time()` and
`network()` are those of the room being ticked; `room()` gives the room, `requestQuit()` is client only.

Every system and hook receives a `Context&`. It is the only door into the engine. Valid only
during the call; never store it.

| Method | Returns | Section |
|---|---|---|
| `world()` | `Registry&` of the active scene | 4 |
| `commands()` | `Commands&` | 4 |
| `scenes()` | `SceneManager&` | 8 |
| `events()` | `EventBus&` | 5 |
| `input()` | `const ActionState&` | 7 |
| `inputMap()` | `InputMap&` | 7 |
| `assets()` | `AssetManager&` | 8 |
| `render()` | `RenderQueue&` | 9 |
| `audio()` | `IAudio&` | 10 |
| `network()` | `NetworkSession&` | 11 |
| `time()` | `const Time&` | 12 |
| `log()`, `cvars()`, `console()`, `metrics()` | services | 13 |
| `role()` | `Role` | 2 |
| `requestQuit()` | quits after this frame | — |

---

## 4 · World: entities and components · guide [05](guide/05-entities.md), [20](guide/20-hierarchy-and-identity.md)

### `Entity` · `ecs/Entity.hpp`

`index`, `generation`, `Entity::null()`, `isNull()`, `==`, usable as a key in `std::unordered_map`.
Keep an `Entity`, never a pointer to a component.

### `Registry` (via `ctx.world()`) · `ecs/Registry.hpp`

| Method | Allowed |
|---|---|
| `get<T>(e)`, `tryGet<T>(e)`, `has<T>(e)` | always |
| `replace<T>(e, value)` | always (fires `onReplace`) |
| `alive(e)`, `aliveCount()` | always |
| `view<Ts...>()`, `view<Ts...>(exclude<Xs...>)` | always |
| `find(name)`, `find(uuid)`, `withTag(tag)` | always |
| `parent(e)`, `children(e)` | always |
| `hooks<T>()` → `onCreate`, `onReplace`, `onDestroy` | in `onStart`; keep the `Subscription` |
| `create()`, `emplace<T>`, `remove<T>`, `setParent` | **outside** any view loop only (e.g. `onStart`); inside systems use `Commands` |
| `destroy`, `clear`, raw methods | **no**: engine only |

### `View<Ts...>` · `ecs/View.hpp`

`for (auto [e, a, b] : view)`, `each(fn)`, `front()`, `sizeHint()`. Reading and writing component
values inside is safe; structural changes go through `Commands`.

### `Commands` (via `ctx.commands()`) · `ecs/Commands.hpp`

| Method | Applied at the end of the stage |
|---|---|
| `Entity spawn()` | empty entity (handle valid now) |
| `Entity spawn(prefab, position)` | prefab instance |
| `destroy(e)` | with its children |
| `add<T>(e, value)`, `set<T>(e, value)`, `remove<T>(e)` | add / add-or-replace / remove |
| `setParent(child, parent, offset = {})` | attach; `Entity::null()` detaches |

### Engine components · `components/*.hpp`, `net/NetComponents.hpp` · reference [13](reference/13-components-systems.md)

| Component | Fields | Read by |
|---|---|---|
| `Position` | `x, y` | everything |
| `PreviousPosition` | `x, y` | interpolation (reset it with `Position` on teleport) |
| `Velocity` | `dx, dy` (units/s) | `engine.movement` |
| `Sprite` | `texture, frame, layer, tint, scale, origin, rotation, visible` | `engine.render.sprites` |
| `Animation` | `frames, fps, loop, time, current` | `engine.animation` |
| `Text` | `value, font, size, color, layer, screenSpace` | `engine.render.text` |
| `Collider` | `size, offset, layer, mask, trigger` | `engine.collision` |
| `Lifetime` | `seconds` | `engine.lifetime` |
| `Parallax` | `factor, wrapWidth` | `engine.parallax` |
| `Name`, `Uuid`, `Tags` | identity | `find`, `withTag`, scene files |
| `Parent`, `Children` | `value, offset` / `list` | `engine.hierarchy` |
| `Script` | `file` | `engine.script.*` |
| `NetworkId`, `NetPrefab`, `NetOwner{player}` | network identity, prefab, owning player | `engine.net.*` |
| `RemoteInput` | `state` (`ActionState`) | server: a player's actions |
| `NetInterpolation` | snapshot positions | client, engine |

---

## 5 · Events · `events/EventBus.hpp`, `core/Signal.hpp` · guide [06](guide/06-events.md)

| Item | Use |
|---|---|
| `Subscription subscribe<E>(fn)` | listen; keep the returned `Subscription` (unsubscribes when destroyed) |
| `publish<E>(event)` | queued, delivered at the next dispatch (after each fixed step, after `LateUpdate`) |
| `Subscription` | `reset()`, `active()`, move-only |
| `Signal<void(Args...)>` | `connect(fn)` → `Subscription` (component hooks) |

Do not call `emit`, `publishFromAnyThread`, `dispatch` (engine only).

Engine events (`events/EngineEvents.hpp`): `QuitRequested`, `WindowResized{width, height}`,
`WindowFocusChanged{focused}`, `Collision{a, b}`, `SceneLoaded{name}`, `SceneUnloading{name}`,
`PlayerJoined{player}`, `PlayerLeft{player, reason, message}`, `PlayerDisconnected{player, reason}`,
`PlayerReconnected{player}`, `JoinedServer{me}`, `LeftServer{reason, message}`,
`ConnectionInterrupted{reason}`, `Reconnected`,
`Caption{text, seconds}`, `ConsoleCommand{line}`.

---

## 6 · Engine systems the game orders against · reference [13](reference/13-components-systems.md)

FixedUpdate: `engine.net.receive_input`, `engine.net.apply_snapshot`, `engine.previous_position`,
`engine.movement`, `engine.hierarchy`, `engine.parallax`, `engine.lifetime`, `engine.animation`,
`engine.collision`, `engine.script.lifecycle`, `engine.script.update`, `engine.script.collision`.
LateUpdate: `engine.net.assign_ids`, `engine.net.snapshot`, `engine.net.send_input`.
Render: `engine.net.interpolate`, `engine.render.sprites`, `engine.render.text`, `engine.render.debug`,
`engine.debug_overlay`.

Typical: player input `before("engine.movement")`, damage `after("engine.collision")`.

---

## 7 · Input · `input/*.hpp` · guide [07](guide/07-input.md)

| Item | Use |
|---|---|
| `ActionId action("MoveUp")` | `constexpr`; name as in `input.yaml` |
| `ActionState` (via `ctx.input()` or `RemoteInput::state`) | `pressed(a)`, `held(a)`, `released(a)`, `axis(a)` |
| `InputMap` (via `ctx.inputMap()`) — options menu | `bind`, `unbind`, `clear`, `bindings`, `boundTo`, `captureNextBinding`, `setOptions`, `options`, `save`, `load` |
| `Binding` | `Key`, `MouseButton`, `GamepadButton` or `AxisBinding{axis, direction, threshold}` |
| `InputOptions` | `holdToToggle`, `autoRepeat`, `autoRepeatHz`, `deadzone`, `oneHanded` |
| `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis`, `keyName(Key)` | for rebinding screens |

Do not call `feed`, `beginTick`, `ActionState::toMask` / `fromMask` (engine only).

---

## 8 · Scenes, prefabs, assets · guide [10](guide/10-scenes-prefabs-assets.md), [21](guide/21-saving.md)

### `SceneManager` (via `ctx.scenes()`)

| Method | Use |
|---|---|
| `switchTo(file)` | change level at the end of the frame |
| `markPersistent(e)` | keep an entity across the switch |
| `save(file)` | write the active scene (save games, editor) |
| `world()`, `activeName()`, `prefabs()` | read |

### `AssetManager` (via `ctx.assets()`)

| Method | Use |
|---|---|
| `texture(path)`, `font(path)`, `sound(path)`, `music(path)`, `script(path)` | handle, loaded once; fallback on failure |
| `pathOf(handle)` | reverse lookup |
| `retain(h)`, `release(h)` | keep an asset alive across scenes |
| `resolve(path)` | game root + path |

Handles (`render/Handles.hpp`): `TextureHandle`, `FontHandle`, `SoundHandle`, `MusicHandle`,
`ScriptHandle` — `valid()`, `==`.

---

## 9 · Drawing · `render/*.hpp` · guide [08](guide/08-rendering.md)

| `RenderQueue` (via `ctx.render()`) | Use |
|---|---|
| `sprite(tex, source, pos, layer, scale, rotation, tint, origin)` | one sprite this frame |
| `text(s, font, size, pos, layer, color, screenSpace = true)` | HUD text |
| `rect(r, color, layer, filled, screenSpace)`, `line(a, b, c, layer)`, `circle(c, r, color, layer)` | shapes, bars, debug |
| `push(DrawCmd)` | raw form |
| `camera()`, `setCamera(Camera2D)` | scrolling, zoom |
| `setClearColor(Color)` | background |

`Camera2D`: `center`, `zoom`, `rotation`, `screenToWorld`, `worldToScreen`. `DrawCmd`, `WindowDesc`,
`ColorFilter` as data. Only in `Render` systems. Do not call `frame()` or `clear()`.

---

## 10 · Sound · `audio/IAudio.hpp` · guide [09](guide/09-audio.md)

| `IAudio` (via `ctx.audio()`) | Use |
|---|---|
| `VoiceId play(sound, PlayParams = {})` | `PlayParams`: `volume`, `pitch`, `pan`, `bus`, `position`, `loop`, `caption` |
| `stop(voice)`, `setVoiceVolume(voice, v)` | control a playing sound |
| `playMusic(music, fadeIn)`, `stopMusic(fadeOut)` | music |
| `setBusVolume(bus, v)`, `busVolume(bus)` | options menu (`AudioBus`: `Master`, `Music`, `Effects`, `Interface`) |
| `setListener(position, radius)` | positional sound |

Load sounds through `AssetManager`, not `createSound`. Do not call `update()`. On the server it is
`NullAudio`: calls are harmless.

---

## 11 · Network · `net/*.hpp` · guide [12](guide/12-networking.md), [22](guide/22-roles.md)

| `NetworkSession` (via `ctx.network()`) | Use |
|---|---|
| `startServer(port = config)`, `stopServer()` | server: open (UDP + TCP) and close; nothing opens by itself |
| `connectTo(host, port, name)`, `connectTo(host, port, name, data)`, `disconnect()` | client: join with a player name (and the game's admission data), leave; reconnects by itself after a loss |
| `players()`, `name(p)`, `ping(p)`, `stats(p)`, `kick(p, reason)` | server: who is there |
| `admission<T>(fn(request, data) → Admission::accept() / refuse(text))` | server lobby code: who may join (ADR 0042) |
| `localPlayer()`, `isServer()`, `connected()` | state |
| `send(player, msg)`, `broadcast(msg, except)` | server → clients, typed messages |
| `send(msg)` | client → server |
| `on<T>(fn(ctx, from, msg))` / `on<T>(fn(ctx, msg))` | receive; returns a `Subscription`; runs at the start of the frame on the game thread |

Types: `PlayerId`, `DisconnectReason` (`Quit`, `Timeout`, `Kicked`, `ProtocolError`, `ServerShutdown`,
`ServerFull`, `FormatMismatch`, `ProtocolMismatch`, `WrongGame`, `InvalidName`, `RefusedByGame`,
`ConnectionLost`), `ConnectionStats`, `AdmissionRequest`, `Admission`, component `NetPriority{weight}`.
Settings (ADR 0041, 0043): `net.snapshotRate`, `net.sendBudgetBytesPerSecond`, `net.interpolationDelay`,
`net.inputRedundancy`, `net.reconnectGraceSeconds`, `net.autoReconnect`, `net.shareOnlinePlayers`;
field option `.quantize(min, max)`. Bytes: RFC [`docs/network/rfc.md`](../network/rfc.md). The game never writes bytes: messages are structs
(`ENGINE_MESSAGE`) serialized by the engine; `Entity` fields arrive as local entities or
`Entity::null()`. Replication needs nothing more than the `Replicated` flag and a `NetPrefab`.
`ByteWriter` / `ByteReader` remain only for a custom `binary()` codec. Bytes, ids, reliability:
protocol RFC (Network part).

---

## 12 · Time · `core/Time.hpp` · guide [16](guide/16-time-and-loop.md)

`ctx.time()`: `fixedDelta` (use it for every movement), `tick`, `total`, `alpha` (render only),
`frameDelta` (UI only). `Clock` (`restart`, `elapsed`) for tools and measurements.

---

## 13 · Services · `services/*.hpp` · guide [13](guide/13-console-cvars-metrics.md)

| Service | Methods for games |
|---|---|
| `Logger` | `trace`, `debug`, `info`, `warn`, `error` (`std::format` syntax), `tagged(tag)` |
| `CVars` | `define<T>(name, default, description, flags, range)` → `CVar<T>&`; `find<T>`, `get`, `set` |
| `CVar<T>` | `value()`, `set(v)`, `onChange(fn)` → `Subscription` |
| `CVarFlags` | `Archive` (saved in `user/settings.yaml`), `Cheat`, `ReadOnly`, `ServerOnly` |
| `Console` | `registerCommand(name, help, fn)`, `execute(line)`, `print(text)` |
| `Metrics` | `record(series, value)`, `fps()`, `frameMs()` |

Engine CVars a game may read or set: `r_vsync`, `r_ui_scale`, `r_color_filter`, `r_debug_colliders`,
`snd_master`, `snd_music`, `snd_effects`, `snd_interface`, `in_deadzone`, `net_snapshot_rate`,
`net_timeout`, `asset_hot_reload`, `phys_grid_cell`, `sim_tick_rate` (read-only).

---

## 14 · Vocabulary · `core/*.hpp`

| Type | Members |
|---|---|
| `Vec2` | `x, y`, `+ - *`, `dot`, `length`, `normalized`, `lerp` |
| `Rect` | `x, y, w, h`, `contains`, `intersects` |
| `Color` | `r, g, b, a`, `white()`, `black()`… |
| `Result<T>`, `Error`, `Errc` | return values of fallible calls; `Error::toString()` |
| `fnv1a64(text)` | compile-time hash (custom keys) |

---

## 15 · Lua API (game designers) · guide [11](guide/11-scripting-lua.md)

Hooks: `on_spawn(self)`, `on_update(self, dt)`, `on_collision(self, other)`, `on_destroy(self)`.
Calls: `self:get(name)`, `self:set(name, t)`, `self:has(name)`, `self:remove(name)`, `self:has_tag(tag)`,
`self:id()`, `spawn(prefab, x, y)`, `destroy(e)`, `find(name)`, `play(sound, params)`,
`publish(event, t)`, `time()`, `cvar(name)`, `log(text)`.

---

## 16 · Lobby, parties and rooms · `net/Room*.hpp`, `server/*.hpp`, `client/*.hpp` · guide [28](guide/28-lobby-parties-rooms.md) · reference [14](reference/14-rooms.md)

| Where | API |
|---|---|
| Server lobby (`LobbyContext::lobby()`) | `Lobby`: `party`, `partyOf`, `parties`, `sendInvite`, `addToParty`, `removeFromParty`, `setLeader`, `sendToParty<T>`, `createRoom(RoomSettings)`, `moveParty`, `moveToLobby(party, reason)`, `closeRoom(room, reason)`, `sendRoomList`, `refuse(player, request, reason)`, `room`, `rooms` |
| Server room (`Context::room()`) | `RoomHandle`: `id`, `settings`, `players`, `parties`, `keep(Subscription)`, `reportFinished<T>` |
| Server room (`Context::network()`) | `RoomNetwork`: `players`, `send`, `broadcast`, `on<T>`, `ping`, `setRelevance(fn)` — the room's players only |
| Client (`Context::lobby()`) | `ClientLobby`: `myParty`, `invite`, `answerInvite`, `leaveParty`, `requestRoomList`, `currentRoom`, `onlinePlayers`, `name(p)`, `connected(p)` |
| World | `Registry::singleton<T>()`, `trySingleton<T>()`: the state of one game |

Types: `PartyId`, `RoomId`, `RoomState`, `RoomSettings` (`name`, `level`, `maxPlayers`, `listed`), `Party`,
`RoomInfo`. Lobby events: `PartyChanged`, `InviteRequested`, `InviteAnswered`, `LeavePartyRequested`,
`RoomListRequested`, `RoomStarted`, `RoomFinished` (`resultAs<T>()`), `RoomFailed`, `RoomClosed`. Room
events: `PlayerEnteredRoom`, `PlayerLeftRoom` (plus `PlayerDisconnected`, `PlayerReconnected`). Client
events: `PartyUpdated`, `InviteReceived`, `RoomListReceived`, `EnteredRoom`, `ReturnedToLobby`,
`RoomInterrupted`, `RequestRefused{request, reason}`, `OnlinePlayersChanged`.

Where to include (ADR 0040): game `server/` code includes `engine/server/`, game `client/` code includes
`engine/client/`, game `shared/` code includes neither.

Rules: game state lives in the room (singletons, components), never in module members or globals;
rooms are created through the lobby; starting a game is a game message (`PlayRequest`); systems with
members are copied once per room.

## Never in game code

`Engine`, `EngineConfig::load`, `ModuleManager`, `DynamicLibrary`, `StaticModuleTable`, `IPlugin`,
`IPlatform`, `IRenderer`, `INetwork`, `IScripting`, `ConnectionId`, `NetworkSession::sendRaw`, `RoomRuntime`, `IRoomHost`, `ThreadPoolRoomHost`, `WorkerPool`, `PartyTable`, `RoomDirectory`, `RoomMailbox`, `PlayerRouting`, `IStorage`, `SparseSet`, `EntityAllocator`,
`SceneLoader`, `NetworkIdMap`, `TypeInfo` function pointers, and every method marked *engine only* in
the reference (`Scheduler::build/run`, `Commands::apply`, `EventBus::dispatch`, `InputMap::feed`,
`RenderQueue::frame`, `SceneManager::applyPending`, …). Writing a backend instead of a game? See
guide [14 · Writing a plugin](guide/14-writing-a-plugin.md).
