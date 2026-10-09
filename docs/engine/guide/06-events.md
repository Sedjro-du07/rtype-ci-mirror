# 06 · Events

## Define and register

```cpp
struct PlayerDied { engine::Entity player; int lives; };
ENGINE_EVENT(PlayerDied)

void MyGameModule::registerTypes(engine::TypeRegistry& t) { t.addEvent<PlayerDied>(); }
```

## Publish

```cpp
ctx.events().publish(PlayerDied{e, lives});     // delivered at the next dispatch point
```

Dispatch points: after each `FixedUpdate` step and after `LateUpdate`. An event published while
events are being delivered waits for the next point, so handlers can never loop forever.

## Listen

```cpp
class ScoreSystem {
public:
    explicit ScoreSystem(engine::EventBus& bus)
        : sub_(bus.subscribe<engine::Collision>([this](const engine::Collision& c) { /* … */ })) {}
    void operator()(engine::Context& ctx) { /* … */ }
private:
    engine::Subscription sub_;      // unsubscribes automatically when the system is destroyed
};
```

Subscribe in `onStart` or in a system's constructor; keep the `Subscription` alive.

## Engine events

| Event | Fields |
|---|---|
| `QuitRequested` | — |
| `WindowResized` | `width, height` |
| `WindowFocusChanged` | `focused` |
| `Collision` | `a, b` (once per overlapping pair per tick) |
| `SceneLoaded`, `SceneUnloading` | `name` |
| `ClientConnected` | `id` (server) |
| `ClientDisconnected` | `id, reason` (server) |
| `ConnectedToServer`, `DisconnectedFromServer` | `reason` (client) |
| `Caption` | `text, seconds` (accessibility) |
| `ConsoleCommand` | `line` |

## Events or direct code?

Use an event when the sender should not know who reacts (damage → sound, score, network, FX).
Call code directly when one system owns the whole behaviour.
