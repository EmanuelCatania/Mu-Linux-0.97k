---
name: runtime-entity-change
description: Add or modify an item, jewel, monster, wing, bow, glow, map, effect, shop, drop, mix, or other entity represented across encoder, client, server, runtime data, database, or web assets.
---

# Runtime entity change

Implement a cross-component entity without conflicting IDs, missing
representations, or stale generated data.

## Load

Always read `docs/runtime-data.md`, `docs/architecture.md`, relevant parts of
`docs/testing.md`, the nearest existing entries, and each changed format's parser
and consumers. Read protocol, database, or reverse-engineering docs only when
those contracts change.

Load only the applicable differential checklist:

- items, jewels, equipment, wings, bows, glows, and item-bound effects:
  [`references/item-system.md`](references/item-system.md);
- monsters, NPCs, summons, maps, gates, terrain, minimaps, and spawns:
  [`references/world-entity.md`](references/world-entity.md);
- shops, drops, rewards, events, and mixes:
  [`references/economy-and-progression.md`](references/economy-and-progression.md).

Load multiple references only when the requested behavior crosses those domains.

## Workflow

1. **Classify.** Define behavior, canonical ID namespace/range, authoritative
   component, and whether the change is data-only, code-backed, protocol-visible,
   persisted, generated, or configurable.
2. **Map representations.** Search IDs, names, assets, options, parsers, and
   consumers across encoder, client, server, runtime data, database, web, and
   editor. Mark each relevant location `Changed`, `Reviewed / no change`, or
   `Not applicable`.
3. **Preserve contracts.** Update the authoritative source before mirrors;
   preserve format syntax, ordering, sentinels, ranges, duplicated layouts, and
   server authority. Do not hand-edit generated binaries or use unlicensed
   assets.
4. **Generate and validate.** Re-run the encoder when its inputs change; compare
   deterministic outputs; select checks from the representation matrix,
   differential references, and `docs/testing.md`. Use the protocol skill when
   wire state changes.
5. **Report.** Record ID evidence, loaded references, representation matrix,
   authoritative and mirrored sources, generated outputs, asset provenance,
   validation, and restart or compatibility requirements.

Stop rather than guess when the namespace, parser, authority, asset provenance,
or compatibility boundary is unresolved.
