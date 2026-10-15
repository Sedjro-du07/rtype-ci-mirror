# 22 · Roles: client, server, editor

Every process runs the same engine in one role, set in the config (`role:`) or with `--role`.

| | Client | Server | Editor (Part 2) |
|---|---|---|---|
| Window, rendering | platform plugin | none (Null) | platform plugin |
| Audio | audio plugin | none (Null) | audio plugin (preview) |
| Network | connects to a server | hosts clients | none |
| Scripting | yes | yes | yes |
| Stage `Render` | runs | empty | runs |
| A system throws | message, clean exit (code 5) | system disabled, game continues | message |
| Who decides gameplay | nobody: shows and predicts | **the server** | nobody: edits |

## Writing systems for roles

```cpp
void RTypeModule::registerSystems(engine::Role role, engine::Scheduler& s) {
    s.add(Stage::FixedUpdate, "rtype.player_control", playerControl);           // both roles
    s.add(Stage::FixedUpdate, "rtype.damage", DamageSystem{...}).roles(engine::ServerRole);
    s.add(Stage::Render, "rtype.hud", hud).roles(engine::ClientRole);
}
```

| Kind of system | Role |
|---|---|
| anything that changes the outcome: damage, death, spawning, score, pickups | **server only** |
| movement of networked entities | server (clients receive it); a client may also run it for its own ship to feel responsive |
| drawing, HUD, menus, camera, local effects | client only |
| pure helpers (math on components) | both |

## Null Objects

On the server, `ctx.audio()` and `ctx.render()` are Null Objects: calls do nothing and never crash.
A shared system that plays a sound is therefore safe. Prefer registering client-only systems with
`.roles(ClientRole)` anyway; the Null Objects are a safety net, not a design.

## Same machine, several roles

During development you run one server and up to four clients locally:

```bash
./r-type_server &
./r-type_client & ./r-type_client
```

How solo play, lobbies and rooms work is decided in the Server part of the architecture.
