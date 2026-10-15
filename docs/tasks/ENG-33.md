# ENG-33 · Colour-blindness filters and UI scale in platform_sfml

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **08 Rendering** · **Wave 7** · status and owner live on the board.

## Goal

Colour-blindness filters and UI scale (accessibility).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/platform_sfml/ColorFilter.frag`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/platform_sfml/ColorFilter.frag`

```glsl
uniform sampler2D texture;
uniform int       mode;          // 0 none, 1 protanopia, 2 deuteranopia, 3 tritanopia, 4 high contrast
uniform float     strength;      // 0..1
vec3 simulate(vec3 rgb, int mode);
vec3 daltonize(vec3 rgb, int mode);
void main();
```

### `plugins/platform_sfml/SfmlRenderer.hpp`

```cpp
void SfmlRenderer::setColorFilter(ColorFilter f) override;   // selects the shader mode
bool SfmlRenderer::supports(Feature f) const override;        // ColorFilter: sf::Shader::isAvailable()
// private:
sf::Shader        filter_;
sf::RenderTexture offscreen_;                                 // the frame is drawn here, then through the shader
float             uiScale_ = 1.f;                             // applied to screenSpace commands
```

## Depends on

- [ENG-31](ENG-31.md) — platform_sfml plugin: window, events to InputMap, batching renderer

## Unlocks

- no task waits for this one

## Read first

- [ADR 0031](../adr/0031-render-api.md)

## To implement

- [ ] Fragment shader with protanopia, deuteranopia, tritanopia matrices and high contrast
- [ ] `setColorFilter`, UI scale applied to screen-space draws

## Done when

- `r_color_filter` changes the picture live; `r_ui_scale 1.5` enlarges HUD text

## Watch out

- Without shader support, `supports(ColorFilter)` returns false

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
