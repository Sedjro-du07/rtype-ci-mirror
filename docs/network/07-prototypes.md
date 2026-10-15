# Network part · Step 7: prototypes and measurements

Step 7 needs code, so it runs at the start of the implementation. Each prototype is a task on the
server board; their results come back here and may correct the RFC through a version 1.1 (only if a
byte layout must change, which increments the protocol version).

| Prototype | What it proves | Task | Can start |
|---|---|---|---|
| Wireshark dissector (Lua) | the RFC is precise enough to read real traffic; debugging and defense demo | [SRV-19](../tasks/SRV-19.md) | now: needs only the RFC |
| Fuzz tests | no input crashes or exhausts the server (RFC §4.6, §11) | [SRV-18](../tasks/SRV-18.md) | after the decoders (SRV-07, SRV-17) |
| Measurements under netem | real sizes and bandwidth against the step 1 budgets; the subject's 5 KB/s link | [SRV-20](../tasks/SRV-20.md) | after replication (SRV-16, SRV-17) |

## Measurements to bring back

Results go in `07-measurements.md` (written by SRV-20):

| Measure | Budget from step 1 | Measured |
|---|---|---|
| bytes per entity in a snapshot (Position only, Position + Velocity + Sprite) | ~16 | |
| snapshot bytes per second, full room of 200 entities, steady state | ≤ 64 KB/s | |
| same, at 5 KB/s budget with quantised positions | playable | |
| client → server bytes per second | ≤ 8 KB/s | |
| reliable delivery time at 5 % / 20 % loss | | |
| time to reconnect after a 10 s cut | | |

## Tools

- Linux: `tc qdisc add dev lo root netem delay 150ms loss 2% duplicate 1% rate 40kbit`
  (`scripts/netem.sh` in SRV-20). Windows: clumsy.
- The console command `net` and the metrics overlay show RTT, loss and bandwidth per player.
