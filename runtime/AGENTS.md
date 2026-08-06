# Runtime - Agent Instructions

These rules extend the root `AGENTS.md` for `runtime/`.

- This directory contains tracked templates and data; it is not the external executable runtime.
- Treat INI, DAT, TXT, SQL, assets, and game data as code with compatibility contracts.
- Preserve format, order, terminators, encoding, case, and parser-required cross-references.
- Duplicated IDs, versions, serials, ports, and flags must remain synchronized across components.
- Do not replace tracked binaries during normal source-code changes.
- Do not include secrets, real data, production values, or local paths.
- New binaries and assets require verifiable provenance, hashes, purpose, and authorization.
- Distinguish fresh-install validation from compatibility with an existing runtime or database.

## Read as needed

- Custom entities: `../.agents/skills/runtime-entity-change/SKILL.md`.
- Encoder, formats, and assets: `../docs/runtime-data.md`.
- Packet changes: `../.agents/skills/protocol-change/SKILL.md`.
- SQL and schema: `../docs/database.md`.
- Validation: `../docs/testing.md`.
