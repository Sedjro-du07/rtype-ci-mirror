# Legal and privacy

What the engine and the game do with personal data (GDPR), the licences of what we ship, and the
intellectual property around R-Type. This page describes the project as it is; it is not legal
advice.

## Personal data (GDPR)

The GDPR applies to any information about an identifiable person. An IP address counts, even a
dynamic one. A player name counts when it identifies someone.

### What is processed

| Data | Why | Where it lives | How long |
|---|---|---|---|
| IP address and port | to send the game to the right machine | server memory, per connection | until the player leaves (plus the reconnection grace period, 30 s by default) |
| player name (max 16 characters, chosen freely) | shown to other players in the lobby and the game | server memory; sent to the other players of the same server | until the player leaves |
| session key, link and reconnection tokens | to tie UDP packets and reconnections to the right player | server and client memory | the session; tokens are single use |
| game inputs (action bits) | to play | server memory | a few ticks |
| logs | to find bugs | `logs/engine.log` on the machine running the engine | until deleted by whoever runs it |

**Not processed:** no account, no password, no e-mail, no payment, no location, no analytics, no
advertising, no cookies, no third-party service. Nothing is sent anywhere except between the players'
machines and the server they chose.

### How the principles are met

| GDPR principle | In the project |
|---|---|
| lawfulness (art. 6) | the data is needed to provide the game the player asks to join (performance of the service) |
| data minimisation (art. 5.1.c) | only what the protocol needs; a name, not an identity; inputs, not a camera or microphone |
| storage limitation (art. 5.1.e) | everything stays in memory and disappears when the player leaves; nothing is written to disk about players |
| integrity and confidentiality (art. 5.1.f) | tokens come from the operating system's secure random source; invalid packets are dropped (see [Security](security.md)) |
| privacy by default (art. 25) | the logs name connections by a number, never by address or player name |

**Known limit:** traffic is not encrypted, so someone on the same network can see addresses, names
and inputs. The game carries no sensitive data, which is why encryption was left out of version 1
([RFC §12](../network/rfc.md)).

### Rules for the team

- Do not log IP addresses or player names. If debugging needs them, log at `Debug` level only and do
  not ship builds with that level.
- Do not add accounts, chat history, statistics or telemetry without updating this page first: each
  one needs a purpose, a retention time and a way for a player to ask for deletion.
- Text chat (if added) is personal data too: keep it in memory only, never in logs.

### For whoever runs a public server

They become the data controller for the IP addresses and names their server sees. A short notice
in the game ("your IP address and the name you choose are used to connect you and are shown to other
players; nothing is kept after you leave") is enough for this data.

## Licences of what we ship

| Dependency | Licence | What it asks |
|---|---|---|
| SFML 3 | zlib | nothing in binaries; do not claim you wrote it |
| miniaudio | public domain (Unlicense) or MIT No Attribution | nothing |
| standalone Asio | Boost Software License 1.0 | nothing in binaries |
| yaml-cpp | MIT | keep the notice |
| Lua 5.4 / sol2 | MIT / MIT | keep the notice |
| Dear ImGui, ImGui-SFML, ImPlot | MIT | keep the notice |
| tl-expected | CC0 1.0 | nothing |
| Catch2 (tests only) | Boost Software License 1.0 | not shipped |
| Google Benchmark (benchmarks only) | Apache 2.0 | not shipped |

SFML itself uses FreeType, libpng and others under permissive licences. All of them allow use in a
closed or open project. A release must ship a `THIRD_PARTY_NOTICES` file with the MIT notices
(to do with the CPack packages, ENG-62).

**Project licence.** The repository has no `LICENSE` file yet. The team must choose one (or state
"all rights reserved" for a school project) before the code is made public.

## Intellectual property

- **R-Type** is a trademark of Irem. The project is a non-commercial school exercise inspired by the
  game. Sprites and sounds taken from the original game must not be redistributed outside the school
  or sold; a public release needs original or freely licensed assets.
- **Fonts and assets** added to the repository must come with their licence (for example DejaVu,
  used by the tests, is free).
- **Our own code** belongs to its authors under the licence the team chooses; contributions by each
  member are traceable in the git history.
