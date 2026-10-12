# 19 · Game module and game state

## The module class, in full

```cpp
class RTypeModule : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry& t) override {
        registerComponents(t);          // split by feature: Components.cpp
        registerEvents(t);
    }
    void registerSystems(engine::Role role, engine::Scheduler& s) override {
        registerShared(s);              // movement helpers, used by both roles
        if (role == engine::Role::Server) registerServer(s);
        if (role == engine::Role::Client) registerClient(s);
    }
    std::string_view firstScene() const override { return "scenes/menu.yaml"; }

    void onStart(engine::Context& ctx) override {
        ctx.audio().playMusic(ctx.assets().music("assets/menu.ogg"));
        highScores_ = loadHighScores("user/scores.yaml");     // file I/O is fine here
        subs_.push_back(ctx.events().subscribe<PlayerDied>([this](auto& e) { onPlayerDied(e); }));
    }
    void onShutdown(engine::Context& ctx) override {
        saveHighScores("user/scores.yaml", highScores_);
    }
private:
    HighScores highScores_;
    std::vector<engine::Subscription> subs_;
};
ENGINE_GAME_MODULE(RTypeModule)
```

| Method | Called | Put here |
|---|---|---|
| `registerTypes` | once at boot | every component, event and enum of the game |
| `registerSystems` | once at boot, with the role | every system, with names and ordering |
| `firstScene` | once at boot | the path of the first scene for this role |
| `onStart` | once, after the first scene, before the loop | music, files to read, subscriptions, console commands, CVars |
| `onShutdown` | once, the world still exists | files to write |

## Where game state lives

Global game facts (score, lives, current wave, pause, difficulty) need a home. Two good places:

**1. A game-state entity (recommended).** One entity with the components, created by the first scene:

```yaml
# scenes/level1.yaml
- uuid: 00000000000000a1
  Name: { value: "GameState" }
  Score: { value: 0 }
  Lives: { value: 3 }
  Wave: { index: 0 }
```

```cpp
auto gs = ctx.world().view<Score, Lives>().front();     // or world.find("GameState")
ctx.world().get<Score>(gs).value += 100;
```

It is saved with the scene, visible in the console (`inspect GameState`), replicated if its
components are `Replicated`, and kept across scenes with `ctx.scenes().markPersistent(gs)`.

**2. Members of a system functor** for state only that system needs (a spawn timer, a random generator):

```cpp
struct WaveSpawner {
    std::mt19937 rng{12345};           // seeded: reproducible tests; the server owns randomness
    float timer = 0.f;
    void operator()(engine::Context& ctx);
};
```

**Never** use globals or `static` variables for game state: they survive a scene reload, are
duplicated per module, and hide dependencies.

## Organising the code

```
games/rtype/src/
├── RTypeModule.cpp        the IGameModule class, ENGINE_GAME_MODULE
├── Components.hpp/.cpp    every component + registerComponents()
├── Events.hpp             every event struct
├── systems/
│   ├── Player.cpp         control, shooting
│   ├── Enemies.cpp        AI, spawner
│   ├── Combat.cpp         damage, death
│   └── Hud.cpp            client only
└── Registration.cpp       registerShared / registerServer / registerClient
```

One file per feature, not one file per system. Keep systems small: one job each.
