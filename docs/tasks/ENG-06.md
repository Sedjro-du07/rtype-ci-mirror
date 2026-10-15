# ENG-06 · Signal and RAII Subscription

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **01 Core** · **Wave 1** · status and owner live on the board.

## Goal

Synchronous observer list with RAII unsubscription, used by component hooks, CVars and the EventBus.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/core/Signal.hpp`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `include/engine/core/Signal.hpp`

```cpp
class Subscription {                 // move-only; unsubscribes when destroyed
public:
    Subscription() = default;
    Subscription(Subscription&&) noexcept; Subscription& operator=(Subscription&&) noexcept;
    ~Subscription();                 // calls reset()
    void reset() noexcept;           // unsubscribe now
    bool active() const noexcept;
};

template <class... Args>
class Signal<void(Args...)> {
public:
    Signal() = default;              // move-only; allocates nothing until the first connect()
    [[nodiscard]] Subscription connect(std::move_only_function<void(Args...)> fn);
    void emit(Args... args);         // calls every listener now, in connection order
    size_t size() const;
};
```

## Depends on

- nothing: can start now

## Unlocks

- [ENG-18](ENG-18.md) — Registry: typed and type-erased access, identity, hierarchy
- [ENG-21](ENG-21.md) — Component hooks onCreate / onReplace / onDestroy
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events
- [ENG-45](ENG-45.md) — CVars: typed variables, flags, ranges, archive

## Read first

- [ADR 0026](../adr/0026-registry-api.md)
- [ADR 0029](../adr/0029-queued-event-bus.md)
- [reference 06](../engine/reference/06-events.md)

## To implement

- [ ] `Subscription`: move-only, `reset()`, `active()`, unsubscribes in its destructor
- [ ] `Signal<void(Args...)>`: `connect` returns a `Subscription`, `emit` calls listeners in connection order, `size`
- [ ] Safe if a listener disconnects itself or another listener during `emit`

## Done when

- Tests: destroying the Subscription stops calls; disconnect during emit; Signal destroyed before its Subscriptions (no crash)

## Watch out

- A Subscription can outlive its Signal: use a shared control block, not a raw back-pointer

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
