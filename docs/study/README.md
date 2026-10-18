# Technical and comparative study

Why the engine is built the way it is. Each page compares the options we considered against the
same criteria, says what we chose and what we gave up, and links the decision record (ADR) that
holds the full reasoning. Numbers come from our own measurements where we have them.

| Page | Covers |
|---|---|
| [Libraries](libraries.md) | graphics, audio, network, UI, scripting, data, tests, build |
| [Algorithms](algorithms.md) | ECS storage, system ordering, collision, game loop, replication |
| [Storage](storage.md) | scenes, prefabs, configuration, input bindings, wire format |
| [Security](security.md) | protocol validation, limits, malformed input, server robustness |
| [Accessibility](accessibility.md) | motor, visual, audio and cognitive needs, per engine feature |
| [Legal and privacy](legal-and-privacy.md) | personal data (GDPR), licences, intellectual property |
| [01 · ECS storage benchmark](01-ecs-storage.md) | measurements behind the ECS choice |
| [Graphics libraries benchmark (French)](../../benchmark-graphics-rtype.md) | the 2D library comparison behind SFML 3 |

## Every choice at a glance

| Area | Chosen | Main alternatives | Why, in one line | Record |
|---|---|---|---|---|
| Language | C++23 with an allow-list | C++20, full C++23 | `std::expected` and deducing `this`, only what GCC, Clang and MSVC all compile | [ADR 0017](../adr/0017-cpp23-with-allow-list.md) |
| Build and packages | CMake presets + vcpkg manifest | Conan, CPM | one file pins every dependency; same preset names in terminal and IDEs | [ADR 0045](../adr/0045-tooling-vcpkg-presets-ci.md) |
| Window, 2D, input | SFML 3 (plugin) | SDL3, raylib, SFML 2.6, GLFW + OpenGL | native C++, everything 2D in one dependency, easy to hide behind an interface | [ADR 0007](../adr/0007-sfml-and-raylib-backends.md) |
| Audio | miniaudio (plugin) | SFML Audio, OpenAL | one file, works with every platform plugin, groups for buses | [ADR 0008](../adr/0008-shared-audio-plugin.md) |
| Network | standalone Asio (plugin) | raw sockets, SFML Network, ENet | portable async UDP and TCP on one I/O thread, no Boost | [ADR 0033](../adr/0033-engine-networking.md) |
| Data files | YAML (yaml-cpp) | JSON, TOML, binary | readable and diffable, comments, errors with `file:line` | [ADR 0018](../adr/0018-data-in-yaml-behaviour-in-scripts.md) |
| Scripting | Lua + sol2 planned; Pulsar proposed | Python, a custom DSL | small, sandboxable, reloadable; decision pending (see [Libraries](libraries.md#scripting)) | [ADR 0019](../adr/0019-lua-first-custom-language-optional.md) |
| Engine tools UI | Dear ImGui planned; own widgets proposed | Qt | decision pending (see [Libraries](libraries.md#engine-tools-ui)) | [ADR 0035](../adr/0035-services-and-tools.md) |
| Tests | Catch2 v3 + CTest | GoogleTest, doctest | expressive, one header style, ctest discovery | [ADR 0045](../adr/0045-tooling-vcpkg-presets-ci.md) |
| ECS storage | paged sparse sets | sparse arrays, archetypes | fast enough by far, predictable, simplest to explain | [ADR 0013](../adr/0013-sparse-set-storage.md) |
| Game loop | fixed time step, interpolated render | variable step | deterministic simulation, same on client and server | [ADR 0003](../adr/0003-fixed-timestep-loop.md) |
| Modules | dynamic plugins, static fallback | static only | swap backends without rebuilding; one binary for release | [ADR 0002](../adr/0002-dynamic-plugins-from-day-one.md), [0009](../adr/0009-dual-linking-mode.md) |
| Protocol | own binary protocol (MSEP) on UDP, TCP outside the game | text, protobuf | compact, bounded, every byte specified | [RFC](../network/rfc.md), [ADR 0043](../adr/0043-protocol-v1-game-api.md) |
