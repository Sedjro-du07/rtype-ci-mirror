# Making a game with the engine

Status: **Locked** with the architecture. Topic pages: [`guide/README.md`](guide/README.md).
Every API used here is described in [`reference/README.md`](reference/README.md).

We build a Pong, from an empty folder to a running game.

## Start from a Pong without an engine

This is the version you would write with SFML alone, in one `main()`. Each block has a letter.

```cpp
int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Pong");          // [A] window
    sf::Texture ballImg; ballImg.loadFromFile("ball.png");               // [B] assets
    float ballX = 400, ballY = 300, ballSpeedX = 250, ballSpeedY = 120;   // [C] starting data
    float leftY = 300, rightY = 300; int scoreLeft = 0, scoreRight = 0;
    sf::Clock clock;
    while (window.isOpen()) {                                             // [D] loop + time
        float dt = clock.restart().asSeconds();
        bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);      // [E] keyboard
        if (up) leftY -= 300 * dt;                                        // [F1] paddle
        ballX += ballSpeedX * dt; ballY += ballSpeedY * dt;               // [F2] ball moves
        if (ballY < 0 || ballY > 600) ballSpeedY = -ballSpeedY;           // [F3] bounce
        if (ballX < 0) { scoreRight++; ballX = 400; }                     // [F4] goal
        window.clear(); /* draw */ window.display();                      // [G] draw
    }
}
```

Its problems: another game copies A, B, D, E, G; changing the ball speed needs a recompile;
switching to raylib rewrites the file; fifty balls need a rewrite.

## Sort the pieces

Ask of each block: *would it be the same in R-Type?*

| Block | What | Same in R-Type? | Goes to |
|---|---|---|---|
| A | open a window | yes | engine (platform plugin) |
| B | load images and sounds | yes | engine (AssetManager) |
| D | loop and time | yes | engine (`Engine::loop`) |
| E | read the keyboard | yes | engine (InputMap) |
| G | draw | yes | engine (render system + plugin) |
| F2 | move by velocity | yes | engine (`movementSystem`) |
| F1, F3, F4 | paddles, bounce, goal | no | game: C++ systems |
| C | which objects, where, how fast | no | game: text files (scene + prefabs) |

What remains for the game: its specific rules, and its starting objects.

## The project folder

```
pong/
├── CMakeLists.txt        build recipe: compile src/ and plug it into the engine
├── pong.yaml             launch settings: which module, which plugins, tickRate
├── src/                  compiled into libpong_game.so / pong_game.dll
│   ├── Components.hpp      data the engine does not know: Ball, Paddle, Score
│   ├── Systems.cpp         the rules: paddles, bounce, goal
│   └── PongModule.cpp      the game's business card for the engine
├── prefabs/              molds, read at launch
│   ├── ball.yaml
│   └── paddle.yaml
├── scenes/               starting layout, read at launch
│   └── match.yaml
└── assets/               images and sounds, loaded at launch
    ├── ball.png  paddle.png
    └── bounce.wav
```

A **prefab** is a mold: what one kind of object looks like. A **scene** is the layout: how many
objects, which ones, and where, built from the molds.

## 1. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.25)
project(pong CXX)
find_package(engine REQUIRED)
engine_add_game_module(pong_game SOURCES src/PongModule.cpp src/Systems.cpp)
engine_add_launcher(pong CONFIG pong.yaml GAME pong_game)
```

## 2. Components

```cpp
// src/Components.hpp
#include <engine/reflection/TypeKey.hpp>

struct Ball   { float speed = 250.f; };
ENGINE_COMPONENT(Ball)
struct Paddle { int player = 1; };
ENGINE_COMPONENT(Paddle)
struct Score  { int left = 0, right = 0; };
ENGINE_COMPONENT(Score)
```

`Position`, `Velocity`, `Sprite` and `Collider` already come from the engine.

## 3. Systems

```cpp
// src/Systems.cpp
using namespace engine;
static constexpr auto MoveUp   = action("MoveUp");
static constexpr auto MoveDown = action("MoveDown");

void paddleControlSystem(Context& ctx) {
    for (auto [e, vel, pad] : ctx.world().view<Velocity, Paddle>()) {
        vel.dy = 0;
        if (ctx.input().held(MoveUp))   vel.dy = -300;
        if (ctx.input().held(MoveDown)) vel.dy =  300;
    }
}

void bounceSystem(Context& ctx) {
    for (auto [e, pos, vel, ball] : ctx.world().view<Position, Velocity, Ball>())
        if (pos.y < 0 || pos.y > 600) {
            vel.dy = -vel.dy;
            ctx.audio().play(ctx.assets().sound("assets/bounce.wav"));
        }
}
```

A system works on every entity that has the listed components: one ball or fifty, same code.

## 4. The module

```cpp
// src/PongModule.cpp
class PongModule : public engine::IGameModule {
public:
    void registerTypes(engine::TypeRegistry& t) override {
        t.add<Ball>().field("speed", &Ball::speed);
        t.add<Paddle>().field("player", &Paddle::player);
        t.add<Score>().field("left", &Score::left).field("right", &Score::right);
    }
    void registerSystems(engine::Role role, engine::Scheduler& s) override {
        using engine::Stage;
        s.add(Stage::FixedUpdate, "pong.paddle_control", paddleControlSystem).before("engine.movement");
        s.add(Stage::FixedUpdate, "pong.bounce", bounceSystem).after("engine.movement");
    }
    std::string_view firstScene() const override { return "scenes/match.yaml"; }
};
ENGINE_GAME_MODULE(PongModule)
```

## 5. Prefabs and scene

```yaml
# prefabs/ball.yaml
Sprite:   { texture: assets/ball.png }
Collider: { width: 16, height: 16 }
Velocity: { dx: 250, dy: 120 }
Ball:     { speed: 250 }
```

```yaml
# scenes/match.yaml
name: Match
entities:
  - prefab: prefabs/paddle.yaml
    Position: { x: 30, y: 300 }
    Paddle:   { player: 1 }
  - prefab: prefabs/paddle.yaml
    Position: { x: 770, y: 300 }
    Paddle:   { player: 2 }
  - prefab: prefabs/ball.yaml
    Position: { x: 400, y: 300 }
  - Score: {}
```

## 6. Config and input

```yaml
# input.yaml
actions:
  MoveUp:   [Up, W]
  MoveDown: [Down, S]
```

```yaml
# pong.yaml
role: client
game: pong_game
platform: sfml
audio: miniaudio
tickRate: 60
```

## 7. Build and run

```bash
cmake -B build && cmake --build build
./build/pong
```

## What happens at launch

| Without engine | With the engine |
|---|---|
| [A] open a window | engine reads `pong.yaml`, loads the SFML plugin |
| — | engine loads `libpong_game.so`, calls `registerTypes` and `registerSystems` |
| [C] create objects | engine reads `scenes/match.yaml`, opens the prefabs, creates 2 paddles + 1 ball |
| [B] load images | engine loads the files named in the prefabs |
| [D] loop + time | engine loop starts, 60 ticks per second |
| [E] keyboard | engine reads devices and turns them into actions |
| [F] rules | engine runs its `movementSystem`, then Pong's systems |
| [G] draw | engine draws everything with a `Position` and a `Sprite` |

## Day to day

| I want to… | I edit | Then |
|---|---|---|
| make the ball faster | `prefabs/ball.yaml` | relaunch, no recompile |
| move a paddle, add an obstacle | `scenes/match.yaml` | relaunch |
| change an image or a sound | `assets/` | relaunch |
| switch SFML to raylib | `pong.yaml` | relaunch |
| add a rule | `src/Systems.cpp` | rebuild the game module only |
| add a data type | `Components.hpp` + `registerTypes` | rebuild the game module only |

The shipped folder contains the executable, `libengine`, `plugins/`, `games/libpong_game.so`,
and the YAML and asset folders. `src/` is not shipped: it is already inside the module.
