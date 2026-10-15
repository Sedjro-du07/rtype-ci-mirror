# 14 · Lobby, parties and rooms (server side of several games)

Status: **Locked**. ADRs 0038 (rooms, shared host), 0039 (room API), 0040 (server and client folders).
Folders (ADR 0040): `include/engine/net/` (vocabulary shared by both sides), `include/engine/server/` and
`src/server/` (server features), `include/engine/client/` and `src/client/` (client features),
`include/engine/ServiceContext.hpp`.

Inclusion rule: game `server/` code may include `engine/server/`, game `client/` code may include
`engine/client/`, game `shared/` code includes neither.

## Role

Let several groups of players play at the same time on one server without sharing a game.

- **Lobby**: every connected player who is not in a game (the server level).
- **Party**: a group of players who stay together between games (one leader). Every player is always
  in a party; a player alone is a party of 1. Whether a party of 1 may start a game is a game rule.
- **Room**: one game: its own world, systems, events, tick and players.

The engine provides the mechanics; the game provides the rules. Client requests (invite, leave, list
rooms) reach the game as events on the lobby; the game answers by calling `Lobby` methods. The engine
never applies a game rule by itself.

Threads (shared host): the lobby runs on the main thread; rooms run on a fixed pool of worker threads,
each room sequentially (never on two threads at once); the network has its own I/O thread. The lobby
and the rooms communicate only through messages (ADR 0038, rule 3).

---

## Identifiers and records — `net/RoomTypes.hpp`

```cpp
using PartyId = uint32_t;                          // 0 = none
using RoomId  = uint32_t;                          // 0 = none (in the lobby)

enum class RoomState : uint8_t { Starting, Running, Finished, Failed, Closing };

struct RoomSettings {                              // given by the game to createRoom
    std::string name;                              // shown in room lists
    std::string level;                             // first scene of the room, relative to the game root
    uint32_t    maxPlayers = 4;
    bool        listed = true;                     // false: private, joined only by the game moving a party in
};

struct Party {
    PartyId                   id;
    PlayerId                  leader;
    std::vector<PlayerId>     members;             // leader included
    RoomId                    room = 0;            // 0 = in the lobby
    size_t size() const;
};

struct RoomInfo {
    RoomId                    id;
    RoomSettings              settings;
    RoomState                 state;
    std::vector<PartyId>      parties;
    std::vector<PlayerId>     players;
};
```

## Events — `net/RoomEvents.hpp`

```cpp
// Lobby (server, published on the lobby EventBus, received with a LobbyContext)
struct PartyChanged         { PartyId party; };                               ENGINE_EVENT(PartyChanged)
struct InviteRequested      { PlayerId from; PlayerId to; };                  ENGINE_EVENT(InviteRequested)
struct InviteAnswered       { PlayerId player; PartyId party; bool accepted; }; ENGINE_EVENT(InviteAnswered)
struct LeavePartyRequested  { PlayerId player; };                             ENGINE_EVENT(LeavePartyRequested)
struct RoomListRequested    { PlayerId player; };                             ENGINE_EVENT(RoomListRequested)
struct RoomStarted          { RoomId room; };                                 ENGINE_EVENT(RoomStarted)
struct RoomFinished         { RoomId room; std::vector<std::byte> result;     // decode with resultAs<T>()
                              template <class T> std::optional<T> resultAs() const; };  ENGINE_EVENT(RoomFinished)
struct RoomFailed           { RoomId room; std::string error; };              ENGINE_EVENT(RoomFailed)
struct RoomClosed           { RoomId room; };                                 ENGINE_EVENT(RoomClosed)

// Room (published on the room's own EventBus, received by systems with a Context)
struct PlayerEnteredRoom    { PlayerId player; PartyId party; };              ENGINE_EVENT(PlayerEnteredRoom)
struct PlayerLeftRoom       { PlayerId player; DisconnectReason reason; };    ENGINE_EVENT(PlayerLeftRoom)

// Client
struct PartyUpdated         { Party party; };                                 ENGINE_EVENT(PartyUpdated)
struct InviteReceived       { PlayerId from; PartyId party; };                ENGINE_EVENT(InviteReceived)
struct RoomListReceived     { std::vector<RoomInfo> rooms; };                 ENGINE_EVENT(RoomListReceived)
struct EnteredRoom          { RoomId room; RoomSettings settings; };          ENGINE_EVENT(EnteredRoom)
struct ReturnedToLobby      { RoomId room; std::string reason; };             ENGINE_EVENT(ReturnedToLobby)
struct RoomInterrupted      { RoomId room; };                                 ENGINE_EVENT(RoomInterrupted)  // the room failed (ADR 0043)
struct OnlinePlayersChanged { };                                              ENGINE_EVENT(OnlinePlayersChanged)
enum class LobbyRequest : uint8_t { Invite = 1, InviteAnswer, LeaveParty, RoomList, Game };       ENGINE_ENUM(LobbyRequest)
struct RequestRefused       { LobbyRequest request; std::string reason; };     ENGINE_EVENT(RequestRefused)
```

## `class Lobby` — `server/Lobby.hpp` (server, game-facing)

Reached with `LobbyContext::lobby()`. Main thread only.

```cpp
class Lobby {
public:
    // Parties
    const Party& party(PartyId p) const;
    const Party& partyOf(PlayerId p) const;           // always exists while the player is connected
    std::span<const PartyId> parties() const;
    void sendInvite(PartyId p, PlayerId to);          // the player receives InviteReceived
    void addToParty(PartyId p, PlayerId player);      // leaves their current party (dissolved if empty)
    void removeFromParty(PlayerId player);            // back to a new party of 1
    void setLeader(PartyId p, PlayerId player);
    template <class T> void sendToParty(PartyId p, const T& msg);   // T: ENGINE_MESSAGE

    // Rooms
    RoomId createRoom(const RoomSettings& settings);  // fails (0 + log) when maxRooms is reached
    void   moveParty(PartyId p, RoomId room);         // whole party enters; PlayerEnteredRoom in the room
    void   moveToLobby(PartyId p, std::string_view reason = {});  // whole party leaves the room; ReturnedToLobby{room, reason}
    void   closeRoom(RoomId room, std::string_view reason = {});  // parties still inside go back to the lobby
    void   refuse(PlayerId to, LobbyRequest request, std::string_view reason);  // client: RequestRefused (ADR 0043)
    void   sendRoomList(PlayerId to, std::span<const RoomId> rooms);   // answer to RoomListRequested
    const RoomInfo& room(RoomId r) const;
    std::span<const RoomId> rooms() const;
};
```

## `class RoomHandle` — `server/RoomHandle.hpp` (in a room, game-facing)

Reached with `Context::room()` on the server.

```cpp
class RoomHandle {
public:
    RoomId id() const;
    const RoomSettings& settings() const;
    std::span<const PlayerId> players() const;
    std::span<const PartyId>  parties() const;
    void keep(Subscription s);                                 // lives as long as the room (subscriptions made in onRoomStart)
    template <class T> void reportFinished(const T& result);   // T: ENGINE_MESSAGE or ENGINE_STRUCT;
                                                               // the lobby receives RoomFinished; the room keeps
                                                               // running until the game closes it
};
```

## `class ClientLobby` — `client/ClientLobby.hpp` (client, game-facing)

Reached with `Context::lobby()` on the client. Every call is a request; answers come back as events.

```cpp
class ClientLobby {
public:
    const Party& myParty() const;
    void invite(PlayerId player);                     // server: InviteRequested
    void answerInvite(PartyId p, bool accept);        // server: InviteAnswered
    void leaveParty();                                // server: LeavePartyRequested
    void requestRoomList();                           // server: RoomListRequested → client: RoomListReceived
    RoomId currentRoom() const;                       // 0 = in the lobby
    std::span<const PlayerId> onlinePlayers() const;  // if the server shares it (net.shareOnlinePlayers); OnlinePlayersChanged
    std::string_view name(PlayerId p) const;          // online players and party members
    bool connected(PlayerId p) const;                 // false while a party member is in their grace period
};

// Room entry: on EnteredRoom the game switches scene; the engine sends RoomReady by itself once that
// switch has completed (or at the end of the frame if none), and replication starts (RFC §8.3).
```

Starting a game is a game rule: the client sends a game message (for example `PlayRequest`) and the
game's lobby code decides.

## Contexts — `ServiceContext.hpp`, `Context.hpp`, `server/LobbyContext.hpp`

ADR 0022 is kept: the game reaches the engine only through a context. There are now three types so
that the compiler refuses world access where there is no world.

```cpp
class ServiceContext {                 // everywhere: shared, thread-safe services
public:
    Logger&            log();          // lines tagged with the room in a room
    const CVars&       cvars() const;  // read-only (written from the lobby or the console)
    AssetManager&      assets();       // locked on the server
    Metrics&           metrics();      // series tagged with the room in a room
    Role               role() const;
};

// class Context : public ServiceContext  — declared in reference 01 (Context.hpp)

class LobbyContext : public ServiceContext { // server lobby code, main thread
public:
    Lobby&             lobby();
    EventBus&          events();            // the lobby's events
    NetworkSession&    network();           // every connected player
    Console&           console();
    CVars&             cvars();             // read and write
    void               requestQuit();       // stops the server
};
```

The engine builds one `Context` per room when the room is created and passes it to every system of
that room at each tick; it builds one `LobbyContext` at server start and passes it to lobby callbacks.
Game code never creates or stores a context.

## `class RoomNetwork` — `net/RoomNetwork.hpp`

The network as seen from a room: the same typed-message API as `NetworkSession` (ADR 0037), limited
to the players of the room.

```cpp
class RoomNetwork {
public:
    std::span<const PlayerId> players() const;                 // the room's players
    template <class T> void send(PlayerId to, const T& msg);   // to a player of the room
    template <class T> void send(const T& msg);                // client → server
    template <class T> void broadcast(const T& msg, std::optional<PlayerId> except = {});   // the room's players
    template <class T> [[nodiscard]] Subscription on(std::move_only_function<void(Context&, PlayerId, const T&)> fn);
    double ping(PlayerId p) const;
    void setRelevance(std::move_only_function<bool(PlayerId, Entity)> fn);   // server: who receives what (RFC §9.6); default: all
};
```

Routing: a game message from a player goes to the `on<T>` callbacks of the player's room when the
player is in a game, to the lobby's `on<T>` callbacks otherwise. Replication sends a player only the
entities of their room; entering or leaving a room resets that player's replication state.

## World singletons (addition to `Registry`, block 3)

```cpp
template <class T> T& singleton();         // the one T of this world, created on first access (ENGINE_COMPONENT)
template <class T> T* trySingleton();      // nullptr if never created
```

Stored on a hidden entity of the world, so they are saved, inspected and replicated like any
component. This is where a game keeps the state of one game (wave, score, ready players).

## Systems per room (change to `Scheduler`, block 5)

Systems are registered once and **instantiated once per room** (ADR 0039): a system with member state
(a functor) gets its own copy in every room. `SystemFn` therefore keeps a copyable prototype.

```cpp
using SystemFn = std::move_only_function<void(Context&)>;
using SystemFactory = std::function<SystemFn()>;               // builds one instance per room

template <class F> SystemBuilder add(Stage stage, std::string name, F fn);   // F copyable: copied per room
SystemBuilder addFactory(Stage stage, std::string name, SystemFactory make);  // F not copyable
std::vector<SystemFn> instantiate(Stage stage) const;          // engine only: one set per room
```

---

## Internal structure (engine only) — `engine/src/server/`

| Class | File | Responsibility |
|---|---|---|
| `PartyTable` | `src/server/PartyTable.hpp` | parties, leaders, membership; one party per player; party of 1 on join |
| `RoomDirectory` | `src/server/RoomDirectory.hpp` | room records, which player and party is in which room, `maxRooms` |
| `RoomRuntime` | `src/server/RoomRuntime.hpp` | one game: world (SceneManager), Commands, EventBus, Time, NetworkIdMap, system instances, its Context, mailboxes |
| `RoomMailbox` | `src/server/RoomMailbox.hpp` | thread-safe message queue lobby → room and room → lobby |
| `IRoomHost` | `src/server/IRoomHost.hpp` | start, stop and post to rooms; hides where a room runs |
| `ThreadPoolRoomHost` | `src/server/ThreadPoolRoomHost.hpp` | shared host (Part 1): ticks every room on the worker pool at the room's tick rate |
| `WorkerPool` | `src/server/WorkerPool.hpp` | fixed pool of threads running jobs |
| `PlayerRouting` | `src/server/PlayerRouting.hpp` | where each player is (lobby or room) for message routing and replication |
| `ServerLoop` | `src/server/ServerLoop.hpp` | the server's main-thread loop (network poll, lobby events, room host), started by `Engine::run()` when the role is Server |

```cpp
class PartyTable {
public:
    PartyId createSolo(PlayerId p);                   // on PlayerJoined
    void    move(PlayerId p, PartyId to);             // dissolves the old party if empty
    void    remove(PlayerId p);                       // on PlayerLeft; promotes a new leader
    void    setLeader(PartyId party, PlayerId p);
    const Party& get(PartyId p) const;  PartyId of(PlayerId p) const;
    std::span<const PartyId> all() const;
};

class RoomDirectory {
public:
    explicit RoomDirectory(uint32_t maxRooms);
    std::optional<RoomId> add(const RoomSettings& s);
    void remove(RoomId r);
    void setState(RoomId r, RoomState s);
    void enter(RoomId r, const Party& party);  void leave(RoomId r, PartyId party);
    const RoomInfo& get(RoomId r) const;  RoomId roomOf(PlayerId p) const;
    std::span<const RoomId> all() const;
};

struct RoomMessage { enum class Kind : uint8_t { PlayerEntered, PlayerLeft, NetMessage, Finished, Failed, Close };
                     Kind kind; PlayerId player = 0; uint16_t messageId = 0; std::vector<std::byte> payload; };

class RoomMailbox {                                   // multi-producer, single-consumer
public:
    void push(RoomMessage m);
    std::vector<RoomMessage> drain();
};

class RoomRuntime {
public:
    RoomRuntime(RoomId id, const RoomSettings& s, const EngineServices& services, const Scheduler& prototypes);
    Result<void> start();                             // loads settings.level, instantiates systems
    void tick();                                      // FixedUpdate / LateUpdate of this room; one thread at a time
    RoomMailbox& inbox();  RoomMailbox& outbox();
    bool failed() const;  RoomId id() const;
};

class IRoomHost {
public:
    virtual ~IRoomHost() = default;
    virtual Result<void> start(std::unique_ptr<RoomRuntime> room) = 0;
    virtual void stop(RoomId room) = 0;
    virtual void post(RoomId room, RoomMessage m) = 0;
    virtual void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) = 0;   // lobby thread
};

class ThreadPoolRoomHost final : public IRoomHost {
public:
    ThreadPoolRoomHost(WorkerPool& pool, double tickRate, Logger& log);
    Result<void> start(std::unique_ptr<RoomRuntime> room) override;
    void stop(RoomId room) override;
    void post(RoomId room, RoomMessage m) override;
    void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) override;
};

class WorkerPool {
public:
    explicit WorkerPool(unsigned threads);            // 0 = std::thread::hardware_concurrency()
    ~WorkerPool();                                    // joins
    void submit(std::move_only_function<void()> job);
    unsigned size() const;
};

class PlayerRouting {
public:
    void   setRoom(PlayerId p, RoomId r);             // 0 = lobby
    RoomId roomOf(PlayerId p) const;
    std::span<const PlayerId> playersIn(RoomId r) const;
};
```

A system that throws in a room stops that room (`RoomState::Failed`, `RoomFailed` in the lobby); other
rooms keep running; the game decides what to do.

## Configuration — `EngineConfig` (`server.yaml`)

```yaml
rooms:
  host: shared          # Part 1; "dedicated" in Part 2 (ADR 0038)
  maxRooms: 8
  workerThreads: 0      # 0 = number of cores
```
