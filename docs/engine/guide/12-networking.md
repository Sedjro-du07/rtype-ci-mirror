# 12 · Networking

The server is part of the engine. A game never touches sockets, threads, connections or bytes: it
**opens** the server or **connects**, deals with **players**, and sends **typed messages**. The engine
does the rest: transport, timeouts, replication, player input, serialization (ADR 0037). The bytes
themselves are specified in the protocol RFC (Network part).

## Open the server, connect a client

Nothing opens by itself. The game decides when, usually in `onStart`:

```cpp
void RType::onStart(engine::Context& ctx) {
    if (ctx.role() == engine::Role::Server)
        ctx.network().startServer();                         // port from --port or server.yaml
    else
        ctx.network().connectTo("127.0.0.1", 4242, "Sedjro"); // or later, from the connection menu
}
```

```yaml
# server.yaml
role: server
game: rtype
net: { port: 4242, maxClients: 32, snapshotRate: 20 }   # maxClients: whole server, lobby included
```

| Call | Side | Does |
|---|---|---|
| `startServer(port = config)` | server | opens UDP and TCP, accepts players |
| `stopServer()` | server | tells every player (`ServerShutdown`) and closes |
| `connectTo(host, port, name)` | client | joins; `name` is the player's name (16 characters max by default) |
| `disconnect()` | client | leaves |

Because opening is explicit, headless tests do not open ports unless they ask to, and a client could
later host a game from a menu with the same call.

## Players

The game sees players, never connections:

| Server | Client |
|---|---|
| event `PlayerJoined{player}` | event `JoinedServer{me}` |
| event `PlayerLeft{player, reason}` | event `LeftServer{reason}` |
| `players()`, `name(p)`, `ping(p)`, `kick(p, reason)` | `localPlayer()`, `connected()` |

`reason` is one of `Quit`, `Timeout`, `Kicked`, `ProtocolError`, `ServerShutdown`, `ServerFull`,
`FormatMismatch`. Your game will also be able to refuse a player with its own reason (version,
password, bans…) through an admission step, after the engine's checks (ADR 0042; API with the protocol
RFC). A crashed client times out after `net_timeout` seconds and is reported as `PlayerLeft{Timeout}`.

```cpp
// server, in onStart: give each new player a ship
joinSub_ = ctx.events().subscribe<engine::PlayerJoined>([&ctx](const engine::PlayerJoined& e) {
    auto ship = ctx.commands().spawn("prefabs/player_ship.yaml", {40, 90});
    ctx.commands().add(ship, engine::NetOwner{e.player});
});
leftSub_ = ctx.events().subscribe<engine::PlayerLeft>([&ctx](const engine::PlayerLeft& e) {
    for (auto [ship, owner] : ctx.world().view<engine::NetOwner>())
        if (owner.owner == e.player) ctx.commands().destroy(ship);
});
```

Keep the returned `Subscription`s in members of your module.

## Replicated entities

1. Mark components to send: `.flags(engine::Replicated)` in `registerTypes`.
   Engine `Position`, `Velocity` and `Sprite` already are.
2. Spawn networked entities from a prefab on the server: `ctx.commands().spawn("prefabs/bydos.yaml", pos)`.
   The prefab gives `NetPrefab`; the engine assigns a `NetworkId` and clients spawn the same prefab.
3. Run gameplay systems with `.roles(engine::ServerRole)`. Clients receive the results.

Client positions are interpolated between snapshots by default; no game code is needed. To use your
own interpolation, disable `engine.net.interpolate` and write a client system. Prediction of the
local player and other network optimizations are your team's work, with the means the engine gives
(ADR 0041); the settings and extension points come with the protocol RFC.

## Player input

The client sends its actions each tick automatically. On the server, the ship owned by a player
(`NetOwner`) receives them in `RemoteInput`:

```cpp
void playerControlSystem(engine::Context& ctx) {           // server
    for (auto [e, in, vel] : ctx.world().view<engine::RemoteInput, engine::Velocity>())
        vel.dy = in.state.held(MoveUp) ? -speed : in.state.held(MoveDown) ? speed : 0.f;
}
```

## Game messages

For what is not component state: chat, "ready", "wave cleared", the lobby's room list.

**1. Declare and register.** A message is a struct with `ENGINE_MESSAGE`, registered like any type.
The engine serializes it from its fields; you never write bytes.

```cpp
struct Chat        { std::string text; };                ENGINE_MESSAGE(Chat)
struct Ready       { };                                  ENGINE_MESSAGE(Ready)
struct WaveCleared { int32_t wave; };                    ENGINE_MESSAGE(WaveCleared)
struct PlayerKilled{ engine::Entity victim; };           ENGINE_MESSAGE(PlayerKilled)

types.add<Chat>().field("text", &Chat::text).maxLength(128).stream();
types.add<Ready>();
types.add<WaveCleared>().field("wave", &WaveCleared::wave);
types.add<PlayerKilled>().field("victim", &PlayerKilled::victim);
```

**2. Choose how it travels**, once, on the message:

| Option | Transport | Use for |
|---|---|---|
| `.unreliable()` | UDP, may be lost | frequent values where the next one replaces the last |
| `.reliable()` (default) | UDP, acknowledged and resent | in-game messages that must arrive (the subject requires UDP in game) |
| `.stream()` | TCP, reliable and ordered | out of game: lobby, room lists, text chat |

**3. Send.**

```cpp
ctx.network().send(player, WaveCleared{3});          // server → one player
ctx.network().broadcast(WaveCleared{3});             // server → everyone
ctx.network().send(Ready{});                         // client → server
```

**4. Receive** with a callback, kept alive by its `Subscription`:

```cpp
// server
readySub_ = ctx.network().on<Ready>([this](engine::Context& ctx, engine::PlayerId from, const Ready&) {
    ready_.insert(from);
    if (ready_.size() == ctx.network().players().size()) ctx.network().broadcast(StartGame{});
});

// client: the sender is always the server, so the short form has no `from`
waveSub_ = ctx.network().on<WaveCleared>([](engine::Context& ctx, const WaveCleared& m) {
    showBanner(ctx, m.wave);
});
```

Rules for callbacks:

- They run on the game thread at the start of the frame, before `FixedUpdate`. No mutex needed.
- Create or destroy entities through `ctx.commands()`, as everywhere.
- An exception is caught by the engine, logged with the message name, and the message is ignored.
- `Entity` fields arrive as the matching entity on this machine, or `Entity::null()` if it is not
  replicated or not spawned here yet: always check `isNull()`.
- A malformed or oversized message never reaches your callback.

**Limits.** Text fields are limited to 256 characters and lists to 64 elements unless you set
`.maxLength(n)` on the field. Keep them tight: they are your first line of defence against bad input.

**Versions.** Client and server compare a fingerprint of all messages and replicated components at
connection. If someone changed a type on one side only, the client is refused with
`LeftServer{FormatMismatch}` and a log line saying to rebuild both.

## Several games on one server

With rooms, a game's systems see only the players of their room through `ctx.network()`, and
replication sends each player only the entities of their room. Lobby, parties and rooms: page 28.

## Diagnostics

`net` in the console prints RTT, loss and bandwidth per player; the metrics overlay shows the
lagometer. `net_snapshot_rate` and `net_timeout` are console variables.
