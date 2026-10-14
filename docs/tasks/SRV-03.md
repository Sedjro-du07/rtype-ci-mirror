# SRV-03 · Typed messages: MessageDispatcher, send / broadcast / on<T>

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **02 Messages** · **Wave 7** · status and owner live on the board.

## Goal

Typed messages for games: send, broadcast and on<T> callbacks.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/MessageDispatcher.hpp`
- `src/net/MessageDispatcher.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/MessageDispatcher.hpp`

```cpp
class MessageDispatcher {
public:
    MessageDispatcher(const TypeRegistry& types, Logger& log);
    uint16_t        idOf(uint64_t typeKey) const;                 // position in TypeRegistry::messages()
    const TypeInfo* typeOf(uint16_t id) const;                    // nullptr → dropped and counted
    Subscription    add(uint64_t typeKey, std::move_only_function<void(Context&, PlayerId, const void*)> fn);
    void            dispatch(Context& ctx, PlayerId from, uint16_t id, ByteReader& payload, const WireContext& wire);
                                                                  // decode with Codec, call every callback, catch exceptions
private:
    std::vector<std::vector<Callback>> callbacks_;                // indexed by message id
};
```

### `include/engine/net/NetworkSession.hpp`

```cpp
// The message part of NetworkSession (core: ENG-38, players: SRV-01).
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
};
```

## Depends on

- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-04](SRV-04.md) — Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits
- [SRV-05](SRV-05.md) — ReliableChannel: packet acks, RTT and loss, reliable ordered messages
- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit

## Unlocks

- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input
- [SRV-13](SRV-13.md) — Lobby and RoomHandle: the server game API and the server loop

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 10](../engine/reference/10-network.md)

## To implement

- [ ] `MessageDispatcher` (private): message id ↔ TypeInfo, callback lists per id
- [ ] `send<T>` ×2, `broadcast<T>` choosing the transport declared on the message
- [ ] `on<T>` ×2 returning a Subscription
- [ ] Call callbacks during `poll()` on the loop thread; catch exceptions and log with the message name

## Done when

- Tests: Chat over stream, WaveCleared over reliable, Ping over unreliable; exception in a callback does not stop the server

## Watch out

- Message ids follow `TypeRegistry::messages()` order
- Callbacks must check `Entity::isNull()`
- Engine lobby messages use the same mechanism with their own id range (network step 3; ranges in the RFC)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
