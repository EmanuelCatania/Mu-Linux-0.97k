# MU 0.97k

[Documentação principal em português brasileiro](README.md)

MU 0.97k is an independent fork focused on evolving both the game and its development
platform while keeping the classic 0.97k identity recognizable.

> [!IMPORTANT]
> MU Online, its original closed-source client, trademarks, and original assets belong
> to Webzen. Community code in this repository has separate origins and licensing
> conditions. Read [NOTICE.md](NOTICE.md) before reusing or redistributing anything.

## Project pillars

- **Gameplay and content:** fixes, quality of life, configurable features, events,
  items, maps, and new systems.
- **Classic identity:** changes that alter the classic experience should remain
  configurable whenever practical.
- **Development platform:** reproducible builds, automation, tests, tooling, and
  maintainability across Windows and Linux.

The original `main.exe` client is closed source. The community-developed `Main.dll`
plugin uses hooks to provide fixes and extensions, while `InfoEncoder.exe` generates
the configuration consumed by the DLL. The community server emulator runs natively on
Linux through WSL2 and Docker.

## Development environment

- Client, plugin, and encoder: Windows, Win32, MSVC, and VS Code.
- Server: C++, Linux, CMake, WSL2, and Docker Compose.
- Web panel and optional editor: Node.js and Docker.

The canonical setup, build, debug, and operation instructions are maintained in
Portuguese in the [development guide](docs/development.md).

## Direction

Work is organized into two equal tracks: **Gameplay and Content** and **Platform and
Development**. The detailed backlog lives in GitHub Issues and Milestones. New content
follows a classic-extensible model instead of treating preservation as the project's
only purpose.

## Credits

- [Nico Muratona (Kayito)](https://github.com/nicomuratona/MuEmu-0.97k-kayito) —
  base MuEmu 0.97k sources and tools.
- [Emanuel Catania](https://github.com/EmanuelCatania/Mu-Linux-0.97k) — Linux,
  Docker, and MySQL project line from which this fork originated.
- Trifon Dinev — Simple MU Online Templates web template.
- Kapocha33, SetecSoft, Zeus, and ogocx — specific contributions recorded in the
  [upstream history](docs/history/upstream-readme.es.md).
- [Aldo Migge](https://github.com/aldomigge) — current independent fork maintainer.

This project is not affiliated with or endorsed by Webzen. No repository-wide license
has been identified for the legacy material, and no rights over third-party content
are granted here.
