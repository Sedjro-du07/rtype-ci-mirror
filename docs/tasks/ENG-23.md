# ENG-23 · YAML wrappers and field codecs

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **04 Reflection** · **Wave 4** · status and owner live on the board.

## Goal

YAML reading and writing for any registered type, without yaml-cpp in game headers.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/reflection/Yaml.hpp`
- `src/reflection/Yaml.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/reflection/Yaml.hpp`

```cpp
class YamlNode {                              // read-only view; hides yaml-cpp from game headers
public:
    bool isMap() const; bool isSequence() const; bool isScalar() const; bool isNull() const;
    YamlNode operator[](std::string_view key) const;     // null node if absent
    YamlNode operator[](size_t index) const;
    size_t   size() const;
    std::vector<std::string> keys() const;
    template <class T> Result<T> as() const;              // bool, ints, float, double, std::string
    int      line() const;                                // 1-based, for errors
    const std::string& file() const;
};

class YamlWriter {
public:
    void beginMap(); void endMap(); void beginSequence(); void endSequence();
    void key(std::string_view k);
    template <class T> void value(const T& v);
    std::string str() const;
};

struct LoadContext {                              // Entity fields in files are uuids
    AssetManager& assets;
    std::function<Entity(const Uuid&)> entityOf;  // resolved in pass 2
    std::string file;
};
struct SaveContext {
    const AssetManager& assets;
    std::function<Uuid(Entity)> uuidOf;
};

Result<YamlNode> loadYamlFile(const std::filesystem::path& file);
Result<void>     fromYaml(const TypeInfo& type, void* obj, const YamlNode& node, LoadContext& ctx); // generic, per FieldKind
void             toYaml(const TypeInfo& type, const void* obj, YamlWriter& out, SaveContext& ctx);
```

## Depends on

- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- [ENG-07](ENG-07.md) — EngineConfig: YAML file and command-line overrides
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [ENG-41](ENG-41.md) — PrefabLibrary and instantiate
- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive

## Read first

- [ADR 0018](../adr/0018-data-in-yaml-behaviour-in-scripts.md)
- [ADR 0027](../adr/0027-reflection-by-name.md)

## To implement

- [ ] `YamlNode`, `YamlWriter`, `LoadContext` / `SaveContext` (Entity ↔ Uuid)
- [ ] Generic YAML codec per FieldKind: lists as sequences, structs as maps, enums by name

## Done when

- Tests: round trip of every field kind; error with file:line on wrong type

## Watch out

- Unknown key in a component → error with closest name

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
