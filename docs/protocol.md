# Client/server protocol

This document defines packet contracts. Use `.agents/skills/protocol-change/SKILL.md` when implementing a packet or handler change. The inventory can grow as packets are touched.

## Packet contract

For each packet, record:

- direction and owning process;
- header type;
- opcode and subcode;
- minimum, maximum, or exact size;
- structure, packing, byte order, and signedness;
- state in which it is accepted;
- authority of each field;
- sender, receiver, and persistent effects;
- backward compatibility.

## Safe reception

Before accessing the body:

1. validate the header and declared length;
2. validate opcode/subcode;
3. reject truncation and excessive size;
4. validate indices, IDs, coordinates, ranges, and session state;
5. do not trust client-provided prices, stats, currency, permissions, or item data;
6. handle duplicate, out-of-order, or replayed messages after reconnect;
7. do not partially mutate state when a later step can fail.

## Changes

- Search the entire repository for the opcode, structure name, and handlers.
- Update client and server together when layout changes.
- Preserve byte order, packing, and field widths.
- Define behavior for mismatched client/server versions.
- Add automated validation when size or code can be checked mechanically.
- Test valid, truncated, oversized, invalid-state, and boundary-value packets.

## Record template

```text
Name:
Direction:
Owner:
Header:
Opcode/Subcode:
Size:
Layout:
Allowed state:
Authoritative fields:
Sender:
Receiver:
Side effects:
Compatibility:
Tests:
```
