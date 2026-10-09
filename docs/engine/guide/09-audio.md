# 09 · Audio

## Sounds

```cpp
auto boom = ctx.assets().sound("assets/explosion.wav");     // loaded once, cached
ctx.audio().play(boom, {.volume = 0.8f, .position = engine::Vec2{pos.x, pos.y},
                        .caption = "[explosion]"});
```

`position` pans and attenuates from the listener (by default the camera center). On the server
`audio()` does nothing, so shared systems can play sounds safely.

## Music

```cpp
ctx.audio().playMusic(ctx.assets().music("assets/stage1.ogg"), /*fadeIn*/ 1.0f);
ctx.audio().stopMusic(/*fadeOut*/ 0.5f);
```

## Buses and settings

| Bus | CVar |
|---|---|
| Master | `snd_master` |
| Music | `snd_music` |
| Effects | `snd_effects` |
| Interface | `snd_interface` |

Values 0–1, saved in the user settings. Use `.bus = engine::AudioBus::Interface` for menu sounds.

## Captions

A non-empty `caption` publishes `engine::Caption{text, seconds}`. Show captions in the HUD so that
deaf and hard-of-hearing players get every important sound as text.
