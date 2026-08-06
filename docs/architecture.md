# Project architecture

This document describes responsibilities and boundaries. For setup and commands, use `development.md`.

## Overview

```text
InfoEncoder.exe -> ClientInfo.bmd -> Main.dll
main.exe ----------- loads ------------> Main.dll

main.exe + Main.dll -> ConnectServer
main.exe + Main.dll -> GameServer
                         |-> JoinServer
                         \-> DataServer -> MySQL

services/web ---------------------------> MySQL
services/editor -> shared runtime data and backups
```

- `main.exe`: original closed-source client.
- `Main.dll`: hooks, extensions, UI, rendering, and client integration.
- `InfoEncoder.exe`: converts tracked configuration and tables into `ClientInfo.bmd`.
- `ConnectServer`: initial server list and routing.
- `JoinServer`: account authentication and session handling.
- `DataServer`: persistence and MySQL data exchange.
- `GameServer`: world, characters, items, events, and gameplay rules.
- `services/web`: web panel backed by MySQL.
- `services/editor`: optional administrative editing of shared runtime files.

The diagram shows responsibility, not every socket or process-to-process connection.

## Boundaries

- The client is not authoritative for inventory, currency, stats, rewards, or permissions.
- Account and character persistence must follow the existing DataServer flows.
- Code under `Common/` must represent genuinely shared responsibility.
- Configuration and data under `runtime/` affect behavior even without recompilation.
- The editor must not become a generic filesystem or remote-execution API.

## Impact matrix

| Change | Also verify |
|---|---|
| Packet/opcode | client, server, packing, length checks, and states |
| Item/monster/map | encoder, client, GameServer, runtime, and web assets |
| Login/session | JoinServer, GameServer, database, web, and security |
| Persistence | DataServer, schema, seeds, web, and editor |
| Port/address | encoder, ConnectServer, Compose, and health checks |
| `ClientInfo.bmd` field | InfoEncoder, `Main.dll`, defaults, and determinism |
| GameServer data | parser, references, restart/reload behavior, and client |

## Choosing the implementation location

1. Identify who owns the state and makes the authoritative decision.
2. Locate consumers and duplicated representations.
3. Preserve public contracts and persisted formats.
4. Avoid creating a second source of truth.
5. Document changes that cross component boundaries.
