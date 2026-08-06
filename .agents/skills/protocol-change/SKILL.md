---
name: protocol-change
description: Add or modify a client/server packet, opcode, subcode, serialized structure, sender, receiver, or handler across Main.dll and the MU server processes.
---

# Protocol change

## Goal

Change the protocol as one coordinated contract with explicit ownership, validation, compatibility, and tests.

## Read first

Read only:

1. `docs/protocol.md`;
2. `docs/architecture.md`;
3. `docs/coding-patterns.md`;
4. the relevant sections of `docs/testing.md`;
5. affected senders, receivers, packet definitions, and state code.

Read `docs/database.md` or `docs/runtime-data.md` only when persistence or runtime configuration is affected.

## Workflow

### 1. Define the contract before coding

Record:

- packet name and purpose;
- direction;
- owning process and authoritative state;
- header type;
- opcode/subcode;
- exact, minimum, or maximum size;
- allowed connection/game state;
- authoritative and untrusted fields;
- side effects;
- compatibility expectation.

Do not allocate an opcode before checking for collisions in every component.

### 2. Build the impact matrix

Search the entire repository for:

- opcode/subcode and header constants;
- structure and handler names;
- serialization and deserialization;
- sender and receiver call sites;
- client display or prediction logic;
- ConnectServer, JoinServer, DataServer, and GameServer handling;
- runtime flags and version checks;
- database, web, or editor side effects.

Mark each component as changed, reviewed/no change, or not applicable.

### 3. Fix the wire layout

- Use explicit-width fields for new wire data.
- Preserve packing, byte order, signedness, and header length.
- Avoid pointers, compiler-dependent enums, `bool`, or platform-sized types on the wire.
- Verify structure size at compile time where possible.
- Define variable-length bounds and overflow-safe size calculations.
- Avoid exposing internal object layout directly as a packet.

### 4. Define validation and authority

Before mutation, validate:

- declared and received lengths;
- opcode/subcode;
- indices, IDs, coordinates, counts, and ranges;
- account, character, map, inventory, and session state;
- duplicate, replayed, or out-of-order behavior;
- permissions and server authority;
- resource availability for the complete operation.

Do not trust client-provided currency, prices, stats, item state, rewards, or permissions.

### 5. Implement coherently

- Update sender, receiver, and shared definitions together.
- Keep process ownership consistent with `docs/architecture.md`.
- Do not partially mutate state before all failure-prone checks complete.
- Define error or rejection behavior without leaking sensitive details.
- Preserve unknown-client behavior or add an explicit version gate.
- Coordinate persisted side effects through the existing DataServer/database path.

### 6. Document and test

Add or update the packet record in `docs/protocol.md`.

Test at least:

- valid request and response;
- shortest/truncated packet;
- oversized packet;
- invalid opcode/subcode or state;
- boundary IDs/counts/coordinates;
- duplicate or replay behavior when relevant;
- disconnect or persistence failure during side effects;
- old/new version mismatch when compatibility changes.

Build every changed client and server component and run integration validation selected from `docs/testing.md`.

## Output

Return an impact matrix:

| Component | Status | Reason |
|---|---|---|
| `Main.dll` | changed/reviewed/N/A | |
| `ConnectServer` | changed/reviewed/N/A | |
| `JoinServer` | changed/reviewed/N/A | |
| `DataServer` | changed/reviewed/N/A | |
| `GameServer` | changed/reviewed/N/A | |
| Runtime/config | changed/reviewed/N/A | |
| Database | changed/reviewed/N/A | |
| Web/editor | changed/reviewed/N/A | |

Also report layout, authority, compatibility, tests, and unverified cases.

## Stop conditions

Stop rather than guess when ownership is unclear, the opcode collides, layout differs between compilers, packet size cannot be validated safely, only one side of a required contract is available, or backward compatibility is undefined.
