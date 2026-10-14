# SRV-08 · Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 5** · status and owner live on the board.

## Goal

The vocabulary of several games on one server: ids, records and events shared by the engine and the game.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/RoomTypes.hpp`
- `include/engine/net/RoomEvents.hpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/net/RoomTypes.hpp`

```cpp
using PartyId = uint32_t;                          // 0 = none
using RoomId  = uint32_t;                          // 0 = none (in the lobby)

enum class RoomState : uint8_t { Starting, Running, Finished, Failed, Closing };

struct RoomSettings {                              // given by the game to createRoom
    std::string name;                              // shown in room lists
    std::string level;                             // first scene of the room, relative to the game root
    uint32_t    maxPlayers = 4;
    bool        listed = true;                     // false: private, joined only by the game moving a party in
};

struct Party {
    PartyId                   id;
    PlayerId                  leader;
    std::vector<PlayerId>     members;             // leader included
    RoomId                    room = 0;            // 0 = in the lobby
    size_t size() const;
};

struct RoomInfo {
    RoomId                    id;
    RoomSettings              settings;
    RoomState                 state;
    std::vector<PartyId>      parties;
    std::vector<PlayerId>     players;
};
```

### `include/engine/net/RoomEvents.hpp`

```cpp
// Lobby (server, published on the lobby EventBus, received with a LobbyContext)
struct PartyChanged         { PartyId party; };                               ENGINE_EVENT(PartyChanged)
struct InviteRequested      { PlayerId from; PlayerId to; };                  ENGINE_EVENT(InviteRequested)
struct InviteAnswered       { PlayerId player; PartyId party; bool accepted; }; ENGINE_EVENT(InviteAnswered)
struct LeavePartyRequested  { PlayerId player; };                             ENGINE_EVENT(LeavePartyRequested)
struct RoomListRequested    { PlayerId player; };                             ENGINE_EVENT(RoomListRequested)
struct RoomStarted          { RoomId room; };                                 ENGINE_EVENT(RoomStarted)
struct RoomFinished         { RoomId room; std::vector<std::byte> result;     // decode with resultAs<T>()
                              template <class T> std::optional<T> resultAs() const; };  ENGINE_EVENT(RoomFinished)
struct RoomFailed           { RoomId room; std::string error; };              ENGINE_EVENT(RoomFailed)
struct RoomClosed           { RoomId room; };                                 ENGINE_EVENT(RoomClosed)

// Room (published on the room's own EventBus, received by systems with a Context)
struct PlayerEnteredRoom    { PlayerId player; PartyId party; };              ENGINE_EVENT(PlayerEnteredRoom)
struct PlayerLeftRoom       { PlayerId player; DisconnectReason reason; };    ENGINE_EVENT(PlayerLeftRoom)

// Client
struct PartyUpdated         { Party party; };                                 ENGINE_EVENT(PartyUpdated)
struct InviteReceived       { PlayerId from; PartyId party; };                ENGINE_EVENT(InviteReceived)
struct RoomListReceived     { std::vector<RoomInfo> rooms; };                 ENGINE_EVENT(RoomListReceived)
struct EnteredRoom          { RoomId room; RoomSettings settings; };          ENGINE_EVENT(EnteredRoom)
struct ReturnedToLobby      { RoomId room; std::string reason; };             ENGINE_EVENT(ReturnedToLobby)
```

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-36](ENG-36.md) — INetwork and network types

## Unlocks

- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room
- [SRV-10](SRV-10.md) — PartyTable and RoomDirectory: parties, leaders, room records, maxRooms

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `PartyId`, `RoomId`, `RoomState`, `RoomSettings`, `Party`, `RoomInfo`
- [ ] Lobby events (`PartyChanged`, `InviteRequested`, `InviteAnswered`, `LeavePartyRequested`, `RoomListRequested`, `RoomStarted`, `RoomFinished` with `resultAs<T>()`, `RoomFailed`, `RoomClosed`)
- [ ] Room events (`PlayerEnteredRoom`, `PlayerLeftRoom`) and client events (`PartyUpdated`, `InviteReceived`, `RoomListReceived`, `EnteredRoom`, `ReturnedToLobby`)
- [ ] Register them in `registerEngineTypes`; `RoomInfo`, `RoomSettings` and `Party` as `ENGINE_STRUCT` so they can travel

## Done when

- The types compile in a game test module; every event registers without `DuplicateName`

## Watch out

- Ids are 0 for none, like `PlayerId`
- No engine-internal type (ConnectionId, RoomRuntime) in these headers

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
