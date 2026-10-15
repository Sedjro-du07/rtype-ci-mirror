# SRV-02 · Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits

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

struct ServerIdentity { uint64_t fingerprint; std::string gameName; };
using AdmissionFn = std::move_only_function<Admission(const AdmissionRequest&, uint16_t dataId,
                                                       std::span<const std::byte> data)>;   // set by admission<T>

class ServerHandshake {                       // one per pending connection (RFC §6.1–6.4)
public:
    ServerHandshake(const NetConfig& cfg, const ServerIdentity& self, double startedAt);
    bool         onPreamble(std::span<const std::byte, 3> bytes);   // false → Refused(ProtocolMismatch)
    // Checks of RFC §6.4.1 in order; a valid reconnection token is returned for SRV-16 to resume.
    Result<void> onHello(ByteReader& in, size_t playersNow, AdmissionFn* admission,
                         std::function<bool(const ReconnectToken&)> isDisconnected);
    void         writeChallenge(ByteWriter& out);       // random linkToken (OS secure random)
    bool         onUdpLink(uint64_t linkToken, const Endpoint& from);   // token matches → Accepted
    void         writeAccepted(ByteWriter& out, PlayerId id, uint32_t sessionKey, const ReconnectToken& next, bool resumed);
    void         writeRefused(ByteWriter& out) const;   // frozen layout (RFC §4.1)
    bool         expired(double now) const;             // net.handshakeSeconds
    HandshakeState state() const;
    DisconnectReason refusal() const; std::string_view refusalMessage() const;
    const std::string& playerName() const;
    const Endpoint& udpEndpoint() const;
    std::optional<ReconnectToken> resumeToken() const;
};

class ClientHandshake {
public:
    ClientHandshake(std::string playerName, uint64_t fingerprint, std::string gameName,
                    std::optional<ReconnectToken> token, uint16_t dataId, std::vector<std::byte> data);
    void         writePreambleAndHello(ByteWriter& out);
    Result<void> onChallenge(ByteReader& in);   // keeps the link token
    void         writeUdpLink(ByteWriter& out); // 24-byte UDP packet, every 100 ms until Accepted
    Result<void> onResult(ByteReader& in);      // Accepted (id, session key, token, timings) or Refused (reason, message)
    HandshakeState state() const;
    PlayerId me() const; uint32_t sessionKey() const; const ReconnectToken& nextToken() const;
    DisconnectReason refusal() const; std::string_view refusalMessage() const;
};

class PendingLimiter {                        // RFC §11: 16 pending, 4 per address
public:
    explicit PendingLimiter(uint32_t maxPending, uint32_t perAddress = 4);
    bool tryAdd(const Endpoint& from);  void remove(const Endpoint& from);
};
```

## Depends on

- [SRV-01](SRV-01.md) — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping
- [SRV-06](SRV-06.md) — StreamFramer: TCP message framing with size limit
- [ENG-22](ENG-22.md) — TypeRegistry, TypeInfo, builders and type macros (component, event, message, struct, enum)
- [ENG-60](ENG-60.md) — OS layer: secure random, user data folder, executable folder, quit signals

## Unlocks

- [SRV-07](SRV-07.md) — Tests: players, handshake, messages, codec, malformed input
- [SRV-16](SRV-16.md) — Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink

## Read first

- [ADR 0037](../adr/0037-game-facing-networking.md)
- [ADR 0042](../adr/0042-admission-by-the-game.md)
- [ADR 0043](../adr/0043-protocol-v1-game-api.md)
- [RFC §6](../network/rfc.md) (MSEP v1.0)

## To implement

- [ ] `Handshake` (private) state machines of RFC §6.1 on both sides; preamble on both streams
- [ ] Hello: name, fingerprint, game name, reconnection token, admission data (RFC §6.3.1)
- [ ] Checks in the order of RFC §6.4.1: ProtocolMismatch, WrongGame, FormatMismatch, InvalidName, resume, ServerFull, game admission
- [ ] `admission<T>(fn)` on NetworkSession, `Admission`, `AdmissionRequest` (ADR 0042, 0043)
- [ ] Challenge with a random link token; UdpLink every 100 ms; Accepted (id, session key, token, timings); Refused (reason, message)
- [ ] Flood limits: 16 pending, 4 per address, 5 s deadline (`PendingLimiter`)

## Done when

- Tests: each refusal reason with its log line on the client; game admission refuses with its text; handshake times out at 5 s; the 17th pending connection is closed

## Watch out

- Never answer an unauthenticated UDP packet (RFC §6.2)
- The Refused frame layout is frozen across versions (RFC §4.1)
- Tokens and keys from the OS secure random source, never std::rand

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
