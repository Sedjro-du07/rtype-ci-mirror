# 07 · Input

Game code reads **actions**, never keys. Players and designers bind keys to actions.

## `input.yaml`

```yaml
actions:
  MoveUp:    [Up, W, "Gamepad:DPadUp", "Axis:LeftY-"]
  MoveDown:  [Down, S, "Gamepad:DPadDown", "Axis:LeftY+"]
  MoveLeft:  [Left, A, "Gamepad:DPadLeft", "Axis:LeftX-"]
  MoveRight: [Right, D, "Gamepad:DPadRight", "Axis:LeftX+"]
  Fire:      [Space, "Gamepad:South"]
  Console:   [F1]
  Metrics:   [F2]

one_handed:                 # alternate set loaded when options.oneHanded is true
  MoveUp: [I]
  MoveDown: [K]
  Fire: [J]

options:
  holdToToggle: [Fire]      # press once to keep firing, press again to stop
  autoRepeat: [Fire]        # held Fire repeats "pressed" (auto-fire)
  autoRepeatHz: 8
  deadzone: 0.2
  oneHanded: false
```

Keys are physical positions: `W` is the key at the W position on QWERTY, Z on AZERTY.

## Read in code

```cpp
using engine::action;
static constexpr auto MoveUp = action("MoveUp");
static constexpr auto Fire   = action("Fire");

const auto& in = ctx.input();
if (in.held(MoveUp)) vel.dy = -speed;
if (in.pressed(Fire)) shoot();        // edges are kept between ticks: a quick tap is never lost
float x = in.axis(action("MoveRight")) - in.axis(action("MoveLeft"));
```

On the server, the actions of each player are in `engine::RemoteInput` on the entities they own (12).

## Rebinding from an options menu

```cpp
auto& map = ctx.inputMap();
map.captureNextBinding([&](engine::Binding b) {
    if (auto other = map.boundTo(b)) { /* show conflict */ }
    map.bind(action("Fire"), b);
    map.save("input.yaml");
});
```

## Accessibility options

| Option | Helps |
|---|---|
| Rebinding of every action | motor disabilities, layouts |
| `holdToToggle` | players who cannot hold a key |
| `autoRepeat` | players who cannot tap repeatedly |
| `oneHanded` binding set | one-handed play |
| `deadzone` | unsteady sticks |
