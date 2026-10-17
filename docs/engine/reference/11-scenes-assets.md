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
    FontHandle    font(std::string_view path);     // failure → invalid handle (no default font yet)
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
    void reloadChanged();                          // dev: reloads scripts whose timestamp changed; other kinds
                                                   // are only reported (CVar asset_hot_reload)
    std::filesystem::path resolve(std::string_view path) const; // game root + path; rejects "../" escapes and
                                                                 // a case that differs from the file on disk (ADR 0044)
    AssetStats stats() const;                      // counts and bytes per kind
};

struct AssetStats { uint32_t textures, fonts, sounds, musics, scripts; uint64_t bytes; uint32_t failed; };
```

On the server the renderer and audio are Null Objects: handles are still valid, nothing is decoded.

The "missing" texture is a built-in 8×8 magenta and black checkerboard PNG. A path that fails is logged once;
later calls return the same fallback silently and count in `AssetStats::failed`.

## `ComponentData`, `struct Prefab` and `class PrefabLibrary` — `scene/Prefab.hpp`

```cpp
class ComponentData {                              // one component value of a type known only at run time
public:
    explicit ComponentData(const TypeInfo& type);  // default-constructed value
    static Result<ComponentData> parse(const TypeInfo&, const YamlNode&, LoadContext&); // keeps the node
    template <class T> static ComponentData of(const TypeRegistry&, T value);           // from code
    const TypeInfo& type() const; void* value(); const void* value() const;
    const YamlNode& source() const;                // defined when parsed: an override then only writes
};                                                 // the keys of that node; otherwise it replaces the value

struct PrefabChild {
    std::string                prefab;             // empty: inline entity
    const Prefab*              resolved = nullptr;
    Vec2                       offset;             // from its `Parent: { offset }` entry
    std::vector<ComponentData> components;         // overrides of the child prefab, or the inline entity
    std::vector<PrefabChild>   children;
};

struct Prefab {
    std::string                path;
    uint32_t                   key;                // prefabKey(path), used by NetPrefab
    std::vector<ComponentData> components;         // parsed values, already type-checked
    std::vector<PrefabChild>   children;
};

uint32_t prefabKey(std::string_view path);         // fnv1a64 folded to 32 bits; two prefabs with the same
                                                   // key are refused at load
class PrefabLibrary {
public:
    PrefabLibrary(const TypeRegistry& types, AssetManager& assets);
    Result<const Prefab*> get(std::string_view path);   // parses on first use, cached; children prefabs too
    const Prefab* findByKey(uint32_t key) const;        // replication spawn
    Entity instantiate(Registry& r, const Prefab& p, Vec2 position,
                       std::span<const ComponentData> overrides = {}); // creates the entity and its children
    void instantiateInto(Registry& r, Entity e, const Prefab& p, Vec2 position,
                         std::span<const ComponentData> overrides = {}); // same on an existing entity
    void clear();                                       // invalidates every Prefab pointer
};
```

Errors: unknown component with the closest name, unknown prefab, a prefab that includes itself,
`children` without the `Parent` component registered, all with `file:line`. `instantiate` sets the
`Position` component (a registered component named `Position` with float fields `x` and `y`) to
`position`, and each child's to `position + offset`, then attaches it with `Registry::setParent`.
Until ENG-25 registers the engine components, this lookup is by name.

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
