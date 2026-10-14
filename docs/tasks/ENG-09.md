# ENG-09 · Context facade

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · status and owner live on the board.

## Goal

The only door from game code into the engine (ADR 0022).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/Context.hpp`
- `src/core/Context.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/Context.hpp`

```cpp
class Context {
public:
    Registry&          world();             // registry of the ACTIVE scene; re-read it after a scene switch
    Commands&          commands();          // deferred spawn / destroy / add / remove
    SceneManager&      scenes();            // switchTo(), markPersistent(), active scene name
    EventBus&          events();            // publish / subscribe
    const ActionState& input() const;       // local player's actions this tick
    InputMap&          inputMap();          // bindings and options (options menu, rebinding)
    AssetManager&      assets();            // textures, sounds, music, fonts, scripts by path
    RenderQueue&       render();            // describe what to draw this frame
    IAudio&            audio();             // NullAudio on the server
    NetworkSession&    network();           // connections, send, stats (block 10)
    const Time&        time() const;        // fixedDelta, tick, alpha, total
    Logger&            log();
    CVars&             cvars();
    Console&           console();
    Metrics&           metrics();           // record your own series for the overlay
    Role               role() const;
    void               requestQuit();       // a request: the frame finishes cleanly

private:
    friend class Engine;
    explicit Context(Engine& engine);
    Engine* engine_;
};
```

## Depends on

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy

## Unlocks

- no task waits for this one

## Read first

- [ADR 0022](../adr/0022-context-facade.md)
- [reference 01](../engine/reference/01-core.md)

## To implement

- [ ] Accessors of reference 01 forwarding to `Engine` members
- [ ] Private constructor, `friend class Engine`

## Done when

- A game test module compiles using only `Context.hpp` and block headers, never `Engine.hpp`

## Watch out

- Context holds no state of its own; never cache it in game code

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
