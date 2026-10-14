# SRV-10 · PartyTable and RoomDirectory: parties, leaders, room records, maxRooms

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 6** · status and owner live on the board.

## Goal

Keep track of parties and rooms on the lobby thread, with the fixed rules of the engine.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/room/PartyTable.hpp`
- `src/room/PartyTable.cpp`
- `src/room/RoomDirectory.hpp`
- `src/room/RoomDirectory.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/room/PartyTable.hpp`

```cpp
class PartyTable {
public:
    PartyId createSolo(PlayerId p);                   // on PlayerJoined
    void    move(PlayerId p, PartyId to);             // dissolves the old party if empty
    void    remove(PlayerId p);                       // on PlayerLeft; promotes a new leader
    void    setLeader(PartyId party, PlayerId p);
    const Party& get(PartyId p) const;  PartyId of(PlayerId p) const;
    std::span<const PartyId> all() const;
};
```

### `src/room/RoomDirectory.hpp`

```cpp
class RoomDirectory {
public:
    explicit RoomDirectory(uint32_t maxRooms);
    std::optional<RoomId> add(const RoomSettings& s);
    void remove(RoomId r);
    void setState(RoomId r, RoomState s);
    void enter(RoomId r, const Party& party);  void leave(RoomId r, PartyId party);
    const RoomInfo& get(RoomId r) const;  RoomId roomOf(PlayerId p) const;
    std::span<const RoomId> all() const;
};
```

## Depends on

- [SRV-08](SRV-08.md) — Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events

## Unlocks

- [SRV-13](SRV-13.md) — Lobby and RoomHandle: the server game API, lobby loop in Engine

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `PartyTable`: a party of 1 on `PlayerJoined`, move, remove (new leader promoted), setLeader, lookups
- [ ] `RoomDirectory`: add (refused over `maxRooms`), remove, state, enter / leave with a party, `roomOf(player)`

## Done when

- Tests: join → party of 1; invite + accept → one party of 2 and the old party dissolved; leader leaves → a new leader; maxRooms respected

## Watch out

- A player is always in exactly one party
- Lobby thread only: no locks here

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
