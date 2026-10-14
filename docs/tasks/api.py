"""Declarations (classes, methods, functions) each task must write, WITHOUT their implementation.

generate.py builds the "Classes and methods" section of every task page from two sources:
  1. automatically: the code blocks of the reference sections whose heading names one of the
     task's files (e.g. "## `class Registry` — `ecs/Registry.hpp`");
  2. this file: declarations that the reference describes only in prose or tables (plugins,
     systems, components, events, private classes, tests), and overrides.

API[id] = dict(auto=True|False, extra=[(file, language, code), ...], note="...")
  auto=False  → only the extra declarations are shown (used when the auto block is too broad).
  skip=[keys] → leave out these reference sections (replaced by more precise extra declarations).
"""

API = {
# ---------------------------------------------------------------- 00 Build
'ENG-01': dict(extra=[
    ('CMakeLists.txt', 'cmake', '''cmake_minimum_required(VERSION 3.25)
project(rtype_engine LANGUAGES CXX)
include(cmake/EngineOptions.cmake)
include(cmake/EngineModules.cmake)

add_library(engine STATIC|SHARED <sources of src/**>)
target_include_directories(engine PUBLIC include)
target_compile_features(engine PUBLIC cxx_std_23)
target_link_libraries(engine PUBLIC <third-party targets>)   # yaml-cpp stays PRIVATE
add_subdirectory(plugins/<each plugin>)
if(ENGINE_BUILD_TESTS)       add_subdirectory(tests)       endif()
if(ENGINE_BUILD_BENCHMARKS)  add_subdirectory(benchmarks)  endif()'''),
    ('cmake/EngineOptions.cmake', 'cmake', '''option(ENGINE_DYNAMIC_MODULES   "Plugins and games as runtime-loaded libraries" ON)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors"                      OFF)
option(ENGINE_SANITIZERS         "Build with address and undefined sanitizers"   OFF)
option(ENGINE_BUILD_TESTS        "Build engine/tests"                            ON)
option(ENGINE_BUILD_BENCHMARKS   "Build engine/benchmarks"                       OFF)

function(engine_set_warnings target)        # -Wall -Wextra -Wpedantic / /W4 (+ errors if enabled)'''),
]),
'ENG-03': dict(extra=[
    ('cmake/FeatureProbe.cpp', 'cpp', '''// One function per allowed feature (ADR 0017); compiled with try_compile, never linked.
std::expected<int, int>            probeExpected();
std::move_only_function<void()>    probeMoveOnlyFunction();
void                               probePrint();                 // std::print
struct ProbeDeducingThis { template <class Self> auto& get(this Self& self); };
int                                probeToUnderlying();          // std::to_underlying
[[noreturn]] void                  probeUnreachable();           // std::unreachable'''),
    ('cmake/EngineOptions.cmake', 'cmake', '''try_compile(ENGINE_HAS_STD_EXPECTED ... SOURCES cmake/FeatureProbe.cpp ...)
# if FALSE: target_compile_definitions(engine PUBLIC ENGINE_USE_TL_EXPECTED)'''),
]),
# ---------------------------------------------------------------- 01 Core
'ENG-04': dict(skip=['core/math/'], extra=[
    ('include/engine/core/Hash.hpp', 'cpp', '''constexpr uint64_t fnv1a64(std::string_view text) noexcept;   // compile-time hash for type, event and action keys'''),
    ('include/engine/core/math/Vec2.hpp', 'cpp', '''struct Vec2 {
    float x = 0, y = 0;
    constexpr Vec2 operator+(Vec2 o) const;   constexpr Vec2 operator-(Vec2 o) const;
    constexpr Vec2 operator*(float k) const;  constexpr Vec2 operator/(float k) const;
    constexpr Vec2& operator+=(Vec2 o);       constexpr Vec2& operator-=(Vec2 o);
    constexpr Vec2& operator*=(float k);
    constexpr bool operator==(const Vec2&) const = default;
    constexpr float dot(Vec2 o) const;
    float length() const;
    Vec2  normalized() const;                 // {0, 0} stays {0, 0}
    static constexpr Vec2 lerp(Vec2 a, Vec2 b, float t);
};'''),
    ('include/engine/core/math/Rect.hpp', 'cpp', '''struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    constexpr bool contains(Vec2 p) const;           // edges included on the left/top, excluded on the right/bottom
    constexpr bool intersects(const Rect& o) const;  // touching edges do not intersect
    constexpr Vec2 center() const;
    constexpr bool operator==(const Rect&) const = default;
};'''),
    ('include/engine/core/math/Color.hpp', 'cpp', '''struct Color {
    uint8_t r = 255, g = 255, b = 255, a = 255;
    static constexpr Color white(); static constexpr Color black();
    static constexpr Color red();   static constexpr Color green(); static constexpr Color blue();
    static constexpr Color transparent();
    constexpr bool operator==(const Color&) const = default;
};'''),
]),
# ---------------------------------------------------------------- 02 Modules
'ENG-15': dict(extra=[
    ('include/engine/module/NullPlugins.hpp', 'cpp', '''class NullRenderer final : public IRenderer {
public:
    Result<TextureHandle> createTexture(std::span<const std::byte>, std::string_view) override;  // valid handle, no decode
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;                                             // {0, 0}
    Result<FontHandle> createFont(std::span<const std::byte>, std::string_view) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame&) override;                                                   // ignores
    bool   supports(Feature) const override;                                                      // false
    void   setColorFilter(ColorFilter) override;
    RenderStats stats() const override;
};

class NullPlatform final : public IPlatform {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void> openWindow(const WindowDesc&) override;  void closeWindow() override;
    bool windowOpen() const override;                     // true until requestQuit
    void pollEvents(InputMap&, EventBus&) override;       // produces nothing
    IRenderer& renderer() override;                       // a NullRenderer member
    void setWindow(const WindowDesc&) override;
    void beginDebugUi() override; void endDebugUi() override;
    void setClipboard(std::string_view) override; std::string clipboard() const override;
private:
    NullRenderer renderer_;
};

class NullAudio final : public IAudio {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<SoundHandle> createSound(std::span<const std::byte>, std::string_view) override;  // valid handle
    void   destroySound(SoundHandle) override;
    Result<MusicHandle> openMusic(const std::filesystem::path&) override;
    void   closeMusic(MusicHandle) override;
    VoiceId play(SoundHandle, const PlayParams&) override;           // returns 0
    void    stop(VoiceId) override;
    void    setVoiceVolume(VoiceId, float) override;
    void    playMusic(MusicHandle, float) override;
    void    stopMusic(float) override;
    void    setBusVolume(AudioBus, float) override;                  // remembered, so busVolume() is consistent
    float   busVolume(AudioBus) const override;
    void    setListener(Vec2, float) override;
    void    update() override;
private:
    std::array<float, 4> volumes_{1, 1, 1, 1};
};

class NullNetwork final : public INetwork {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void> openUdp(uint16_t) override;                         // succeeds, nothing bound
    void closeUdp() override;
    void sendUdp(const Endpoint&, std::span<const std::byte>) override;   // drops
    void setReceiver(std::move_only_function<void(Datagram&&)>) override; // never called
    Result<void> listenTcp(uint16_t) override;
    Result<uint32_t> connectTcp(const Endpoint&) override;           // NetworkError "offline"
    void sendTcp(uint32_t, std::span<const std::byte>) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)>) override;
    void closeTcp(uint32_t) override;
};

class NullScripting final : public IScripting {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "null"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext&) override; void shutdown() noexcept override;
    Result<void>         bind(ScriptHost&) override;                 // succeeds
    Result<ScriptHandle> load(const std::filesystem::path&) override;   // ScriptError "scripting disabled"
    void                 unload(ScriptHandle) override;
    Result<void>         reload(ScriptHandle) override;
    Result<void>         call(ScriptHandle, std::string_view, Entity, Context&) override;
    bool                 has(ScriptHandle, std::string_view) const override;   // false
};

std::unique_ptr<IPlugin> makeNullPlugin(PluginKind kind);   // used by ModuleManager for the name "null"'''),
]),
'ENG-07': dict(extra=[
    ('include/engine/EngineConfig.hpp', 'cpp', '''struct RoomsConfig {                      // server.yaml "rooms:" (ADR 0038)
    std::string host = "shared";          // "dedicated" in Part 2
    uint32_t    maxRooms = 8;
    uint32_t    workerThreads = 0;        // 0 = number of cores
};'''),
]),
'ENG-13': dict(extra=[
    ('include/engine/module/StaticModuleTable.hpp', 'cpp', '''struct PluginEntry { std::string_view stem; StaticModuleTable::CreatePlugin create; StaticModuleTable::DestroyPlugin destroy; };
struct GameEntry   { std::string_view name; StaticModuleTable::CreateGame create;   StaticModuleTable::DestroyGame destroy; };'''),
]),
# ---------------------------------------------------------------- 03 ECS
'ENG-21': dict(auto=False, extra=[
    ('include/engine/ecs/Registry.hpp', 'cpp', '''// The hook part of Registry (the rest belongs to ENG-18).
struct ComponentHooks {
    Signal<void(Registry&, Entity)> onCreate;   // after the component is added
    Signal<void(Registry&, Entity)> onReplace;  // after replace()
    Signal<void(Registry&, Entity)> onDestroy;  // before the component is removed
};

class Registry {
public:
    template <class T> ComponentHooks& hooks();
    ComponentHooks& hooks(ComponentId id);
private:
    std::vector<ComponentHooks> hooks_;         // indexed by ComponentId
    void fireCreate(ComponentId, Entity);       // called by emplace / emplaceRaw / Commands::apply
    void fireReplace(ComponentId, Entity);      // called by replace
    void fireDestroy(ComponentId, Entity);      // called by remove / removeRaw / destroy, before removal
};'''),
]),
# ---------------------------------------------------------------- 04 Reflection
'ENG-22': dict(extra=[
    ('include/engine/reflection/TypeBuilder.hpp', 'cpp', '''using FromYamlFn = Result<void> (*)(void* obj, const YamlNode& node, LoadContext& ctx);
using ToYamlFn   = void         (*)(const void* obj, YamlWriter& out, SaveContext& ctx);
using EncodeFn   = void         (*)(const void* obj, ByteWriter& out, const WireContext& wire);
using DecodeFn   = Result<void> (*)(void* obj, ByteReader& in, const WireContext& wire);'''),
    ('include/engine/reflection/TypeInfo.hpp', 'cpp', '''struct EnumInfo { std::vector<std::pair<std::string, int64_t>> values; };   // name ↔ value, filled by ENGINE_ENUM
struct WireContext {                                   // Entity ↔ NetworkId for the binary codecs (block 10)
    std::function<uint32_t(Entity)> networkIdOf;       // 0 if not replicated
    std::function<Entity(uint32_t)> entityOf;          // Entity::null() if unknown
};'''),
]),
'ENG-23': dict(extra=[
    ('include/engine/reflection/Yaml.hpp', 'cpp', '''class YamlNode {                              // read-only view; hides yaml-cpp from game headers
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
void             toYaml(const TypeInfo& type, const void* obj, YamlWriter& out, SaveContext& ctx);'''),
]),
'ENG-25': dict(extra=[
    ('include/engine/components/*.hpp', 'cpp', '''struct Position         { float x = 0, y = 0; };                          ENGINE_COMPONENT(Position)
struct PreviousPosition { float x = 0, y = 0; };                          ENGINE_COMPONENT(PreviousPosition)
struct Velocity         { float dx = 0, dy = 0; };                        ENGINE_COMPONENT(Velocity)
struct Sprite           { TextureHandle texture; Rect frame; int16_t layer = 0; Color tint;
                          Vec2 scale{1, 1}; Vec2 origin; float rotation = 0; bool visible = true; };
                                                                          ENGINE_COMPONENT(Sprite)
struct Animation        { std::vector<Rect> frames; float fps = 10; bool loop = true;
                          float time = 0; uint32_t current = 0; };        ENGINE_COMPONENT(Animation)
struct Text             { std::string value; FontHandle font; uint16_t size = 16; Color color;
                          int16_t layer = 100; bool screenSpace = true; };  ENGINE_COMPONENT(Text)
struct Collider         { Vec2 size; Vec2 offset; uint32_t layer = 1; uint32_t mask = ~0u; bool trigger = false; };
                                                                          ENGINE_COMPONENT(Collider)
struct Lifetime         { float seconds = 0; };                           ENGINE_COMPONENT(Lifetime)
struct Parallax         { float factor = 1; float wrapWidth = 0; };       ENGINE_COMPONENT(Parallax)
// Identity.hpp
struct Name             { std::string value; };                           ENGINE_COMPONENT(Name)
struct Uuid             { uint64_t hi = 0, lo = 0; static Uuid generate(); bool operator==(const Uuid&) const = default; };
                                                                          ENGINE_COMPONENT(Uuid)
struct Tags             { std::vector<uint64_t> ids; bool has(std::string_view tag) const; };
                                                                          ENGINE_COMPONENT(Tags)
// Hierarchy.hpp
struct Parent           { Entity value; Vec2 offset; };                   ENGINE_COMPONENT(Parent)
struct Children         { std::vector<Entity> list; };                    ENGINE_COMPONENT(Children)
// Script.hpp
struct Script           { ScriptHandle handle; std::string file; };       ENGINE_COMPONENT(Script)'''),
    ('src/systems/RegisterEngineTypes.cpp', 'cpp', '''void registerEngineTypes(TypeRegistry& types);   // every component above with fields and flags,
                                                  // net components (block 10), engine events (block 6)'''),
]),
# ---------------------------------------------------------------- 06 Events
'ENG-27': dict(extra=[
    ('include/engine/events/EngineEvents.hpp', 'cpp', '''struct QuitRequested      {};                                           ENGINE_EVENT(QuitRequested)
struct WindowResized      { uint32_t width, height; };                    ENGINE_EVENT(WindowResized)
struct WindowFocusChanged { bool focused; };                              ENGINE_EVENT(WindowFocusChanged)
struct Collision          { Entity a, b; };                               ENGINE_EVENT(Collision)
struct SceneLoaded        { std::string name; };                          ENGINE_EVENT(SceneLoaded)
struct SceneUnloading     { std::string name; };                          ENGINE_EVENT(SceneUnloading)
struct PlayerJoined       { PlayerId player; };                           ENGINE_EVENT(PlayerJoined)
struct PlayerLeft         { PlayerId player; DisconnectReason reason; };  ENGINE_EVENT(PlayerLeft)
struct JoinedServer       { PlayerId me; };                               ENGINE_EVENT(JoinedServer)
struct LeftServer         { DisconnectReason reason; };                   ENGINE_EVENT(LeftServer)
struct Caption            { std::string text; float seconds; };           ENGINE_EVENT(Caption)
struct ConsoleCommand     { std::string line; };                          ENGINE_EVENT(ConsoleCommand)'''),
]),
# ---------------------------------------------------------------- 07 Input
'ENG-28': dict(extra=[
    ('include/engine/input/Keys.hpp', 'cpp', '''std::string_view keyName(Key k);                         // label of the current keyboard layout, for display
std::optional<Key>           keyFromName(std::string_view name);      // input.yaml ↔ enum
std::optional<GamepadButton> gamepadButtonFromName(std::string_view name);
std::optional<GamepadAxis>   gamepadAxisFromName(std::string_view name);
std::string_view             toName(Key k);  std::string_view toName(GamepadButton b);  std::string_view toName(GamepadAxis a);'''),
]),
# ---------------------------------------------------------------- 08 Rendering
'ENG-30': dict(extra=[
    ('include/engine/render/IRenderer.hpp', 'cpp', '''struct RenderStats { uint32_t commands = 0, batches = 0, drawCalls = 0; double submitMs = 0; };'''),
]),
'ENG-31': dict(extra=[
    ('plugins/platform_sfml/SfmlPlatform.hpp', 'cpp', '''class SfmlPlatform final : public IPlatform {
public:
    // IPlugin
    PluginKind kind() const override;                 // Platform
    std::string_view name() const override;           // "sfml"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;
    void shutdown() noexcept override;
    // IPlatform
    Result<void> openWindow(const WindowDesc& desc) override;
    void closeWindow() override;
    bool windowOpen() const override;
    void pollEvents(InputMap& input, EventBus& events) override;
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override;                     // ENG-32
    void endDebugUi() override;                       // ENG-32
    void setClipboard(std::string_view text) override;
    std::string clipboard() const override;
private:
    sf::RenderWindow window_;
    SfmlRenderer     renderer_;
    WindowDesc       desc_;
};
ENGINE_PLUGIN(SfmlPlatform)'''),
    ('plugins/platform_sfml/SfmlRenderer.hpp', 'cpp', '''class SfmlRenderer final : public IRenderer {
public:
    explicit SfmlRenderer(sf::RenderWindow& window);
    Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) override;
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;
    Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame& frame) override;   // sort by layer, batch by texture, draw, present
    bool   supports(Feature f) const override;
    void   setColorFilter(ColorFilter f) override;      // ENG-33
    RenderStats stats() const override;
private:
    void applyView(const Camera2D& camera);             // virtual resolution + letterbox
    void flushBatch();                                  // one sf::VertexArray draw
    sf::RenderWindow&                          window_;
    std::vector<std::unique_ptr<sf::Texture>>  textures_;   // index = handle id - 1
    std::vector<std::unique_ptr<sf::Font>>     fonts_;
    sf::VertexArray                            batch_;
    RenderStats                                stats_;
};'''),
    ('plugins/platform_sfml/SfmlKeys.cpp', 'cpp', '''Key           toKey(sf::Keyboard::Scancode code);
MouseButton   toMouseButton(sf::Mouse::Button button);
GamepadButton toGamepadButton(unsigned int sfmlButton);
GamepadAxis   toGamepadAxis(sf::Joystick::Axis axis);
std::optional<RawInputEvent> toRawInputEvent(const sf::Event& event, const WindowDesc& desc);'''),
    ('plugins/platform_sfml/CMakeLists.txt', 'cmake', '''engine_add_plugin(platform_sfml SOURCES SfmlPlatform.cpp SfmlRenderer.cpp SfmlKeys.cpp SfmlImGui.cpp
                  DEPENDS SFML::Graphics ImGui-SFML::ImGui-SFML implot)'''),
]),
'ENG-32': dict(extra=[
    ('plugins/platform_sfml/SfmlImGui.cpp', 'cpp', '''// Implements the debug-UI part of SfmlPlatform.
Result<void> initImGui(sf::RenderWindow& window);      // ImGui::SFML::Init + ImPlot::CreateContext
void         shutdownImGui();
void         SfmlPlatform::beginDebugUi();             // ImGui::SFML::Update
void         SfmlPlatform::endDebugUi();               // ImGui::SFML::Render, after submit()
bool         imguiWantsInput();                        // pollEvents skips InputMap::feed when true'''),
]),
'ENG-33': dict(extra=[
    ('plugins/platform_sfml/ColorFilter.frag', 'glsl', '''uniform sampler2D texture;
uniform int       mode;          // 0 none, 1 protanopia, 2 deuteranopia, 3 tritanopia, 4 high contrast
uniform float     strength;      // 0..1
vec3 simulate(vec3 rgb, int mode);
vec3 daltonize(vec3 rgb, int mode);
void main();'''),
    ('plugins/platform_sfml/SfmlRenderer.hpp', 'cpp', '''void SfmlRenderer::setColorFilter(ColorFilter f) override;   // selects the shader mode
bool SfmlRenderer::supports(Feature f) const override;        // ColorFilter: sf::Shader::isAvailable()
// private:
sf::Shader        filter_;
sf::RenderTexture offscreen_;                                 // the frame is drawn here, then through the shader
float             uiScale_ = 1.f;                             // applied to screenSpace commands'''),
]),
# ---------------------------------------------------------------- 09 Audio
'ENG-34': dict(extra=[
    ('src/audio/AudioCaptions.cpp', 'cpp', '''// Decorator placed between Context::audio() and the plugin: every backend gets captions.
class CaptionedAudio final : public IAudio {
public:
    CaptionedAudio(IAudio& backend, EventBus& events);
    VoiceId play(SoundHandle s, const PlayParams& p) override;   // publishes Caption{p.caption} if not empty, then forwards
    // every other IAudio / IPlugin method forwards to backend_
private:
    IAudio&   backend_;
    EventBus& events_;
};'''),
]),
'ENG-35': dict(extra=[
    ('plugins/audio_miniaudio/MiniaudioAudio.hpp', 'cpp', '''class MiniaudioAudio final : public IAudio {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "miniaudio"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;      // ma_engine_init, one ma_sound_group per bus
    void shutdown() noexcept override;
    Result<SoundHandle> createSound(std::span<const std::byte> encoded, std::string_view debugName) override;
    void   destroySound(SoundHandle) override;
    Result<MusicHandle> openMusic(const std::filesystem::path& file) override;
    void   closeMusic(MusicHandle) override;
    VoiceId play(SoundHandle s, const PlayParams& p) override;
    void    stop(VoiceId v) override;
    void    setVoiceVolume(VoiceId v, float volume) override;
    void    playMusic(MusicHandle m, float fadeInSeconds) override;
    void    stopMusic(float fadeOutSeconds) override;
    void    setBusVolume(AudioBus bus, float volume) override;
    float   busVolume(AudioBus bus) const override;
    void    setListener(Vec2 position, float hearingRadius) override;
    void    update() override;                                 // recycles finished voices
private:
    ma_engine                                   engine_;
    std::array<ma_sound_group, 4>               buses_;
    std::vector<ma_decoder>                     sounds_;        // decoded once, index = handle id - 1
    std::unordered_map<VoiceId, ma_sound>       voices_;
};
ENGINE_PLUGIN(MiniaudioAudio)'''),
]),
# ---------------------------------------------------------------- 10 Network
'ENG-37': dict(extra=[
    ('plugins/network_asio/AsioNetwork.hpp', 'cpp', '''class AsioNetwork final : public INetwork {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "asio"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override;       // starts the I/O thread
    void shutdown() noexcept override;                          // stops io_context, joins the thread
    // UDP
    Result<void> openUdp(uint16_t port) override;
    void closeUdp() override;
    void sendUdp(const Endpoint& to, std::span<const std::byte> bytes) override;
    void setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) override;
    // TCP
    Result<void> listenTcp(uint16_t port) override;
    Result<uint32_t> connectTcp(const Endpoint& to) override;
    void sendTcp(uint32_t stream, std::span<const std::byte> bytes) override;
    void setTcpReceiver(std::move_only_function<void(uint32_t, std::span<const std::byte>)> fn) override;
    void closeTcp(uint32_t stream) override;
private:
    void startReceive();                                        // async_receive_from loop
    void startAccept();                                         // async_accept loop
    void startRead(uint32_t stream);                            // async_read_some loop
    asio::io_context                                     io_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_;
    std::thread                                          thread_;
    asio::ip::udp::socket                                udp_;
    std::optional<asio::ip::tcp::acceptor>               acceptor_;
    std::unordered_map<uint32_t, asio::ip::tcp::socket>  streams_;
};
ENGINE_PLUGIN(AsioNetwork)'''),
]),
'ENG-38': dict(note='The class below is shared: ENG-38 writes the core (constructor, attach, poll, sendRaw, isServer, connected, stats); SRV-01 the player methods; SRV-03 the message methods.'),
'ENG-39': dict(extra=[
    ('src/systems/NetSystems.cpp', 'cpp', '''void netAssignIds(Context& ctx);       // "engine.net.assign_ids"     LateUpdate, server
void netSnapshot(Context& ctx);        // "engine.net.snapshot"       LateUpdate, server
void netApplySnapshot(Context& ctx);   // "engine.net.apply_snapshot" FixedUpdate (first), client
void netInterpolate(Context& ctx);     // "engine.net.interpolate"    Render (first), client
void netSendInput(Context& ctx);       // "engine.net.send_input"     LateUpdate, client
void netReceiveInput(Context& ctx);    // "engine.net.receive_input"  FixedUpdate (first), server
void registerNetSystems(Scheduler& s); // called by registerEngineSystems'''),
]),
# ---------------------------------------------------------------- 11 Scenes & assets
'ENG-41': dict(extra=[
    ('include/engine/scene/Prefab.hpp', 'cpp', '''struct ComponentData {                 // one parsed component value, ready to copy into a registry
    ComponentId            id;
    std::vector<std::byte> value;      // constructed with TypeInfo::construct, filled by fromYaml
    int                    line = 0;   // for error messages
};
struct PrefabChild {
    std::string                prefab;      // nested prefab path, or empty for an inline entity
    std::vector<ComponentData> components;  // inline components or overrides of the nested prefab
    Vec2                       offset;      // local offset from the parent
};'''),
]),
# ---------------------------------------------------------------- 12 Services
'ENG-44': dict(extra=[
    ('src/services/Logger.cpp', 'cpp', '''class ConsoleSink final : public ILogSink {          // coloured stdout (Info and below) / stderr (Warn, Error)
public:
    explicit ConsoleSink(bool color);
    void write(const LogRecord& r) override;
    void flush() override;
};
class FileSink final : public ILogSink {             // rotating file
public:
    FileSink(std::filesystem::path file, size_t maxBytes, int keepFiles);
    void write(const LogRecord& r) override;
    void flush() override;
};
class ConsoleUiSink final : public ILogSink {        // feeds the developer console
public:
    explicit ConsoleUiSink(Console& console);
    void write(const LogRecord& r) override;
};'''),
]),
'ENG-45': dict(extra=[
    ('include/engine/services/CVars.hpp', 'cpp', '''struct CVarInfo {                      // read-only view for the console and the archive
    std::string_view name, description;
    uint32_t         flags;
    std::string      value;            // current value as text
    std::string      defaultValue;
};'''),
]),
'ENG-47': dict(extra=[
    ('include/engine/services/Metrics.hpp', 'cpp', '''struct MetricsSnapshot {
    double   frameMs, fps;
    std::vector<std::pair<std::string_view, double>> systemMs;   // per system, last frame
    size_t   entities;
    uint32_t drawCalls;
    double   rttMs, lossPercent, kbIn, kbOut;                     // network, 0 offline
};'''),
    ('src/systems/DebugOverlaySystem.cpp', 'cpp', '''void debugOverlay(Context& ctx);   // "engine.debug_overlay": Render, client and editor, last
// private helpers
void drawConsole(Console& console);
void drawFrameGraph(const Metrics& metrics);
void drawSystemTimings(std::span<const SystemStats> stats);
void drawLagometer(const NetworkSession& net);'''),
]),
'ENG-48': dict(extra=[
    ('src/script/ScriptSystems.cpp', 'cpp', '''void scriptLifecycle(Context& ctx);   // "engine.script.lifecycle": on_spawn once; on_destroy through the Script onDestroy hook
void scriptUpdate(Context& ctx);      // "engine.script.update":    on_update(self, dt) for every entity with Script
void scriptCollision(Context& ctx);   // "engine.script.collision": on_collision(self, other) from Collision events
void registerScriptSystems(Scheduler& s, IScripting& scripting);'''),
]),
'ENG-49': dict(extra=[
    ('plugins/scripting_lua/LuaScripting.hpp', 'cpp', '''class LuaScripting final : public IScripting {
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
ENGINE_PLUGIN(LuaScripting)'''),
    ('plugins/scripting_lua/LuaApi.cpp', 'cpp', '''// The Lua API of guide 11, built on reflection (no binding code per component).
void bindEntityApi(sol::state& lua, ScriptHost& host);   // self:get / set / has / remove / has_tag / id
void bindGlobalApi(sol::state& lua, ScriptHost& host);   // spawn, destroy, find, play, publish, time, cvar, log
sol::table componentToTable(sol::state& lua, const TypeInfo& type, const void* obj);
Result<void> tableToComponent(const TypeInfo& type, const sol::table& t, std::vector<std::byte>& out);'''),
    ('plugins/scripting_lua/Sandbox.cpp', 'cpp', '''sol::environment makeSandbox(sol::state& lua);          // base (minus load, dofile), math, string, table
void installInstructionLimit(lua_State* L, uint32_t maxInstructions);   // lua_sethook count hook'''),
]),
# ---------------------------------------------------------------- 13 Engine systems
'ENG-50': dict(extra=[
    ('src/systems/MovementSystems.cpp', 'cpp', '''void previousPosition(Context& ctx);   // "engine.previous_position" FixedUpdate, all
void movement(Context& ctx);           // "engine.movement"          FixedUpdate, all
void hierarchy(Context& ctx);          // "engine.hierarchy"         FixedUpdate, all
void parallax(Context& ctx);           // "engine.parallax"          FixedUpdate, client
void lifetime(Context& ctx);           // "engine.lifetime"          FixedUpdate, all'''),
    ('src/systems/RegisterEngineSystems.cpp', 'cpp', '''void registerEngineSystems(Scheduler& s, Role role);   // every engine system with its name, stage, roles and order (reference 13)'''),
]),
'ENG-51': dict(extra=[
    ('src/systems/CollisionSystem.cpp', 'cpp', '''void collision(Context& ctx);   // "engine.collision" FixedUpdate, all, after engine.hierarchy

class UniformGrid {             // private to this file
public:
    explicit UniformGrid(float cellSize);
    void clear();
    void insert(Entity e, const Rect& box);
    template <class Fn> void forEachPair(Fn&& fn) const;   // each candidate pair once
};
bool layersMatch(const Collider& a, const Collider& b);    // (a.layer & b.mask) && (b.layer & a.mask)
Rect worldBox(const Position& p, const Collider& c);'''),
]),
'ENG-52': dict(extra=[
    ('src/systems/RenderSystems.cpp', 'cpp', '''void renderSprites(Context& ctx);   // "engine.render.sprites" Render, client/editor: Position lerped with alpha
void renderText(Context& ctx);      // "engine.render.text"    Render, after sprites
void renderDebug(Context& ctx);     // "engine.render.debug"   Render, when r_debug_colliders'''),
    ('src/systems/AnimationSystem.cpp', 'cpp', '''void animation(Context& ctx);       // "engine.animation" FixedUpdate, client/editor: advances Animation, writes Sprite.frame'''),
]),
# ---------------------------------------------------------------- 14 Tests & bench
'ENG-53': dict(extra=[
    ('tests/', 'text', '''tests/CMakeLists.txt          engine_add_test(<name> SOURCES ...) helper, test framework (chosen in Tooling)
tests/fixtures/TestPlugin.cpp  class TestPlatform : public IPlatform { ... };  ENGINE_PLUGIN(TestPlatform)
tests/fixtures/TestGame.cpp    class TestGame : public IGameModule { ... };    ENGINE_GAME_MODULE(TestGame)
tests/core/   HashTest, MathTest, ErrorTest, TimeTest, SignalTest, EngineConfigTest
tests/module/ DynamicLibraryTest, StaticModuleTableTest, ModuleManagerTest (version mismatch, wrong kind, null, unload order)'''),
]),
'ENG-54': dict(extra=[
    ('tests/', 'text', '''tests/ecs/        EntityAllocatorTest, SparseSetTest, RegistryTest, ViewTest, CommandsTest, HooksTest
tests/reflection/ TypeRegistryTest (each category, DuplicateName, fingerprint), BuilderCompileFailTest,
                  YamlRoundTripTest (every FieldKind), ByteStreamTest'''),
]),
'ENG-55': dict(extra=[
    ('tests/', 'text', '''tests/scheduler/ OrderingTest, CycleTest, FailurePolicyTest (server disables, critical ends, client code 5)
tests/events/    DispatchOrderTest, PublishDuringDispatchTest, ThreadInboxTest
tests/input/     EdgeLatchTest, HoldToToggleTest, AutoRepeatTest, DeadzoneTest, MaskTest
tests/scene/     TwoPassLoadTest, UnknownComponentTest, SwitchPersistentTest, SaveRoundTripTest
tests/net/       MalformedDatagramTest, OversizedDatagramTest, TimeoutTest (session core only)'''),
]),
'ENG-56': dict(extra=[
    ('benchmarks/ecs_storage/', 'cpp', '''template <class T> class PagedSparseSet;   // the engine's SparseSet<T>
template <class T> class SparseArray;      // plain std::vector<std::optional<T>> by entity index
class MinimalArchetypeStore;               // one table per component combination
void benchIterate1(benchmark::State&);  void benchIterate2(benchmark::State&);  void benchIterate3(benchmark::State&);
void benchAddRemove(benchmark::State&);'''),
]),
'ENG-57': dict(extra=[
    ('tests/headless/', 'text', '''tests/headless/CMakeLists.txt   add_test(NAME headless_smoke COMMAND r-type_test_client --platform null --audio null --ticks 1000)
tests/headless/config.yaml      role: client, game: testgame, scripting: ""'''),
]),
'ENG-58': dict(extra=[
    ('plugins/platform_raylib/RaylibPlatform.hpp', 'cpp', '''class RaylibPlatform final : public IPlatform {
public:
    PluginKind kind() const override; std::string_view name() const override;   // "raylib"
    uint32_t apiVersion() const override;
    Result<void> init(const PluginContext& ctx) override; void shutdown() noexcept override;
    Result<void> openWindow(const WindowDesc& desc) override;   // InitWindow
    void closeWindow() override;
    bool windowOpen() const override;                           // !WindowShouldClose()
    void pollEvents(InputMap& input, EventBus& events) override;// raylib polling (IsKeyPressed…) → RawInputEvent
    IRenderer& renderer() override;
    void setWindow(const WindowDesc& desc) override;
    void beginDebugUi() override; void endDebugUi() override;   // rlImGui
    void setClipboard(std::string_view) override; std::string clipboard() const override;
private:
    RaylibRenderer renderer_;
    std::array<bool, 512> previousKeys_{};                      // to turn polling into edges
};
ENGINE_PLUGIN(RaylibPlatform)'''),
    ('plugins/platform_raylib/RaylibRenderer.hpp', 'cpp', '''class RaylibRenderer final : public IRenderer {
public:
    Result<TextureHandle> createTexture(std::span<const std::byte> encodedImage, std::string_view debugName) override;
    void   destroyTexture(TextureHandle) override;
    Vec2   textureSize(TextureHandle) const override;
    Result<FontHandle> createFont(std::span<const std::byte> fontFile, std::string_view debugName) override;
    void   destroyFont(FontHandle) override;
    void   submit(const RenderFrame& frame) override;           // BeginDrawing / DrawTexturePro / EndDrawing
    bool   supports(Feature f) const override;                  // Render3D: true
    void   setColorFilter(ColorFilter f) override;
    RenderStats stats() const override;
private:
    std::vector<Texture2D> textures_;
    std::vector<Font>      fonts_;
};'''),
]),
'ENG-59': dict(note='Part 2: the editor classes are designed when Track 1 opens. Expected pieces: an Editor role, a hierarchy panel, an inspector driven by TypeRegistry, scene save through SceneLoader::save.'),
# ================================================================ SERVER
'SRV-01': dict(auto=False, extra=[
    ('src/net/PlayerTable.hpp', 'cpp', '''class PlayerTable {
public:
    PlayerId add(ConnectionId c, std::string name);       // next free id, never 0
    void     remove(PlayerId p);
    PlayerId playerOf(ConnectionId c) const;              // 0 if unknown
    ConnectionId connectionOf(PlayerId p) const;          // 0 if unknown
    std::string_view name(PlayerId p) const;
    std::span<const PlayerId> players() const;
    size_t   size() const;
private:
    std::vector<PlayerId>                       order_;
    std::unordered_map<PlayerId, ConnectionId>  toConnection_;
    std::unordered_map<ConnectionId, PlayerId>  toPlayer_;
    std::unordered_map<PlayerId, std::string>   names_;
    PlayerId                                    next_ = 1;
};'''),
    ('include/engine/net/NetworkSession.hpp', 'cpp', '''// The player part of NetworkSession (core: ENG-38, messages: SRV-03).
class NetworkSession {
public:
    Result<void> startServer(std::optional<uint16_t> port = {});
    void stopServer();
    Result<void> connectTo(std::string_view host, uint16_t port, std::string_view playerName);
    template <class T> Result<void> connectTo(std::string_view host, uint16_t port,
                                              std::string_view playerName, const T& admissionData);
    void disconnect();
    std::span<const PlayerId> players() const;
    PlayerId         localPlayer() const;
    std::string_view name(PlayerId p) const;
    void             kick(PlayerId p, std::string_view reason);    // Kicked{message} (RFC §6.3.6)
    double           ping(PlayerId p) const;
    ConnectionStats  stats(PlayerId p) const;
private:
    PlayerTable players_;
    PlayerId    localPlayer_ = 0;
    void onPlayerAccepted(ConnectionId c, std::string name);       // publishes PlayerJoined / JoinedServer
    void onConnectionClosed(ConnectionId c, DisconnectReason r, std::string message);
                                                                   // PlayerLeft / LeftServer with message;
                                                                   // ConnectionLost / Timeout go to SRV-16 (grace)
};'''),
]),
'SRV-02': dict(auto=False, extra=[
    ('src/net/Handshake.hpp', 'cpp', '''enum class HandshakeState : uint8_t { Idle, Hello, Challenge, LinkUdp, Accepted, Refused };

struct ServerIdentity { uint64_t fingerprint; std::string gameName; };
using AdmissionFn = std::move_only_function<Admission(const AdmissionRequest&, uint16_t dataId,
                                                       std::span<const std::byte> data)>;   // set by admission<T>

class ServerHandshake {                       // one per pending connection (RFC §6.1–6.4)
public:
    ServerHandshake(const NetConfig& cfg, const ServerIdentity& self, double startedAt);
    bool         onPreamble(std::span<const std::byte, 3> bytes);   // false → Refused(ProtocolMismatch)
    // Checks of RFC §6.4.1 in order; a valid reconnection token is returned for SRV-16 to resume.
    Result<void> onHello(ByteReader& in, size_t playersNow, AdmissionFn* admission,
                         std::function<bool(const ReconnectToken&)> isDisconnected);
    void         writeChallenge(ByteWriter& out);       // random linkToken (OS secure random)
    bool         onUdpLink(uint64_t linkToken, const Endpoint& from);   // token matches → Accepted
    void         writeAccepted(ByteWriter& out, PlayerId id, uint32_t sessionKey, const ReconnectToken& next, bool resumed);
    void         writeRefused(ByteWriter& out) const;   // frozen layout (RFC §4.1)
    bool         expired(double now) const;             // net.handshakeSeconds
    HandshakeState state() const;
    DisconnectReason refusal() const; std::string_view refusalMessage() const;
    const std::string& playerName() const;
    const Endpoint& udpEndpoint() const;
    std::optional<ReconnectToken> resumeToken() const;
};

class ClientHandshake {
public:
    ClientHandshake(std::string playerName, uint64_t fingerprint, std::string gameName,
                    std::optional<ReconnectToken> token, uint16_t dataId, std::vector<std::byte> data);
    void         writePreambleAndHello(ByteWriter& out);
    Result<void> onChallenge(ByteReader& in);   // keeps the link token
    void         writeUdpLink(ByteWriter& out); // 24-byte UDP packet, every 100 ms until Accepted
    Result<void> onResult(ByteReader& in);      // Accepted (id, session key, token, timings) or Refused (reason, message)
    HandshakeState state() const;
    PlayerId me() const; uint32_t sessionKey() const; const ReconnectToken& nextToken() const;
    DisconnectReason refusal() const; std::string_view refusalMessage() const;
};

class PendingLimiter {                        // RFC §11: 16 pending, 4 per address
public:
    explicit PendingLimiter(uint32_t maxPending, uint32_t perAddress = 4);
    bool tryAdd(const Endpoint& from);  void remove(const Endpoint& from);
};'''),
]),
'SRV-03': dict(auto=False, extra=[
    ('src/net/MessageDispatcher.hpp', 'cpp', '''class MessageDispatcher {
public:
    MessageDispatcher(const TypeRegistry& types, Logger& log);
    uint16_t        idOf(uint64_t typeKey) const;                 // position in TypeRegistry::messages()
    const TypeInfo* typeOf(uint16_t id) const;                    // nullptr → dropped and counted
    Subscription    add(uint64_t typeKey, std::move_only_function<void(Context&, PlayerId, const void*)> fn);
    void            dispatch(Context& ctx, PlayerId from, uint16_t id, ByteReader& payload, const WireContext& wire);
                                                                  // decode with Codec, call every callback, catch exceptions
private:
    std::vector<std::vector<Callback>> callbacks_;                // indexed by message id
};'''),
    ('include/engine/net/NetworkSession.hpp', 'cpp', '''// The message part of NetworkSession (core: ENG-38, players: SRV-01).
class NetworkSession {
public:
    template <class T> void send(PlayerId to, const T& msg);
    template <class T> void send(const T& msg);
    template <class T> void broadcast(const T& msg, std::optional<PlayerId> except = {});
    template <class T> [[nodiscard]] Subscription on(std::move_only_function<void(Context&, PlayerId, const T&)> fn);
    template <class T> [[nodiscard]] Subscription on(std::move_only_function<void(Context&, const T&)> fn);
private:
    MessageDispatcher dispatcher_;
    void sendEncoded(PlayerId to, const TypeInfo& type, std::span<const std::byte> payload);  // picks UDP / reliable UDP / TCP
};'''),
]),
'SRV-04': dict(extra=[
    ('src/reflection/Codec.hpp', 'cpp', '''// Uses WireContext (Entity <-> NetworkId), declared in reflection/TypeInfo.hpp by ENG-22.
namespace codec {
void         encode(const TypeInfo& type, const void* obj, ByteWriter& out, const WireContext& wire);
Result<void> decode(const TypeInfo& type, void* obj, ByteReader& in, const WireContext& wire);   // obj default-constructed
void         encodeField(const FieldInfo& f, const std::byte* field, ByteWriter& out, const WireContext& wire);
Result<void> decodeField(const FieldInfo& f, std::byte* field, ByteReader& in, const WireContext& wire);
uint64_t     fingerprint(std::span<const TypeInfo* const> types);    // used by TypeRegistry::formatFingerprint
}'''),
]),
'SRV-05': dict(auto=False, extra=[
    ('src/net/ReliableChannel.hpp', 'cpp', '''// RFC §7: packet sequence + ack/ackBits on every Data packet, reliable messages ordered per direction.
struct Delivered { uint16_t messageId; std::vector<std::byte> payload; };

class ReliableChannel {                    // one per session
public:
    uint16_t nextSequence();                                         // numbers an outgoing Data packet, from 1
    void     queue(uint16_t messageId, std::span<const std::byte> payload);   // ≤ 1024 bytes (RFC §4.4)
    void     fill(uint16_t sequence, ByteWriter& out, double now);   // due resends then new messages, recorded per packet
    void     writeAcks(uint16_t& ack, uint32_t& ackBits) const;
    bool     onPacket(uint16_t sequence, uint16_t ack, uint32_t ackBits, double now); // false: duplicate or > 32 behind
    void     onReliable(uint16_t messageSeq, uint16_t messageId, std::vector<std::byte> payload);
    std::vector<Delivered> deliverable();                            // in messageSeq order, each once
    std::vector<uint16_t>  newlyAcked();                             // acknowledged packet sequences (replication, SRV-17)
    bool     ackOnlyDue(double now) const;                           // reliable data received, nothing sent for 33 ms
    double   rttMs() const; double lossPercent() const;
private:
    // resend after clamp(1.5 × rtt, 50 ms, 1 s); 256 in flight, the rest queued; receive window 256
};'''),
]),
'SRV-06': dict(auto=False, extra=[
    ('src/net/StreamFramer.hpp', 'cpp', '''class StreamFramer {                      // one per TCP stream
public:
    explicit StreamFramer(uint32_t maxMessageBytes);
    Result<void> feed(std::span<const std::byte> bytes);      // error → close the stream
    std::optional<std::vector<std::byte>> next();             // one complete message, or nothing yet
    static void frame(std::span<const std::byte> message, std::vector<std::byte>& out);  // length prefix + bytes
private:
    std::vector<std::byte> buffer_;
    uint32_t               max_;
};'''),
]),
'SRV-07': dict(extra=[
    ('tests/', 'text', '''tests/net/players/        JoinLeaveTest, KickTest, TimeoutTest, NamesTest
tests/net/players/        HandshakeTest (ProtocolMismatch, WrongGame, FormatMismatch, InvalidName, ServerFull,
                          RefusedByGame, UDP link, 5 s deadline, pending limits)
tests/net/messages/       UnreliableTest, ReliableUnderLossTest (20 % loss, reorder, duplicates: once, in order),
                          StreamTest, CallbackExceptionTest, AggregationTest (several messages per datagram)
tests/reflection/codec/   RoundTripTest (every FieldKind), LimitsTest, EntityMappingTest, FuzzTest (10 000 inputs)'''),
]),

# ================================================================ ROOMS (ADR 0038, 0039)
'SRV-10': dict(extra=[
    ('src/server/PartyTable.hpp', 'cpp', '''class PartyTable {
public:
    PartyId createSolo(PlayerId p);                   // on PlayerJoined
    void    move(PlayerId p, PartyId to);             // dissolves the old party if empty
    void    remove(PlayerId p);                       // on PlayerLeft; promotes a new leader
    void    setLeader(PartyId party, PlayerId p);
    const Party& get(PartyId p) const;  PartyId of(PlayerId p) const;
    std::span<const PartyId> all() const;
};'''),
    ('src/server/RoomDirectory.hpp', 'cpp', '''class RoomDirectory {
public:
    explicit RoomDirectory(uint32_t maxRooms);
    std::optional<RoomId> add(const RoomSettings& s);
    void remove(RoomId r);
    void setState(RoomId r, RoomState s);
    void enter(RoomId r, const Party& party);  void leave(RoomId r, PartyId party);
    const RoomInfo& get(RoomId r) const;  RoomId roomOf(PlayerId p) const;
    std::span<const RoomId> all() const;
};'''),
]),
'SRV-11': dict(extra=[
    ('src/server/RoomRuntime.hpp', 'cpp', '''struct EngineServices {           // the shared, thread-safe services a room may use (ADR 0039)
    const TypeRegistry& types;  Logger& log;  const CVars& cvars;  AssetManager& assets;  Metrics& metrics;
    const EngineConfig& config;
};'''),
    ('src/server/RoomMailbox.hpp', 'cpp', '''struct RoomMessage { enum class Kind : uint8_t { PlayerEntered, PlayerLeft, NetMessage, Finished, Failed, Close };
                     Kind kind; PlayerId player = 0; uint16_t messageId = 0; std::vector<std::byte> payload; };

class RoomMailbox {                                   // multi-producer, single-consumer
public:
    void push(RoomMessage m);
    std::vector<RoomMessage> drain();
};'''),
    ('src/server/RoomRuntime.hpp', 'cpp', '''class RoomRuntime {
public:
    RoomRuntime(RoomId id, const RoomSettings& s, const EngineServices& services, const Scheduler& prototypes);
    Result<void> start();                             // loads settings.level, instantiates systems
    void tick();                                      // FixedUpdate / LateUpdate of this room; one thread at a time
    RoomMailbox& inbox();  RoomMailbox& outbox();
    bool failed() const;  RoomId id() const;
};'''),
]),
'SRV-12': dict(extra=[
    ('src/server/WorkerPool.hpp', 'cpp', '''class WorkerPool {
public:
    explicit WorkerPool(unsigned threads);            // 0 = std::thread::hardware_concurrency()
    ~WorkerPool();                                    // joins
    void submit(std::move_only_function<void()> job);
    unsigned size() const;
};'''),
    ('src/server/IRoomHost.hpp', 'cpp', '''class IRoomHost {
public:
    virtual ~IRoomHost() = default;
    virtual Result<void> start(std::unique_ptr<RoomRuntime> room) = 0;
    virtual void stop(RoomId room) = 0;
    virtual void post(RoomId room, RoomMessage m) = 0;
    virtual void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) = 0;   // lobby thread
};'''),
    ('src/server/ThreadPoolRoomHost.hpp', 'cpp', '''class ThreadPoolRoomHost final : public IRoomHost {
public:
    ThreadPoolRoomHost(WorkerPool& pool, double tickRate, Logger& log);
    Result<void> start(std::unique_ptr<RoomRuntime> room) override;
    void stop(RoomId room) override;
    void post(RoomId room, RoomMessage m) override;
    void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) override;
};'''),
]),
'SRV-13': dict(extra=[
    ('src/server/ServerLoop.hpp', 'cpp', '''// The server's main-thread loop, called by Engine::run() when role == Server (ADR 0040)
class ServerLoop {
public:
    ServerLoop(EngineServices services, NetworkSession& network, Lobby& lobby, IRoomHost& rooms, IGameModule& game);
    void run(const std::atomic_bool& running);   // network poll, lobby events (LobbyContext), rooms.collect(), console
private:
    void onRoomMessage(RoomId room, RoomMessage&& m);   // Finished / Failed → RoomFinished / RoomFailed events
};'''),
]),
'SRV-14': dict(extra=[
    ('src/server/PlayerRouting.hpp', 'cpp', '''class PlayerRouting {
public:
    uint16_t setRoom(PlayerId p, RoomId r);           // 0 = lobby; returns the new roomSession (RFC §8.3.1)
    RoomId   roomOf(PlayerId p) const;
    uint16_t roomSession(PlayerId p) const;
    void     setReady(PlayerId p, uint16_t roomSession);   // RoomReady: replication may start
    bool     ready(PlayerId p) const;
    std::span<const PlayerId> playersIn(RoomId r) const;
};'''),
    ('include/engine/client/ClientLobby.hpp', 'cpp', '''// Additions of protocol v1 (ADR 0043); the rest comes from reference 14.
class ClientLobby {
public:
    std::span<const PlayerId> onlinePlayers() const;  // OnlinePlayers / PlayerOnline / PlayerOffline
    std::string_view name(PlayerId p) const;
    bool connected(PlayerId p) const;
private:
    void onEnteredRoom(uint16_t roomSession);         // waits for the scene switch, then sends RoomReady
};'''),
]),
'SRV-16': dict(extra=[
    ('src/net/Reconnection.hpp', 'cpp', '''using ReconnectToken = std::array<std::byte, 16>;

class GraceTable {                            // server: lost sessions kept for net.reconnectGraceSeconds (RFC §6.7)
public:
    explicit GraceTable(double graceSeconds);
    ReconnectToken issue(PlayerId p);                    // new single-use token, OS secure random
    void     disconnected(PlayerId p, DisconnectReason why, double now);   // → PlayerDisconnected
    std::optional<PlayerId> claim(const ReconnectToken& t);                // matches a disconnected player, consumes it
    std::vector<std::pair<PlayerId, DisconnectReason>> expired(double now); // → PlayerLeft
    bool     isDisconnected(PlayerId p) const;
};

class ClientReconnector {                     // client: Interrupted → retry every 2 s while the grace lasts
public:
    ClientReconnector(double graceSeconds, bool enabled);
    void interrupted(DisconnectReason why, double now);  // → ConnectionInterrupted
    bool shouldRetry(double now);
    bool givenUp(double now) const;                      // → LeftServer
    void succeeded();                                    // → Reconnected
};

class UdpRelink {                             // client: 1 s of UDP silence with TCP open → UdpLink every 100 ms (RFC §6.7.3)
public:
    bool due(double lastUdpReceived, bool tcpOpen, double now);
};'''),
    ('include/engine/events/EngineEvents.hpp', 'cpp', '''struct PlayerDisconnected   { PlayerId player; DisconnectReason reason; };   ENGINE_EVENT(PlayerDisconnected)
struct PlayerReconnected    { PlayerId player; };                            ENGINE_EVENT(PlayerReconnected)
struct ConnectionInterrupted{ DisconnectReason reason; };                    ENGINE_EVENT(ConnectionInterrupted)
struct Reconnected          { };                                             ENGINE_EVENT(Reconnected)'''),
]),
'SRV-17': dict(extra=[
    ('src/net/ReplicationState.hpp', 'cpp', '''// RFC §9.3: per client, what it has acknowledged; snapshots carry only what is missing.
class ReplicationState {                      // one per player in a room
public:
    void recordSent(uint16_t packetSeq, std::span<const SentItem> items);   // spawns, destroys, component hashes
    void onPacketAcked(uint16_t packetSeq);
    bool spawnAcked(uint32_t networkId) const;
    bool componentDiffers(uint32_t networkId, uint16_t componentIndex, uint64_t bytesHash) const;
    bool destroyPending(uint32_t networkId) const;
    void reset();                                        // new roomSession
};

class SnapshotWriter {                        // builds the Snapshot messages of one player for one tick
public:
    SnapshotWriter(uint16_t roomSession, uint32_t tick, uint32_t lastInputProcessed, uint32_t budgetBytes);
    bool add(const EntityEntry& e, float priority);      // false when the budget is reached
    std::vector<std::vector<std::byte>> finish();        // self-contained parts of ≤ 1024 bytes (RFC §4.4)
};

class PriorityAccumulator {                   // NetPriority: weight added each snapshot an entity misses
public:
    float bump(uint32_t networkId, float weight);  void sent(uint32_t networkId);
};'''),
    ('include/engine/net/NetComponents.hpp', 'cpp', '''struct NetPriority { float weight = 1.0f; };   ENGINE_COMPONENT(NetPriority)'''),
    ('include/engine/net/RoomNetwork.hpp', 'cpp', '''void setRelevance(std::move_only_function<bool(PlayerId, Entity)> fn);   // default: every entity of the room'''),
]),
'SRV-18': dict(extra=[
    ('tests/net/fuzz/', 'text', '''tests/net/fuzz/  UdpPacketFuzzTest (random and mutated valid packets: header, every message id),
                 TcpStreamFuzzTest (random frames, split reads, huge lengths),
                 HandshakeFuzzTest, SnapshotDecodeFuzzTest, InputDecodeFuzzTest
                 — 100 000 inputs each, ASan + UBSan; seed corpus = the RFC §15 examples;
                 property: never crashes, never allocates beyond the limits, never reaches game code'''),
]),
'SRV-19': dict(extra=[
    ('tools/wireshark/rtep.lua', 'lua', '''-- Wireshark dissector for RTEP v1.0 (heuristic on magic 0x5254 + version 1, UDP and TCP)
local rtep = Proto("rtep", "R-Type Engine Protocol")
-- fields: header (kind, sessionKey, sequence, ack, ackBits), messages (id → name from RFC §8.5,
-- messageSeq for reliable ids, length), Hello / Accepted / Refused / Snapshot / Input payloads
function rtep.dissector(buffer, pinfo, tree) end
local function heuristic(buffer, pinfo, tree) end   -- registered for udp and tcp'''),
]),
'SRV-20': dict(extra=[
    ('benchmarks/net/', 'text', '''benchmarks/net/  SnapshotSizeBench (bytes per entity and per snapshot for R-Type rooms of 50 / 100 / 200 entities),
                 BandwidthUnderNetemBench (scripts/netem.sh: 150 ms, 2 % loss, duplicates, 5 KB/s; clumsy on Windows),
                 ReliableLatencyBench (delivery time under 5 % and 20 % loss)
                 — results written to docs/network/07-measurements.md'''),
]),
'SRV-15': dict(extra=[
    ('tests/room/', 'text', '''tests/room/  ParallelRoomsTest (no shared entities, events, ids), RoomCrashTest (RoomFailed, others continue),
             PartyFlowTest (invite, answer, leave, leader change, leader disconnect),
             RoomFlowTest (create, move, finish, close, maxRooms), SystemCopyPerRoomTest — run with ThreadSanitizer'''),
]),
}
