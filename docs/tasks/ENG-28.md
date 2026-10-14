# ENG-28 · Keys, RawInputEvent, ActionId

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **07 Input** · status and owner live on the board.

## Goal

Device vocabulary and action ids.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/input/Keys.hpp`
- `include/engine/input/RawInputEvent.hpp`
- `include/engine/input/Action.hpp`
- `src/input/Keys.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/input/Keys.hpp`

```cpp
enum class Key : uint16_t { A, B, /* … */ Z, Num0, /* … */ Up, Down, Left, Right, Space, Enter, Escape,
                            LShift, RShift, LCtrl, RCtrl, Tab, Backspace, F1, /* … */ F12, Unknown };
enum class MouseButton : uint8_t { Left, Right, Middle };
enum class GamepadButton : uint8_t { South, East, West, North, LeftShoulder, RightShoulder,
                                     Start, Select, DPadUp, DPadDown, DPadLeft, DPadRight };
enum class GamepadAxis : uint8_t { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger };

std::string_view keyName(Key k);                         // label of the current keyboard layout, for display
std::optional<Key>           keyFromName(std::string_view name);      // input.yaml ↔ enum
std::optional<GamepadButton> gamepadButtonFromName(std::string_view name);
std::optional<GamepadAxis>   gamepadAxisFromName(std::string_view name);
std::string_view             toName(Key k);  std::string_view toName(GamepadButton b);  std::string_view toName(GamepadAxis a);
```

### `include/engine/input/RawInputEvent.hpp`

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

### `include/engine/input/Action.hpp`

```cpp
struct ActionId { uint64_t key; constexpr bool operator==(const ActionId&) const = default; };
constexpr ActionId action(std::string_view name) noexcept;   // action("MoveUp") = fnv1a64("MoveUp")
```

## Depends on

- [ENG-04](ENG-04.md) — Core types: hash, Vec2/Rect/Color, Error, Result, BootError

## Unlocks

- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks

## Read first

- [ADR 0030](../adr/0030-actions-and-input-map.md)

## To implement

- [ ] `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis` enums; name ↔ enum tables; `keyName`
- [ ] `RawInputEvent`
- [ ] `ActionId`, `action()` constexpr

## Done when

- Tests: every key name parses back to its enum

## Watch out

- Keys are physical positions (scancodes)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
