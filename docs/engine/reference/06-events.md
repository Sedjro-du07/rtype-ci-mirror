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
    [[nodiscard]] Subscription connect(std::move_only_function<void(Args...)> fn);
    void emit(Args... args);         // calls every listener now, in connection order
    size_t size() const;
};
```

## `class EventBus` — `events/EventBus.hpp`

Events are plain structs declared with `ENGINE_EVENT(Type)` and registered with
`TypeRegistry::addEvent<E>()` (block 4).

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

Dispatch points: after each `FixedUpdate` step (after `Commands::apply`) and after `LateUpdate`.

## Engine events — `events/EngineEvents.hpp`

| Event | Fields | Published by | When |
|---|---|---|---|
| `QuitRequested` | — | `Context::requestQuit`, window close | quit asked |
| `WindowResized` | `uint32_t width, height` | platform plugin | window size changed |
| `WindowFocusChanged` | `bool focused` | platform plugin | focus lost or gained |
| `Collision` | `Entity a, b` | `engine.collision` | two colliders overlap this tick (once per pair) |
| `SceneLoaded` | `std::string name` | `SceneManager` | after a scene finished loading |
| `SceneUnloading` | `std::string name` | `SceneManager` | before a scene is cleared |
| `ClientConnected` | `ConnectionId id` | `NetworkSession` | server: a client completed the handshake |
| `ClientDisconnected` | `ConnectionId id; DisconnectReason reason` | `NetworkSession` | server: quit, timeout or error |
| `ConnectedToServer` / `DisconnectedFromServer` | `DisconnectReason reason` | `NetworkSession` | client side |
| `Caption` | `std::string text; float seconds` | `IAudio::play` with a caption | accessibility: visual cue for a sound |
| `ConsoleCommand` | `std::string line` | `Console` | a command was executed |

`DisconnectReason`: `Quit`, `Timeout`, `Kicked`, `ProtocolError`, `ServerShutdown`.
