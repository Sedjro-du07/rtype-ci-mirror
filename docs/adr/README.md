# Architecture decision records

Each file records one architectural decision: what forced it, what was chosen, and what the
project now has to live with. The format is Context, Decision, Alternatives, Consequences.

The discussion ids (E-xx, ECS-x) match the decision log kept in Notion during the architecture
rounds. A record is never edited once accepted. It is superseded by a new one, which names it.

| # | Decision | Discussion id | Status |
|---|---|---|---|
| [0001](0001-hybrid-engine-identity.md) | The engine is a code-first ECS core with a Unity-like data layer | E-01 | Accepted |
| [0002](0002-dynamic-plugins-from-day-one.md) | Backends are dynamic plugins from the first week | E-02 | Accepted |
| [0003](0003-fixed-timestep-loop.md) | The simulation advances in fixed steps and rendering interpolates | E-04 | Accepted |
| [0004](0004-plugin-boundaries.md) | Four plugin kinds: platform, audio, network, scripting | E-05 | Accepted |
| [0005](0005-game-as-a-module.md) | The game is a module and the engine ships main() | E-06 | Accepted |
| [0006](0006-command-list-renderer.md) | Rendering goes through a per-frame command list | E-07 | Accepted |
| [0007](0007-sfml-and-raylib-backends.md) | SFML for 2D and raylib for optional 3D, as swappable platform plugins | E-08 | Accepted |
| [0008](0008-shared-audio-plugin.md) | One audio plugin shared by both platform backends | E-09 | Accepted |
| [0009](0009-dual-linking-mode.md) | Modules are designed once and linked dynamically or statically | E-10 | Accepted |
| [0010](0010-content-layer.md) | Engine, game module and content are three layers | E-11 | Accepted |
| [0011](0011-generic-server-role.md) | The server is the engine in a generic server role | E-13 | Accepted |
| [0012](0012-entity-handle.md) | An entity is a 64-bit index and generation | ECS-1 | Accepted |
| [0013](0013-sparse-set-storage.md) | Components are stored in paged sparse sets | ECS-2 | Accepted |
| [0014](0014-type-erased-storage.md) | Component storages are type-erased and indexed by the TypeRegistry | ECS-3 | Accepted |
| [0015](0015-command-buffer.md) | Structural changes go through a command buffer | ECS-4 | Accepted |
| [0016](0016-identity-as-components.md) | Entity identity is a set of optional components | ECS-5 | Accepted |
| [0017](0017-cpp23-with-allow-list.md) | C++23 with an allow-list of features | E-14 | Accepted |
| [0018](0018-data-in-yaml-behaviour-in-scripts.md) | Data lives in YAML, behaviour lives in scripts | E-15 | Accepted |
| [0019](0019-lua-first-custom-language-optional.md) | Scripting ships with Lua first, a custom language stays optional | E-16 | Accepted |
| [0020](0020-game-module-helpers.md) | A macro and a CMake function plug a game into the engine | E-17 | Accepted |
| [0021](0021-engine-composition-root.md) | Engine is the composition root and member order is shutdown order | Block 1 | Accepted |
| [0022](0022-context-facade.md) | Game code reaches the engine only through Context | Block 1 | Amended by 0039 |
| [0023](0023-null-object-services.md) | Absent services are Null Objects | Block 1 | Accepted |
| [0024](0024-error-handling-policy.md) | Boot errors stop cleanly, system errors never stop the server | Block 1 | Accepted |
| [0025](0025-module-loading.md) | Plugins and games load through one ModuleManager | Block 2 | Accepted |
| [0026](0026-registry-api.md) | Registry API, views, commands and hooks | Block 3 | Accepted |
| [0027](0027-reflection-by-name.md) | Types are identified by name hash and described by registration | Block 4 | Amended by 0036 |
| [0028](0028-named-ordered-systems.md) | Systems are named and ordered by constraints inside fixed stages | Block 5 | Accepted |
| [0029](0029-queued-event-bus.md) | Events are typed, queued and unsubscribed by RAII | Block 6 | Accepted |
| [0030](0030-actions-and-input-map.md) | Input is read as named actions with accessibility options | Block 7 | Accepted |
| [0031](0031-render-api.md) | Rendering API: handles, draw commands, camera and accessibility filters | Block 8 | Accepted |
| [0032](0032-audio-buses-captions.md) | Audio has buses, positional sound and captions | Block 9 | Accepted |
| [0033](0033-engine-networking.md) | Engine networking: transport plugin, session, replication | Block 10 | Amended by 0037, 0041 |
| [0034](0034-scenes-prefabs-assets.md) | One active scene, prefabs with overrides, handle-based assets | Block 11 | Accepted |
| [0035](0035-services-and-tools.md) | Logger, CVars, console, metrics and the scripting interface | Block 12 | Accepted |
| [0036](0036-one-add-and-type-categories.md) | One `add<T>()`, the type category comes from the macro | SRV-A | Accepted (amends 0027) |
| [0037](0037-game-facing-networking.md) | The game sees players and typed messages, never sockets or bytes | SRV-1 to SRV-4 | Accepted (amends 0033), amended by 0042 |
| [0038](0038-rooms-shared-host.md) | Several games on one server: rooms, hosted in-process first | SRV-5 | Accepted |
| [0039](0039-room-api.md) | Room API: lobby, parties, rooms, contexts | SRV-6 to SRV-9 | Accepted (amends 0022) |
| [0040](0040-server-client-split.md) | One game code, two builds; server and client folders in the engine and the game | E-12, SRV-10 | Accepted |
| [0041](0041-netcode-means-in-engine-policy-in-game.md) | Network optimizations: the engine gives the means, the game team optimizes | NET-1 | Accepted (amends 0033) |
| [0042](0042-admission-by-the-game.md) | Admission: the engine checks it can decode, the game decides who joins | NET-2 | Accepted (amends 0037) |

## Not yet recorded

Nothing pending: E-12 is recorded as ADR 0040.
