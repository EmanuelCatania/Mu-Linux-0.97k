# Web and editor service patterns

## Web panel

`services/web/` uses Node.js 24, npm, Express, EJS, MySQL, and persisted sessions.

- Preserve `/healthz` and Compose contracts.
- Use POST/PUT/PATCH/DELETE for mutations; do not mutate state through GET.
- Validate path, query, form, and JSON input with explicit limits.
- Parameterize SQL and limit result sets.
- Escape output by default; sanitize HTML only with an allowlist.
- Do not expose stack traces, SQL, secrets, or internal paths.
- Preserve rate limiting on authentication and sensitive operations.

## Authentication and sessions

- Enforce authorization on every protected server-side action.
- Do not use unsafe fallbacks for `SESSION_SECRET`, administrative credentials, or Turnstile.
- When changing cookies or proxy behavior, review HTTPS, `secure`, `SameSite`, fixation, expiration, logout, and `TRUST_PROXY`.
- Do not log passwords, session IDs, tokens, or secrets.

## Editor

`services/editor/` is optional and has write access to shared volumes.

Every file operation must:

1. normalize and resolve the path;
2. restrict access to allowed roots and extensions;
3. reject traversal, absolute paths, and symlink escapes;
4. limit size before reading;
5. preserve encoding, order, and terminators;
6. create a backup before destructive writes;
7. use a temporary file and atomic replacement when possible;
8. detect concurrent writes or stale versions;
9. report partial failure without claiming success.

Do not add arbitrary shell execution, URL fetching, generic filesystem browsing, or default public exposure.

## Dependencies and environment

- Use npm and preserve the web lockfile.
- Add new variables to code, Compose, `.env.example`, and documentation.
- Do not expose a new host port without a documented need.
