# SRV-16 · Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **01 Players** · **Wave 8** · status and owner live on the board.

## Goal

A lost connection is not a lost player: the grace period, the token and the way back (RFC §6.7).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/Reconnection.hpp`
- `src/net/Reconnection.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/Reconnection.hpp`

```cpp
using ReconnectToken = std::array<std::byte, 16>;

class GraceTable {                            // server: lost sessions kept for net.reconnectGraceSeconds (RFC §6.7)
public:
    explicit GraceTable(double graceSeconds);
    ReconnectToken issue(PlayerId p);                    // new single-use token, OS secure random
    void     disconnected(PlayerId p, DisconnectReason why, double now);   // → PlayerDisconnected
    std::optional<PlayerId> claim(const ReconnectToken& t);                // matches a disconnected player, consumes it
    std::vector<std::pair<PlayerId, DisconnectReason>> expired(double now); // → PlayerLeft
    bool     isDisconnected(PlayerId p) const;
};

class ClientReconnector {                     // client: Interrupted → retry every 2 s while the grace lasts
public:
    ClientReconnector(double graceSeconds, bool enabled);
    void interrupted(DisconnectReason why, double now);  // → ConnectionInterrupted
    bool shouldRetry(double now);
    bool givenUp(double now) const;                      // → LeftServer
    void succeeded();                                    // → Reconnected
};

class UdpRelink {                             // client: 1 s of UDP silence with TCP open → UdpLink every 100 ms (RFC §6.7.3)
public:
    bool due(double lastUdpReceived, bool tcpOpen, double now);
};
```

### `include/engine/events/EngineEvents.hpp`

```cpp
struct PlayerDisconnected   { PlayerId player; DisconnectReason reason; };   ENGINE_EVENT(PlayerDisconnected)
struct PlayerReconnected    { PlayerId player; };                            ENGINE_EVENT(PlayerReconnected)
struct ConnectionInterrupted{ DisconnectReason reason; };                    ENGINE_EVENT(ConnectionInterrupted)
struct Reconnected          { };                                             ENGINE_EVENT(Reconnected)
```

## Depends on

- [SRV-02](SRV-02.md) — Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits
- [SRV-05](SRV-05.md) — ReliableChannel: packet acks, RTT and loss, reliable ordered messages

## Unlocks

- [SRV-20](SRV-20.md) — Network measurements: snapshot sizes, bandwidth under netem (5 KB/s), reliable latency

## Read first

- [ADR 0043](../adr/0043-protocol-v1-game-api.md)

## To implement

- [ ] `GraceTable`: Disconnected sessions kept `net.reconnectGraceSeconds`, `PlayerDisconnected` / `PlayerReconnected` / `PlayerLeft` (lobby and room)
- [ ] Single-use 128-bit reconnection tokens from the OS secure random source, issued in every Accepted
- [ ] Resume: same PlayerId, party (`PartyUpdated` with the connected flag), room (`EnteredRoom` again, replication from zero)
- [ ] `ClientReconnector`: `ConnectionInterrupted`, retry every 2 s, `Reconnected` or `LeftServer`
- [ ] `UdpRelink`: 1 s of UDP silence with TCP open → UdpLink again (RFC §6.7.3)

## Done when

- Test: cable pulled 10 s in a room → same PlayerId back in the same room with the full state; 40 s → PlayerLeft; NAT rebinding simulated by changing the client port → traffic resumes

## Watch out

- A disconnected leader stays leader during the grace period
- The game's admission runs again on reconnection (ADR 0042)

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
