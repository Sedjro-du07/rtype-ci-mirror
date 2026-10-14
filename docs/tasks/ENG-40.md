# ENG-40 · AssetManager: handles, cache, refcount, fallback, hot reload

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **11 Scenes & assets** · status and owner live on the board.

## Goal

Load each resource once behind a handle, with fallbacks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/assets/AssetManager.hpp`
- `src/assets/AssetManager.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/assets/AssetManager.hpp`

```cpp
class AssetManager {
public:
    AssetManager(Logger& log, std::filesystem::path root);
    void attach(IRenderer& renderer, IAudio& audio, IScripting& scripting); // engine only, after plugins load

    TextureHandle texture(std::string_view path);  // loads once, returns the cached handle afterwards;
                                                   // on failure logs the error and returns the "missing" texture
    FontHandle    font(std::string_view path);     // failure → engine default font
    SoundHandle   sound(std::string_view path);    // failure → invalid handle (play does nothing)
    MusicHandle   music(std::string_view path);
    ScriptHandle  script(std::string_view path);

    // Reverse lookup (saving scenes) and reference counting, one overload per handle kind.
    std::string_view pathOf(TextureHandle) const; std::string_view pathOf(FontHandle) const;
    std::string_view pathOf(SoundHandle) const;   std::string_view pathOf(MusicHandle) const;
    std::string_view pathOf(ScriptHandle) const;
    void retain(TextureHandle);  void retain(FontHandle);  void retain(SoundHandle);
    void retain(MusicHandle);    void retain(ScriptHandle);
    void release(TextureHandle); void release(FontHandle); void release(SoundHandle);
    void release(MusicHandle);   void release(ScriptHandle);
    void collectUnused();                          // frees zero-count assets; called after a scene switch
    void reloadChanged();                          // dev: re-reads files whose timestamp changed (CVar asset_hot_reload)
    std::filesystem::path resolve(std::string_view path) const; // game root + path; rejects "../" escapes
    AssetStats stats() const;                      // counts and bytes per kind
};

struct AssetStats { uint32_t textures, fonts, sounds, musics, scripts; uint64_t bytes; uint32_t failed; };
```

## Depends on

- [ENG-30](ENG-30.md) — Render API: handles, WindowDesc, Camera2D, DrawCmd, RenderQueue, IRenderer, IPlatform
- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-48](ENG-48.md) — IScripting, Script component, script systems

## Unlocks

- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate

## Read first

- [ADR 0034](../adr/0034-scenes-prefabs-assets.md)
- [reference 11](../engine/reference/11-scenes-assets.md)

## To implement

- [ ] texture, font, sound, music, script with cache
- [ ] pathOf, retain / release, collectUnused, reloadChanged, resolve (reject `../`), stats

## Done when

- Missing texture shows the checkerboard and logs once

## Watch out

- On the server, handles are valid but nothing is decoded

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
