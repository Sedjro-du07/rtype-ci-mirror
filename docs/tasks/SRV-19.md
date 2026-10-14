# SRV-19 · Wireshark dissector for RTEP (Lua)

Board: **Server — folder tree & tasks** ([Notion](https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d)) · Phase **04 Tests** · **Wave 1** · status and owner live on the board.

## Goal

See RTEP in Wireshark: debugging aid and defense demo (network step 7).

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `tools/wireshark/rtep.lua`

## Classes and methods

**Declarations only**: what this task must write, without the bodies. Write the implementation in the matching `.cpp` (or in the header for templates). Behaviour of each method: the comments here and the pages in *Read first*.

### `tools/wireshark/rtep.lua`

```lua
-- Wireshark dissector for RTEP v1.0 (heuristic on magic 0x5254 + version 1, UDP and TCP)
local rtep = Proto("rtep", "R-Type Engine Protocol")
-- fields: header (kind, sessionKey, sequence, ack, ackBits), messages (id → name from RFC §8.5,
-- messageSeq for reliable ids, length), Hello / Accepted / Refused / Snapshot / Input payloads
function rtep.dissector(buffer, pinfo, tree) end
local function heuristic(buffer, pinfo, tree) end   -- registered for udp and tcp
```

## Depends on

- nothing: can start now

## Unlocks

- no task waits for this one

## Read first

- RFC (all) · network step 7

## To implement

- [ ] Lua dissector: heuristic on magic + version, UDP and TCP
- [ ] Header fields, message list, names from RFC §8.5, reliable messageSeq
- [ ] Hello, Accepted, Refused, Snapshot and Input payloads decoded

## Done when

- The RFC §15 examples, replayed from a capture, are shown field by field

## Watch out

- Only needs the RFC: can start before the code exists

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
