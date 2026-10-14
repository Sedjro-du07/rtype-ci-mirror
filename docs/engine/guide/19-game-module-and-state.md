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
    void onShutdown(engine::ServiceContext& ctx) override {
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
| `onStart` | client: once, after the first scene, before the loop | music, files to read, subscriptions, console commands, CVars |
| `onLobbyStart` | server: once | lobby rules: subscriptions to party and room events, lobby `on<T>` (guide 28) |
| `onRoomStart` | server: once per room | room subscriptions, kept with `ctx.room().keep(...)` (guide 28) |
| `onShutdown` | once, before the module is unloaded | files to write |

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

A game is one code built twice (ADR 0040): what both sides need goes in `shared/`, the rest in
`server/` or `client/`. The server build never contains client code, and the other way round.

```
games/rtype/
├── shared/                    compiled into both builds
│   ├── Types.hpp/.cpp         components, events, messages, ENGINE_STRUCTs + registerTypes()
│   └── Movement.cpp           systems both sides run (movement, prediction later)
├── server/                    compiled into librtype_server only
│   ├── ServerModule.cpp       IGameModule: registerSystems(Server), onLobbyStart, onRoomStart
│   ├── Lobby.cpp              party and room rules: invitations, PlayRequest, end of game
│   ├── Waves.cpp, Enemies.cpp spawner, AI
│   └── Combat.cpp             collisions → damage, death, score
└── client/                    compiled into librtype_client only
    ├── ClientModule.cpp       IGameModule: registerSystems(Client), onStart
    ├── Menus.cpp              lobby screens: party, invitations, room list
    └── Hud.cpp, Effects.cpp   HUD, explosions, sounds
```

Each build has its own module class, both registering the **same types** from `shared/`:

```cpp
// shared/Types.cpp
void registerRTypeTypes(engine::TypeRegistry& t);       // called by both module classes

// server/ServerModule.cpp
class RTypeServer : public engine::IGameModule {
    void registerTypes(engine::TypeRegistry& t) override { registerRTypeTypes(t); }
    void registerSystems(engine::Role, engine::Scheduler& s) override { registerSharedSystems(s); registerServerSystems(s); }
    void onLobbyStart(engine::LobbyContext& ctx) override;   // lobby rules
    void onRoomStart(engine::Context& ctx) override;         // per-room subscriptions
    ...
};
ENGINE_GAME_MODULE(RTypeServer)
```

Include by role: `shared/` includes the common engine headers and `engine/net/`; `server/` may also
include `engine/server/`; `client/` may also include `engine/client/`. If the two builds come from
different `shared/` code, the client is refused at connection (`FormatMismatch`).

One file per feature, not one file per system. Keep systems small: one job each.
