# SRV-14 · Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 15** · status and owner live on the board.

## Goal

Make the network follow players between the lobby and the rooms.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/PlayerRouting.hpp`
- `src/net/PlayerRouting.cpp`
- `include/engine/room/RoomNetwork.hpp`
- `src/room/RoomNetwork.cpp`
- `include/engine/room/ClientLobby.hpp`
- `src/room/ClientLobby.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/room/ClientLobby.hpp`

```cpp
class ClientLobby {
public:
    const Party& myParty() const;
    void invite(PlayerId player);                     // server: InviteRequested
    void answerInvite(PartyId p, bool accept);        // server: InviteAnswered
    void leaveParty();                                // server: LeavePartyRequested
    void requestRoomList();                           // server: RoomListRequested → client: RoomListReceived
    RoomId currentRoom() const;                       // 0 = in the lobby
};
```

### `include/engine/room/RoomNetwork.hpp`

```cpp
class RoomNetwork {
public:
    std::span<const PlayerId> players() const;                 // the room's players
    template <class T> void send(PlayerId to, const T& msg);   // to a player of the room
    template <class T> void send(const T& msg);                // client → server
    template <class T> void broadcast(const T& msg, std::optional<PlayerId> except = {});   // the room's players
    template <class T> [[nodiscard]] Subscription on(std::move_only_function<void(Context&, PlayerId, const T&)> fn);
    double ping(PlayerId p) const;
};
```

### `src/net/PlayerRouting.hpp`

```cpp
class PlayerRouting {
public:
    void   setRoom(PlayerId p, RoomId r);             // 0 = lobby
    RoomId roomOf(PlayerId p) const;
    std::span<const PlayerId> playersIn(RoomId r) const;
};
```

## Depends on

- [SRV-13](SRV-13.md) — Lobby and RoomHandle: the server game API, lobby loop in Engine
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems

## Unlocks

- [SRV-15](SRV-15.md) — Tests: parallel rooms, isolation, crash in a room, party and room flows

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `PlayerRouting`: where each player is (lobby or room)
- [ ] Route game messages to the room's `on<T>` or the lobby's `on<T>`
- [ ] `RoomNetwork`: players, send, broadcast, on<T>, ping limited to the room
- [ ] Replication per room: only the room's entities; reset a player's replication state on enter / leave
- [ ] `ClientLobby`: myParty, invite, answerInvite, leaveParty, requestRoomList, currentRoom; client events

## Done when

- A player in room 1 never receives an entity or a message of room 2; returning to the lobby stops every snapshot

## Watch out

- Messages of a player who just left a room must not reach that room any more

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
