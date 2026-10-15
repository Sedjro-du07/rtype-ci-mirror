# 25 · Accessibility checklist

The subject asks for answers for physical and motor, audio and visual, mental and cognitive
disabilities. The engine provides the tools; each game must turn them on and use them.

## Motor

| Engine provides | Game must |
|---|---|
| rebinding of every action, saved (`InputMap`, page 07) | offer a controls menu with rebinding |
| several bindings per action, gamepad support | ship sensible keyboard and gamepad defaults |
| `holdToToggle`, `autoRepeat` (auto-fire), `deadzone` | expose them in the options menu |
| a one-handed binding set (`oneHanded`) | provide that set in `input.yaml` |
| fixed tick rate | offer a slower game speed or easier difficulty if relevant |

Menu navigation without a mouse needs the UI block, which is not in the engine yet (page 27).

## Visual

| Engine provides | Game must |
|---|---|
| colour-blindness filters (`r_color_filter`) | never rely on colour alone: give the four players different shapes or markers |
| UI scale (`r_ui_scale`) | size HUD text from it |
| high-contrast filter | keep backgrounds less busy than the play area |
| letterboxed virtual resolution | keep the play area readable at small window sizes |

## Audio

| Engine provides | Game must |
|---|---|
| captions: `PlayParams.caption` → `Caption` event (page 09) | give every gameplay-relevant sound a caption, and show captions in the HUD |
| separate buses: master, music, effects, interface | expose four sliders |
| positional panning | also show a visual cue for off-screen threats |

## Cognitive

| Engine provides | Game must |
|---|---|
| CVars for difficulty and speed | offer difficulty levels |
| a `Paused` flag pattern (page 16) | allow pause in solo play |
| clear error messages for players | write short, plain texts; avoid time-limited menus |

## Before a defense

- [ ] Every action rebindable, and the game playable with a gamepad only.
- [ ] Players distinguishable without colour.
- [ ] Captions on for explosions, enemy shots, boss attacks, low health.
- [ ] Volumes, UI scale and colour filter in the options and saved.
- [ ] One accessibility section per category in the comparative study.
