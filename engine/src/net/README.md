# `engine/src/net/`

Headers in this folder are **private** to the engine: they are not under `include/`, so games cannot include them.

Placeholder created from the architecture. Replace this file's list as files are written.

| File to create | Task | Details |
|---|---|---|
| `Handshake.cpp` | SRV-02 — Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits | `docs/tasks/SRV-02.md` |
| `Handshake.hpp` | SRV-02 — Handshake: Hello checks, game admission, Challenge / UdpLink, Accepted / Refused, flood limits | `docs/tasks/SRV-02.md` |
| `MessageDispatcher.cpp` | SRV-03 — Typed messages: MessageDispatcher, send / broadcast / on<T> | `docs/tasks/SRV-03.md` |
| `MessageDispatcher.hpp` | SRV-03 — Typed messages: MessageDispatcher, send / broadcast / on<T> | `docs/tasks/SRV-03.md` |
| `NetTypes.cpp` | ENG-36 — INetwork and network types | `docs/tasks/ENG-36.md` |
| `NetworkIdMap.cpp` | ENG-39 — Replication: NetworkIdMap, net components, snapshot and input systems | `docs/tasks/ENG-39.md` |
| `NetworkSession.cpp` | ENG-38, SRV-01 — NetworkSession core: connections, bounded inbox, validation, heartbeat, timeouts | `docs/tasks/ENG-38.md` |
| `PlayerTable.cpp` | SRV-01 — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping | `docs/tasks/SRV-01.md` |
| `PlayerTable.hpp` | SRV-01 — Players: PlayerTable, startServer / connectTo, player events, names, kick, ping | `docs/tasks/SRV-01.md` |
| `Reconnection.cpp` | SRV-16 — Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink | `docs/tasks/SRV-16.md` |
| `Reconnection.hpp` | SRV-16 — Reconnection: grace period, single-use tokens, client auto-reconnect, UDP relink | `docs/tasks/SRV-16.md` |
| `ReliableChannel.cpp` | SRV-05 — ReliableChannel: packet acks, RTT and loss, reliable ordered messages | `docs/tasks/SRV-05.md` |
| `ReliableChannel.hpp` | SRV-05 — ReliableChannel: packet acks, RTT and loss, reliable ordered messages | `docs/tasks/SRV-05.md` |
| `ReplicationState.cpp` | SRV-17 — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize | `docs/tasks/SRV-17.md` |
| `ReplicationState.hpp` | SRV-17 — Replication v1: send until acknowledged, budget, NetPriority, relevance, quantize | `docs/tasks/SRV-17.md` |
| `RoomNetwork.cpp` | SRV-14 — Routing per room: PlayerRouting, RoomNetwork, ClientLobby, replication reset | `docs/tasks/SRV-14.md` |
| `StreamFramer.cpp` | SRV-06 — StreamFramer: TCP message framing with size limit | `docs/tasks/SRV-06.md` |
| `StreamFramer.hpp` | SRV-06 — StreamFramer: TCP message framing with size limit | `docs/tasks/SRV-06.md` |

Design: `docs/engine/reference/`. Rules: `docs/engine/folder-tree.md`.
