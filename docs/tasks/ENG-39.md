# ENG-39 · Replication: NetworkIdMap, net components, snapshot and input systems

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · **Wave 6** · status and owner live on the board.

## Goal

Replication of entities from server to clients.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/NetworkIdMap.hpp`
- `include/engine/net/NetComponents.hpp`
- `src/net/NetworkIdMap.cpp`
- `src/systems/NetSystems.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/net/NetComponents.hpp`

```cpp
struct NetworkId  { uint32_t value = 0; };        // shared identity; never reused during a session
struct NetPrefab  { uint32_t prefabKey = 0; };    // fnv1a of the prefab path, to spawn on clients
struct NetOwner   { PlayerId owner = 0; };        // which player controls this entity
struct RemoteInput{ ActionState state; uint64_t lastMask = 0; uint32_t lastTick = 0; }; // server side
struct NetInterpolation { Vec2 from, to; double fromTime = 0, toTime = 0; };              // client side
struct NetPriority { float weight = 1.0f; };       // server: share of the send budget (RFC §9.6); absent = 1
```

### `include/engine/net/NetworkIdMap.hpp`

```cpp
class NetworkIdMap {
public:
    uint32_t allocate();                              // server: next id, monotonic
    void     bind(uint32_t id, Entity e);
    void     unbind(uint32_t id);
    Entity   find(uint32_t id) const;                 // client: unknown id → Entity::null(), never a crash
    void     clear();
};
```

### `src/systems/NetSystems.cpp`

```cpp
void netAssignIds(Context& ctx);       // "engine.net.assign_ids"     LateUpdate, server
void netSnapshot(Context& ctx);        // "engine.net.snapshot"       LateUpdate, server
void netApplySnapshot(Context& ctx);   // "engine.net.apply_snapshot" FixedUpdate (first), client
void netInterpolate(Context& ctx);     // "engine.net.interpolate"    Render (first), client
void netSendInput(Context& ctx);       // "engine.net.send_input"     LateUpdate, client
void netReceiveInput(Context& ctx);    // "engine.net.receive_input"  FixedUpdate (first), server
void registerNetSystems(Scheduler& s); // called by registerEngineSystems
```

## Depends on

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-29](ENG-29.md) — InputMap and ActionState: input.yaml, rebinding, accessibility options, masks
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits

## Unlocks

- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages
- [SRV-14](SRV-14.md) — Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset
- [SRV-17](SRV-17.md) — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize

## Read first

- [ADR 0033](../adr/0033-engine-networking.md)
- [ADR 0041](../adr/0041-netcode-means-in-engine-policy-in-game.md)
- [reference 10](../engine/reference/10-network.md)
- [RFC §9](../network/rfc.md) (MSEP v1.0)

## To implement

- [ ] `NetworkIdMap`
- [ ] Net components and their registration
- [ ] Systems assign_ids, snapshot, apply_snapshot, interpolate, send_input, receive_input
- [ ] Snapshot and Input messages of RFC §9.2 and §9.4 (roomSession, tick, lastInputProcessed; input redundancy and the 4-input buffer)
- [ ] Client rules: newest tick per entity, unknown updates ignored, destroyed ids remembered

## Done when

- Two headless clients see the same spawned entities as the server

## Watch out

- Uses the generic codec (SRV-04) for Replicated components
- Unknown NetworkId on a client → ignored, never a crash
- `engine.net.interpolate` is a default a game can disable to use its own (ADR 0041); prediction is the game team's

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
