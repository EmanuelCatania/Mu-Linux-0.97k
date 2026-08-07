# World-entity checklist

Use for monsters, NPCs, summons, maps, gates, terrain, minimaps, and spawn data.

## Check

- Confirm actor/map ID ranges, reserved values, serialized widths, persisted
  locations, and a neighboring entity with similar behavior.
- For actors, trace server stats, AI, skills, interaction, rewards, lifecycle,
  spawn/despawn ownership, and client model, animation, sound, effects, and
  selection behavior.
- For maps, trace terrain/attribute loaders, dimensions, safe and blocked zones,
  gates, coordinates, spawns, events, logout position, minimap, name, music,
  lighting, weather, and loading transitions.
- Verify coordinates, directions, radii, pathfinding, reload/restart semantics,
  event cleanup, and duplicate-reward or stale-actor prevention.
- Record all asset provenance and exact file naming/case.
- Load
  [`economy-and-progression.md`](economy-and-progression.md)
  only when drops, rewards, shops, or mixes are part of the requested change.

## Validate

Test spawn or entry paths, boundaries, movement or interaction, death/despawn or
exit, reconnect, reload/restart, missing assets, concurrent instances when
relevant, and one neighboring legacy actor or map.
