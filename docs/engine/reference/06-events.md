# 6 · Events

Status: **Locked**. ADRs 0015, 0029.
Folder: `engine/include/engine/events/`, `engine/src/events/`.

## Role

Let systems react to each other without knowing each other (Mediator), with delivery at defined
points of the frame (Event Queue), and RAII unsubscription.

---

## `class Subscription` — `core/Signal.hpp`

```cpp
class Subscription {                 // move-only; unsubscribes when destroyed
public:
    Subscription() = default;
    Subscription(Subscription&&) noexcept; Subscription& operator=(Subscription&&) noexcept;
    ~Subscription();                 // calls reset()
    void reset() noexcept;           // unsubscribe now
    bool active() const noexcept;
};
```

Keep subscriptions in an object whose lifetime matches the listener (a system functor, the game
module). Destroying the owner can never leave a dangling callback.

## `class Signal<void(Args...)>` — `core/Signal.hpp`

Synchronous observer list, used by `ComponentHooks` (block 3) and internally.

```cpp
template <class... Args>
class Signal<void(Args...)> {
public:
    Signal() = default;              // move-only; allocates nothing until the first connect()
    [[nodiscard]] Subscription connect(std::move_only_function<void(Args...)> fn);
    void emit(Args... args);         // calls every listener now, in connection order
    size_t size() const;
};
```

## `class EventBus` — `events/EventBus.hpp`

Events are plain structs declared with `ENGINE_EVENT(Type)` and registered with
`TypeRegistry::add<E>()` (block 4, ADR 0036).

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
    void beginGameSubscriptions();               // engine only: later subscriptions belong to the game
    void clearGameSubscriptions();               // before unloading a game module: drops the game's listeners
                                                 // and every pending event (their code may be the game's)
};
```

Dispatch points: after each `FixedUpdate` step (after `Commands::apply`) and after `LateUpdate`.

## Engine events — `events/EngineEvents.hpp`

| Event | Fields | Published by | When |
|---|---|---|---|
| `QuitRequested` | — | `Context::requestQuit`, window close | quit asked |
| `WindowResized` | `uint32_t width, height` | platform plugin | window size changed |
| `WindowFocusChanged` | `bool focused` | platform plugin | focus lost or gained |
| `AppSuspended` | — | platform plugin | reserved for mobile (ADR 0044): the app goes to the background; desktop plugins never send it |
| `AppResumed` | — | platform plugin | reserved for mobile: back to the foreground |
| `Collision` | `Entity a, b` | `engine.collision` | two colliders overlap this tick (once per pair) |
| `SceneLoaded` | `std::string name` | `SceneManager` | after a scene finished loading |
| `SceneUnloading` | `std::string name` | `SceneManager` | before a scene is cleared |
| *Network events below: declared in `EngineEvents.hpp` with the other engine events (ENG-36), published by `NetworkSession`* | | | |
| `PlayerJoined` | `PlayerId player` | `NetworkSession` | server: a player completed the handshake (ADR 0037) |
| `PlayerLeft` | `PlayerId player; DisconnectReason reason; std::string message` | `NetworkSession` | server: quit, kick, error, or end of the grace period |
| `PlayerDisconnected` | `PlayerId player; DisconnectReason reason` | `NetworkSession` | server: connection lost, grace period starts (ADR 0043); also in the player's room |
| `PlayerReconnected` | `PlayerId player` | `NetworkSession` | server: came back with its token; also in the player's room |
| `JoinedServer` | `PlayerId me` | `NetworkSession` | client: accepted by the server |
| `LeftServer` | `DisconnectReason reason; std::string message` | `NetworkSession` | client: left, kicked, refused, server shut down, or gave up reconnecting |
| `ConnectionInterrupted` | `DisconnectReason reason` | `NetworkSession` | client: connection lost, reconnecting by itself |
| `Reconnected` | — | `NetworkSession` | client: back in the same session |
| `Caption` | `std::string text; float seconds` | `IAudio::play` with a caption | accessibility: visual cue for a sound |
| `ConsoleCommand` | `std::string line` | `Console` | a command was executed |

Lobby, party and room events (`PartyChanged`, `InviteRequested`, `RoomFinished`, `PlayerEnteredRoom`,
`EnteredRoom`…) are listed in reference 14. On the server, every room has its own `EventBus`; the lobby
has another one. An event published in a room is never seen by another room or by the lobby.

`DisconnectReason`: `Quit`, `Timeout`, `Kicked`, `ProtocolError`, `ServerShutdown`, `ServerFull`, `FormatMismatch`,
`ProtocolMismatch`, `WrongGame`, `InvalidName`, `RefusedByGame`, `ConnectionLost` (meanings: RFC §6.8).

Events never leave the process. To tell other machines, use a network message (`ENGINE_MESSAGE`, block 10).
