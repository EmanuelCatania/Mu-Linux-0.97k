---
name: runtime-entity-change
description: Add or modify an item, jewel, monster, wing, bow, glow, map, effect, shop, drop, mix, or other entity represented across encoder, client, server, runtime data, database, or web assets.
---

# Runtime entity change

## Goal

Implement a cross-component entity without leaving conflicting IDs, missing assets, incomplete server rules, or stale generated data.

## Read first

Read only:

1. `docs/runtime-data.md`;
2. `docs/architecture.md`;
3. the relevant sections of `docs/testing.md`;
4. existing entries nearest to the requested entity;
5. parsers and consumers of every file to be changed.

Read `docs/protocol.md`, `docs/database.md`, or `docs/client-reverse-engineering.md` only when the entity changes those contracts.

## Workflow

### 1. Classify the entity

Define:

- entity kind and requested behavior;
- canonical ID namespace and allowed range;
- authoritative component;
- whether the change is data-only, code-backed, protocol-visible, or persisted;
- whether classic behavior changes and must be configurable.

Do not choose an ID only because it appears unused in one file.

### 2. Build the representation matrix

Search for matching IDs, names, models, textures, options, and parsers in:

- `runtime/encoder/`;
- `src/client/InfoEncoder/`;
- `src/client/Main/`;
- `runtime/client/Data/`;
- `src/server/GameServer/` and related server code;
- `runtime/server/`;
- `runtime/server/MySQL/`, when persisted;
- `services/web/data/`, views, and public assets;
- `services/editor/`, when editable.

Mark each location as changed, reviewed/no change, or not applicable.

### 3. Preserve file contracts

For every INI, DAT, TXT, SQL, or generated format:

- locate the parser;
- preserve encoding, delimiters, section markers, terminators, order, and case;
- verify ranges, sentinels, duplicate behavior, and cross-file references;
- avoid mass formatting;
- keep deterministic ordering when it affects generated indices or bytes.

### 4. Coordinate client and encoder

Verify:

- encoded structure and deterministic defaults;
- generated `ClientInfo.bmd` compatibility;
- item/model/texture/effect references;
- names, descriptions, positions, options, and rendering paths;
- source-side limits and array bounds;
- asset provenance and authorization.

Do not hand-edit generated binaries or import assets from another game without permission.

### 5. Coordinate server and persistence

Verify:

- authoritative stats and behavior;
- drop, shop, mix, reward, trade, inventory, and modification rules;
- duplication and disconnect behavior;
- serialization and database compatibility;
- packet changes, if the entity exposes new wire state;
- restart or reload requirements;
- classic-mode or feature-flag behavior when appropriate.

Use `.agents/skills/protocol-change/SKILL.md` when the wire contract changes.

### 6. Coordinate web and editor

Verify display metadata, images, limits, database queries, and editor parsing only when the entity is represented there. Administrative editing must preserve backups and file contracts.

### 7. Generate and validate

When encoder inputs change:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action Encode
```

Generate twice and compare hashes for identical inputs.

Run the client, server, Compose, web, database, and editor validations selected by the representation matrix and `docs/testing.md`. Test one normal use, boundary/invalid cases, persistence or reconnect behavior, and missing-asset behavior when relevant.

## Output

Report:

- chosen ID and namespace evidence;
- representation matrix;
- canonical source and duplicated contracts;
- generated files;
- client, server, database, and web effects;
- asset provenance;
- validation performed;
- restart/reload and compatibility requirements;
- reviewed locations that required no change.

## Stop conditions

Stop rather than guess when the ID namespace is unclear, a parser contract is unknown, an asset lacks provenance, a generated layout cannot remain compatible, server authority is undefined, or required client/server representations are unavailable.
