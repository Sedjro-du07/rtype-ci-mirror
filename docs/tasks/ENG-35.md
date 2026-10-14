# ENG-35 · audio_miniaudio plugin

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **09 Audio** · status and owner live on the board.

## Goal

One audio plugin for all platforms, on miniaudio.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `plugins/audio_miniaudio/CMakeLists.txt`
- `plugins/audio_miniaudio/MiniaudioAudio.hpp`
- `plugins/audio_miniaudio/MiniaudioAudio.cpp`

## Depends on

- [ENG-34](ENG-34.md) — IAudio, PlayParams, buses, captions
- [ENG-02](ENG-02.md) — CMake functions engine_add_plugin, engine_add_game_module, engine_add_launcher

## Unlocks

- no task waits for this one

## Read first

- [ADR 0008](../adr/0008-shared-audio-plugin.md)
- [ADR 0032](../adr/0032-audio-buses-captions.md)

## To implement

- [ ] One `ma_engine`, one sound group per bus
- [ ] Sounds from memory, streamed music, positional sound, fades

## Done when

- Effects and music play; bus volumes change live; no device → NullAudio with a warning

## Watch out

- miniaudio callbacks run on its own thread

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
