"""What each task contains in detail. Keyed by task id; read by generate.py.

goal  : one or two sentences, why the task exists.
todo  : what to implement, in a sensible order.
done  : how to know it is finished (tests, observable behaviour).
watch : traps and rules that are easy to miss.
"""

DETAILS = {
# ---------------------------------------------------------------- 00 Build
'ENG-01': dict(
    goal="Create the `engine` CMake project every other task builds into, with the C++23 setup and the build options.",
    todo=["`engine/CMakeLists.txt`: project, `engine` library target, `include/` as public include dir, `src/` sources by glob or list",
          "`cmake/EngineOptions.cmake`: `ENGINE_DYNAMIC_MODULES` (ON), warnings as errors option, sanitizers option",
          "C++23 required (`cxx_std_23`), `-Wall -Wextra -Wpedantic` / `/W4`, position-independent code",
          "Package manager hook (Conan or vcpkg, chosen in the Tooling part): leave one clearly marked place for it",
          "Add `tests/` and `benchmarks/` as optional sub-directories"],
    done=["`cmake -B build && cmake --build build` succeeds on Linux and Windows with an empty engine",
          "Options appear in `cmake -LH`"],
    watch=["No global `include_directories` or `add_definitions`: everything on targets",
           "Third-party types must never leak into public headers (reference README)"]),
'ENG-02': dict(
    goal="Give games and plugins one-line CMake functions so nobody copies build boilerplate.",
    todo=["`engine_add_plugin(<kind>_<name> SOURCES … [DEPENDS …])`: SHARED or STATIC per option, output to `<bin>/plugins`, hidden visibility, `ENGINE_BUILDING_PLUGIN`",
          "`engine_add_game_module(<name> SHARED_SOURCES … SERVER_SOURCES … CLIENT_SOURCES …)`: builds `lib<name>_server` and `lib<name>_client` in `<bin>/games` (or one `lib<name>` with `SOURCES`), defines `ENGINE_GAME_SERVER` / `ENGINE_GAME_CLIENT` (ADR 0040)",
          "`engine_add_launcher(<exe> CONFIG <file> [GAME <name>])`: builds `launcher/main.cpp` with `ENGINE_DEFAULT_CONFIG`, copies the config, links plugins and the game in static mode"],
    done=["A test plugin and a test game (ENG-53 fixtures) build in both modes",
          "`ENGINE_DYNAMIC_MODULES=OFF` produces one executable with no `.so` / `.dll` next to it"],
    watch=["Windows: `.dll` must land next to the executable or in `plugins/` with the search path set",
           "Static mode needs `WHOLE_ARCHIVE` (or `/WHOLEARCHIVE`) so registrar objects are not dropped"]),
'ENG-03': dict(
    goal="Prove early that every C++23 feature on our allow-list compiles on GCC, Clang and MSVC.",
    todo=["`cmake/FeatureProbe.cpp` using each allowed feature: `std::expected`, `std::move_only_function`, `std::print`, deducing this, `std::to_underlying`, `std::unreachable`",
          "`try_compile` in CMake; on `std::expected` failure, switch `Result` to `tl::expected` with a define"],
    done=["CI job compiles the probe on the three compilers and prints which fallbacks are active"],
    watch=["Features outside the allow-list (ADR 0017) must not be added here silently: they need an ADR change"]),
# ---------------------------------------------------------------- 01 Core
'ENG-04': dict(
    goal="Write the leaf vocabulary every block uses: hashing, math types, errors and results.",
    todo=["`fnv1a64` as `constexpr` (usable in templates and macros)",
          "`Vec2` (operators, `dot`, `length`, `normalized`, `lerp`), `Rect` (`contains`, `intersects`), `Color` (named constants)",
          "`Errc` (15 codes), `Error` with `toString()` → `file:line: message`",
          "`Result<T>` alias, `BootError` with exit code"],
    done=["Unit tests: hash known values at compile time (`static_assert`), rect intersection edges, `Error::toString` with and without file"],
    watch=["No engine include in `core/`: it is the bottom layer",
           "`Error::message` is a full sentence; suggestions (\"did you mean\") are added by callers"]),
'ENG-05': dict(
    goal="Time values for the fixed-step loop and a monotonic clock to measure frames.",
    todo=["`Time` struct (fixedDelta, tick, alpha, total, frameDelta)", "`Clock` over `std::chrono::steady_clock`: `restart`, `elapsed`"],
    done=["Test: `restart()` returns increasing positive values; `elapsed()` does not reset"],
    watch=["Never `system_clock` for durations: it jumps with NTP"]),
'ENG-06': dict(
    goal="Synchronous observer list with RAII unsubscription, used by component hooks, CVars and the EventBus.",
    todo=["`Subscription`: move-only, `reset()`, `active()`, unsubscribes in its destructor",
          "`Signal<void(Args...)>`: `connect` returns a `Subscription`, `emit` calls listeners in connection order, `size`",
          "Safe if a listener disconnects itself or another listener during `emit`"],
    done=["Tests: destroying the Subscription stops calls; disconnect during emit; Signal destroyed before its Subscriptions (no crash)"],
    watch=["A Subscription can outlive its Signal: use a shared control block, not a raw back-pointer"]),
'ENG-07': dict(
    goal="Read the configuration file and the command line into one `EngineConfig`.",
    todo=["YAML keys of `EngineConfig` (role, game, plugins, tickRate, root, window, net, log, cvars)",
          "`rooms` section (host, maxRooms, workerThreads)", "CLI options: --config, --role, --platform, --audio, --network, --scripting, --port, --host, --log, --scene, --ticks, --set name=value",
          "Unknown key → error with `file:line`; CLI overrides the file"],
    done=["Tests: valid file, unknown key, wrong type, every CLI option overriding the file"],
    watch=["`YAML::Node` stays in `EngineConfig.hpp`, which games never include"]),
'ENG-08': dict(
    goal="The composition root: owns every subsystem, runs boot → load → loop → shutdown.",
    todo=["Members in the order of reference 01 (declaration order = shutdown order)",
          "`boot()`: logger, CVars + `loadArchive`, plugins of the role, game module, engine types then `registerTypes`",
          "`load()`: engine systems then `registerSystems(role)`, `Scheduler::build`, first scene (or `--scene`), `onStart`",
          "`loop()`: fixed step with accumulator clamp, input, `network.poll(ctx)`, FixedUpdate × N, LateUpdate, scene switch, Render, submit, audio, metrics; `--ticks` stop",
          "`shutdown()`: `onShutdown`, `saveArchive`, scenes cleared, then RAII",
          "Exit codes 1–5 from `BootError` and the client failure policy"],
    done=["Headless run with null plugins and `--ticks 100` exits 0 (ENG-57)",
          "Each boot failure returns its exit code with a readable message"],
    watch=["Game objects (systems, components, subscriptions) must be destroyed before the game module is unloaded",
           "Never call game code from the network thread"]),
'ENG-09': dict(
    goal="The only door from game code into the engine (ADR 0022).",
    todo=["Accessors of reference 01 forwarding to the engine (client) or to a room (server)", "Private constructor, `friend class Engine` and `RoomRuntime`; base class `ServiceContext` comes from SRV-09"],
    done=["A game test module compiles using only `Context.hpp` and block headers, never `Engine.hpp`"],
    watch=["Context holds no state of its own; never cache it in game code"]),
'ENG-10': dict(
    goal="The engine's `main()`, shared by every executable.",
    todo=["Load `EngineConfig` from `ENGINE_DEFAULT_CONFIG` + argv", "SIGINT / SIGTERM and Windows console handler → `requestQuit()`", "Return `Engine::run()`; print the error and return 1 on bad config"],
    done=["Ctrl+C on client and server shuts down cleanly (log shows shutdown, exit code 0)"],
    watch=["Signal handlers may only set an atomic flag"]),
# ---------------------------------------------------------------- 02 Modules
'ENG-11': dict(
    goal="The contracts between the engine, plugins and games, plus the macros that export them.",
    todo=["`IPlugin`, `PluginKind`, `PluginContext`", "`IGameModule` with default `onStart`, `onLobbyStart`, `onRoomStart`, `onShutdown` (ADR 0039)",
          "`ENGINE_PLUGIN_API_VERSION`, `ENGINE_GAME_API_VERSION`, `ENGINE_EXPORT`",
          "`ENGINE_GAME_MODULE(C)` and `ENGINE_PLUGIN(C)`: extern \"C\" create / destroy / version, try/catch → nullptr; static registrar in static mode"],
    done=["Test fixture plugin and game build and are found by ModuleManager in both modes"],
    watch=["No exception may leave an extern \"C\" function", "Objects are destroyed by the module that created them"]),
'ENG-12': dict(
    goal="The only OS-specific loading code.",
    todo=["`open` (dlopen / LoadLibraryW) with the OS error text", "`symbol<Fn>` / `rawSymbol`", "`decorate(stem)` per OS", "RAII close, move-only"],
    done=["Tests: open missing file, missing symbol, valid fixture on Linux and Windows"],
    watch=["`RTLD_NOW | RTLD_LOCAL` on Linux; wide paths on Windows"]),
'ENG-13': dict(
    goal="Static-link replacement for dynamic loading (`ENGINE_DYNAMIC_MODULES=OFF`).",
    todo=["Tables of plugin and game factories filled by registrars", "`findPlugin`, `findGame`"],
    done=["Static build loads the same fixtures as the dynamic build"],
    watch=["Static init order: use function-local statics for the tables"]),
'ENG-14': dict(
    goal="Load plugins by kind and name and the game module, check versions, unload in reverse order.",
    todo=["`loadPlugin<I>(kind, name, ctx)`: `null` → built-in Null Object; else library + version + kind check + `init`",
          "`loadGame(name, role)`: `lib<name>_<role>` first, then `lib<name>` (ADR 0040)", "`unloadGame`, `unloadAll` in reverse load order", "`loaded()` for the console"],
    done=["Tests: version mismatch → ApiMismatch, wrong kind, null plugin, unload order logged"],
    watch=["A client whose platform plugin fails stops with code 2: never fall back to NullPlatform"]),
'ENG-15': dict(
    goal="Do-nothing implementations used on the server, in CI and when a device is missing (ADR 0023).",
    todo=["NullPlatform, NullRenderer (valid handles, no decoding), NullAudio, NullNetwork, NullScripting"],
    done=["Headless smoke test (ENG-57) runs with all Null Objects"],
    watch=["Each Null Object in use is logged once at boot"]),
# ---------------------------------------------------------------- 03 ECS
'ENG-16': dict(
    goal="Entity handles with generations so stale handles are detected.",
    todo=["`Entity` (index, generation, null, isNull, ==, std::hash)", "`EntityAllocator`: free list, generation bump on release"],
    done=["Tests: reused slot gets a new generation; stale handle is not alive"],
    watch=["An Entity never leaves the process (network uses NetworkId)"]),
'ENG-17': dict(
    goal="Component storage: type-erased interface and the paged sparse set behind it.",
    todo=["`IStorage` interface", "`SparseSet<T>` with pages of 4096, packed `dense` / `data`, swap-and-pop remove, raw overrides"],
    done=["Tests: add/remove/has across pages, iteration order, raw access", "Benchmark numbers recorded (ENG-56)"],
    watch=["Removing invalidates references: document it in the header"]),
'ENG-18': dict(
    goal="The per-scene world: entities, typed and raw component access, identity and hierarchy.",
    todo=["Entities: create, destroy (recursive over Children), alive, clear",
          "Typed: emplace, get, tryGet, has, replace, remove via TypeRegistry ids", "Raw: storage, getRaw, emplaceRaw, removeRaw, componentsOf",
          "Identity: find(name), find(Uuid) indexed, withTag", "Hierarchy: setParent, parent, children", "World singletons: `singleton<T>()`, `trySingleton<T>()` on a hidden entity (ADR 0039)"],
    done=["Tests: every method, unregistered type error message, recursive destroy"],
    watch=["Hooks fire on every path, including raw and Commands"]),
'ENG-19': dict(
    goal="Queries over entities that have a set of components.",
    todo=["`View<Ts...>` iterating the smallest storage", "`exclude<Xs...>`", "iterator yielding `tuple<Entity, Ts&...>`, `each`, `front`, `sizeHint`"],
    done=["Tests: structured bindings, exclude, empty view"],
    watch=["Writing component values during iteration is fine; structural changes are not"]),
'ENG-20': dict(
    goal="Deferred structural changes so systems can spawn and destroy while iterating (ADR 0015).",
    todo=["spawn (empty, prefab), destroy, add, set, remove, setParent, addRaw", "`apply` in recording order, skipping dead entities"],
    done=["Tests: spawn inside a view loop, destroy then add on the same entity, prefab spawn"],
    watch=["The Entity returned by spawn is valid immediately but has components only after apply"]),
'ENG-21': dict(
    goal="onCreate / onReplace / onDestroy signals per component type.",
    todo=["`ComponentHooks` per ComponentId", "Fire from emplace, replace, remove, destroy and Commands::apply"],
    done=["Tests: each hook fires once per change, including via Commands and destroy"],
    watch=["onDestroy fires before removal so listeners can still read the component"]),
# ---------------------------------------------------------------- 04 Reflection
'ENG-22': dict(
    goal="Teach the engine the types a game invents: names, kinds, fields, options (ADR 0027, 0036).",
    todo=["`TypeCategory`, `TypeName<T>`, `HasTypeName`, `typeKey<T>`",
          "Macros `ENGINE_COMPONENT`, `ENGINE_EVENT`, `ENGINE_MESSAGE`, `ENGINE_STRUCT`, `ENGINE_ENUM` (qualified name → last part)",
          "`FieldKind` (with List, Struct), `FieldInfo` (maxLength, element kind), `TypeFlags`, `Transport`, `TypeInfo`",
          "`FieldsBuilder<T>` (`field` with deducing this, `maxLength`) and the five builders with their options",
          "`TypeRegistry::add<T>()` choosing the builder with `if constexpr`; `id`, `find`, `findByKey`, `info`, `all`, `messages`, `formatFingerprint`, `closestName`, `clearGameTypes`"],
    done=["Tests: each category registers; wrong option fails to compile (compile-fail test); DuplicateName; fingerprint changes when a field changes"],
    watch=["`static_assert` message must name the five macros", "Engine types register before the game's"]),
'ENG-23': dict(
    goal="YAML reading and writing for any registered type, without yaml-cpp in game headers.",
    todo=["`YamlNode`, `YamlWriter`, `LoadContext` / `SaveContext` (Entity ↔ Uuid)", "Generic YAML codec per FieldKind: lists as sequences, structs as maps, enums by name"],
    done=["Tests: round trip of every field kind; error with file:line on wrong type"],
    watch=["Unknown key in a component → error with closest name"]),
'ENG-24': dict(
    goal="Bounds-checked byte writing and reading: a malformed packet can never read or allocate out of range.",
    todo=["`ByteWriter` with a max size and `overflowed`", "`ByteReader` returning `std::optional`, `failed`", "Bit packing (`writeBits`, `bits`)"],
    done=["Tests: every read past the end fails cleanly; string longer than the limit fails"],
    watch=["Byte order is fixed by the protocol RFC: keep it in one place"]),
'ENG-25': dict(
    goal="The engine's built-in components and their registration.",
    todo=["Headers of reference 13 with `ENGINE_COMPONENT`", "`registerEngineTypes`: fields and flags (Position/Velocity/Sprite Replicated, PreviousPosition Transient…) and engine events"],
    done=["Inspector-style dump (`types` console command) lists every engine component with its fields"],
    watch=["Names are reserved: a game type with the same name is a boot error"]),
# ---------------------------------------------------------------- 05 Scheduler
'ENG-26': dict(
    goal="Named systems in fixed stages, ordered by constraints, timed, with the failure policy (ADR 0024, 0028).",
    todo=["`add(stage, name, fn)` + `SystemBuilder` (after, before, roles, disabled, critical)", "`build`: topological sort per stage; unknown name or cycle → error",
          "`run`: time each system, catch exceptions, apply the policy per role", "`setEnabled`, `stats`, `clearGameSystems`", "Per-room instances: `add<F>` keeps a copyable prototype, `addFactory`, `instantiate(stage)` (ADR 0039)"],
    done=["Tests: ordering, cycle message, server disables a throwing system, critical ends the session, client quits with code 5"],
    watch=["Ties keep registration order: deterministic"]),
# ---------------------------------------------------------------- 06 Events
'ENG-27': dict(
    goal="Typed queued events with RAII subscriptions, plus the engine's own events.",
    todo=["`subscribe`, `publish` (queued), `emit` (immediate), `publishFromAnyThread` (mutex inbox)", "`dispatch`: deliver in publish order; events published during dispatch wait",
          "Engine events of reference 06 (player events included)"],
    done=["Tests: order, publish during dispatch, unsubscribe during dispatch, thread inbox"],
    watch=["Events never leave the process"]),
# ---------------------------------------------------------------- 07 Input
'ENG-28': dict(
    goal="Device vocabulary and action ids.",
    todo=["`Key`, `MouseButton`, `GamepadButton`, `GamepadAxis` enums; name ↔ enum tables; `keyName`", "`RawInputEvent`", "`ActionId`, `action()` constexpr"],
    done=["Tests: every key name parses back to its enum"],
    watch=["Keys are physical positions (scancodes)"]),
'ENG-29': dict(
    goal="Turn raw input into actions with rebinding and accessibility options.",
    todo=["`InputMap`: load / save input.yaml, bind, unbind, boundTo, captureNextBinding, options", "Edge latching in `beginTick`, hold-to-toggle, auto-repeat, deadzone, one-handed set",
          "`ActionState`: pressed, held, released, axis, toMask / fromMask"],
    done=["Tests: a press and release between two ticks is seen once; options behave as documented"],
    watch=["Games read actions, never keys"]),
# ---------------------------------------------------------------- 08 Rendering
'ENG-30': dict(
    goal="The rendering API as plain data: the engine never draws itself.",
    todo=["Handles, WindowDesc, Feature, ColorFilter, Camera2D (screen ↔ world with letterbox)", "DrawCmd, RenderFrame, RenderStats", "RenderQueue (text arena)", "IRenderer, IPlatform declarations"],
    done=["Tests: Camera2D conversions round trip; RenderQueue keeps capacity after clear"],
    watch=["No SFML or raylib type in these headers"]),
'ENG-31': dict(
    goal="The Part 1 window, input and renderer on SFML 3.",
    todo=["`SfmlPlatform`: window, events → RawInputEvent → InputMap::feed, window events → EventBus", "`SfmlRenderer`: textures, fonts, sort by layer, batch by texture with VertexArray, letterbox", "SFML key mapping"],
    done=["A sprite, a text and shapes appear; resizing keeps the virtual resolution; input reaches actions"],
    watch=["One draw call per texture and layer, not per sprite"]),
'ENG-32': dict(
    goal="Dear ImGui and ImPlot inside the SFML plugin for the console and metrics.",
    todo=["ImGui-SFML init / shutdown", "`beginDebugUi` / `endDebugUi`"],
    done=["The console opens with F1 and the FPS graph draws"],
    watch=["ImGui must not eat game input when the console is closed"]),
'ENG-33': dict(
    goal="Colour-blindness filters and UI scale (accessibility).",
    todo=["Fragment shader with protanopia, deuteranopia, tritanopia matrices and high contrast", "`setColorFilter`, UI scale applied to screen-space draws"],
    done=["`r_color_filter` changes the picture live; `r_ui_scale 1.5` enlarges HUD text"],
    watch=["Without shader support, `supports(ColorFilter)` returns false"]),
# ---------------------------------------------------------------- 09 Audio
'ENG-34': dict(
    goal="The audio interface, buses and captions for every backend.",
    todo=["`AudioBus`, `PlayParams`, `VoiceId`, `IAudio` declarations", "Caption wrapper publishing `Caption` events"],
    done=["Test with NullAudio: a play with a caption publishes Caption"],
    watch=["Captions come from the wrapper, not the plugin"]),
'ENG-35': dict(
    goal="One audio plugin for all platforms, on miniaudio.",
    todo=["One `ma_engine`, one sound group per bus", "Sounds from memory, streamed music, positional sound, fades"],
    done=["Effects and music play; bus volumes change live; no device → NullAudio with a warning"],
    watch=["miniaudio callbacks run on its own thread"]),
# ---------------------------------------------------------------- 10 Network
'ENG-36': dict(
    goal="Network vocabulary and the byte transport interface.",
    todo=["`PlayerId`, `ConnectionId`, `Endpoint` (`resolve`, `toString`), `Datagram`, `DisconnectReason`, `NetConfig`, `ConnectionStats`", "`INetwork` (UDP and TCP)"],
    done=["Endpoint resolves IPv4 and IPv6 hosts"],
    watch=["`ConnectionId` is engine-internal; games use `PlayerId`"]),
'ENG-37': dict(
    goal="The transport plugin: UDP and TCP on an I/O thread with standalone Asio (ADR 0037: TCP from Part 1).",
    todo=["One `io_context` on its own thread", "UDP socket: open, send (thread-safe copy), receiver callback", "TCP acceptor and streams: listen, connect, send, receiver, close"],
    done=["Loopback test: datagrams and stream bytes arrive; closing during traffic does not crash"],
    watch=["The receiver runs on the I/O thread: only push into the session inbox"]),
'ENG-38': dict(
    goal="The core of NetworkSession: connections and the safe path from the network thread to the game thread.",
    todo=["Bounded, mutex-protected inbox (`maxQueuedDatagrams`)", "`poll` skeleton: drain, size and header validation, route to connections",
          "Connections: heartbeats, timeouts, stats (RTT, loss, bytes)", "`attach`, `sendRaw`, `isServer`, `connected`"],
    done=["Tests: malformed and oversized datagrams dropped and counted; timeout publishes the leave event"],
    watch=["Nothing from the I/O thread reaches game code directly", "Game-facing API is built on top by SRV-01 and SRV-03"]),
'ENG-39': dict(
    goal="Replication of entities from server to clients.",
    todo=["`NetworkIdMap`", "Net components and their registration", "Systems assign_ids, snapshot, apply_snapshot, interpolate, send_input, receive_input"],
    done=["Two headless clients see the same spawned entities as the server"],
    watch=["Uses the generic codec (SRV-04) for Replicated components", "Unknown NetworkId on a client → ignored, never a crash",
           "`engine.net.interpolate` is a default a game can disable to use its own (ADR 0041); prediction is the game team's"]),
# ---------------------------------------------------------------- 11 Scenes & assets
'ENG-40': dict(
    goal="Load each resource once behind a handle, with fallbacks.",
    todo=["texture, font, sound, music, script with cache", "pathOf, retain / release, collectUnused, reloadChanged, resolve (reject `../`), stats"],
    done=["Missing texture shows the checkerboard and logs once"],
    watch=["On the server, handles are valid but nothing is decoded"]),
'ENG-41': dict(
    goal="Prefabs: parsed once, instantiated with overrides and children.",
    todo=["`PrefabLibrary::get` (cached), `findByKey`, `instantiate`, `clear`"],
    done=["Tests: overrides, nested children with offsets"],
    watch=["`Prefab::key` is used by replication: same hash on every machine"]),
'ENG-42': dict(
    goal="Scene files in two passes with precise errors, and saving back.",
    todo=["Pass 1 entities + prefabs + overrides; pass 2 links by uuid", "Errors with file:line, no partial scene", "`save`: only differences from the prefab, skip Transient"],
    done=["Tests: forward references, unknown component suggestion, round trip"],
    watch=["A failed load leaves the current scene untouched"]),
'ENG-43': dict(
    goal="One active scene, deferred switching, persistent entities.",
    todo=["`load`, `switchTo` (end of frame), `markPersistent`, `save`, `world`, `applyPending`, `clear`"],
    done=["Switching keeps marked entities; SceneLoaded / SceneUnloading are published"],
    watch=["Systems must re-read `world()` after a switch"]),
# ---------------------------------------------------------------- 12 Services
'ENG-44': dict(
    goal="Thread-safe logging with tags and several sinks.",
    todo=["Levels, `std::format` API, `tagged`", "ConsoleSink (colours), FileSink (rotating), ConsoleUiSink"],
    done=["Logs from the network thread and the game thread never interleave mid-line"],
    watch=["Never a singleton"]),
'ENG-45': dict(
    goal="Typed console variables with flags, ranges and persistence.",
    todo=["`CVar<T>` with onChange", "`CVars::define`, set, get, find, forEach", "`loadArchive` / `saveArchive` (Archive flag)", "Engine CVars list of reference 12"],
    done=["Tests: range check, archive round trip"],
    watch=["`sim_tick_rate` is read-only"]),
'ENG-46': dict(
    goal="The developer console and its built-in commands.",
    todo=["registerCommand, execute, print, history, complete, toggle", "Built-ins: help, set, get, cvars, quit, systems, system enable/disable, scene, entities, inspect, modules, net, bind, assets"],
    done=["Every built-in works headless through `execute`"],
    watch=["Cheat commands refused on a server unless `sv_cheats 1`"]),
'ENG-47': dict(
    goal="Metrics and the debug overlay.",
    todo=["`Metrics`: frame timing, series ring buffers, snapshot", "`engine.debug_overlay`: console, FPS graph, system timings, lagometer"],
    done=["Overlay shows per-system times matching `systems`"],
    watch=["Recording must stay cheap: no allocation per sample"]),
'ENG-48': dict(
    goal="The scripting interface, the Script component and the script systems.",
    todo=["`ScriptHost`, `IScripting` declarations", "Script systems: lifecycle, update, collision; error → script disabled"],
    done=["With NullScripting, a Script component logs one clear error"],
    watch=["Script errors never stop the game"]),
'ENG-49': dict(
    goal="Lua 5.4 + sol2 implementation of IScripting.",
    todo=["API of guide 11 by reflection (components by name)", "Sandbox (no io, os, package, debug, load)", "Instruction limit hook", "Reload keeping the handle"],
    done=["The guide 11 boss script runs; an infinite loop is stopped"],
    watch=["No binding code per component type"]),
# ---------------------------------------------------------------- 13 Engine systems
'ENG-50': dict(
    goal="Movement-related built-in systems and the registration of all engine systems.",
    todo=["previous_position, movement, hierarchy, parallax, lifetime", "`registerEngineSystems` with names and order of reference 13"],
    done=["Tests: movement uses fixedDelta; children follow parents; lifetime destroys through Commands"],
    watch=["Names are fixed: games order against them"]),
'ENG-51': dict(
    goal="Collision detection on a uniform grid.",
    todo=["Grid of `phys_grid_cell`", "AABB test", "Layer/mask both ways", "Publish `Collision{a,b}` every overlapping tick"],
    done=["Tests: mask rule both ways; 1000 colliders within budget"],
    watch=["Detection only in Part 1: no response"]),
'ENG-52': dict(
    goal="Built-in drawing and animation.",
    todo=["render.sprites interpolated with alpha", "render.text", "render.debug (collider outlines)", "animation (frames, loop)"],
    done=["Smooth motion at 144 Hz display with 60 Hz simulation"],
    watch=["Render systems only fill the RenderQueue"]),
# ---------------------------------------------------------------- 14 Tests & bench
'ENG-53': dict(
    goal="Test setup and tests for core and modules, with a test plugin and a test game.",
    todo=["`tests/CMakeLists.txt` with the chosen framework", "Fixtures: test plugin, test game", "Tests of ENG-04..15"],
    done=["`ctest` runs green on Linux and Windows"],
    watch=["Fixtures must build in both linking modes"]),
'ENG-54': dict(
    goal="Tests for ECS and reflection.",
    todo=["Entities, storages, registry, views, commands, hooks", "Type registration, YAML round trips"],
    done=["Coverage of every public method of blocks 3 and 4"],
    watch=["Compile-fail tests for wrong builder options"]),
'ENG-55': dict(
    goal="Tests for scheduler, events, input, scenes and the session core.",
    todo=["One folder per block", "Session: malformed datagrams, timeouts"],
    done=["All green in CI"],
    watch=["Network tests on loopback only, with fixed ports avoided (port 0)"]),
'ENG-56': dict(
    goal="Measure ECS storage choices for the comparative study.",
    todo=["Sparse set vs sparse array vs minimal archetype store", "10 000 entities, 1–3 components, add/remove"],
    done=["Results table written into the study"],
    watch=["Build in Release; repeat runs"]),
'ENG-57': dict(
    goal="A CI smoke test: a client with null plugins runs 1000 ticks.",
    todo=["Test game + `--platform null --audio null --ticks 1000`"],
    done=["Exit code 0 in CI on every push"],
    watch=["No port opened unless the test game calls startServer"]),
# ---------------------------------------------------------------- 15 Part 2
'ENG-58': dict(
    goal="The raylib backend, then optional 3D (Part 2).",
    todo=["RaylibPlatform / RaylibRenderer for 2D", "Polling converted to RawInputEvent", "`supports(Render3D)` and 3D commands later"],
    done=["The game runs unchanged with `--platform raylib`"],
    watch=["No raylib type outside the plugin"]),
'ENG-59': dict(
    goal="The editor (Part 2, Track 1).",
    todo=["Hierarchy, inspector driven by reflection, scene save"],
    done=["A level edited and saved reloads identically"],
    watch=["Editor role uses the same engine; no editor code in the core"]),
# ================================================================ SERVER
'SRV-01': dict(
    goal="Give the game players instead of connections, and explicit server start / client connect (ADR 0037).",
    todo=["`PlayerTable` (private): PlayerId ↔ ConnectionId, names, allocation, removal",
          "`startServer(port)`, `stopServer()`, `connectTo(host, port, name)`, `disconnect()` on NetworkSession",
          "`players`, `localPlayer`, `name`, `kick`, `ping`, `stats(PlayerId)`",
          "Publish `PlayerJoined`, `PlayerLeft`, `JoinedServer`, `LeftServer`"],
    done=["Headless server + two clients: both PlayerJoined received, names correct, kick produces PlayerLeft{Kicked}"],
    watch=["`ConnectionId` never appears in a public signature", "Nothing opens automatically"]),
'SRV-02': dict(
    goal="The connection handshake: who you are, whether your game data matches, and linking TCP with UDP.",
    todo=["`Handshake` (private) state machine on both sides", "Carry the player name (≤ `maxNameLength`) and `TypeRegistry::formatFingerprint()`",
          "Refuse with `ServerFull`, `FormatMismatch`, invalid name", "Link the player's TCP stream and UDP endpoint (token given on TCP, echoed on UDP)"],
    done=["Tests: mismatched fingerprint refused with a clear log line on the client; full server refused"],
    watch=["Bytes and token format come from the protocol RFC", "Unauthenticated UDP from an unknown endpoint is dropped",
           "Admission by the game comes after the engine's checks (ADR 0042); its stage follows the RFC"]),
'SRV-03': dict(
    goal="Typed messages for games: send, broadcast and on<T> callbacks.",
    todo=["`MessageDispatcher` (private): message id ↔ TypeInfo, callback lists per id",
          "`send<T>` ×2, `broadcast<T>` choosing the transport declared on the message", "`on<T>` ×2 returning a Subscription",
          "Call callbacks during `poll()` on the loop thread; catch exceptions and log with the message name"],
    done=["Tests: Chat over stream, WaveCleared over reliable, Ping over unreliable; exception in a callback does not stop the server"],
    watch=["Message ids follow `TypeRegistry::messages()` order", "Callbacks must check `Entity::isNull()`"]),
'SRV-04': dict(
    goal="One binary codec for any registered type, used by messages and replication.",
    todo=["`encode` / `decode` walking `TypeInfo::fields` (private `Codec`)", "Lists: count then elements, count checked against `maxLength` (default 64) before allocating",
          "Text limit (default 256), structs, enums as integers", "`Entity` through `WireContext` (NetworkId, null if unknown)"],
    done=["Round-trip tests for every FieldKind; fuzz test: random bytes never crash or over-allocate"],
    watch=["A decode failure rejects the whole value", "Byte order and integer widths from the protocol RFC"]),
'SRV-05': dict(
    goal="Make `reliable()` UDP messages arrive: acknowledgements and resends.",
    todo=["`ReliableChannel` (private) per connection: sequence numbers, ack tracking, resend timer, duplicate drop"],
    done=["Test with simulated 20 % loss: every reliable message delivered exactly once"],
    watch=["Scheme (ack bitfield, ordering) is defined in the protocol RFC: implement after it"]),
'SRV-06': dict(
    goal="Cut the TCP byte stream into messages safely.",
    todo=["`StreamFramer` (private): length-prefixed framing, partial reads, `maxStreamMessageBytes` limit → close the stream"],
    done=["Tests: messages split across reads, several messages in one read, oversized announcement closes the stream"],
    watch=["Never allocate the announced size before checking the limit"]),
'SRV-07': dict(
    goal="Prove the server layer is correct and robust.",
    todo=["Players: join, leave, kick, timeout", "Handshake: mismatch, full, bad name", "Messages on the three transports", "Codec fuzzing and malformed input"],
    done=["All green in CI; fuzz test runs at least 10 000 random inputs"],
    watch=["Loopback only, port 0"]),

'SRV-08': dict(
    goal="The vocabulary of several games on one server: ids, records and events shared by the engine and the game.",
    todo=["`PartyId`, `RoomId`, `RoomState`, `RoomSettings`, `Party`, `RoomInfo`",
          "Lobby events (`PartyChanged`, `InviteRequested`, `InviteAnswered`, `LeavePartyRequested`, `RoomListRequested`, `RoomStarted`, `RoomFinished` with `resultAs<T>()`, `RoomFailed`, `RoomClosed`)",
          "Room events (`PlayerEnteredRoom`, `PlayerLeftRoom`) and client events (`PartyUpdated`, `InviteReceived`, `RoomListReceived`, `EnteredRoom`, `ReturnedToLobby`)",
          "Register them in `registerEngineTypes`; `RoomInfo`, `RoomSettings` and `Party` as `ENGINE_STRUCT` so they can travel"],
    done=["The types compile in a game test module; every event registers without `DuplicateName`"],
    watch=["Ids are 0 for none, like `PlayerId`", "No engine-internal type (ConnectionId, RoomRuntime) in these headers"]),
'SRV-09': dict(
    goal="Split the door into the engine in three so that world access in lobby code does not compile (ADR 0039 amends 0022).",
    todo=["`ServiceContext`: log (tagged with the room), read-only CVars, assets, metrics, role",
          "`Context : ServiceContext`: world, commands, scenes, events, time, network (`RoomNetwork`), `room()` on the server, `lobby()` on the client, client-only input / render / audio / console / requestQuit",
          "`LobbyContext : ServiceContext`: lobby, events, network (whole session), console, writable CVars, requestQuit",
          "Constructors private: built by `Engine` (lobby, client) and `RoomRuntime` (one per room)"],
    done=["A test lobby callback using `ctx.world()` fails to compile (compile-fail test)",
          "A system receives the Context of its room: two rooms see two different worlds"],
    watch=["Contexts never own anything", "`room()` on the client and `lobby()` on the server assert with a clear message"]),
'SRV-10': dict(
    goal="Keep track of parties and rooms on the lobby thread, with the fixed rules of the engine.",
    todo=["`PartyTable`: a party of 1 on `PlayerJoined`, move, remove (new leader promoted), setLeader, lookups",
          "`RoomDirectory`: add (refused over `maxRooms`), remove, state, enter / leave with a party, `roomOf(player)`"],
    done=["Tests: join → party of 1; invite + accept → one party of 2 and the old party dissolved; leader leaves → a new leader; maxRooms respected"],
    watch=["A player is always in exactly one party", "Lobby thread only: no locks here"]),
'SRV-11': dict(
    goal="One isolated game: its own world, commands, events, time, network ids and system instances.",
    todo=["`RoomRuntime`: owns a SceneManager (loads `settings.level`), Commands, EventBus, Time, NetworkIdMap, its Context",
          "Instantiate the game systems for this room (`Scheduler::instantiate`, ADR 0039 option B)",
          "`tick()`: drain the inbox, FixedUpdate, apply commands, dispatch events, LateUpdate; catch exceptions → `failed()`",
          "`RoomMailbox`: thread-safe queue (multi-producer, single consumer) for lobby → room and room → lobby"],
    done=["Two RoomRuntimes ticked one after the other never see each other's entities or events",
          "A throwing system marks the room failed and posts `Failed` to the outbox"],
    watch=["A room is never ticked by two threads at once", "No pointer to the lobby inside a room: messages only (ADR 0038 rule 3)"]),
'SRV-12': dict(
    goal="Run every room in parallel on a fixed pool of threads, each room sequentially.",
    todo=["`WorkerPool`: N threads (0 = cores), `submit(job)`, clean join",
          "`IRoomHost` interface: start, stop, post, collect",
          "`ThreadPoolRoomHost`: schedules each room's tick at its tick rate on the pool; never two ticks of one room at once; collects outboxes for the lobby"],
    done=["50 rooms at 60 ticks/s on 8 threads keep their tick rate in a stress test",
          "A slow room does not delay the others"],
    watch=["`ProcessRoomHost` (dedicated) will implement the same interface in Part 2: keep it free of thread assumptions"]),
'SRV-13': dict(
    goal="The server-side game API for parties and rooms, and the server loop (`src/server/`).",
    todo=["`Lobby`: parties (party, partyOf, sendInvite, addToParty, removeFromParty, setLeader, sendToParty) and rooms (createRoom, moveParty, moveToLobby, closeRoom, sendRoomList, room, rooms)",
          "Publish the lobby events; turn client requests into `InviteRequested` / `InviteAnswered` / `LeavePartyRequested` / `RoomListRequested`",
          "`RoomHandle`: id, settings, players, parties, `keep(Subscription)`, `reportFinished<T>`",
          "Call `IGameModule::onLobbyStart` once and `onRoomStart` for every room after its level is loaded",
          "`ServerLoop` (private, `src/server/`): the main-thread loop of the server, started by `Engine::run()` when role is Server: network poll, lobby events, room host collect"],
    done=["Headless server: two parties create two rooms, play in parallel, one reports finished, the game closes it, its party is back in the lobby"],
    watch=["The engine never applies a game rule (who may start, solo allowed…): it only publishes events and executes calls"]),
'SRV-14': dict(
    goal="Make the network follow players between the lobby and the rooms.",
    todo=["`PlayerRouting`: where each player is (lobby or room)",
          "Route game messages to the room's `on<T>` or the lobby's `on<T>`",
          "`RoomNetwork`: players, send, broadcast, on<T>, ping limited to the room",
          "Replication per room: only the room's entities; reset a player's replication state on enter / leave",
          "`ClientLobby`: myParty, invite, answerInvite, leaveParty, requestRoomList, currentRoom; client events"],
    done=["A player in room 1 never receives an entity or a message of room 2; returning to the lobby stops every snapshot"],
    watch=["Messages of a player who just left a room must not reach that room any more"]),
'SRV-15': dict(
    goal="Prove rooms are isolated, parallel and robust.",
    todo=["Parallel rooms (no shared entities, events, ids)", "Crash in one room → RoomFailed, others continue",
          "Party flows (invite, answer, leave, leader change, disconnect of a leader)", "Room flows (create, move, finish, close, maxRooms)",
          "Systems with state get one copy per room"],
    done=["All green in CI with thread sanitizer enabled"],
    watch=["Run the room tests under ThreadSanitizer: data races are the main risk"]),
}
