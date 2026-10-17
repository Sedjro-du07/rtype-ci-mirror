# SRV-09 · Contexts: ServiceContext, LobbyContext, one Context per room

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 9** · status and owner live on the board.

## Goal

Split the door into the engine in three so that world access in lobby code does not compile (ADR 0039 amends 0022).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/ServiceContext.hpp` (shared with [ENG-09](ENG-09.md))
- `src/core/ServiceContext.cpp` (shared with [ENG-09](ENG-09.md))
- `include/engine/server/LobbyContext.hpp`
- `src/server/LobbyContext.cpp`
- `include/engine/Context.hpp` (shared with [ENG-09](ENG-09.md))

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/Context.hpp`

```cpp
class Context : public ServiceContext {     // log(), cvars(), assets(), metrics(), role() come from ServiceContext
public:
    Registry&          world();             // this room's world (server) / the game's world (client)
    Commands&          commands();          // deferred spawn / destroy / add / remove
    SceneManager&      scenes();            // switchTo(), markPersistent(), active scene name
    EventBus&          events();            // this room's events only
    const Time&        time() const;        // fixedDelta, tick, alpha, total of this room
    RoomNetwork&       network();           // server: the players of this room; client: the session
    RoomHandle&        room();              // server only: id, players, reportFinished (reference 14)
    ClientLobby&       lobby();             // client only: party, invitations, room list (reference 14)
    const ActionState& input() const;       // client: local player's actions this tick
    InputMap&          inputMap();          // client: bindings and options (options menu, rebinding)
    RenderQueue&       render();            // client: describe what to draw this frame
    IAudio&            audio();             // client (NullAudio on the server)
    Console&           console();           // client (the server console belongs to the lobby)
    void               requestQuit();       // client only: a room cannot stop the server

private:
    friend class Engine;  friend class RoomRuntime;
    Context(/* engine-owned references of one room or of the client */);
};
```

### `include/engine/ServiceContext.hpp`

```cpp
class ServiceContext {                 // everywhere: shared, thread-safe services
public:
    Logger&            log();          // lines tagged with the room in a room
    const CVars&       cvars() const;  // read-only (written from the lobby or the console)
    AssetManager&      assets();       // locked on the server
    Metrics&           metrics();      // series tagged with the room in a room
    Role               role() const;
};

// class Context : public ServiceContext  — declared in reference 01 (Context.hpp)

class LobbyContext : public ServiceContext { // server lobby code, main thread
public:
    Lobby&             lobby();
    EventBus&          events();            // the lobby's events
    NetworkSession&    network();           // every connected player
    Console&           console();
    CVars&             cvars();             // read and write
    void               requestQuit();       // stops the server
};
```

## Depends on

- [SRV-08](SRV-08.md) — Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload
- [ENG-44](ENG-44.md) — Logger and sinks (console, file, developer console)
- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive
- [ENG-47](ENG-47.md) — Metrics and engine.debug_overlay (ImGui, ImPlot)

## Unlocks

- [ENG-09](ENG-09.md) — Context facade
- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0022](../adr/0022-context-facade.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `ServiceContext`: log (tagged with the room), read-only CVars, assets, metrics, role
- [ ] `Context : ServiceContext`: world, commands, scenes, events, time, network (`RoomNetwork`), `room()` on the server, `lobby()` on the client, client-only input / render / audio / console / requestQuit
- [ ] `LobbyContext : ServiceContext`: lobby, events, network (whole session), console, writable CVars, requestQuit
- [ ] Constructors private: built by `Engine` (lobby, client) and `RoomRuntime` (one per room)

## Done when

- A test lobby callback using `ctx.world()` fails to compile (compile-fail test)
- A system receives the Context of its room: two rooms see two different worlds

## Watch out

- Contexts never own anything
- `room()` on the client and `lobby()` on the server assert with a clear message

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
