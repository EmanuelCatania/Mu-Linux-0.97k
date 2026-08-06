# Services - Agent Instructions

These rules extend the root `AGENTS.md` for `services/`.

- `web/` is the Express/EJS panel using Node.js 24, npm, MySQL, and sessions.
- `editor/` is an optional administrative service with write access to shared data.
- Preserve `/healthz`, ports, environment variables, and Compose contracts.
- Authentication and authorization must be enforced server-side, not only in the UI.
- Validate every input for type, size, range, and state; parameterize every SQL query.
- Escape output by default; sanitize HTML only with an explicit allowlist.
- The editor must restrict operations to allowed roots and prevent traversal, symlink escapes, and partial writes.
- Destructive editor writes require a backup and defined failure behavior.
- Preserve the web service's `package-lock.json`; do not change package manager or framework incidentally.

## Read as needed

- Web, sessions, and editor: `../docs/services-patterns.md`.
- Persistence: `../docs/database.md`.
- General patterns: `../docs/coding-patterns.md`.
- Validation: `../docs/testing.md`.

## Quick start

```bash
node --check services/web/server.js
node --check services/editor/server.js
docker compose config --quiet
```
