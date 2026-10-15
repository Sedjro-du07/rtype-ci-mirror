# ENG-27 · EventBus: queued, immediate, thread inbox; engine events

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **06 Events** · **Wave 4** · status and owner live on the board.

## Goal

Typed queued events with RAII subscriptions, plus the engine's own events.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/events/EventBus.hpp`
- `include/engine/events/EngineEvents.hpp`
- `src/events/EventBus.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/events/EventBus.hpp`

```cpp
class EventBus {
public:
    template <class E> [[nodiscard]] Subscription subscribe(std::move_only_function<void(const E&)> fn);

    template <class E> void publish(E event);   // queued; delivered at the next dispatch()
    template <class E> void emit(const E& event); // immediate; only for engine internals and tests
    template <class E> void publishFromAnyThread(E event); // thread-safe inbox (network thread),
                                                         // merged into the queue at dispatch()
    void dispatch();                             // engine only; delivers queued events in publish order;
                                                 // events published during dispatch wait for the next one
    void clearGameSubscriptions();               // before unloading a game module
};
```

### `include/engine/events/EngineEvents.hpp`

```cpp
struct QuitRequested      {};                                           ENGINE_EVENT(QuitRequested)
struct WindowResized      { uint32_t width, height; };                    ENGINE_EVENT(WindowResized)
struct WindowFocusChanged { bool focused; };                              ENGINE_EVENT(WindowFocusChanged)
struct Collision          { Entity a, b; };                               ENGINE_EVENT(Collision)
struct SceneLoaded        { std::string name; };                          ENGINE_EVENT(SceneLoaded)
struct SceneUnloading     { std::string name; };                          ENGINE_EVENT(SceneUnloading)
struct PlayerJoined       { PlayerId player; };                           ENGINE_EVENT(PlayerJoined)
struct PlayerLeft         { PlayerId player; DisconnectReason reason; };  ENGINE_EVENT(PlayerLeft)
struct JoinedServer       { PlayerId me; };                               ENGINE_EVENT(JoinedServer)
struct LeftServer         { DisconnectReason reason; };                   ENGINE_EVENT(LeftServer)
struct Caption            { std::string text; float seconds; };           ENGINE_EVENT(Caption)
struct ConsoleCommand     { std::string line; };                          ENGINE_EVENT(ConsoleCommand)
```

## Depends on

- [ENG-06](ENG-06.md) — Signal and RAII Subscription
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [ENG-43](ENG-43.md) — Scene and SceneManager: switch, persistent entities
- [ENG-46](ENG-46.md) — Console and built-in commands
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-51](ENG-51.md) — Collision system: uniform grid, AABB, layers, Collision events
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session
- [SRV-08](SRV-08.md) — Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events
- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Read first

- [ADR 0029](../adr/0029-queued-event-bus.md)
- [ADR 0044](../adr/0044-platforms-and-portability.md)
- [reference 06](../engine/reference/06-events.md)

## To implement

- [ ] `subscribe`, `publish` (queued), `emit` (immediate), `publishFromAnyThread` (mutex inbox)
- [ ] `dispatch`: deliver in publish order; events published during dispatch wait
- [ ] Engine events of reference 06 (player events included), with `AppSuspended` / `AppResumed` reserved for mobile (ADR 0044)

## Done when

- Tests: order, publish during dispatch, unsubscribe during dispatch, thread inbox

## Watch out

- Events never leave the process

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
