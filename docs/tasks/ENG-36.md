# ENG-36 · INetwork and network types

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **10 Network** · **Wave 3** · status and owner live on the board.

## Goal

Network vocabulary and the byte transport interface.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/net/NetTypes.hpp`
- `include/engine/net/INetwork.hpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/net/NetTypes.hpp`

```cpp
using PlayerId     = uint32_t;                         // what the game sees; 0 = none; one per connected player
using ConnectionId = uint32_t;                         // engine-internal: one UDP endpoint and its TCP stream
struct Endpoint { std::array<uint8_t, 16> address{}; uint16_t port = 0; bool v6 = false;
                  std::string toString() const; static Result<Endpoint> resolve(std::string_view host, uint16_t port); };
struct Datagram { Endpoint from; std::vector<std::byte> bytes; };
// Transport (Unreliable, Reliable, Stream) is declared in reflection/TypeInfo.hpp and chosen per message.
enum class DisconnectReason : uint8_t { Quit, Timeout, Kicked, ProtocolError, ServerShutdown,
                                        ServerFull, FormatMismatch, ProtocolMismatch, WrongGame,
                                        InvalidName, RefusedByGame, ConnectionLost };   // codes: RFC §6.8

struct NetConfig {
    uint16_t port = 4242;
    uint32_t maxClients = 32;             // whole server, lobby included: rooms.maxRooms (8) × 4 players
    double   heartbeatSeconds = 0.5;
    double   timeoutSeconds = 5.0;        // no packet for this long → PlayerLeft{Timeout} / LeftServer{Timeout}
    uint32_t maxDatagramBytes = 1200;     // larger datagrams are dropped, never allocated
    uint32_t maxQueuedDatagrams = 1024;   // inbox bound per frame; excess is dropped and counted
    double   snapshotRate = 20.0;         // replication snapshots per second
    uint32_t maxNameLength = 16;          // player names longer than this are refused at handshake
    uint32_t maxStreamMessageBytes = 65536; // TCP: a larger announced message closes the stream
    double   handshakeSeconds = 5.0;      // TCP connect → Accepted, else Refused(Timeout) (RFC §6.2)
    uint32_t maxPendingHandshakes = 16;   // and 4 per address (RFC §11)
    double   reconnectGraceSeconds = 30.0;// lost session kept this long for its token; 0 = none (RFC §6.7)
    bool     autoReconnect = true;        // client: retry every 2 s during the grace period
    uint32_t sendBudgetBytesPerSecond = 65536; // snapshot bytes per player (RFC §9.6)
    double   interpolationDelay = 0.1;    // client: other entities drawn this far behind (RFC §9.5)
    uint32_t inputRedundancy = 3;         // copies of each input, 1 to 4 (RFC §9.4)
    bool     shareOnlinePlayers = true;   // send the online players list to lobby clients (RFC §8.1.8)
};

struct ConnectionStats { double rttMs; double lossPercent; uint64_t bytesIn, bytesOut;
                         uint64_t packetsIn, packetsOut, dropped; };
```

### `include/engine/net/INetwork.hpp`

```cpp
class INetwork : public IPlugin {
public:
    virtual Result<void> openUdp(uint16_t port) = 0;          // server: bind port; client: port 0
    virtual void         closeUdp() = 0;
    virtual void         sendUdp(const Endpoint& to, std::span<const std::byte> bytes) = 0; // thread-safe, copies
    virtual void         setReceiver(std::move_only_function<void(Datagram&&)> onDatagram) = 0; // called on the I/O thread

    // TCP, for Stream messages (lobby, room lists, text chat; ADR 0037). Same shape, stream-based.
    virtual Result<void> listenTcp(uint16_t port) = 0;
    virtual Result<uint32_t> connectTcp(const Endpoint& to) = 0;
    virtual void         sendTcp(uint32_t stream, std::span<const std::byte> bytes) = 0;
    virtual void         setTcpReceiver(std::move_only_function<void(uint32_t stream, std::span<const std::byte>)>) = 0;
    virtual void         closeTcp(uint32_t stream) = 0;
};
```

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE

## Unlocks

- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-37](ENG-37.md) — network_asio plugin: UDP and TCP on an I/O thread
- [ENG-38](ENG-38.md) — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts
- [SRV-08](SRV-08.md) — Room types and events: PartyId, RoomId, RoomSettings, Party, RoomInfo, lobby / room / client events

## Read first

- [ADR 0033](../adr/0033-engine-networking.md)
- [reference 10](../engine/reference/10-network.md)

## To implement

- [ ] `PlayerId`, `ConnectionId`, `Endpoint` (`resolve`, `toString`), `Datagram`, `DisconnectReason`, `NetConfig`, `ConnectionStats`
- [ ] `INetwork` (UDP and TCP)

## Done when

- Endpoint resolves IPv4 and IPv6 hosts

## Watch out

- `ConnectionId` is engine-internal; games use `PlayerId`

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
