# Server - Agent Instructions

These rules extend the root `AGENTS.md` for `src/server/`.

- The server targets Linux/C++17 and builds `ConnectServer`, `JoinServer`, `DataServer`, and `GameServer`.
- Identify the owning process before implementing a feature or fix.
- Use `Common/` only for genuinely shared responsibilities.
- Treat packets and client-provided data as untrusted input.
- Preserve layout, width, byte order, and compatibility of network and persisted structures.
- Review concurrency, lifetime, disconnect, and partial-failure paths when changing shared state.
- Gameplay or persistence changes must consider runtime, database, client, and web panel impact.
- Do not edit `Dependencies/` unless dependency maintenance is the explicit task.

## Read as needed

- Responsibilities and boundaries: `../../docs/architecture.md`.
- C++ implementation: `../../docs/coding-patterns.md`.
- Packet changes: `../../.agents/skills/protocol-change/SKILL.md`.
- Custom entities: `../../.agents/skills/runtime-entity-change/SKILL.md`.
- Persistence: `../../docs/database.md`.
- Validation: `../../docs/testing.md`.

## Quick start

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug
```

Use Release and Compose when required by `docs/testing.md`.
