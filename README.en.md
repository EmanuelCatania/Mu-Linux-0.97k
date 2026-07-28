# MU 0.97k

[Versão em português brasileiro](README.md)

Independent preservation and modernization of MU Online 0.97k, with a Windows
client, a C++ Linux server, and a local WSL2/Docker Compose environment.

> [!IMPORTANT]
> This repository contains legacy code and assets for which no original license
> has been identified. Read [NOTICE.md](NOTICE.md) and
> [docs/provenance.md](docs/provenance.md) before redistributing or reusing any
> content.

## Project status

- Client: Windows, Win32, Visual Studio Build Tools, and MSVC.
- Server: Linux, CMake, and Docker.
- Supported development environment: Windows 11 with WSL2 Ubuntu 24.04.
- Current goal: preserve 0.97k compatibility while modernizing the build,
  repository structure, and developer experience.
- Public binary and container distribution: not supported yet.

## Components

| Component | Location | Runtime |
| --- | --- | --- |
| Client and encoder | `src/client` | Windows |
| Server | `src/server` | Linux/WSL2 |
| Client runtime | `runtime/client` | Tracked template |
| Server data | `runtime/server` | Docker |
| Web panel | `services/web` | Node.js/Docker |
| Optional editor | `services/editor` | Node.js/Docker |

## Requirements

On Windows:

- PowerShell 7;
- Git and VS Code;
- Visual Studio Build Tools 2026 with Desktop C++, v145 toolset, and MSVC 14.44;
- the extensions recommended by this workspace.

On WSL2:

- Ubuntu 24.04;
- Docker Engine and Docker Compose;
- a Linux clone separate from the Windows clone.

## Local server

From the WSL2 clone:

```bash
cp .env.example .env
# Review all passwords and keep PUBLIC_IP=127.0.0.1 for local development.
docker compose config --quiet
docker compose up --build -d
```

Exposed services:

- ConnectServer: `127.0.0.1:44405/tcp`;
- GameServer: `127.0.0.1:55901/tcp`;
- web panel: <http://127.0.0.1:8085>.

Stop the stack without deleting the database:

```bash
docker compose down
```

## Windows client

For the first run, from the Windows clone:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

The executable runtime is created outside Git at:

```text
C:\Dev\runtime\mu-097k\client
```

Launch the client with the correct working directory:

```powershell
Start-Process `
    -FilePath "C:\Dev\runtime\mu-097k\client\main.exe" `
    -WorkingDirectory "C:\Dev\runtime\mu-097k\client"
```

The script also supports `Build`, `Deploy`, `Encode`, `Clean`, and Release builds.
Use `Client:*` VS Code tasks from the Windows clone and `Server:*` tasks from WSL.

## Repository layout

```text
src/          C++ client, server, and tool sources
runtime/      Client/encoder templates and server data
services/     Web panel and optional editor
deploy/       Docker resources and legacy integrations
docs/         Architecture, development, operations, and history
scripts/      Local development automation
```

## Documentation

- [Architecture](docs/architecture.md)
- [Windows client](docs/development/windows-client.md)
- [WSL2 server](docs/development/wsl-server.md)
- [Docker operations](docs/operations/docker.md)
- [Provenance and attribution](docs/provenance.md)
- [Upstream history in Spanish](docs/history/upstream-readme.es.md)
- [Contributing](CONTRIBUTING.md)
- [Security policy](SECURITY.md)

## Maintenance

`main` is protected. Changes use short-lived branches and pull requests in this
repository. The upstream remote is read-only and used only as a reference; external
fixes are reviewed and imported manually.

## Disclaimer

This project is not affiliated with or endorsed by the owners of MU Online. Names,
trademarks, and assets remain the property of their respective owners.
