# SRV-11 · RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 12** · status and owner live on the board.

## Goal

One isolated game: its own world, commands, events, time, network ids and system instances.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/server/RoomRuntime.hpp`
- `src/server/RoomRuntime.cpp`
- `src/server/RoomMailbox.hpp`
- `src/server/RoomMailbox.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/server/RoomRuntime.hpp`

```cpp
struct EngineServices {           // the shared, thread-safe services a room may use (ADR 0039)
    const TypeRegistry& types;  Logger& log;  const CVars& cvars;  AssetManager& assets;  Metrics& metrics;
    const EngineConfig& config;
};

class RoomRuntime {
public:
    RoomRuntime(RoomId id, const RoomSettings& s, const EngineServices& services, const Scheduler& prototypes);
    Result<void> start();                             // loads settings.level, instantiates systems
    void tick();                                      // FixedUpdate / LateUpdate of this room; one thread at a time
    RoomMailbox& inbox();  RoomMailbox& outbox();
    bool failed() const;  RoomId id() const;
};
```

### `src/server/RoomMailbox.hpp`

```cpp
struct RoomMessage { enum class Kind : uint8_t { PlayerEntered, PlayerLeft, NetMessage, Finished, Failed, Close };
                     Kind kind; PlayerId player = 0; uint16_t messageId = 0; std::vector<std::byte> payload; };

class RoomMailbox {                                   // multi-producer, single-consumer
public:
    void push(RoomMessage m);
    std::vector<RoomMessage> drain();
};
```

## Depends on

- [ENG-09](ENG-09.md) — Context facade
- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-20](ENG-20.md) — Commands buffer (spawn, prefab spawn, destroy, add, set, remove)
- [ENG-26](ENG-26.md) — Scheduler: stages, names, after/before, roles, stats, failure policy
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room

## Unlocks

- [SRV-12](SRV-12.md) — WorkerPool, IRoomHost and ThreadPoolRoomHost: rooms in parallel on a thread pool

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `RoomRuntime`: owns a SceneManager (loads `settings.level`), Commands, EventBus, Time, NetworkIdMap, its Context
- [ ] Instantiate the game systems for this room (`Scheduler::instantiate`, ADR 0039 option B)
- [ ] `tick()`: drain the inbox, FixedUpdate, apply commands, dispatch events, LateUpdate; catch exceptions → `failed()`
- [ ] `RoomMailbox`: thread-safe queue (multi-producer, single consumer) for lobby → room and room → lobby

## Done when

- Two RoomRuntimes ticked one after the other never see each other's entities or events
- A throwing system marks the room failed and posts `Failed` to the outbox

## Watch out

- A room is never ticked by two threads at once
- No pointer to the lobby inside a room: messages only (ADR 0038 rule 3)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
