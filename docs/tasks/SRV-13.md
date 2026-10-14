# SRV-13 · Lobby and RoomHandle: the server game API and the server loop

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 14** · status and owner live on the board.

## Goal

The server-side game API for parties and rooms, and the server loop (`src/server/`).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/server/Lobby.hpp`
- `src/server/Lobby.cpp`
- `include/engine/server/RoomHandle.hpp`
- `src/server/RoomHandle.cpp`
- `src/server/ServerLoop.hpp`
- `src/server/ServerLoop.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/server/Lobby.hpp`

```cpp
class Lobby {
public:
    // Parties
    const Party& party(PartyId p) const;
    const Party& partyOf(PlayerId p) const;           // always exists while the player is connected
    std::span<const PartyId> parties() const;
    void sendInvite(PartyId p, PlayerId to);          // the player receives InviteReceived
    void addToParty(PartyId p, PlayerId player);      // leaves their current party (dissolved if empty)
    void removeFromParty(PlayerId player);            // back to a new party of 1
    void setLeader(PartyId p, PlayerId player);
    template <class T> void sendToParty(PartyId p, const T& msg);   // T: ENGINE_MESSAGE

    // Rooms
    RoomId createRoom(const RoomSettings& settings);  // fails (0 + log) when maxRooms is reached
    void   moveParty(PartyId p, RoomId room);         // whole party enters; PlayerEnteredRoom in the room
    void   moveToLobby(PartyId p);                    // whole party leaves the room; ReturnedToLobby on clients
    void   closeRoom(RoomId room);                    // parties still inside go back to the lobby
    void   sendRoomList(PlayerId to, std::span<const RoomId> rooms);   // answer to RoomListRequested
    const RoomInfo& room(RoomId r) const;
    std::span<const RoomId> rooms() const;
};
```

### `include/engine/server/RoomHandle.hpp`

```cpp
class RoomHandle {
public:
    RoomId id() const;
    const RoomSettings& settings() const;
    std::span<const PlayerId> players() const;
    std::span<const PartyId>  parties() const;
    void keep(Subscription s);                                 // lives as long as the room (subscriptions made in onRoomStart)
    template <class T> void reportFinished(const T& result);   // T: ENGINE_MESSAGE or ENGINE_STRUCT;
                                                               // the lobby receives RoomFinished; the room keeps
                                                               // running until the game closes it
};
```

### `src/server/ServerLoop.hpp`

```cpp
// The server's main-thread loop, called by Engine::run() when role == Server (ADR 0040)
class ServerLoop {
public:
    ServerLoop(EngineServices services, NetworkSession& network, Lobby& lobby, IRoomHost& rooms, IGameModule& game);
    void run(const std::atomic_bool& running);   // network poll, lobby events (LobbyContext), rooms.collect(), console
private:
    void onRoomMessage(RoomId room, RoomMessage&& m);   // Finished / Failed → RoomFinished / RoomFailed events
};
```

## Depends on

- [SRV-10](SRV-10.md) — PartyTable and RoomDirectory: parties, leaders, room records, maxRooms
- [SRV-12](SRV-12.md) — WorkerPool, IRoomHost and ThreadPoolRoomHost: rooms in parallel on a thread pool
- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>

## Unlocks

- [SRV-14](SRV-14.md) — Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset
- [SRV-15](SRV-15.md) — Tests: parallel rooms, isolation, crash in a room, party and room flows

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `Lobby`: parties (party, partyOf, sendInvite, addToParty, removeFromParty, setLeader, sendToParty) and rooms (createRoom, moveParty, moveToLobby, closeRoom, sendRoomList, room, rooms)
- [ ] Publish the lobby events; turn client requests into `InviteRequested` / `InviteAnswered` / `LeavePartyRequested` / `RoomListRequested`
- [ ] `RoomHandle`: id, settings, players, parties, `keep(Subscription)`, `reportFinished<T>`
- [ ] Call `IGameModule::onLobbyStart` once and `onRoomStart` for every room after its level is loaded
- [ ] `ServerLoop` (private, `src/server/`): the main-thread loop of the server, started by `Engine::run()` when role is Server: network poll, lobby events, room host collect

## Done when

- Headless server: two parties create two rooms, play in parallel, one reports finished, the game closes it, its party is back in the lobby

## Watch out

- The engine never applies a game rule (who may start, solo allowed…): it only publishes events and executes calls

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
