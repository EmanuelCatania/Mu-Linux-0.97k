---
name: ghidra-offset-analysis
description: Locate or revalidate a function, global, hook site, offset, byte signature, cross-reference, or x86 ABI detail in the supported MU Online main.exe using Ghidra and bethington/ghidra-mcp.
---

# Ghidra offset analysis

Produce a reproducible finding tied to the exact supported executable.

## Load

Always read:

- `docs/client-reverse-engineering.md`;
- `docs/reverse-engineering/main-exe.md`;
- the relevant existing finding and source files.

Run [`scripts/fingerprint-main.ps1`](scripts/fingerprint-main.ps1) when the
executable must be identified or compared. Read
[`references/ghidra-mcp-tool-selection.md`](references/ghidra-mcp-tool-selection.md)
only when instance selection, capability discovery, tool groups, or a version
mismatch requires it.

## Guardrails

- Select the Ghidra program explicitly and default to read-only analysis.
- Never patch `main.exe` through MCP.
- Do not mutate the project, execute scripts, emulate, or control the debugger
  without explicit task approval.
- Stop when the imported program, repository executable, and deployed runtime do
  not share the documented fingerprint.

## Workflow

1. **Verify the target.** Confirm PE32/x86, completed analysis, SHA-256, size, PE
   timestamp, image base, and entry-point RVA. A different executable is a
   separate target.
2. **Build anchors.** Derive at least two independent anchors when available:
   strings, imports, call sites, packet shapes, constants, globals, established
   callers/callees, or matching source behavior.
3. **Inspect candidates.** Compare decompilation with bounded x86 disassembly;
   inspect boundaries, references, callers, callees, and surrounding state;
   distinguish an address from a pointer stored at that address.
4. **Establish the contract.** Determine ABI or data layout, record VA and RVA
   separately, and derive file offsets only through PE section mapping. Build
   signatures from complete stable instructions, explain wildcards, and measure
   match count.
5. **Confirm only as needed.** Use bounded debugger evidence only when static
   analysis cannot establish identity, ABI, state, or hook safety.
6. **Record the result.** Update `docs/reverse-engineering/findings/`; add it to
   the accepted index in `docs/reverse-engineering/main-exe.md` only at Medium or
   High confidence.

## Result standard

Report fingerprint, VA/RVA, anchors, references, ABI or layout, relevant
instructions, signature evidence, facts, inferences, uncertainty, validation,
and affected source files.

- **High:** independent static anchors plus runtime confirmation.
- **Medium:** multiple coherent static anchors plus ABI/layout evidence.
- **Low:** plausible candidate with unresolved identity, ABI, or uniqueness.
- **Rejected:** fingerprint mismatch or contradictory evidence.

Never publish a production hard-coded address from a Low result. Stop rather than
guess when evidence conflicts, analysis is incomplete, ABI is unsafe to infer,
a signature has unexplained matches, or a required capability is unavailable.
