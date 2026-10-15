# 21 · Saving

Three kinds of things are saved, each in its own way.

## 1. Player settings: CVars with `Archive`

Volumes, colour filter, UI scale, deadzone and your own options are console variables. Those
flagged `Archive` are written to the user settings file and read back at the next launch.

```cpp
auto& diff = ctx.cvars().define("rtype_difficulty", 1, "0 easy, 1 normal, 2 hard",
                                engine::Archive, std::pair{0, 2});
```

The engine loads the archive after the config file and saves it at shutdown
(`user/settings.yaml` next to the executable). Engine variables `snd_*`, `r_ui_scale`,
`r_color_filter`, `in_deadzone` are already `Archive`.

## 2. Key bindings: `InputMap::save`

```cpp
ctx.inputMap().save("user/input.yaml");     // after rebinding in the options menu
```

At launch, load `user/input.yaml` if it exists, else the game's default `input.yaml` (do it in `onStart`).

## 3. The world: `SceneManager::save`

```cpp
ctx.scenes().save("user/save1.yaml");          // the active scene, as a scene file
ctx.scenes().switchTo("user/save1.yaml");      // load it back like any scene
```

What is written:

- every entity's `uuid`, its prefab, and **only the components that differ from the prefab**;
- not the components flagged `Transient` (runtime state: `PreviousPosition`, `Children`, timers you
  marked transient);
- asset handles as paths, entity references as uuids.

Mark runtime-only components `Transient` at registration, or they end up in save files.

## Game-specific files

High scores, unlocked levels, statistics: read in `onStart`, write in `onShutdown` (or when they
change, from a console command or menu action). Never do file I/O inside a system that runs every tick.

## Where

Write under `user/` next to the executable. The asset manager refuses paths outside the game root,
so saves never escape the game folder.
