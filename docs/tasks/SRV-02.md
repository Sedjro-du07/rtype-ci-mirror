# SRV-02 · Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **01 Players** · **Wave 7** · status and owner live on the board.

## Goal

The connection handshake: who you are, whether your game data matches, and linking TCP with UDP.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/net/Handshake.hpp`
- `src/net/Handshake.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/net/Handshake.hpp`

```cpp
enum class HandshakeState : uint8_t { Idle, Hello, Challenge, LinkUdp, Accepted, Refused };

class ServerHandshake {                       // one per pending connection
public:
    ServerHandshake(const NetConfig& cfg, uint64_t serverFingerprint, size_t playersNow);
    // Each method reads or writes one handshake message; the bytes come from the protocol RFC.
    Result<void> onHello(ByteReader& in);     // name, fingerprint → Refused(ServerFull | FormatMismatch | bad name) or Challenge
    void         writeChallenge(ByteWriter& out);   // the token the client must echo on UDP
    Result<void> onUdpLink(ByteReader& in, const Endpoint& from);   // token matches → Accepted
    HandshakeState state() const;
    DisconnectReason refusal() const;
    const std::string& playerName() const;
    const Endpoint& udpEndpoint() const;
};

class ClientHandshake {
public:
    ClientHandshake(std::string playerName, uint64_t clientFingerprint);
    void         writeHello(ByteWriter& out);
    Result<void> onChallenge(ByteReader& in);   // keeps the token
    void         writeUdpLink(ByteWriter& out); // sent on UDP until Accepted
    Result<void> onResult(ByteReader& in);      // Accepted with my PlayerId, or Refused with a reason
    HandshakeState state() const;
    PlayerId me() const;
    DisconnectReason refusal() const;
};
```

## Depends on

- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)

## Unlocks

- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [reference 10](../engine/reference/10-network.md)
- protocol RFC (Network part, not written yet)

## To implement

- [ ] `Handshake` (private) state machine on both sides
- [ ] Carry the player name (≤ `maxNameLength`) and `TypeRegistry::formatFingerprint()`
- [ ] Refuse with `ServerFull`, `FormatMismatch`, invalid name
- [ ] Link the player's TCP stream and UDP endpoint (token given on TCP, echoed on UDP)

## Done when

- Tests: mismatched fingerprint refused with a clear log line on the client; full server refused

## Watch out

- Bytes and token format come from the protocol RFC
- Unauthenticated UDP from an unknown endpoint is dropped
- Admission by the game comes after the engine's checks (ADR 0042); its stage follows the RFC

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
