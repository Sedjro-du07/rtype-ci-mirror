# Network part · Step 4: design choices

The questions of step 2, decided one by one, layer by layer from the bottom. Each decision gives the
options, what real protocols do, and the choice. They are written into the RFC (step 5).

| Round | Layer | Questions | Status |
|---|---|---|---|
| 1 | Frame | byte order, header, protocol version (1), engine / game id ranges, aggregation (25), splitting over 1200 bytes (21) | in progress |
| 2 | Session | 1–6, 7–11, 32–37 | |
| 3 | Reliability | 20, 22, 24 | |
| 4 | Replication | 3, 23, 26, 27 | |
| 5 | Lobby, party, rooms | 12–19, 28–31 | |

## Round 1 · Frame

### D1 · Byte order: big-endian

Every multi-byte number on the wire, UDP and TCP, engine and game, is **big-endian** (most significant
byte first, "network byte order"). Floats are IEEE 754 (Linux and Windows), sent as their 32-bit
pattern (`std::bit_cast<uint32_t>`) in the same order. Conversion lives in one place,
`ByteWriter` / `ByteReader` (ENG-24); the game never sees it (ADR 0037).

- Options: big-endian (IP, TCP, UDP, QUIC, most RFCs, the CMP spec, the subject's example on slide
  track2_9, Corewar) or little-endian (x86 and ARM CPUs, Quake 3, Unreal, Source, Protobuf).
- Why: hex dumps read directly (debugging, Wireshark demo), consistent with the subject and the
  RFC tradition; the cost (one `bswap` per field) is invisible at a few thousand fields per second.
