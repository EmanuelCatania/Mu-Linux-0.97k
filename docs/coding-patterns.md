# Implementation patterns

Apply these patterns to new or modified code without reformatting entire legacy files.

## Principles

- Prefer small, local, compatible changes.
- Follow existing naming, organization, and error flow in the affected area.
- Do not introduce abstractions before a stable, repeated responsibility exists.
- Preserve classic behavior unless a change is intentional and documented.
- Separate refactoring, functional changes, balancing, and format migrations when possible.

## C and C++

- Validate indices, lengths, pointers, states, and results before use.
- Use fixed-width types for new network or persistence formats.
- Do not replace types in binary structures without proving size, alignment, and offsets.
- Use `static_assert` and `offsetof` to document verified layouts.
- Use RAII for new resources, but do not impose broad modernization on legacy code.
- Define ownership, lifetime, and shutdown paths for asynchronous work.
- Do not hold locks across network, database, or reentrant callbacks.
- Do not throw exceptions across boundaries that are not exception-safe.
- Logs must provide operational context without exposing credentials or personal data.

## Client credential lifecycle

- Store saved credentials only through Windows Credential Manager or DPAPI; do
  not use `Config.ini` or another plaintext file.
- Keep placeholders and masks visual. Native input buffers must contain only the
  values required by the existing client and protocol.
- Define the state transitions for load, edit, foreground submit, authentication
  result, reconnect, logout, disconnect, and deletion.
- A reconnect response must not be mistaken for a pending foreground login
  result or overwrite the user's saved-login choice.
- Clear submit snapshots, credential blobs, temporary render copies, and
  reconnect-owned plaintext buffers as soon as their lifecycle permits.
- When legacy code retains plaintext longer than the desired policy, document
  the limitation and fix it in a focused change rather than claiming full
  compliance.

## JavaScript and services

- Keep handlers small; extract logic only when there is a clear responsibility.
- Validate input before querying the database or accessing files.
- Use parameterized queries, limits, and pagination.
- Return stable public errors without stack traces, SQL, or internal paths.
- Preserve the current EJS and Express stack; framework migrations require a dedicated task.
- Update `package.json` and its lockfile together.

## Dependencies

- Do not edit vendored code during unrelated changes.
- A new dependency requires justification, build/container integration, and maintenance review.
- Prefer the standard library or an existing utility when sufficient.

## Comments and documentation

- Comment invariants, reverse-engineering evidence, and non-obvious decisions; do not narrate trivial code.
- Mark inferences as inferences, including confidence and a revalidation method.
- Update the relevant specialized document when a recurring contract is discovered.
