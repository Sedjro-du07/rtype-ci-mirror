# ENG-34 · IAudio, PlayParams, buses, captions

Board: **Engine — folder tree & tasks** ([Notion](https://app.notion.com/p/3f4efac8d89981ba846fe7cddf25ead7)) · Phase **09 Audio** · status and owner live on the board.

## Goal

The audio interface, buses and captions for every backend.

## Files

Relative to `engine/`. Each file already exists as a skeleton naming this task.

- `include/engine/audio/IAudio.hpp`
- `src/audio/AudioCaptions.cpp`

## Depends on

- [ENG-11](ENG-11.md) — IPlugin, IGameModule, export macros ENGINE_PLUGIN / ENGINE_GAME_MODULE
- [ENG-27](ENG-27.md) — EventBus: queued, immediate, thread inbox; engine events

## Unlocks

- [ENG-15](ENG-15.md) — Null Objects: NullPlatform, NullRenderer, NullAudio, NullNetwork, NullScripting
- [ENG-35](ENG-35.md) — audio_miniaudio plugin
- [ENG-40](ENG-40.md) — AssetManager: handles, cache, refcount, fallback, hot reload

## Read first

- [ADR 0032](../adr/0032-audio-buses-captions.md)
- [reference 09](../engine/reference/09-audio.md)

## To implement

- [ ] `AudioBus`, `PlayParams`, `VoiceId`, `IAudio` declarations
- [ ] Caption wrapper publishing `Caption` events

## Done when

- Test with NullAudio: a play with a caption publishes Caption

## Watch out

- Captions come from the wrapper, not the plugin

## Workflow

1. Claim the task on the board (Owner, Status = Claimed, Claimed on).
2. Start when every dependency is at least *In review*, or agree on the interface with its owner.
3. Implement, with tests; open a pull request and set *In review*.
4. Merged → *Done* and Completed on.
