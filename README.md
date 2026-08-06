# MU 0.97k

Independent MU Online 0.97k fork focused on evolving the game without losing its
classic identity, while making development easier to build, test, and debug.

## Goals

- **Gameplay and content:** fixes, quality-of-life improvements, events, items, maps,
  and new systems.
- **Classic identity:** changes that affect the original experience should remain
  configurable when practical.
- **Development:** reproducible builds, tooling, and documentation that do not depend
  on hidden knowledge.

## Components

The original `main.exe` client is closed source. The project uses `Main.dll`, a
community plugin that applies hooks and extensions, and `InfoEncoder.exe` generates
the `ClientInfo.bmd` configuration consumed by the DLL.

The community server emulator is written in C++ and runs on Linux. The current stack
uses WSL2, Docker, MySQL, and a Node.js web panel. Client, plugin, encoder, and server
changes must remain coordinated when a feature affects protocols or configuration.

## Supported environments

- Client, `Main.dll`, and encoder: Windows 11, Win32/x86, MSVC, and VS Code.
- Server: Ubuntu 24.04 on WSL2, CMake/Ninja, and Docker Compose.
- Web panel: Node.js and Docker.
- The executable runtime is kept outside the repository at
  `C:\Dev\runtime\mu-097k`.
- No official binary or container image distribution is currently provided.

## Quick start

In the WSL2 clone:

```bash
cp .env.example .env
# Review local credentials and keep PUBLIC_IP=127.0.0.1 for local use.
docker compose config --quiet
docker compose up --build -d
```

The web panel is available at <http://127.0.0.1:8085>. Stop the services without
removing the database with `docker compose down`.

In the Windows clone:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

Edit `C:\Dev\runtime\mu-097k\encoder\MainInfo.ini`, then run:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

`BuildDeploy` builds and deploys the artifacts, then runs the encoder. See the
[development guide](docs/development.md) for Release builds, F5 debugging, MSBuild,
CMake, and server validation.

## Repository layout

```text
src/       client, server, and tool sources
runtime/   tracked templates and server data
services/  web panel and optional editor
deploy/    Dockerfiles and legacy integrations
scripts/   local automation
docs/      development and technical documentation
```

See the [contribution guidelines](CONTRIBUTING.md), [security policy](SECURITY.md),
and [legal and provenance notice](NOTICE.md).

## Credits and licensing

This independent line is maintained by
[Aldo Migge](https://github.com/aldomigge). Its community origins include
[MuEmu 0.97k](https://github.com/nicomuratona/MuEmu-0.97k-kayito) and
[Mu-Linux-0.97k](https://github.com/EmanuelCatania/Mu-Linux-0.97k).

MU Online, the original client, trademarks, music, images, and other assets belong to
Webzen and/or their respective owners. This project is not affiliated with or
endorsed by Webzen. See [`NOTICE.md`](NOTICE.md) for complete attribution,
provenance, and licensing limitations.
