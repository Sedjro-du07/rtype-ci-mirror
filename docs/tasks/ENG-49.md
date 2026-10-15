# ENG-49 · scripting_lua plugin: sol2 API, sandbox, reload

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **12 Services** · **Wave 6** · status and owner live on the board.

## Goal

Lua 5.4 + sol2 implementation of IScripting.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/scripting_lua/CMakeLists.txt`
- `plugins/scripting_lua/LuaScripting.hpp`
- `plugins/scripting_lua/LuaScripting.cpp`
- `plugins/scripting_lua/LuaApi.cpp`
- `plugins/scripting_lua/Sandbox.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp`; templates and `constexpr` functions go to a `.inl` next to the header. Behaviour of each method: the comments here and the pages in *Read first*.

### `plugins/scripting_lua/LuaScripting.hpp`

```cpp
class LuaScripting final : public IScripting {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "lua"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    Result<void>         bind(ScriptHost& host) override;
    Result<ScriptHandle> load(const std::filesystem::path& file) override;
    void                 unload(ScriptHandle s) override;
    Result<void>         reload(ScriptHandle s) override;
    Result<void>         call(ScriptHandle s, std::string_view function, Entity self, Context& ctx) override;
    bool                 has(ScriptHandle s, std::string_view function) const override;
private:
    sol::state                         lua_;
    std::vector<sol::environment>      scripts_;   // one sandboxed environment per script
    ScriptHost*                        host_ = nullptr;
};
ENGINE_PLUGIN(LuaScripting)
```

### `plugins/scripting_lua/LuaApi.cpp`

```cpp
// The Lua API of guide 11, built on reflection (no binding code per component).
void bindEntityApi(sol::state& lua, ScriptHost& host);   // self:get / set / has / remove / has_tag / id
void bindGlobalApi(sol::state& lua, ScriptHost& host);   // spawn, destroy, find, play, publish, time, cvar, log
sol::table componentToTable(sol::state& lua, const TypeInfo& type, const void* obj);
Result<void> tableToComponent(const TypeInfo& type, const sol::table& t, std::vector<std::byte>& out);
```

### `plugins/scripting_lua/Sandbox.cpp`

```cpp
sol::environment makeSandbox(sol::state& lua);          // base (minus load, dofile), math, string, table
void installInstructionLimit(lua_State* L, uint32_t maxInstructions);   // lua_sethook count hook
```

## Depends on

- [ENG-48](ENG-48.md) — IScripting, Script component, script systems
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- no task waits for this one

## Read first

- [ADR 0019](../adr/0019-lua-first-custom-language-optional.md)
- [guide 11](../engine/guide/11-scripting-lua.md)

## To implement

- [ ] API of guide 11 by reflection (components by name)
- [ ] Sandbox (no io, os, package, debug, load)
- [ ] Instruction limit hook
- [ ] Reload keeping the handle

## Done when

- The guide 11 boss script runs; an infinite loop is stopped

## Watch out

- No binding code per component type

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
