# ENG-09 · Context facade

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 11** · status and owner live on the board.

## Goal

The only door from game code into the engine (ADR 0022).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/Context.hpp` (shared with [SRV-09](SRV-09.md))
- `src/core/Context.cpp`
- `include/engine/ServiceContext.hpp` (shared with [SRV-09](SRV-09.md))
- `src/core/ServiceContext.cpp` (shared with [SRV-09](SRV-09.md))

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

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [SRV-09](SRV-09.md) — Contexts: ServiceContext, LobbyContext, one Context per room

## Unlocks

- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0022](../adr/0022-context-facade.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] Accessors of reference 01 forwarding to the engine (client) or to a room (server)
- [ ] Private constructor, `friend class Engine` and `RoomRuntime`; base class `ServiceContext` comes from SRV-09

## Done when

- A game test module compiles using only `Context.hpp` and block headers, never `Engine.hpp`

## Watch out

- Context holds no state of its own; never cache it in game code

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
