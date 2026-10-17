# ENG-29 · InputMap and ActionState: input.yaml, rebinding, accessibility options, masks

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **07 Input** · **Wave 5** · status and owner live on the board.

## Goal

Turn raw input into actions with rebinding and accessibility options.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/input/InputMap.hpp`
- `include/engine/input/ActionState.hpp`
- `src/input/InputMap.cpp`
- `src/input/ActionState.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/input/InputMap.hpp`

```cpp
struct AxisBinding { GamepadAxis axis; float direction; float threshold; }; // e.g. LeftY, -1, 0.5 → MoveUp
using Binding = std::variant<Key, MouseButton, GamepadButton, AxisBinding>;

struct InputOptions {
    std::vector<ActionId> holdToToggle;   // press once to start holding, press again to release
    std::vector<ActionId> autoRepeat;     // held action repeats "pressed" at autoRepeatHz (auto-fire)
    float                 autoRepeatHz = 8.f;
    float                 deadzone = 0.2f;// gamepad axes below this read as 0
    bool                  oneHanded = false; // loads the alternate binding set "one_handed" from input.yaml
};

class InputMap {
public:
    Result<void> load(const std::filesystem::path& file);   // input.yaml: actions → bindings, options
    Result<void> save(const std::filesystem::path& file) const; // after rebinding in the options menu
    void bind(ActionId a, Binding b);                       // adds a binding (several per action)
    void unbind(ActionId a, const Binding& b);
    void clear(ActionId a);
    std::span<const Binding> bindings(ActionId a) const;
    std::optional<ActionId> boundTo(const Binding& b) const;// conflict detection when rebinding
    void setOptions(InputOptions o);
    const InputOptions& options() const;

    void feed(const RawInputEvent& e);   // engine only: called by the platform plugin's pollEvents
    void beginTick();                    // engine only: publishes latched edges to the ActionState
    const ActionState& state() const;    // local player; what Context::input() returns
    void captureNextBinding(std::move_only_function<void(Binding)> onCaptured); // "press a key" in menus
};
```

### `include/engine/input/ActionState.hpp`

```cpp
class ActionState {
public:
    bool  pressed(ActionId a) const;   // went down since the previous tick (edges are latched between ticks)
    bool  held(ActionId a) const;      // is down this tick
    bool  released(ActionId a) const;  // went up since the previous tick
    float axis(ActionId a) const;      // [-1,1] for axis bindings, 0/1 for buttons

    // Compact form for the network (block 10): bit i = held(order[i]).
    uint64_t toMask(std::span<const ActionId> order) const;
    static ActionState fromMask(uint64_t mask, uint64_t previousMask, std::span<const ActionId> order);
};
```

## Depends on

- [ENG-28](ENG-28.md) — Keys, RawInputEvent, ActionId
- [ENG-23](ENG-23.md) — YAML wrappers and field codecs

## Unlocks

- [ENG-08](ENG-08.md) — Engine: lifecycle, member order, fixed-step loop, error policy
- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer
- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [ENG-55](ENG-55.md) — Unit tests: scheduler, events, input, scenes, session

## Read first

- [ADR 0030](../adr/0030-actions-and-input-map.md)
- [reference 07](../engine/reference/07-input.md)

## To implement

- [ ] `InputMap`: load / save input.yaml, bind, unbind, boundTo, captureNextBinding, options
- [ ] Edge latching in `beginTick`, hold-to-toggle, auto-repeat, deadzone, one-handed set
- [ ] `ActionState`: pressed, held, released, axis, toMask / fromMask

## Done when

- Tests: a press and release between two ticks is seen once; options behave as documented

## Watch out

- Games read actions, never keys

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
