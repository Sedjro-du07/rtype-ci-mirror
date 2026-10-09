# 11 · Scenes, prefabs, assets

Status: **Locked**. ADRs 0010, 0016, 0018, 0034.
Folder: `engine/include/engine/scene/`, `engine/include/engine/assets/`, `engine/src/scene/`, `engine/src/assets/`.

## Role

Turn YAML files into entities (scenes, prefabs), switch scenes safely, save them back, and load
each resource file once behind a handle.

---

## `class AssetManager` — `assets/AssetManager.hpp`

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

    std::string_view pathOf(TextureHandle) const;  // reverse lookup for saving scenes; same for other kinds
    void retain(TextureHandle); void release(TextureHandle);   // reference counting (same for other kinds)
    void collectUnused();                          // frees zero-count assets; called after a scene switch
    void reloadChanged();                          // dev: re-reads files whose timestamp changed (CVar asset_hot_reload)
    std::filesystem::path resolve(std::string_view path) const; // game root + path; rejects "../" escapes
    AssetStats stats() const;                      // counts and bytes per kind
};
```

On the server the renderer and audio are Null Objects: handles are still valid, nothing is decoded.

## `struct Prefab` and `class PrefabLibrary` — `scene/Prefab.hpp`

```cpp
struct Prefab {
    std::string                     path;
    uint32_t                        key;       // fnv1a(path), used by NetPrefab
    std::vector<ComponentData>      components;// parsed values, already type-checked
    std::vector<PrefabChild>        children;  // nested prefabs or inline entities, with local offsets
};

class PrefabLibrary {
public:
    PrefabLibrary(const TypeRegistry& types, AssetManager& assets);
    Result<const Prefab*> get(std::string_view path);   // parses on first use, cached
    const Prefab* findByKey(uint32_t key) const;        // replication spawn
    Entity instantiate(Registry& r, const Prefab& p, Vec2 position,
                       std::span<const ComponentData> overrides = {}); // creates the entity and its children
    void clear();
};
```

## `class SceneLoader` — `scene/SceneLoader.hpp`

```cpp
class SceneLoader {
public:
    SceneLoader(const TypeRegistry& types, AssetManager& assets, PrefabLibrary& prefabs);
    Result<std::unique_ptr<Scene>> load(const std::filesystem::path& file);
    // Pass 1: parse; for each entity create it, resolve its prefab, apply overrides, record uuid → entity.
    // Pass 2: link Parent / Children and Entity fields by uuid (declaration order does not matter).
    // Then request the assets. Any error returns file:line and leaves no partial scene behind.
    Result<void> save(const Scene& scene, const std::filesystem::path& file) const;
    // Writes uuids, prefab references and only the components that differ from the prefab; skips Transient types.
};
```

## `class Scene` — `scene/Scene.hpp`

```cpp
class Scene {
public:
    const std::string& name() const;
    const std::filesystem::path& path() const;
    Registry& registry();
};
```

## `class SceneManager` — `scene/SceneManager.hpp`

One active scene at a time.

```cpp
class SceneManager {
public:
    SceneManager(const TypeRegistry&, AssetManager&, EventBus&, Logger&);
    Result<void> load(const std::filesystem::path& file);   // engine only (first scene): immediate
    void switchTo(const std::filesystem::path& file);      // deferred to the end of the frame
    void markPersistent(Entity e);                         // copied into the next scene on switch (the player)
    Result<void> save(const std::filesystem::path& file) const;
    Registry& world();                                     // active scene's registry
    std::string_view activeName() const;
    PrefabLibrary& prefabs();
    void applyPending(Context& ctx);                       // engine only: SceneUnloading → clear → load → SceneLoaded
    void clear();                                          // engine only: shutdown, before the game module unloads
};
```

If loading the next scene fails, the current scene stays active and the error is logged and
shown; the game does not end on a bad level file.

## File formats

Scene and prefab YAML formats are documented in the guide (10-scenes-prefabs-assets).
