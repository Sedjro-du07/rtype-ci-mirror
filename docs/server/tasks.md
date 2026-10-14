# Server tasks

The server is part of the engine (ADR 0037): its files live in `engine/`, mostly `net/`. Decisions: [Notion › 02 · Server](https://app.notion.com/p/3f9efac8d899811da007fd2c3719434f). The live board, where you claim tasks, is in Notion (**R-Type — Architecture › Server — folder tree & tasks**: https://app.notion.com/p/3f9efac8d89981c5ac2cdefe65b6546d). This file is the reference copy. Team: Ola, Sedjro, Michel, Yann-Ange.

Claim a task yourself on the board: set **Owner** and **Status = Claimed**, and the claim date. Nobody assigns tasks to anyone else.
Start a task only when the tasks it depends on are at least *In review*, or agree on the interface with their owner first.
**What each task contains in detail:** click its ID (pages in [`docs/tasks/`](../tasks/README.md)), or open the task on the board.

| ID | Phase | Task | Files | Depends on | Ref |
|---|---|---|---|---|---|
| [SRV-01](../tasks/SRV-01.md) | 01 Players | Players: PlayerTable, startServer / connectTo, player events, names, kick, ping | `engine/src/net/PlayerTable.hpp`<br>`engine/src/net/PlayerTable.cpp`<br>`engine/include/engine/net/NetworkSession.hpp`<br>`engine/src/net/NetworkSession.cpp` | ENG-38 | ADR 0037 · reference 10 |
| [SRV-02](../tasks/SRV-02.md) | 01 Players | Handshake: player name, format fingerprint, ServerFull / FormatMismatch, TCP ↔ UDP link | `engine/src/net/Handshake.hpp`<br>`engine/src/net/Handshake.cpp` | SRV-01, SRV-06, ENG-22 | ADR 0037 · reference 10 · protocol RFC |
| [SRV-03](../tasks/SRV-03.md) | 02 Messages | Typed messages: MessageDispatcher, send / broadcast / on<T> | `engine/src/net/MessageDispatcher.hpp`<br>`engine/src/net/MessageDispatcher.cpp` | SRV-01, SRV-04, SRV-05, SRV-06 | ADR 0037 · reference 10 |
| [SRV-04](../tasks/SRV-04.md) | 02 Messages | Generic binary codec: lists, structs, enums, Entity ↔ NetworkId, limits | `engine/src/reflection/Codec.hpp`<br>`engine/src/reflection/Codec.cpp` | ENG-22, ENG-24 | ADR 0036, 0037 · reference 04 |
| [SRV-05](../tasks/SRV-05.md) | 03 Transports | ReliableChannel: acknowledged and resent UDP messages | `engine/src/net/ReliableChannel.hpp`<br>`engine/src/net/ReliableChannel.cpp` | ENG-38 | ADR 0037 · protocol RFC |
| [SRV-06](../tasks/SRV-06.md) | 03 Transports | StreamFramer: TCP message framing with size limit | `engine/src/net/StreamFramer.hpp`<br>`engine/src/net/StreamFramer.cpp` | ENG-37 | ADR 0037 · protocol RFC |
| [SRV-07](../tasks/SRV-07.md) | 04 Tests | Tests: players, handshake, messages, codec, malformed input | `engine/tests/net/players/`<br>`engine/tests/net/messages/`<br>`engine/tests/reflection/codec/` | SRV-01, SRV-02, SRV-03, SRV-04, SRV-05, SRV-06 | ADR 0037 · guide 12 |
