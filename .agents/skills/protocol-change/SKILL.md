---
name: protocol-change
description: Add or modify a client/server packet, opcode, subcode, serialized structure, sender, receiver, or handler across Main.dll and the MU server processes.
---

# Protocol change

Coordinate the protocol as one contract across every producer, consumer, owner,
and compatibility boundary.

## Load

Read only:

- `docs/protocol.md`;
- `docs/architecture.md`;
- the relevant sections of `docs/testing.md`;
- affected packet definitions, senders, receivers, handlers, and state code.

Read `docs/coding-patterns.md`, `docs/database.md`, or `docs/runtime-data.md` only
when implementation patterns, persistence, or runtime configuration are affected.

## Workflow

1. **Define the contract.** Complete the packet record from `docs/protocol.md`
   before allocating an opcode or changing code. Resolve direction, ownership,
   authority, layout, accepted state, side effects, compatibility, and rejection
   behavior.
2. **Build the impact matrix.** Search the entire repository for opcode/subcode,
   structure names, serialization, handlers, version gates, runtime flags, and
   persistent or operator-facing effects. Mark each component changed,
   reviewed/no change, or not applicable.
3. **Implement coherently.** Update required producers, consumers, and shared
   definitions together. Keep authority with the owning process, preserve
   existing behavior outside the contract change, and avoid partial mutation.
4. **Document and validate.** Update `docs/protocol.md`; build every changed
   component and run the packet, compatibility, persistence, and failure-path
   checks selected from `docs/testing.md`.

## Output

Return this impact matrix:

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

Also report the final contract, compatibility behavior, validation performed,
and unverified cases.

## Stop conditions

Stop rather than guess when ownership is unclear, an opcode collides, wire layout
is compiler-dependent, packet size cannot be validated safely, a required
producer or consumer is unavailable, or backward compatibility is undefined.
