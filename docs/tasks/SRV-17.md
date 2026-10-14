# SRV-17 · Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **03 Transports** · **Wave 16** · status and owner live on the board.

## Goal

Replication that survives loss and fits a budget: send until acknowledged, by priority, to whom it concerns (RFC §9.3, §9.6).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/ReplicationState.hpp`
- `src/net/ReplicationState.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/ReplicationState.hpp`

```cpp
// RFC §9.3: per client, what it has acknowledged; snapshots carry only what is missing.
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
};
```

### `include/engine/net/NetComponents.hpp`

```cpp
struct NetPriority { float weight = 1.0f; };   ENGINE_COMPONENT(NetPriority)
```

### `include/engine/net/RoomNetwork.hpp`

```cpp
void setRelevance(std::move_only_function<bool(PlayerId, Entity)> fn);   // default: every entity of the room
```

## Depends on

- [ENG-39](ENG-39.md) — Replication: NetworkIdMap, net components, snapshot and input systems
- [SRV-05](SRV-05.md) — ReliableChannel: packet acks, RTT and loss, reliable ordered messages
- [SRV-14](SRV-14.md) — Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset

## Unlocks

- [SRV-18](SRV-18.md) — Fuzz tests: UDP packets, TCP frames, handshake, snapshot and input decoding
- [SRV-20](SRV-20.md) — Network measurements: snapshot sizes, bandwidth under netem (5 KB/s), reliable latency

## Read first

- [ADR 0041](../adr/0041-netcode-means-in-engine-policy-in-game.md)
- [ADR 0043](../adr/0043-protocol-v1-game-api.md)

## To implement

- [ ] `ReplicationState` per player: acknowledged spawns, destroys and component hashes, fed by `ReliableChannel::newlyAcked()`
- [ ] `SnapshotWriter`: self-contained parts ≤ 1024 bytes, `net.sendBudgetBytesPerSecond`
- [ ] `NetPriority` and the priority accumulator; `RoomNetwork::setRelevance`
- [ ] `.quantize(min, max)` fields encoded as u16 (with SRV-04)
- [ ] Start a player's replication on `RoomReady`, reset on a new roomSession

## Done when

- Tests: 20 % loss → every spawn and destroy eventually applied, no ghost; at a 4 KB/s budget high-priority entities are always fresh

## Watch out

- Only what changed since the last acknowledged state travels: a lost snapshot costs a resend, not a desync

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
