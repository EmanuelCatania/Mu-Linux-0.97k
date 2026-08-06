# MU 0.97k - Agent Instructions

## Project context

- `main.exe` is the original closed-source client; `Main.dll` applies hooks and extensions.
- `InfoEncoder.exe` generates `ClientInfo.bmd` from files under `runtime/encoder/`.
- The C++ server runs on Linux and is split into `ConnectServer`, `JoinServer`, `DataServer`, and `GameServer`.
- `services/web/` contains the web panel; `services/editor/` is an optional administrative editor.
- The executable client runtime lives outside Git, normally at `C:\Dev\runtime\mu-097k`.

## Supported environments

- Client: Windows 11, MSVC, CMake/Ninja, and Win32/x86.
- Server: Ubuntu 24.04/WSL2, CMake/Ninja, and Docker Compose.
- Web: Node.js and Docker.
- Use separate clones on Windows and WSL2. Do not build the same working tree in both environments.
- See `docs/development.md` for setup, build, runtime, and debugging instructions.

## Progressive documentation loading

**Read only the documents and skills relevant to the current task. Do not load the entire `docs/` or `.agents/skills/` tree.**

| Task | Primary reference |
|---|---|
| Setup, build, or local runtime | `docs/development.md` |
| New feature or structural change | `docs/architecture.md` and `docs/coding-patterns.md` |
| Hook, patch, address, ABI, or `main.exe` analysis | `docs/client-reverse-engineering.md` |
| Locate or revalidate a `main.exe` offset | `.agents/skills/ghidra-offset-analysis/SKILL.md` |
| Add or modify a client hook | `.agents/skills/client-hook-change/SKILL.md` |
| Add or modify a packet or handler | `.agents/skills/protocol-change/SKILL.md` |
| Add or modify an item, monster, map, or custom entity | `.agents/skills/runtime-entity-change/SKILL.md` |
| Encoder, runtime, DAT/INI/TXT, or asset | `docs/runtime-data.md` |
| Client-local secrets or credential persistence | `SECURITY.md` and `docs/coding-patterns.md` |
| Server schema, SQL, or persistence | `docs/database.md` |
| Web panel, authentication, or editor | `docs/services-patterns.md` |
| Planning and validation | `docs/testing.md` |
| Branch, commit, and pull request | `CONTRIBUTING.md` |
| Security, secrets, and vulnerabilities | `SECURITY.md` |
| Licensing and third-party material | `NOTICE.md` |

## General rules

- Before editing, identify the owning component and search for related usages, contracts, and configuration.
- Follow the local style. Avoid mass formatting, broad renames, and unrelated refactors.
- Prefer small, compatible, and verifiable changes. Avoid speculative abstractions and overengineering.
- Do not change a protocol, binary structure, ID, or duplicated setting in only one component.
- Preserve legacy formats, ordering, terminators, case, encoding, and layout when they are runtime contracts.
- Do not add credentials, real player data, user-specific local paths, or
  production values. Canonical development paths documented by the project are
  permitted when they are required by a reproducible workflow.
- Do not add binaries or assets without verifiable provenance and authorization.
- Do not edit vendored code during unrelated changes.
- Do not hide failures with casts, unsafe defaults, generic handling, or removed validation.
- Update documentation and validation when a contract or procedure changes.
- Never claim that a build, test, or scenario was validated unless it was actually run.

## Planning and implementation

- Plans must list affected components, compatibility concerns, risks, and required validation.
- Changes spanning client, server, runtime, database, and web must be implemented coherently.
- For fixes, preserve classic behavior unless the change is explicitly intentional.
- For external input, validate size, range, state, and authority before mutating persistent state.
- Separate balancing, format, architecture, and maintenance changes when they can be reviewed independently.
- Record limitations, assumptions, and untested scenarios.

## Minimum validation

Always run:

```powershell
pwsh -File ./scripts/validate-repository.ps1
git diff --check
git status --short
```

Use `docs/testing.md` to select additional builds and tests based on the affected area and risk.

## Git and completion

- Follow `CONTRIBUTING.md`; do not push directly to `main`.
- Do not mix unrelated changes in the same pull request.
- The final response must include: summary, changed files, validation performed, and limitations.
