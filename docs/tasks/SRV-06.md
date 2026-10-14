# SRV-06 · StreamFramer: TCP message framing with size limit

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **03 Transports** · status and owner live on the board.

## Goal

Cut the TCP byte stream into messages safely.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/StreamFramer.hpp`
- `src/net/StreamFramer.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/StreamFramer.hpp`

```cpp
class StreamFramer {                      // one per TCP stream
public:
    explicit StreamFramer(uint32_t maxMessageBytes);
    Result<void> feed(std::span<const std::byte> bytes);      // error → close the stream
    std::optional<std::vector<std::byte>> next();             // one complete message, or nothing yet
    static void frame(std::span<const std::byte> message, std::vector<std::byte>& out);  // length prefix + bytes
private:
    std::vector<std::byte> buffer_;
    uint32_t               max_;
};
```

## Depends on

- [ENG-37](ENG-37.md) — network_asio plugin: UDP and TCP on an I/O thread

## Unlocks

- [SRV-02](SRV-02.md) — Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link
- [SRV-03](SRV-03.md) — Typed messages: MessageDispatcher, send / broadcast / on<T>
- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- protocol RFC (Network part, not written yet)

## To implement

- [ ] `StreamFramer` (private): length-prefixed framing, partial reads, `maxStreamMessageBytes` limit → close the stream

## Done when

- Tests: messages split across reads, several messages in one read, oversized announcement closes the stream

## Watch out

- Never allocate the announced size before checking the limit

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
