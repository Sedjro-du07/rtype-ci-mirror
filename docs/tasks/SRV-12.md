# SRV-12 · WorkerPool, IRoomHost and ThreadPoolRoomHost: rooms in parallel on a thread pool

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **05 Rooms** · **Wave 13** · status and owner live on the board.

## Goal

Run every room in parallel on a fixed pool of threads, each room sequentially.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `src/server/WorkerPool.hpp`
- `src/server/WorkerPool.cpp`
- `src/server/IRoomHost.hpp`
- `src/server/ThreadPoolRoomHost.hpp`
- `src/server/ThreadPoolRoomHost.cpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `src/server/WorkerPool.hpp`

```cpp
class WorkerPool {
public:
    explicit WorkerPool(unsigned threads);            // 0 = std::thread::hardware_concurrency()
    ~WorkerPool();                                    // joins
    void submit(std::move_only_function<void()> job);
    unsigned size() const;
};
```

### `src/server/IRoomHost.hpp`

```cpp
class IRoomHost {
public:
    virtual ~IRoomHost() = default;
    virtual Result<void> start(std::unique_ptr<RoomRuntime> room) = 0;
    virtual void stop(RoomId room) = 0;
    virtual void post(RoomId room, RoomMessage m) = 0;
    virtual void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) = 0;   // lobby thread
};
```

### `src/server/ThreadPoolRoomHost.hpp`

```cpp
class ThreadPoolRoomHost final : public IRoomHost {
public:
    ThreadPoolRoomHost(WorkerPool& pool, double tickRate, Logger& log);
    Result<void> start(std::unique_ptr<RoomRuntime> room) override;
    void stop(RoomId room) override;
    void post(RoomId room, RoomMessage m) override;
    void collect(std::move_only_function<void(RoomId, RoomMessage&&)> toLobby) override;
};
```

## Depends on

- [SRV-11](SRV-11.md) — RoomRuntime and RoomMailbox: one isolated game, lobby ↔ room messages

## Unlocks

- [SRV-13](SRV-13.md) — Lobby and RoomHandle: the server game API and the server loop

## Read first

- [ADR 0038](../adr/0038-rooms-shared-host.md)
- [ADR 0039](../adr/0039-room-api.md)
- [reference 14](../engine/reference/14-rooms.md)

## To implement

- [ ] `WorkerPool`: N threads (0 = cores), `submit(job)`, clean join
- [ ] `IRoomHost` interface: start, stop, post, collect
- [ ] `ThreadPoolRoomHost`: schedules each room's tick at its tick rate on the pool; never two ticks of one room at once; collects outboxes for the lobby

## Done when

- 50 rooms at 60 ticks/s on 8 threads keep their tick rate in a stress test
- A slow room does not delay the others

## Watch out

- `ProcessRoomHost` (dedicated) will implement the same interface in Part 2: keep it free of thread assumptions

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
