# Client reverse engineering

Use this document for hooks, addresses, internal functions, and layouts in the x86 `main.exe`.

## Fundamental rule

An address is valid only for the executable against which it was verified. Do not reuse offsets from another version, region, Season, packed build, or community source without revalidation.

The supported executable fingerprint and finding index live under `docs/reverse-engineering/`. An undocumented address is a candidate, not an established contract.

## Primary tooling

Use Ghidra with [`bethington/ghidra-mcp`](https://github.com/bethington/ghidra-mcp) as the primary static-analysis workflow.

Current project baseline:

- Ghidra MCP release: `v6.0.0`;
- Ghidra: `12.1.2`;
- Java: `21`;
- bridge command: `bridge-mcp-ghidra`;
- default local endpoint: `http://127.0.0.1:8089`.

The MCP bridge exposes a generated tool catalog. Do not assume every tool name from an old prompt still exists. Use `check_tools`, `search_tools`, `list_tool_groups`, and `load_tool_group` to discover capabilities at runtime. The core `listing`, `function`, and `program` groups are loaded on connection.

Use `.agents/skills/ghidra-offset-analysis/SKILL.md` for offset discovery or revalidation and `.agents/skills/client-hook-change/SKILL.md` for hook implementation.

## MCP safety

- Keep the Ghidra MCP server bound to loopback. Use `GHIDRA_MCP_AUTH_TOKEN` when access is not strictly local.
- Prefer `GHIDRA_MCP_REQUIRE_PROGRAM_SELECTORS=1` and pass the program selector explicitly.
- Default to read-only analysis. Do not rename symbols, change types, create structures, patch bytes, run scripts, emulate code, or control the debugger unless the task explicitly requires it.
- Use `dry_run` for write-capable operations when available and review the proposed target before applying a mutation.
- Never execute model-generated Ghidra scripts without reviewing their complete source.
- Confirm the selected program and SHA-256 before every analysis session; multi-program fallback to the current program is unsafe.
- Treat decompiler output as a hypothesis. Resolve conflicts in favor of disassembly, memory layout, references, and runtime evidence.

## Recommended workflow

1. Identify the exact file: SHA-256, file size, PE timestamp, preferred image base, and architecture.
2. Confirm that the matching program is open, selected explicitly, and fully analyzed in Ghidra.
3. Locate the behavior using independent semantic anchors: strings, imports, constants, callers, callees, globals, packet handlers, or known source behavior.
4. Inspect both decompiled code and raw x86 instructions.
5. Inspect incoming and outgoing cross-references and reject candidates whose surrounding behavior does not match.
6. Determine calling convention, parameter widths, ownership, nullability, register use, stack cleanup, and return behavior.
7. Record VA and RVA. Record file offset only when it is actually needed and can be mapped correctly.
8. Create a byte signature only from stable bytes; wildcard relocations, addresses, and build-dependent immediates.
9. Use runtime debugging when static evidence cannot establish behavior, ABI, or hook safety.
10. Record evidence, assumptions, confidence, and revalidation steps in `docs/reverse-engineering/findings/`.

## Addresses

- Distinguish VA, RVA, file offset, and a pointer stored at an address.
- Prefer centralized, named addresses accompanied by a documented byte signature.
- Do not hide an uncertain signature or prototype with casts.
- Verify whether ASLR or relocations affect address resolution.
- Never calculate an RVA by subtracting an assumed image base; read the preferred image base from the verified PE.

## Patch primitives in `Util.cpp`

`src/client/Main/Util.cpp` contains low-level write helpers. They change the
running image directly and do not provide byte verification, rollback, or
instruction-cache flushing. Treat every call as a binary patch that needs its
own finding.

### Typed writes

- `SetByte`, `SetWord`, `SetDword`, `SetFloat`, and `SetDouble` write exactly
  1, 2, 4, 4, and 8 bytes respectively.
- Record the original value, replacement value, width, type, and endianness.
- State whether the address contains data, a pointer, an immediate operand, or
  an opcode. An expression such as `address + 1` must identify the instruction
  field being changed, not just the resulting address.
- Revalidate all readers and writers when changing a typed value.

### Block writes

- `MemoryCpy` replaces an exact byte sequence. Record the complete original
  bytes, replacement bytes, size, and whether the target is code or data.
- `MemorySet` fills a region. When it writes `0x90`, prove that the region
  covers complete instructions, that no branch enters its middle, and that
  removing each instruction preserves the intended control flow.
- Do not use an approximate byte count for a code patch.

### Relative transfers

`SetCompleteHook` writes exactly five bytes: an opcode followed by a relative
32-bit displacement. It does not create a trampoline, preserve overwritten
instructions, validate the original bytes, verify an ABI, or coordinate with
other patches. Classify each use before changing it:

1. `0xE8` replacing a callsite. The wrapper must match the callsite ABI and
   returns naturally to `callsite + 5`.
2. `0xE9` replacing a complete function. The replacement owns the complete
   function contract; there is no automatic return to the original body.
3. `0xE9` entering a `__declspec(naked)` interceptor in the middle of a
   function. Record overwritten instructions, every continuation address, and
   all machine state that must be preserved or reproduced.
4. `0xE9` jumping directly to another native address. Treat this as a control-
   flow patch, not as a callable function hook.

The `0xFF` option preserves the existing opcode while replacing the relative
operand. It has no validated use in this project and should not be introduced
without a dedicated finding.

`VirtualizeOffset` is legacy code with no known current callsite. Do not use it
for new work without rechecking instruction relocation, a minimum five-byte
size, relative instructions, executable trampoline memory under DEP, allocation
failure, and lifetime. It does not provide those guarantees itself.

## x86 ABI

Explicitly confirm:

- `__cdecl`, `__stdcall`, `__fastcall`, or `__thiscall`;
- who cleans the stack;
- `ECX`/`EDX` use and preserved registers;
- parameter width and signedness;
- return through `EAX`, FPU, or memory;
- packing, alignment, offsets, and virtual order;
- ownership of strings, buffers, handles, and objects.

## Hook safety

- Do not split instructions.
- Do not overwrite fewer bytes than the jump requires.
- Preserve overwritten instructions when they must execute in a trampoline.
- Preserve flags and registers when required by the contract.
- Avoid allocation, locks, or exceptions in sensitive paths without analyzing reentrancy.
- Validate pointers before accessing memory owned by `main.exe`.
- Search the whole project for the address, symbol, and nearby patch sites before
  installing a hook. A patch site has one owner; independent subsystems must not
  install competing hooks at the same address.
- For a `__declspec(naked)` interceptor, do not treat `Pushad/Popad` as complete
  preservation: flags, FPU, and SIMD state may still be live.
- Keep visual presentation separate from native buffers or protocol data.
- Render extensions in the native render pass when possible, and save/restore
  global render state.
- WndProc hooks consume only events owned by the custom control. Test startup,
  scene transitions, and shutdown, not only the steady-state screen.

## Confidence levels

- **High:** multiple independent static anchors plus runtime confirmation.
- **Medium:** multiple independent static anchors and coherent ABI evidence, but no runtime confirmation.
- **Low:** plausible static candidate with unresolved identity or ABI details.
- **Rejected:** contradictory evidence or executable fingerprint mismatch.

Only High or Medium findings should normally become source-level offsets. Mark any remaining inference beside the code and in the finding record.
