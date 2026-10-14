# SRV-05 · ReliableChannel: packet acks, RTT and loss, reliable ordered messages

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **03 Transports** · **Wave 6** · status and owner live on the board.

## Goal

Make `reliable()` UDP messages arrive: acknowledgements and resends.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/ReliableChannel.hpp`
- `src/net/ReliableChannel.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/ReliableChannel.hpp`

```cpp
// RFC §7: packet sequence + ack/ackBits on every Data packet, reliable messages ordered per direction.
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
};
```

## Depends on

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

## Unlocks

- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input
- [SRV-16](SRV-16.md) — Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink
- [SRV-17](SRV-17.md) — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [RFC §7](../network/rfc.md) (RTEP v1.0)

## To implement

- [ ] Packet sequence, ack and 32-bit ackBits on every Data packet; duplicates and packets > 32 behind dropped (RFC §7.1)
- [ ] RTT (smoothed) and loss from acks; ack-only packet 33 ms after reliable data
- [ ] Reliable messages: messageSeq, resend after clamp(1.5 × RTT, 50 ms, 1 s), 256 in flight, in-order delivery (RFC §7.2)
- [ ] `newlyAcked()` for replication (SRV-17)

## Done when

- Test with simulated 20 % loss, reordering and duplicates: every reliable message delivered exactly once and in order

## Watch out

- UDP only: `stream` messages never go through it (network step 3)
- Process ack fields only after the whole packet validated (RFC §4.6)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
