# Local development

This is the supported workflow for developing the client on Windows and the server
on WSL2. Use separate clones: change and build one at a time, then synchronize the
other through Git.

## Overview

```text
Windows: src/client -> CMake/Ninja -> C:\Dev\runtime\mu-097k -> main.exe
                         InfoEncoder -> ClientInfo.bmd

WSL2: src/server -> CMake/Ninja/Docker -> mu-server -> MySQL
                                      \-> mu-web
```

The original `main.exe` is closed source. It loads `Main.dll`, which applies hooks
and extensions. The encoder reads `MainInfo.ini` and generates the configuration
consumed by the DLL.

## Requirements

### Windows

- PowerShell 7, Git, CMake, Ninja, and VS Code;
- Visual Studio Build Tools with C++ and the MSVC 14.44 toolset used by the project;
- the C/C++ extension and the workspace's recommended tasks.

### WSL2

- Ubuntu 24.04;
- Docker Engine and Docker Compose;
- for native builds: `build-essential`, CMake, Ninja, and MySQL Connector/C++.

## Windows client

Use `C:\Dev\projects\mu-097k` on Windows. The executable runtime is kept outside Git
at `C:\Dev\runtime\mu-097k`.

### 1. Initialize the runtime

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action InitializeRuntime
```

The script copies the tracked templates from `runtime/client` and `runtime/encoder`.
To recreate an existing copy, use `-ForceRuntime` only when its local changes may be
discarded.

### 2. Configure the encoder

Edit the external file:

```text
C:\Dev\runtime\mu-097k\encoder\MainInfo.ini
```

For a local server, use `IpAddress=127.0.0.1` and the ConnectServer TCP port, normally
`IpAddressPort=44405`. `ClientSerial` and `ClientVersion` must match `ServerSerial`
and `ServerVersion` in
`runtime/server/GameServer/DATA/GameServerInfo - StartUp.dat`.

### 3. Build, deploy, and generate configuration

CMake/Ninja is the default build system. `BuildDeploy` builds `Main.dll` and
`InfoEncoder.exe`, copies them to the runtime, and runs the encoder automatically:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

The executable client is located at `C:\Dev\runtime\mu-097k\client`. To regenerate
only `ClientInfo.bmd` after changing the INI file, run:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action Encode
```

The encoder is non-interactive when invoked by the script. When run without
arguments, `InfoEncoder.exe` preserves its original manual behavior.

### 4. Debug and Release

In VS Code, `F5` uses the `Client: Debug Main.dll (x86)` configuration. It runs the
`Client: Build + Deploy Debug` task, starts `main.exe` from the external runtime, and
loads the `Main.pdb` symbols.

Release does not use a second F5 configuration. To prepare an optimized build, run:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

The `Client: Build + Deploy Release` task is also available. The PDB is not retained
in the Release runtime.

MSBuild remains available as a fallback:

```powershell
pwsh -File .\scripts\client-workflow.ps1 -Action BuildDeploy `
  -Configuration Debug -BuildSystem MSBuild
```

The `Build`, `Deploy`, `Encode`, and `Clean` actions accept the same `-BuildSystem`
used for compilation. CMake uses the `client-windows-debug` and
`client-windows-release` presets; artifacts are written to
`out/build/<preset>/bin`.

## Server on WSL2

Use `~/Dev/projects/mu-097k` on Ubuntu. Do not build the same working tree on Windows
and WSL2 at the same time.

### 1. Local environment

```bash
cd ~/Dev/projects/mu-097k
cp .env.example .env
```

Review the credentials and keep `PUBLIC_IP=127.0.0.1` for a local Windows client.
The `.env` file must not be committed.

### 2. Optional native build

Install the dependencies once:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libmysqlcppconn-dev
```

Build both server variants:

```bash
cmake --preset server-linux-debug
cmake --build --preset server-linux-debug

cmake --preset server-linux-release
cmake --build --preset server-linux-release
```

The four executables are written to `out/build/<preset>/bin`. Configuration fails if
MySQL Connector/C++ is unavailable.

### 3. Start and stop Compose

```bash
docker compose config --quiet
docker compose up --build -d
docker compose ps
```

The base stack uses `mysql`, `mu-server`, and `mu-web`. The web panel is available at
<http://127.0.0.1:8085>; the server ports are `44405/tcp` and `55901/tcp`. Inspect
logs with `docker compose logs --follow --tail=200` and stop the stack with:

```bash
docker compose down
```

Do not use `docker compose down -v` unless the MySQL volume is intentionally being
removed. The optional editor uses `compose.editor.yaml` when needed.

## Technical documentation

Read only the document relevant to the current task:

- [Architecture](architecture.md)
- [Implementation patterns](coding-patterns.md)
- [Client reverse engineering](client-reverse-engineering.md)
- [Client/server protocol](protocol.md)
- [Runtime, encoder, and data](runtime-data.md)
- [Database](database.md)
- [Web and editor services](services-patterns.md)
- [Validation strategy](testing.md)

## Contribution checks

Use the [validation strategy](testing.md) to select checks for the affected area and
follow the [contribution guidelines](../CONTRIBUTING.md) before opening a pull
request.

MSVC failures normally indicate that Visual Studio Build Tools or the MSVC 14.44
toolset is unavailable. If the runtime already exists, `InitializeRuntime` requires
`-ForceRuntime`; this prevents accidental removal of local configuration.
