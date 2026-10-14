# SRV-05 · ReliableChannel: acknowledged and resent UDP messages

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
class ReliableChannel {                    // one per connection, for reliable() messages over UDP
public:
    struct Pending { uint32_t sequence; uint16_t messageId; std::vector<std::byte> payload; double sentAt; int tries; };
    explicit ReliableChannel(double resendSeconds = 0.2, int maxTries = 10);
    uint32_t send(uint16_t messageId, std::span<const std::byte> payload, double now);   // returns its sequence
    void     onAck(/* ack fields defined by the protocol RFC */);
    bool     accept(uint32_t sequence);                       // false for duplicates
    std::vector<Pending*> due(double now);                    // to resend now
    bool     failed() const;                                  // maxTries reached → connection is dropped
};
```

## Depends on

- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts

## Unlocks

- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- protocol RFC (Network part, not written yet)

## To implement

- [ ] `ReliableChannel` (private) per connection: sequence numbers, ack tracking, resend timer, duplicate drop

## Done when

- Test with simulated 20 % loss: every reliable message delivered exactly once

## Watch out

- Scheme (ack bitfield, ordering) is defined in the protocol RFC: implement after it
- UDP only: `stream` messages never go through it (network step 3)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
