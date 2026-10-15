# 08 · Rendering

Most games never call the renderer: give entities a `Sprite` or a `Text` and the engine draws them.

## Sprites

```yaml
# prefabs/player.yaml
Sprite:
  texture: assets/ships.png
  frame: { x: 0, y: 0, w: 33, h: 17 }    # region of the texture
  layer: 30
  origin: { x: 16, y: 8 }                # pivot
  scale: { x: 2, y: 2 }
```

Positions are interpolated between ticks automatically.

## Layers

`layer` is the draw order: lower first. Suggested ranges:

| Range | Use |
|---|---|
| 0–9 | background, starfield |
| 10–19 | obstacles, scenery |
| 20–29 | enemies |
| 30–39 | players |
| 40–49 | projectiles |
| 50–59 | effects |
| 100+ | HUD (screen space) |

## Animation

`engine.animation` advances `time` by `fixedDelta`, picks the frame `floor(time * fps)` and writes it
into `Sprite.frame`. With `loop: false` it stops on the last frame (an explosion); combine it with
`Lifetime` to remove the entity when the animation ends. To change animation (idle → turning), set a
new `Animation` component through `Commands`.

```yaml
Animation:
  fps: 12
  loop: true
  frames:
    - { x: 0,  y: 0, w: 33, h: 17 }
    - { x: 33, y: 0, w: 33, h: 17 }
```

## Text

```yaml
Text: { value: "SCORE 0", font: assets/font.ttf, size: 24, layer: 100, screenSpace: true }
```

## Parallax starfield

A background layer moves with `Velocity`; `Parallax.factor` scales that speed (0.2 = far, slow;
1.0 = foreground) and `wrapWidth` makes the layer jump back by that width once it has scrolled out,
so two copies side by side give an endless starfield. Put far layers on low `layer` values.

```yaml
Parallax: { factor: 0.3, wrapWidth: 1280 }
Velocity: { dx: -40, dy: 0 }
```

## Custom drawing from a system

```cpp
void hudSystem(engine::Context& ctx) {
    auto& q = ctx.render();
    q.text(std::format("LIVES {}", lives), font_, 20, {16, 16}, 100);
    q.rect({16, 40, hp * 10.f, 6}, engine::Color{200, 40, 40}, 100, true, true);
}
```

## Camera

```cpp
auto cam = ctx.render().camera();
cam.center.x += scrollSpeed * dt;
ctx.render().setCamera(cam);
```

## Debug and accessibility

| CVar | Effect |
|---|---|
| `r_debug_colliders 1` | draws collider outlines |
| `r_color_filter deuteranopia` | protanopia, deuteranopia, tritanopia, highcontrast, none |
| `r_ui_scale 1.5` | larger text and HUD |
| `r_vsync 0` | disable vsync |

Distinguish players by shape or marking as well as colour, so the colour filter is never the only aid.
