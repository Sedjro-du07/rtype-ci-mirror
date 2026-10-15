# ENG-56 · ECS storage benchmark: sparse set vs sparse array vs archetypes

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **14 Tests & bench** · **Wave 3** · status and owner live on the board.

## Goal

Measure ECS storage choices for the comparative study.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `benchmarks/ecs_storage/`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `benchmarks/ecs_storage/`

```cpp
template <class T> class PagedSparseSet;   // the engine's SparseSet<T>
template <class T> class SparseArray;      // plain std::vector<std::optional<T>> by entity index
class MinimalArchetypeStore;               // one table per component combination
void benchIterate1(benchmark::State&);  void benchIterate2(benchmark::State&);  void benchIterate3(benchmark::State&);
void benchAddRemove(benchmark::State&);
```

## Depends on

- [ENG-17](ENG-17.md) — IStorage and paged SparseSet<T>

## Unlocks

- no task waits for this one

## Read first

- [ADR 0013](../adr/0013-sparse-set-storage.md)

## To implement

- [ ] Sparse set vs sparse array vs minimal archetype store
- [ ] 10 000 entities, 1–3 components, add/remove

## Done when

- Results table written into the study

## Watch out

- Build in Release; repeat runs

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
