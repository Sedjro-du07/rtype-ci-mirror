# Security

The server faces the internet: anyone can send it any bytes. The rule is that **no input from the
network can crash the server, block it or make it allocate without limit**. This page lists how the
engine holds that rule and what it leaves out.

## Threats and countermeasures

| Threat | Countermeasure | Where | Status |
|---|---|---|---|
| malformed datagram | checks in a fixed order before any use: size 16–1200, magic and version, kind, session key, sequence window, then every message (id, transport, length) | `NetworkSession::receive`, RFC §4.6 | done |
| one bad message in a packet | all messages are decoded before any is delivered; one failure drops the whole packet | same | done |
| out-of-bounds read | `ByteReader` checks every read and fails instead of reading past the end; strings and lists have a maximum length | `ByteStream`, `Codec` | done |
| packet flood | more than 300 packets per second from a connection are dropped | `admitRate` | done |
| garbage flood | more than 50 invalid packets in 10 s close the connection (`ProtocolError`) | `strike` | done |
| memory exhaustion | the inbox between the I/O thread and the game is bounded (`maxQueuedDatagrams`); excess is dropped and counted | `NetworkSession` inbox | done |
| blind UDP spoofing | the UDP address is bound only through a token received on TCP; every Data packet carries the session key | handshake, RFC §6.3 | SRV-02 |
| handshake flood | 16 pending handshakes, 4 per address, 5 s deadline | RFC §11 | SRV-02 |
| replay and duplicates | sequence window, newest tick wins for snapshots, newest input sequence for inputs, room session checked | `Replication` | done |
| cheating client | the server is authoritative: clients send only action bits and requests; unknown bits are ignored | `Replication::receiveInput` | done |
| path traversal | asset paths outside the game root are refused | `AssetManager` | done |
| exception in game code | caught by the scheduler: on the server the system is disabled and the game goes on | `Scheduler::fault`, [ADR 0024](../adr/0024-error-handling-policy.md) | done |

Every drop is **counted** (`NetCounters`: size, magic, kind, session, sequence, message, rate, inbox,
closed), so an attack is visible instead of silent.

## A bug the rules caught

Writing replication (ENG-39) showed that the binary codec wrote asset handles as if they were
strings, which corrupted memory. It was found by a test and fixed before any release. The lesson
kept: every new wire type gets a round-trip test, and the CI runs the tests under AddressSanitizer
locally before each merge.

## Not covered (on purpose)

- **Encryption.** Someone who can read the traffic (same Wi-Fi) can read tokens and session keys and
  impersonate a player. Protecting against that needs DTLS or a QUIC-like scheme; the subject does
  not ask for it and the game carries no secret ([RFC §12](../network/rfc.md)).
- **Accounts and passwords.** There are none: a player is a name for one session. See
  [Legal and privacy](legal-and-privacy.md).
- **Fuzzing.** Planned as SRV-18 (packets, frames, handshake, snapshot and input decoding).
