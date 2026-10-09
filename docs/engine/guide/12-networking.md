# 12 · Networking

The engine handles transport, connections, timeouts, replication and player input. The wire format
is specified in the protocol RFC (Network part); a game does not deal with bytes for replicated
state.

## Server and client

```yaml
# server.yaml
role: server
game: rtype
net: { port: 4242, maxClients: 4, snapshotRate: 20 }
```

```cpp
// client, e.g. from the menu
ctx.network().connect("192.168.1.10", 4242);
```

Listen to `ConnectedToServer` / `DisconnectedFromServer` on the client, `ClientConnected` /
`ClientDisconnected` on the server (a crashed client times out and other clients are told).

## Replicated entities

1. Mark components to send: `.flags(engine::Replicated)` in `registerTypes`.
   Engine `Position`, `Velocity` and `Sprite` already are.
2. Spawn networked entities from a prefab on the server: `ctx.commands().spawn("prefabs/bydos.yaml", pos)`.
   The prefab gives `NetPrefab`; the engine assigns a `NetworkId` and clients spawn the same prefab.
3. Run gameplay systems with `.roles(engine::ServerRole)`. Clients receive the results.

Client positions are interpolated between snapshots; no game code is needed.

## Player input

The client sends its actions each tick. On the server, give each player's ship a `NetOwner`:

```cpp
void onClientConnected(engine::Context& ctx, engine::ConnectionId id) {
    auto ship = ctx.commands().spawn("prefabs/player_ship.yaml", {40, 90});
    ctx.commands().add(ship, engine::NetOwner{id});
}

void playerControlSystem(engine::Context& ctx) {           // server
    for (auto [e, in, vel] : ctx.world().view<engine::RemoteInput, engine::Velocity>()) {
        vel.dy = in.state.held(MoveUp) ? -speed : in.state.held(MoveDown) ? speed : 0.f;
    }
}
```

## Game messages

For events that are not component state (chat, "wave cleared" banners):

```cpp
// registerSystems or onStart, both sides
ctx.network().onMessage(MsgWaveCleared, [&](engine::ConnectionId, engine::ByteReader& r) {
    auto wave = r.u16(); if (!wave) return;          // malformed → ignored, never a crash
    showBanner(*wave);
});

// server
std::vector<std::byte> buf; engine::ByteWriter w(buf, 64); w.write(uint16_t{3});
ctx.network().broadcast(engine::Channel::Reliable, MsgWaveCleared, buf);
```

Message type ranges for games are reserved in the protocol RFC.

## Diagnostics

`net` in the console prints RTT, loss and bandwidth per connection; the metrics overlay shows the
lagometer. `net_snapshot_rate` and `net_timeout` are console variables.
