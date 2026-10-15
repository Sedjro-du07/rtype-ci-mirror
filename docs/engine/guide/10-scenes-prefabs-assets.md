# 10 · Scenes, prefabs and assets

## Prefab file

A prefab describes one kind of entity, optionally with children.

```yaml
# prefabs/player_ship.yaml
Name:     { value: "Player" }
Sprite:   { texture: assets/ships.png, frame: { x: 0, y: 0, w: 33, h: 17 }, layer: 30 }
Collider: { size: { x: 30, y: 14 }, layer: 1, mask: 6 }
Velocity: {}
Health:   { hp: 3, maxHp: 3 }
Tags:     [player]
children:
  - prefab: prefabs/force.yaml
    Parent: { offset: { x: 26, y: 0 } }
```

## Scene file

```yaml
# scenes/level1.yaml
name: Level 1
entities:
  - uuid: a1f3c2d4e5f60718
    prefab: prefabs/player_ship.yaml
    Position: { x: 40, y: 90 }

  - uuid: 9c2e00000000aa01
    prefab: prefabs/bydos.yaml
    Position: { x: 360, y: 120 }
    Velocity: { dx: -60 }                 # override: only what differs from the prefab

  - uuid: 0000000000000b05
    Name: { value: "Background" }         # an entity without prefab
    Position: { x: 0, y: 0 }
    Sprite: { texture: assets/stars.png, layer: 0 }
    Parallax: { factor: 0.2, wrapWidth: 1280 }
```

- `uuid` keeps an entity's identity across saves and in the editor. Any unique hex string.
- A component written in the scene overrides the prefab's; fields not written keep the prefab value.
- `Entity` fields (`Parent`, a target) refer to other entities by uuid; order does not matter.

## Switching scenes

```cpp
ctx.scenes().markPersistent(player);          // keep this entity in the next scene
ctx.scenes().switchTo("scenes/level2.yaml");  // applied at the end of the frame
```

If the next scene fails to load, the current scene stays and the error is shown.

## Errors

```
scenes/level1.yaml:14: unknown component 'Helth' (did you mean 'Health'?)
prefabs/bydos.yaml:3: field 'hp' expects an integer, got "three"
scenes/level1.yaml:22: unknown prefab 'prefabs/bydoss.yaml'
```

## Assets

```cpp
auto tex   = ctx.assets().texture("assets/ships.png");
auto font  = ctx.assets().font("assets/font.ttf");
auto sound = ctx.assets().sound("assets/shot.wav");
auto music = ctx.assets().music("assets/stage1.ogg");
```

- Paths are relative to the game folder; `..` outside it is refused.
- Each file is loaded once. A missing texture shows a visible checkerboard and logs an error.
- `set asset_hot_reload 1` reloads changed files while the game runs (development).
