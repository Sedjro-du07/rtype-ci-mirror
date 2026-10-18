# Libraries

Every third-party library sits behind an engine interface (`IPlatform`, `IRenderer`, `IAudio`,
`INetwork`, `IScripting`) or inside one engine source file, so no third-party type reaches a game
([ADR 0004](../adr/0004-plugin-boundaries.md)). Changing a library means writing one plugin, not
touching the game.

Criteria used everywhere on this page:

- **Fit**: does it do what R-Type needs, in C++, on Linux and Windows?
- **Packaging**: is it in vcpkg, does it build without system installs?
- **Hideability**: can it live behind an interface without leaking global state?
- **Cost**: how much code do we write around it within six weeks?
- **Allowed**: the subject forbids existing engines and libraries that do the evaluated work.

## Graphics, window and input

| Option | Fit | Packaging | Hideability | Cost | Verdict |
|---|---|---|---|---|---|
| **SFML 3** | 2D sprites, text, window, input, in C++17 | vcpkg default port | objects, no global state | low | **chosen** (`platform_sfml`) |
| SDL3 | excellent gamepads; images, fonts and audio in extra libraries | vcpkg | C API to wrap | medium | documented alternative |
| raylib | very simple, 2D and 3D | vcpkg | global state, polling only | low | Part 2 second backend ([ENG-58](../tasks/README.md)) |
| SFML 2.6 | same as 3, older API, OpenAL | version must be forced | objects | low | rejected: maintenance only |
| GLFW + OpenGL | window only, we write all 2D | vcpkg | good | high | rejected: puts Part 1 at risk |
| Unreal, Godot, bgfx, Ogre… | — | — | — | — | forbidden or too broad |

SFML 3 scored 46/50 in the [graphics benchmark](../../benchmark-graphics-rtype.md) against SDL3 (44)
and raylib (41). Its weak point is gamepads: the engine maps joystick axes and the d-pad hat itself
(`SfmlKeys.cpp`). Because rendering goes through a command list ([ADR 0006](../adr/0006-command-list-renderer.md)),
a second backend needs no game change: that is why raylib can arrive later.

**Given up:** SDL3's gamepad database and remapping, which would help motor accessibility. The
engine's `InputMap` covers rebinding instead (see [Accessibility](accessibility.md)).

## Audio

| Option | Verdict | Why |
|---|---|---|
| **miniaudio** | **chosen** (`audio_miniaudio`) | single file, decodes WAV/MP3/FLAC/OGG from memory, sound groups give the four buses, no device → silent mode instead of a crash |
| SFML Audio | rejected | would tie audio to the SFML plugin; the raylib backend would need another audio stack |
| OpenAL Soft | rejected | lower level (no decoding), LGPL, harder to ship on Windows |

One audio plugin serves every platform plugin ([ADR 0008](../adr/0008-shared-audio-plugin.md)).
The server loads `NullAudio`.

## Network

| Option | Verdict | Why |
|---|---|---|
| **standalone Asio** | **chosen** (`network_asio`) | portable UDP and TCP, async on one I/O thread, header-only, no Boost |
| raw BSD sockets / Winsock | rejected | two code paths to write and test, `select`/`poll` loop by hand |
| SFML Network | rejected | blocking or polling API, ties the server to SFML |
| ENet, GameNetworkingSockets | rejected | they implement reliability and sessions, which is the evaluated work (our [RFC](../network/rfc.md)) |

Asio only moves bytes. Validation, reliability, sessions and replication are ours, in
`NetworkSession` and `Replication` ([ADR 0033](../adr/0033-engine-networking.md)).

## Data files

| Option | Verdict | Why |
|---|---|---|
| **YAML (yaml-cpp)** | **chosen** | comments, readable diffs, anchors; yaml-cpp gives line numbers for error messages |
| JSON (nlohmann) | rejected | no comments, noisy for hand-written scenes |
| TOML | rejected | awkward for nested lists of entities |
| own binary format | rejected | unreadable, needs an editor before anyone can make a level |

yaml-cpp stays inside `Yaml.cpp` behind `YamlNode` / `YamlWriter`
([ADR 0018](../adr/0018-data-in-yaml-behaviour-in-scripts.md)). See [Storage](storage.md).

## Scripting

| Option | Status | For | Against |
|---|---|---|---|
| Lua 5.4 + sol2 | planned (ENG-49, [ADR 0019](../adr/0019-lua-first-custom-language-optional.md)) | known, fast, sandboxable, reloadable | one more language and binding layer |
| Pulsar (own language) | proposed by Yan, on `feature/pulsar-tools` | designed for the engine, engine API by reflection | a language to maintain and test |
| Python | rejected | heavy to embed and to sandbox | |

**Pending team decision.** Either way, scripts go through `IScripting`, so the choice stays a plugin.

## Engine tools UI

| Option | Status | For | Against |
|---|---|---|---|
| Dear ImGui + ImPlot | planned ([ADR 0035](../adr/0035-services-and-tools.md)) | console, metrics and editor fast to write | an extra dependency drawn over the game |
| own widgets on SFML (reSf) | proposed by Yan (ADR 0046 on his branch) | no UI library, full control of the look | more code to write |
| Qt | rejected | heavy to package, not made for a game loop | |

**Pending team decision.** The game's own menus are separate from these tools.

## Tests and benchmarks

| Option | Verdict | Why |
|---|---|---|
| **Catch2 v3** | **chosen** | readable `CHECK(a == b)` expansion, sections, `catch_discover_tests` into CTest |
| GoogleTest | rejected | heavier macros, we already use Google Benchmark only for measures |
| doctest | rejected | close to Catch2, smaller ecosystem |
| **Google Benchmark** | **chosen** for measures | repetitions, aggregates, used in [01 · ECS storage](01-ecs-storage.md) |

## Build and packages

| Option | Verdict | Why |
|---|---|---|
| **vcpkg (manifest) + CMake presets** | **chosen** | one `vcpkg.json` with a pinned baseline; presets give the same commands on Linux, Windows and IDEs |
| Conan | rejected | a second language (Python profiles) for the same result |
| CPM | rejected | builds every dependency from source in every build folder |

C++23 is used through an allow-list checked by a probe on GCC, Clang and MSVC in CI
([ADR 0017](../adr/0017-cpp23-with-allow-list.md)); `tl::expected` replaces `std::expected` where a
compiler lacks it.

## Written ourselves on purpose

ECS, reflection, scheduler, event bus, scenes, protocol and replication are the evaluated work, so
no library does them (no EnTT, no protobuf, no ENet).
