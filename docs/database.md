# Database and persistence

## Responsibilities

- MySQL stores accounts, characters, and persistent state.
- `DataServer` is the server's primary persistence boundary.
- `GameServer`, the web panel, and the editor must respect the same schema and formats.
- Scripts under `runtime/server/MySQL/` must support reproducible local initialization.

## Queries

- Use the existing abstraction and conventions.
- Parameterize every external input.
- Preserve column encoding, width, nullability, and defaults.
- Handle zero, one, or multiple rows according to the contract.
- Do not hide timeouts, connection loss, or truncation.
- Limit lists and use pagination in the web panel.
- Do not log passwords, sensitive hashes, sessions, or full payloads.

## Transactional state

Inventory, warehouse, trade, shop, mix, currency, rewards, and progression must not remain partially updated. Analyze:

- operation order;
- rollback or compensation;
- duplication after retry/reconnect;
- concurrency between sessions;
- failures between memory, packet, and persistence.

## Schema and migration

A schema change must:

1. update initialization scripts;
2. coordinate server, web, and editor changes;
3. define upgrade and rollback behavior;
4. preserve existing data or declare a breaking change;
5. test an empty installation and an existing database when applicable.

Do not use `docker compose down -v` to hide an incorrect migration. Seeds must not contain real data, and destructive operations must remain restricted to the local test environment.

## Binary data

For inventories and blobs, document size, layout, version, and old-data read behavior before changing the format.
