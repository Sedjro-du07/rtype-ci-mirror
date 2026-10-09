# 7 · Input

Status: **Locked**. ADRs 0004, 0030.
Folder: `engine/include/engine/input/`, `engine/src/input/`.

## Role

Turn raw device events from the platform plugin into named actions, with rebinding and
accessibility options written once for every backend. Game code reads actions, never keys.

---

## Device vocabulary — `input/Keys.hpp`

```cpp
enum class Key : uint16_t { A, B, /* … */ Z, Num0, /* … */ Up, Down, Left, Right, Space, Enter, Escape,
                            LShift, RShift, LCtrl, RCtrl, Tab, Backspace, F1, /* … */ F12, Unknown };
enum class MouseButton : uint8_t { Left, Right, Middle };
enum class GamepadButton : uint8_t { South, East, West, North, LeftShoulder, RightShoulder,
                                     Start, Select, DPadUp, DPadDown, DPadLeft, DPadRight };
enum class GamepadAxis : uint8_t { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };
```

Keys are identified by **physical position** (scancode) so that ZQSD and WASD layouts both work;
`keyName(Key)` returns the label of the current layout for display.

## `struct RawInputEvent` — `input/RawInputEvent.hpp`

```cpp
struct RawInputEvent {
    enum class Type : uint8_t { KeyDown, KeyUp, MouseDown, MouseUp, MouseMove, MouseWheel,
                                GamepadDown, GamepadUp, GamepadAxis, GamepadConnected,
                                GamepadDisconnected, TextEntered };
    Type          type;
    uint8_t       device = 0;          // gamepad index
    Key           key = Key::Unknown;
    MouseButton   mouse{};
    GamepadButton button{};
    GamepadAxis   axis{};
    float         value = 0.f;         // axis value [-1,1], wheel delta
    Vec2          position;            // mouse, in virtual resolution
    char32_t      text = 0;            // TextEntered (console, name entry)
};
```

Produced by `IPlatform::pollEvents` (block 8). The raylib plugin derives these from polling.

## `ActionId` — `input/Action.hpp`

```cpp
struct ActionId { uint64_t key; constexpr bool operator==(const ActionId&) const = default; };
constexpr ActionId action(std::string_view name) noexcept;   // action("MoveUp") = fnv1a64("MoveUp")
```

Actions are named by the game (in `input.yaml` and in code); the engine knows only their hashes.

## `struct Binding` — `input/InputMap.hpp`

```cpp
struct AxisBinding { GamepadAxis axis; float direction; float threshold; }; // e.g. LeftY, -1, 0.5 → MoveUp
using Binding = std::variant<Key, MouseButton, GamepadButton, AxisBinding>;
```

## `struct InputOptions` — `input/InputMap.hpp` (accessibility)

```cpp
struct InputOptions {
    std::vector<ActionId> holdToToggle;   // press once to start holding, press again to release
    std::vector<ActionId> autoRepeat;     // held action repeats "pressed" at autoRepeatHz (auto-fire)
    float                 autoRepeatHz = 8.f;
    float                 deadzone = 0.2f;// gamepad axes below this read as 0
    bool                  oneHanded = false; // loads the alternate binding set "one_handed" from input.yaml
};
```

## `class ActionState` — `input/ActionState.hpp`

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

## `class InputMap` — `input/InputMap.hpp`

```cpp
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

`input.yaml` format is documented in the guide (07-input).
