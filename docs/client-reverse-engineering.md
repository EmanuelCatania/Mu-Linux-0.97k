# Client reverse engineering

Use this document to identify and validate addresses, functions, globals,
structures, signatures, cross-references, and x86 ABI details in `main.exe`.
Use [client patching](client-patching.md) only after the target contract is
established.

## Fundamental rule

An address is valid only for the executable against which it was verified. Do
not reuse offsets from another version, region, Season, packed build, or
community source without revalidation.

The supported executable fingerprint and finding index live under
`docs/reverse-engineering/`. An undocumented address is a candidate, not an
established contract.

## Primary tooling

Use Ghidra with
[`bethington/ghidra-mcp`](https://github.com/bethington/ghidra-mcp) as the
primary static-analysis workflow.

Current project baseline:

- Ghidra MCP release: `v6.0.0`;
- Ghidra: `12.1.2`;
- Java: `21`;
- bridge command: `bridge-mcp-ghidra`;
- default local endpoint: `http://127.0.0.1:8089`.

The MCP bridge exposes a generated tool catalog. Do not assume every tool name
from an old prompt still exists. Use `check_tools`, `search_tools`,
`list_tool_groups`, and `load_tool_group` to discover capabilities at runtime.
The core `listing`, `function`, and `program` groups are loaded on connection.

Use `.agents/skills/ghidra-offset-analysis/SKILL.md` for offset discovery or
revalidation.

## MCP safety

- Keep the Ghidra MCP server bound to loopback. Use
  `GHIDRA_MCP_AUTH_TOKEN` when access is not strictly local.
- Prefer `GHIDRA_MCP_REQUIRE_PROGRAM_SELECTORS=1` and pass the program selector
  explicitly.
- Default to read-only analysis. Do not rename symbols, change types, create
  structures, patch bytes, run Ghidra-side scripts, emulate code, or control the
  debugger unless the task explicitly requires it.
- Use `dry_run` for write-capable operations when available and review the
  proposed target before applying a mutation.
- Never execute model-generated Ghidra scripts without reviewing their complete
  source.
- Confirm the selected program and SHA-256 before every analysis session;
  multi-program fallback to the current program is unsafe.
- Treat decompiler output as a hypothesis. Resolve conflicts in favor of
  disassembly, memory layout, references, and runtime evidence.

## Recommended workflow

1. Identify the exact file: SHA-256, file size, PE timestamp, preferred image
   base, entry-point RVA, and architecture.
2. Confirm that the matching program is open, selected explicitly, and fully
   analyzed in Ghidra.
3. Locate the behavior using independent semantic anchors: strings, imports,
   constants, callers, callees, globals, packet handlers, or known source
   behavior.
4. Inspect both decompiled code and bounded raw x86 instructions.
5. Inspect incoming and outgoing cross-references and reject candidates whose
   surrounding behavior does not match.
6. Determine calling convention, parameter widths, ownership, nullability,
   register use, stack cleanup, and return behavior.
7. Record VA and RVA. Record file offset only when needed and correctly mapped
   through PE sections.
8. Create a byte signature only from stable complete instructions; wildcard
   relocations, addresses, and build-dependent immediates.
9. Measure the signature match count before presenting it as a relocation aid.
10. Use runtime debugging only when static evidence cannot establish behavior,
    ABI, state, or hook safety.
11. Record evidence, assumptions, confidence, and revalidation steps in
    `docs/reverse-engineering/findings/`.

## Address and ABI rules

- Distinguish VA, RVA, file offset, and a pointer stored at an address.
- Prefer centralized, named addresses accompanied by a documented signature.
- Do not hide an uncertain signature or prototype with casts.
- Verify whether ASLR or relocations affect address resolution.
- Never calculate an RVA by subtracting an assumed image base; read the preferred
  image base from the verified PE.
- Confirm `__cdecl`, `__stdcall`, `__fastcall`, or `__thiscall`, stack cleanup,
  `ECX`/`EDX` use, preserved registers, parameter width and signedness, return
  mechanism, packing, alignment, offsets, virtual order, and ownership.

## Confidence levels

- **High:** multiple independent static anchors plus runtime confirmation.
- **Medium:** multiple independent static anchors and coherent ABI evidence, but
  no runtime confirmation.
- **Medium — source-backed:** exact fingerprint, source owner, original bytes,
  target calculation, and continuation are recorded, but persisted Ghidra
  identity, signature uniqueness, or runtime confirmation remains incomplete.
  Use this label for existing source-derived records, not as a substitute for a
  new analysis.
- **Low:** plausible static candidate with unresolved identity or ABI details.
- **Rejected:** contradictory evidence or executable fingerprint mismatch.

Only High or Medium findings should normally become new source-level offsets.
A `Medium — source-backed` record may support maintenance of its existing hook,
but must be revalidated before relocation, ABI changes, or reuse elsewhere.
