# Storage

What the engine reads and writes, in which format, and why.

| Data | Format | Where | Written by |
|---|---|---|---|
| engine configuration | YAML + command-line overrides | `<game>.yaml` next to the launcher | the team |
| scenes | YAML, entities with components and prefab overrides | `scenes/*.yaml` | the team, later the editor |
| prefabs | YAML, components and children | `prefabs/*.yaml` | the team |
| input bindings | YAML, several bindings per action | `input.yaml` | the team, then the player (rebinding) |
| logs | text, one line per record | `logs/engine.log` | the engine |
| network messages | binary, big-endian, bounded | on the wire only | the engine |

## Text formats for everything a person edits

YAML was chosen over JSON, TOML and a binary format (see [Libraries](libraries.md#data-files)):
levels and prefabs are written by hand, reviewed in pull requests, and must report mistakes at the
right line.

- **Errors name the place.** `scenes/level1.yaml:12: unknown component 'Helth' (did you mean
  'Health'?)`. A broken scene never leaves half an entity behind: loading is two passes (create
  everything, then link `uuid` references), and a failed load keeps the current scene
  ([ADR 0034](../adr/0034-scenes-prefabs-assets.md)).
- **Saved scenes stay small.** Saving writes only what differs from each entity's prefab, so a level
  is a list of prefabs and positions, not thousands of copied values.
- **No code in data.** YAML holds values; behaviour lives in systems or scripts
  ([ADR 0018](../adr/0018-data-in-yaml-behaviour-in-scripts.md)).
- **One reader for every type.** Components are read and written through reflection
  (`TypeRegistry`), so a new game component needs no parser.

## Paths

Asset paths are relative to the game root, use `/`, and must match the file's case exactly, checked
on Windows too, so a game that runs on one system runs on the other. A path that leaves the game root
(`../../etc/passwd`) is refused ([ADR 0044](../adr/0044-platforms-and-portability.md)).

## Binary only on the wire

The network format is binary because size matters there (1024 bytes per message) and nobody edits it
by hand. Every field has a fixed encoding (RFC §5): big-endian integers, IEEE 754 floats, strings
with a length prefix and a maximum, lists with a count and a maximum. Entities travel as
`NetworkId`, assets as their path. The same reflection drives YAML and binary, so a component needs
no extra code to be saved or replicated.

## Not stored yet

- **Player settings** (volumes, UI scale, colour filter, rebinding) belong in the user data folder
  (`os::userDataDir`) through CVars marked `Archive` (ENG-45, not written yet).
- **Save games**: none in Part 1; R-Type runs are short.
