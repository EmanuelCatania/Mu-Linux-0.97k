# Client/server protocol

This document defines packet contracts. Use
`.agents/skills/protocol-change/SKILL.md` when implementing a packet or handler
change. The inventory can grow as packets are touched.

## Packet contract

For each packet, record:

- name, purpose, direction, and owning process;
- header type, opcode, subcode, and collision search;
- exact, minimum, or maximum size;
- structure, packing, byte order, signedness, and variable-length bounds;
- connection or game state in which it is accepted;
- authoritative and untrusted fields;
- sender, receiver, rejection behavior, and persistent side effects;
- backward compatibility and version-gate behavior;
- validation and tests.

## Wire layout

- Use explicit-width integer fields for new wire data.
- Preserve packing, byte order, signedness, and header length.
- Do not expose pointers, compiler-dependent enums, `bool`, platform-sized
  types, or internal object layout directly on the wire.
- Verify fixed structure sizes at compile time where practical.
- Use overflow-safe calculations for variable-length packets and reject lengths
  outside the documented bounds.

## Safe reception and authority

Before reading or mutating state:

1. validate the received and declared lengths;
2. validate opcode and subcode;
3. reject truncation, excessive size, and malformed variable sections;
4. validate indices, IDs, coordinates, counts, ranges, and session state;
5. verify permissions and the owning process's authority;
6. handle duplicate, out-of-order, or replayed messages after reconnect;
7. confirm resources for the complete operation;
8. avoid partial mutation when a later step can fail.

Do not trust client-provided prices, stats, currency, permissions, item state,
rewards, or other server-authoritative values.

## Change rules

- Search the entire repository for opcode/subcode, structure names,
  serialization, handlers, runtime flags, and version checks.
- Update required senders, receivers, and shared definitions together.
- Preserve process ownership from `docs/architecture.md`.
- Coordinate persisted effects through the existing DataServer/database path.
- Preserve unknown-client behavior or introduce an explicit compatibility gate.
- Define rejection behavior without leaking sensitive details.
- Add automated validation when size, opcode, or layout can be checked
  mechanically.

## Validation

Test at least:

- valid request and response;
- shortest or truncated packet;
- oversized or malformed variable-length packet;
- invalid opcode/subcode or connection state;
- boundary IDs, counts, coordinates, and ranges;
- duplicate, replayed, or out-of-order behavior when relevant;
- disconnect or persistence failure during side effects;
- old/new version mismatch when compatibility changes.

## Record template

```text
Name:
Purpose:
Direction:
Owner:
Header:
Opcode/Subcode:
Size:
Layout:
Allowed state:
Authoritative fields:
Untrusted fields:
Sender:
Receiver:
Rejection behavior:
Side effects:
Compatibility:
Tests:
```
