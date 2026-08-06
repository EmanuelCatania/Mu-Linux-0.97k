---
name: ghidra-offset-analysis
description: Locate or revalidate a function, global, hook site, offset, byte signature, cross-reference, or x86 ABI detail in the supported MU Online main.exe using Ghidra and bethington/ghidra-mcp.
---

# Ghidra offset analysis

## Goal

Produce a reproducible finding tied to one exact executable, not only a hexadecimal address.

## Read first

Read only:

1. `docs/client-reverse-engineering.md`;
2. `docs/reverse-engineering/main-exe.md`;
3. the relevant finding under `docs/reverse-engineering/findings/`, when present;
4. source files related to the requested behavior.

All paths are repository-root-relative.

## Tool baseline

The validated baseline is `bethington/ghidra-mcp` `v6.0.0` with Ghidra `12.1.2`. If the installed version differs, discover capabilities at runtime and update the baseline only after verification.

Use management tools rather than assuming schema-generated names:

- `list_instances` and `connect_instance`;
- `check_tools` and `search_tools`;
- `list_tool_groups` and `load_tool_group`.

The core `listing`, `function`, and `program` groups should already be loaded.

## Safety

- Default to read-only analysis.
- Select the program explicitly for every program-aware operation.
- Do not rename, retype, comment, patch, create data types, run scripts, emulate code, or use the debugger unless the task requires it.
- Do not pause or alter a running client through debugger controls without explicit approval.
- Use `dry_run` before an approved mutation when available.
- Never patch `main.exe` through MCP.
- Keep the MCP service on loopback and do not weaken authentication.

## Workflow

### 1. Verify the target

- Connect to the intended Ghidra instance.
- Confirm `main.exe`, PE32/x86, completed analysis, and explicit program selection.
- Compare SHA-256, size, PE timestamp, and image base with `docs/reverse-engineering/main-exe.md`.
- Stop on a mismatch.
- If required fingerprint fields remain `TBD`, continue only as a candidate investigation and do not publish a permanent source offset.

### 2. Build independent anchors

Derive semantic anchors from repository code, then search with the strongest available evidence:

1. unique strings and references;
2. imported APIs and call sites;
3. packet opcode/subcode handling;
4. distinctive constants or table sizes;
5. known global accesses;
6. callers and callees of established functions;
7. source behavior that matches control flow.

Do not accept a candidate supported by only one weak anchor.

### 3. Inspect candidates

For each candidate:

- inspect function or data boundaries;
- compare decompiled output with raw x86 instructions;
- inspect incoming and outgoing references, callers, and callees;
- compare strings, constants, globals, and control flow with expected behavior;
- reject contradictory candidates.

Treat decompiler names and types as hypotheses.

### 4. Establish the contract

Use `docs/client-reverse-engineering.md` to determine:

- function ABI, arguments, registers, stack cleanup, return behavior, and ownership; or
- global/structure width, signedness, storage, bounds, alignment, packing, and lifetime.

Separate verified facts from inference.

### 5. Record addresses and signatures

- Record VA and RVA separately.
- Record file offset only when derived through PE section mapping.
- State how ASLR affects runtime translation.
- When a signature is needed, use stable complete instructions, wildcard build-dependent bytes, and verify the match count.
- A unique signature supports relocation; it does not prove semantic identity.

### 6. Use dynamic confirmation only when needed

Use bounded debugger tracing or breakpoints when static evidence cannot establish identity, ABI, state, or hook safety. Verify static-to-dynamic translation and record observed arguments, registers, call order, and return behavior.

### 7. Record and report

Create or update a finding under `docs/reverse-engineering/findings/`. Update the index in `docs/reverse-engineering/main-exe.md` only for an accepted result.

Report:

- executable fingerprint;
- VA and RVA;
- identification anchors;
- callers, callees, and references;
- ABI or layout;
- relevant instructions;
- signature and match count, when applicable;
- confidence, facts, inferences, uncertainty, and revalidation steps;
- source files likely affected.

## Confidence and stop conditions

- **High:** independent static anchors plus runtime confirmation.
- **Medium:** multiple coherent static anchors and ABI/layout evidence.
- **Low:** plausible candidate with unresolved identity, ABI, or uniqueness.
- **Rejected:** fingerprint mismatch or contradictory evidence.

Do not add a production hard-coded address from a Low result.

Stop rather than guess when the fingerprint differs, the program cannot be selected explicitly, analysis is incomplete, evidence conflicts, the ABI is unsafe to infer, a signature has unexplained matches, or a required capability is unavailable.
