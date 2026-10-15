# 28 · Lobby, parties and rooms

One server hosts several games at once. Players who are not in a game are in the **lobby**; players
who play together come in a **party**; each game is a **room** (ADR 0038, 0039). The engine provides
the mechanics; your game decides the rules.

```
Server (lobby: every connected player who is not in a game)
 ├── Party 1: Ola (leader), Michel     ── stay together between games
 ├── Party 2: Sedjro                   ── a player alone is a party of 1
 ├── Room 7: Party 1                   ── one game: its own world, systems, events, tick
 └── Room 8: Party 2
```

## Module hooks

| Hook | Where | When |
|---|---|---|
| `onStart(Context&)` | client | once, before the loop |
| `onLobbyStart(LobbyContext&)` | server | once: lobby subscriptions, lobby `on<T>` |
| `onRoomStart(Context&)` | server | once per room, after its level is loaded: room subscriptions (`ctx.room().keep`) |
| `onShutdown(ServiceContext&)` | both | once, before the module is unloaded |

## Where your code runs

| Code | Runs | Receives |
|---|---|---|
| Game systems (movement, AI, waves…) | in **every room**, on a worker thread, one room at a time | `Context&`: the room's world, events, time, network, `room()` |
| Lobby code (invitations, starting a game, end of a game) | once, on the server's main thread | `LobbyContext&`: `lobby()`, lobby events, the whole network, console |
| Client code | on the client | `Context&` with `lobby()` (requests) and the client events |

Your systems do not change: `ctx.world()` is simply the world of the room being ticked. Lobby code has
no world: `ctx.world()` in a `LobbyContext` does not compile.

## Parties

Every player is always in a party; on connection they get a party of 1. Requests from clients reach
your lobby code as events; you decide and call the `Lobby`:

```cpp
// server, lobby code: the module's onLobbyStart(engine::LobbyContext& lobbyCtx)
inviteSub_ = lobbyCtx.events().subscribe<engine::InviteRequested>([&lobbyCtx](const engine::InviteRequested& e) {
    auto& party = lobbyCtx.lobby().partyOf(e.from);
    if (party.leader != e.from || party.size() >= 4) return;     // your rules
    lobbyCtx.lobby().sendInvite(party.id, e.to);
});
answerSub_ = lobbyCtx.events().subscribe<engine::InviteAnswered>([&lobbyCtx](const engine::InviteAnswered& e) {
    if (e.accepted) lobbyCtx.lobby().addToParty(e.party, e.player);
});
```

```cpp
// client
ctx.lobby().invite(friendId);
inviteSub_ = ctx.events().subscribe<engine::InviteReceived>([&ctx](const engine::InviteReceived& e) {
    showInvitePopup(ctx, e.from, e.party);           // later: ctx.lobby().answerInvite(e.party, true)
});
```

## Starting a game

Starting a game is a rule of your game, so it is one of your messages:

```cpp
struct PlayRequest {};  ENGINE_MESSAGE(PlayRequest)          // client → server, reliable by default

// server, lobby code
playSub_ = lobbyCtx.network().on<PlayRequest>([](engine::LobbyContext& ctx, engine::PlayerId from, const PlayRequest&) {
    auto& party = ctx.lobby().partyOf(from);
    if (party.leader != from) {                               // only the leader starts
        ctx.lobby().refuse(from, engine::LobbyRequest::Game, "only the leader can start");
        return;
    }
    if (party.size() == 1 && !kSoloAllowed) return;           // solo allowed? your rule
    auto room = ctx.lobby().createRoom({ .name = "Stage 1", .level = "scenes/level1.yaml", .maxPlayers = 4 });
    if (room) ctx.lobby().moveParty(party.id, room);
});
```

Clients receive `EnteredRoom{room, settings}`; **your client code changes scene** (for example to the
level's background). In the room, subscribe in `onRoomStart`, called once per room:

```cpp
void RType::onRoomStart(engine::Context& ctx) {
    ctx.room().keep(ctx.events().subscribe<engine::PlayerEnteredRoom>([&ctx](const engine::PlayerEnteredRoom& e) {
        auto ship = ctx.commands().spawn("prefabs/player_ship.yaml", {40, 90});
        ctx.commands().add(ship, engine::NetOwner{e.player});
    }));
}
```

`keep` ties the subscription to the room: it disappears with the room. Lobby subscriptions live in your
module's members (there is one lobby).

## The state of one game

Store it in the room's world, never in members of your module class or in globals (another room or a
dedicated room process would share or lose it):

```cpp
struct WaveState { int32_t wave = 1; float timer = 0; };  ENGINE_COMPONENT(WaveState)

void waveSystem(engine::Context& ctx) {
    auto& w = ctx.world().singleton<WaveState>();            // one per room
    w.timer -= ctx.time().fixedDelta;
}
```

A system written as a struct with members is copied once per room, so its members are per room too.

## Ending a game

The room reports; your lobby code decides:

```cpp
struct GameResult { bool victory; int32_t score; };  ENGINE_STRUCT(GameResult)

// in a room system, when the boss dies
ctx.room().reportFinished(GameResult{ true, 12400 });

// lobby code (onLobbyStart)
endSub_ = lobbyCtx.events().subscribe<engine::RoomFinished>([&lobbyCtx](const engine::RoomFinished& e) {
    auto result = e.resultAs<GameResult>();
    lobbyCtx.lobby().closeRoom(e.room, "victory!");           // parties go back to the lobby
});
```

Send results as a `reliable()` message before `closeRoom`: they arrive before `ReturnedToLobby{room,
reason}` (same ordered stream, RFC §8.4). Clients then switch back to their lobby scene. If a system
throws in a room, that room stops, its clients receive `RoomInterrupted`, the lobby receives
`RoomFailed{room, error}`, the other rooms keep running, and your lobby code decides (usually
`closeRoom`).

## Who is online, refused requests

On the client, `ctx.lobby().onlinePlayers()` and `name(p)` list connected players (event
`OnlinePlayersChanged`; turn off with `net.shareOnlinePlayers: false`). When your lobby code says no,
call `lobby().refuse(player, request, reason)`: the client receives `RequestRefused{request, reason}`.
Staying silent is also allowed.

## Network inside a room

`ctx.network()` in a room sees only the room's players: `players()`, `send`, `broadcast`, `on<T>`,
`ping`. A message from a player goes to their room while they play, to the lobby otherwise.
Replication sends a player only the entities of their room.

## Configuration

```yaml
# server.yaml
rooms:
  host: shared        # all rooms in this process (Part 1)
  maxRooms: 8
  workerThreads: 0    # 0 = number of cores
```

`host: dedicated` (one process per room) is planned for Part 2 and needs no change in your game if you
follow the rules above: game state in the room, rooms created through the lobby, lobby and rooms
talking only through messages.
