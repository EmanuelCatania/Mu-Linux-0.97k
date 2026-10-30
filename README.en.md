# Mu Linux 97K

[![Website](https://img.shields.io/badge/website-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/en/) [![XERJ Corpus Hub contributor](https://hub.xerj.org/contributors/EmanuelCatania/badge.svg)](https://hub.xerj.org/contributors/EmanuelCatania)

[![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k?label=server&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-web?label=web&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-web) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mu-editor?label=editor&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mu-editor) [![Docker Pulls](https://img.shields.io/docker/pulls/emapupi/mu-linux-97k-mysql?label=mysql&logo=docker)](https://hub.docker.com/r/emapupi/mu-linux-97k-mysql) 

[🇦🇷 Español](README.md) | 🇺🇸 English | [🇧🇷 Português](README.pt-BR.md)

Native Linux port and operational MuEmu 0.97k stack with Docker, web panel and optional editor.

## What's new in 0.97.30 — Phase 3 completed

- The server builds with CMake on Windows x86 and x64, as well as Linux x64 and ARM64. CI covers all four in Debug and Release.
- MySQL Connector/C++ is downloaded at configure time with a pinned version and hash; its binaries are no longer committed.
- Pointer-width fixes in socket, IOCP and timer callbacks, needed for x64.
- Larger send queue for the item catalog the server sends at login.
- GM commands to bring forward Blood Castle, Devil Square, Golden Archer Bingo and invasions without restarting the server.

**Compatibility: server 0.97.30 ↔ [client 0.97.30](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.30).**

### Upgrading from 0.97.20

With Docker, update only the server:

```bash
docker compose -f docker-compose.images.yml pull mu-server
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

To pin the version, use the `emapupi/mu-linux-97k:0.97.30` image (amd64 and arm64). Pulling does not update a persistent volume: add the four rows of the new GM commands to your `Data/Command.txt`, as explained in [Administration](#administration-gm-events-and-reloads). MySQL, the website and the editor do not change.

**Building from source (`docker-compose.yml`):** its MySQL service moves from the `mysql:5.7` image to `mysql:8.4.11`, the same version as the published image. A volume created with 5.7 does not start with 8.4: before upgrading, follow the [8.4 migration guide](docs/mysql-8.4-upgrade.md). The published-images compose file already used 8.4 and does not change.

**Pterodactyl:** select `0.97.30` in `MU_RELEASE_TAG`. For existing installations, with the server stopped, extract the archive for your architecture from this Release into a temporary folder, replace the four binaries and `start.sh`, and add the `Command.txt` rows, keeping your INI and Data files.

### Build and run on Windows (without Docker)

1. Requirements: Visual Studio 2022 or 2026 with "Desktop development with C++" (v143 toolset, includes CMake) and MySQL 8.
2. Build from the repo root:

   ```
   cmake -S . -B out/win-x86 -A Win32 -T v143
   cmake --build out/win-x86 --config Release
   ```

   The executables end up in `Source/MuServer/bin/Release/`.
3. Copy each executable into its `MuServer/` folder, next to its `.ini`:
   - `ConnectServer/ConnectServer.exe` → `MuServer/ConnectServer/`
   - `GameServer/GameServer.exe` → `MuServer/GameServer/` (reads data from `MuServer/Data/`)
   - `MYSQL/JoinServer/JoinServer.exe` → `MuServer/MySQL/JoinServer/`
   - `MYSQL/DataServer/DataServer.exe` → `MuServer/MySQL/DataServer/`

   Also copy the DLLs, `plugin` directory and `LICENSE-MySQL.txt` that CMake places next to JoinServer and DataServer. Their architecture and configuration must match the executables.
4. Database: run `MuServer/MySQL/DB/CreateDatabase.sql` and then `PoblateDatabase.sql`. The default user is `admin` / `asd123`; if you use another one, change it in `DataServer.ini` and `JoinServer.ini`.
5. In `MuServer/ConnectServer/ServerList.dat`, set the server IP (`127.0.0.1` to test on the same PC).
6. Start them in this order: DataServer, JoinServer, ConnectServer and GameServer.

The `0.97.30` client has the address `mu.server-pups.space` compiled in. To point it at your own server without rebuilding, add the line `127.0.0.1 mu.server-pups.space` to `C:\Windows\System32\drivers\etc\hosts` (as administrator), and remove it when you are done.

#### Windows x64 and Phase 3 dependencies

For x64, from the repository root:

```powershell
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release --parallel 4
```

The `windows-x86-release`, `windows-x86-debug` and `windows-x64-debug` presets are also available. CMake downloads MySQL Connector/C++ **8.0.33**, with pinned SHA256 hashes for x86/x64 and Debug/Release; its binaries are no longer checked in. Initial configuration requires access to the official MySQL archives. Legacy MSBuild projects use the same mechanism for `Debug_MySQL` and `Release_MySQL` and require CMake on PATH.

CI builds all four services for Windows x86/x64 and Linux x64/ARM64, Debug and Release. ARM64 remains build-only, without gameplay validation. Phase 3 fixes pointer widths in callbacks, IOCP and timers, and provides a 256 KiB pending send buffer for the login catalog; individual packets remain limited to 8 KiB. Local Windows x64 testing covered login, gameplay, reset, buying/selling and Chaos mixing; it was not a load test.

### Administration: GM events and reloads

GM level 1 or higher only, through in-game chat:

| Command | Event |
| --- | --- |
| `/tamastart` | Tamachan |
| `/bcstart` | Blood Castle, all levels |
| `/dsstart` | Devil Square, all levels |
| `/bingostart` | Golden Archer Bingo |
| `/invasionstart <index>` | Index in `MuServer/Data/Event/InvasionManager.dat` |

These schedule a single occurrence roughly two minutes later, without editing configured schedules or resetting active events. BC/DS also reject requests while entry is open. Starting an invasion does not reset other invasions. The event must be enabled. New commands record the request and result in `COMMAND_LOG`.

For existing installations, append the four new rows from `MuServer/Data/Command.txt` after `/tamastart` and before `end`, in this order: bcstart, dsstart, bingostart, invasionstart. Row order determines command IDs; preserve other configuration.

#### Example: reload shops through Docker

Typing only `shop` or `/reload shops` in the console **does not work**: Linux GameServer does not read commands from stdin. From the host terminal, in the Compose directory:

```sh
docker compose exec -T mu-server sh -c 'printf "shop\n" > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"'
```

If you are already inside the server container shell:

```sh
printf 'shop\n' > "${MU_DATA_PATH:-/opt/mu/Data}/EditorReload.flag"
```

The server consumes `EditorReload.flag` within about 10 seconds. Look for `Shop reloaded by editor flag` in the log. For image deployments, add `-f docker-compose.images.yml` to `docker compose`. Without Compose, replace `docker compose exec -T mu-server` with `docker exec <container>`.

Supported tokens: `all`, `common`, `shop`, `monster`, `move`/`gate`, `quest`, `util`, `item`, `skill`, `event`, `eventitembag`, `chaosmix`, `command`, `custom`, `character`, `map`, `hack`. Separate multiple tokens with spaces or commas. Wait for one request to be consumed before sending another. `event` and `all` reinitialize events: do not use them to start an event early or during an active event. Windows retains its reload menu.

## What's new in 0.97.20 — Phase 2 completed

**Compatibility: server 0.97.20 ↔ [client 0.97.20](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.20).** Upgrade both together: the extended item format is incompatible with older clients and the historical Main.dll.

- At login, the server sends the catalog of items, monsters, effects, pets and tooltips. The client receives definitions while keeping model and texture assets locally.
- Each item section supports new indices 32..511 and a 7-byte format. The **Behavior** column reuses a classic item's logic in attacks, skills, mixes and events.
- Item JSON files define effects, inventory poses and tooltip text. Custom equipment also appears on other players and in character selection.
- Pets are configured through JSON. **Pet Rudolph (13,400)** is included: it flies to nearby zen and collects it under server control.
- The DLL Encoder's custom configuration files remain supported as catalog inputs.
- The section/index parsing order in `ItemOption.txt` is fixed for GCC, restoring Triple Shot availability.

### Upgrading from 0.97.00

Back up your Data files before integrating the examples. Do not delete volumes or replace your full custom configurations.

With Docker and the image compose file, update only the server:

```bash
docker compose -f docker-compose.images.yml pull mu-server
# Integrate the Data files listed below before starting the new server.
docker compose -f docker-compose.images.yml up -d --no-deps mu-server
```

The compose file uses `latest`; to pin the version, change only the `mu-server` image to `emapupi/mu-linux-97k:0.97.20`. Images support `linux/amd64` and `linux/arm64`.

**Pulling an image does not update a persistent volume.** From this release's `MuServer/Data`, integrate these files into your `/opt/mu/Data` volume:

- `Custom/Items/`: `03_000.json` (Light Spear pose) and `13_400.json` (Rudolph pose and tooltip).
- `Custom/Pets/13_400.json`: Rudolph movement and zen pickup.
- `Custom/Encoder/`: `CustomBow.txt`, `CustomGlow.txt`, `CustomItem.txt`, `CustomMonster.txt`, `ItemPosition.txt` and `MapFog.txt`. Merge existing custom entries instead of overwriting them.
- `Item/Item.txt`: add this release's row **400 inside section 13**, unless already present. Preserve all other rows. If you already use 13,400, resolve that conflict before adding the example.

Install matching assets in client 0.97.20 too. Restart the server and check `[ContentCatalog]` messages, including `13_400.json`, its tooltip and inventory pose. Startup must show `MU Linux server 0.97.20` and the compatible client link.

**MySQL, the web and the editor are unchanged in this release:** no schema migration or image rebuild is required for them. If your installation still uses MySQL 8.0, the [earlier 8.4 migration](docs/mysql-8.4-upgrade.md) is a separate task.

**Pterodactyl:** import the updated server egg, which accepts `0.97.20` without a `v`, and set `MU_RELEASE_TAG` accordingly. The Pterodactyl runtime image only contains dependencies: updating it does not update binaries in the volume. For existing installations, stop the server and extract this Release's `mu-linux-97k-server-amd64.tar.gz` or `mu-linux-97k-server-arm64.tar.gz` into a temporary directory; update the four binaries and `start.sh`, then integrate Data as above while preserving your INI files. Do not blindly reinstall: the installer also extracts configuration and data files. Web and MySQL eggs need no changes. Architecture-specific archives must be attached to the Release before installing this version.

### Adding custom items

The server owns the definitions; the client only needs `.bmd` models and their textures. The source-based client does not require regenerating `item.bmd` or `ClientInfo.bmd`.

1. Add the item row to its section in `Data/Item/Item.txt`. For indices 32..511, specify the classic item to imitate in **Behavior** (for example `00,020`), followed by the folder, model and RGB glow. Copy a complete row from the section and adjust its stats; do not use an abbreviated row containing `...`.
2. For classic customs, keep model/glow definitions in `Data/Custom/Encoder/CustomItem.txt` and `CustomGlow.txt`, along with the other included Encoder tables.
3. Optionally create `Data/Custom/Items/<section>_<index>.json` with `item`, `effects`, `inventory` and/or `tooltip`. See `03_000.json` and `13_400.json` for working examples.
4. For a custom pet, add `Data/Custom/Pets/<section>_<index>.json`. Rudolph uses `13,400`, slot 8 and behavior `*`, with movement and zen pickup definitions. The client animates the pet; the server authorizes and executes pickup.
5. Distribute model assets to the client, restart the server and log in again to receive the catalog. With GM permissions, test Rudolph using `/make 13 400`.

See the [complete examples and client asset paths](https://github.com/EmanuelCatania/Mu-97k-Client-Source/blob/0.97.20/README.md#cómo-agregar-items-custom).

## Base and reference

Based on Kayito's sources.

[![View Kayito's original README](https://img.shields.io/badge/original%20README-Kayito-181717?logo=github)](https://github.com/nicomuratona/MuEmu-0.97k-kayito#readme)

## Credits and goals

- Kayito: MuEmu 0.97k base sources; Trifon Dinev: Simple MU Online Templates used by `mu-web`.
- Native Linux server with epoll and compatibility fixes, MySQL instead of MSSQL, and Docker for VPS deployment.

## Official images

- `emapupi/mu-linux-97k` (server), `emapupi/mu-linux-97k-web` (web).
- `emapupi/mu-linux-97k-mu-editor` (editor API), `emapupi/mu-linux-97k-mysql` (MySQL/init).

## Update 1 (2026-02-20)

- Web editor for data, accounts, characters, shops, spawns, gates, configuration, items, monsters, drops, backups and snapshots.
- Optional `mu-editor` service with `EditorReload.flag` automatic reload.

## Update 2 (2026-03-26)

- Client fixes for staffs, Character Info, guild, macros and quests; Divine drop and Crystal Sword/Morning Star fixes.

## Update 3 (2026-09-20)

- Server: Blood Castle fix and Tamachan jewel-rain event (the source-based client is required to see it fully).
- `MonsterSetBase.txt`: restored missing normal spawn points and required NPCs in reported maps.
- Docker: CMake uses one job (`-j1`) to prevent OOM on low-memory hosts.
- Architectures: native `linux/amd64` and `linux/arm64` support, using MySQL 8.4 LTS on both.
- Database: upgraded to MySQL 8.4 LTS. Read the [migration guide](docs/mysql-8.4-upgrade.md) before reusing an existing volume.
- ARM64: fixed script parsing and inherited `char` semantics, preventing parsing errors and incorrect disconnects.
- CI: every push, pull request and manual run validates a `linux/arm64` image and startup smoke test.

## Client status

The DLL injection-based client is fully deprecated and has been removed from this repository along with its sources and Encoder executable. Use the [source-based client 0.97.30](https://github.com/EmanuelCatania/Mu-97k-Client-Source/releases/tag/0.97.30). Old files remain available in Git history. The tables in `MuServer/Data/Custom/Encoder/` are retained because the server catalog uses them.

## Quick start

1. Copy `.env.example` to `.env` and set credentials, `PUBLIC_IP` and secrets.
2. Start the stack:

   ```bash
   docker compose up -d --build
   ```

3. Open `44405/tcp` and `55901/tcp`.

For published images without a build:

```bash
docker compose -f docker-compose.images.yml up -d
```

On ARM with 1 GB RAM or less, enable 2 GB swap before the first build. `uname -m`: `x86_64` = amd64, `aarch64` = arm64.

## Environment

`.env` is based on `.env.example`; change values before production.

- MySQL: `MYSQL_ROOT_PASSWORD`, `MYSQL_USER`, `MYSQL_PASSWORD`, `MYSQL_DATABASE`.
- Server: `DB_HOST`, `DB_USER`, `DB_PASS`, `DB_NAME`, `PUBLIC_IP`.
- Web: `WEB_PORT`, `SESSION_SECRET`, `TURNSTILE_SITE_KEY`, `TURNSTILE_SECRET_KEY`, `ADMIN_USER`, `ADMIN_PASS`, `TRUST_PROXY`. `GEOIP_ENABLED` and `ACQUISITION_TRACKING` stay `0`: country flags and registration source stay off until configured. `GEOIP_ENABLED` needs the optional `geoip-lite` package (`INSTALL_GEOIP=1` when building the image). `GEOIP_HIDDEN_COUNTRIES` is an empty list of ISO codes. `RANKINGS_RESETS_FIRST=1` sorts the level and class lists by resets before level.
- Test data: `SEED_TEST_DATA`; editor: `EDITOR_ENABLED`, `EDITOR_PORT`, `EDITOR_API_URL`, `EDITOR_MAX_BACKUPS`, `EDITOR_MAX_SNAPSHOTS`.

After changing MySQL credentials, recreate the volume:

```bash
docker compose down -v
docker compose up -d --build
```

## Editor, web panel and notes

The optional editor uses `mu-server` internal volumes:

```bash
docker compose -f docker-compose.yml -f docker-compose.editor.yml up -d --build
```

It keeps per-file backups and snapshots of `MuServer/Data` and `MuServer/GameServer/DATA`. `mu-web` provides registration, login, rankings and news on external port `8085` (internal `8080`). Initial admin is `admin / 123456` and must change password at first login.

- `MD5Encryption=2` uses binary MD5; recreate the DB with `docker compose down -v` when migrating from plain text.
- Save server TXT files as ANSI / Windows-1252 without BOM. Shops live in `MuServer/Data/Shop/*.txt`; `-1 -1` auto-places `SlotX SlotY`.
- Use `docker compose logs -f` for logs. Main folders: `Source/`, `MuServer/`, `docker/`.

## Website and editor 0.97.20

First visit to /admin: mandatory password change and setup for the default language (ES/EN/PT/UK; the stored default remains ES), classic, Elf, Dark Knight or Dark Wizard theme, and editor connection status. Later changes are available at /admin/settings. Visitors can select their own language using the flags; this does not change the admin/editor language or translate news and game data.

Settings are automatically stored in MySQL (web_site_settings); preserve its volume. Update only emapupi/mu-linux-97k-web:0.97.20 and emapupi/mu-linux-97k-mu-editor:0.97.20 and recreate those services. No server or MySQL rebuild is needed. Editor screens are served by the website; the editor API retains its previous behavior.
