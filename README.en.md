# MU 0.97k

[Canonical documentation in Brazilian Portuguese](README.md)

Independent MU Online 0.97k fork focused on game improvements, quality of life and
a development workflow that is easier to build, test and debug.

## Project

- **Gameplay and content:** fixes, quality of life, events, items, maps and systems.
- **Classic identity:** changes should remain configurable when practical.
- **Development:** reproducible builds and a maintainable Windows/Linux workflow.

The original `main.exe` client is closed source. `Main.dll` is a community plugin that
applies hooks and extensions, while `InfoEncoder.exe` creates the `ClientInfo.bmd` file
consumed by the plugin. The community server emulator runs on Linux through WSL2,
Docker and MySQL, with a Node.js web panel.

The client is Win32 on Windows 11. The server is developed in Ubuntu 24.04 on WSL2.
The external runtime is kept at `C:\Dev\runtime\mu-097k`; no official binary or image
distribution exists yet.

Start with the [development guide](docs/development.md). Contributions are described
in [CONTRIBUTING.md](CONTRIBUTING.md), security reports in [SECURITY.md](SECURITY.md),
and ownership/provenance in [NOTICE.md](NOTICE.md).

## Credits

- [Nico Muratona (Kayito)](https://github.com/nicomuratona/MuEmu-0.97k-kayito) —
  MuEmu 0.97k sources and base tools.
- [Emanuel Catania](https://github.com/EmanuelCatania/Mu-Linux-0.97k) — Linux,
  Docker, MySQL and the upstream project line.
- Trifon Dinev — Simple MU Online Templates web template.
- Kapocha33, SetecSoft, Zeus and ogocx — contributions recorded in the
  [upstream history](docs/history/upstream-readme.es.md).
- [Aldo Migge](https://github.com/aldomigge) — current independent maintainer.

MU Online, the original client, trademarks and original assets belong to Webzen and/or
their respective owners. This project is not affiliated with or endorsed by Webzen.
No repository-wide license has been identified for the legacy material.
