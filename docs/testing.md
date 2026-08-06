# Validation strategy

Select validation according to the affected area and risk. Do not claim results that were not executed.

## Always

```powershell
pwsh -File ./scripts/validate-repository.ps1
git diff --check
git status --short
```

When validating a repository-local Codex skill, use the project-independent
`uv` environment so the validator's YAML dependency does not rely on a global
Python installation:

```powershell
uv run --with pyyaml python `
  C:\Users\aldob\.codex\skills\.system\skill-creator\scripts\quick_validate.py `
  .agents/skills/<skill-name>
```

## Matrix

| Change | Additional validation |
|---|---|
| Documentation, JSON, YAML, or scripts | validator and format-specific syntax checks |
| Normal client change | Debug BuildDeploy |
| Hook, ABI, resource, or Release behavior | Debug + Release + manual runtime test |
| Client CMake/project files | CMake and MSBuild |
| Encoder or inputs | Encode twice and compare hashes |
| Server C++ | Debug and Release presets |
| Network, database, startup, or server runtime | Compose, health checks, and logs |
| Web | `node --check`, `npm ci`, and `/healthz` |
| Editor | Compose overlay, fixtures, backup, and path isolation |
| Packet | valid, truncated, oversized, invalid-state, and boundary cases |
| Schema | empty database and existing-database upgrade |

## Windows client

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

When build settings or project files change:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy `
  -Configuration Debug -BuildSystem MSBuild
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy `
  -Configuration Release -BuildSystem MSBuild
```

Encoder:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action Encode
```

## Linux server

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug
cmake --preset server-linux-release
cmake --build --preset server-linux-release
```

## Compose and services

```bash
test -f .env || cp .env.example .env
docker compose config --quiet
docker compose -f compose.yaml -f compose.editor.yaml config --quiet
docker compose up --build -d --wait
docker compose ps
docker compose logs --tail=200 mysql mu-server mu-web
docker compose down --remove-orphans
```

Web:

```bash
node --check services/web/server.js
npm --prefix services/web ci
curl --fail --silent --show-error http://127.0.0.1:8085/healthz
```

Editor:

```bash
node --check services/editor/server.js
npm --prefix services/editor install
docker compose -f compose.yaml -f compose.editor.yaml up --build -d
```

Do not use `docker compose down -v` without an explicit decision to remove the database.
