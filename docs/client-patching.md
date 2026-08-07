# Client patching

Use this document for hooks, detours, trampolines, typed memory writes, block
patches, function-pointer calls, and other changes to the running x86
`main.exe`. Establish the target first through
[client reverse engineering](client-reverse-engineering.md) and a finding under
`docs/reverse-engineering/findings/`.

## Patch-site contract

Before writing code, record:

- executable fingerprint, VA/RVA, and expected original bytes or value;
- patch owner and every other reference to the same or neighboring region;
- complete instruction boundaries and required overwrite length;
- destination, continuation, and whether original instructions or the original
  callee must still execute;
- ABI, argument locations, stack cleanup, return behavior, registers, flags,
  FPU/SIMD state, pointer ownership, lifetime, and reentrancy;
- mismatch behavior, rollback expectations, and runtime validation.

A patch site has one owner. On an expected-byte mismatch, leave the executable
unmodified and report the failure.

## Patch primitives in `Util.cpp`

`src/client/Main/Util.cpp` contains low-level write helpers. They change the
running image directly and do not provide byte verification, rollback, or
instruction-cache flushing. Treat every call as a binary patch with its own
finding and preflight.

### Typed writes

- `SetByte`, `SetWord`, `SetDword`, `SetFloat`, and `SetDouble` write exactly
  1, 2, 4, 4, and 8 bytes.
- Record original and replacement values, width, type, endianness, and whether
  the target is data, a pointer, an immediate operand, or an opcode.
- An expression such as `address + 1` must identify the instruction field being
  changed, not only the resulting address.
- Revalidate all readers and writers when changing a typed value.

### Block writes

- `MemoryCpy` replaces an exact byte sequence. Record complete original and
  replacement bytes, size, and whether the target is code or data.
- `MemorySet` fills a region. When writing `0x90`, prove that it covers complete
  instructions, no branch enters the middle, and removing each instruction
  preserves the intended control flow.
- Never use an approximate byte count for a code patch.

### Relative transfers

`SetCompleteHook` writes exactly five bytes: an opcode followed by a relative
32-bit displacement. It does not create a trampoline, preserve overwritten
instructions, validate original bytes, verify an ABI, or coordinate with other
patches. Its untyped x86 varargs destination does not validate the target
prototype.

Classify every use:

1. `0xE8` replaces a callsite. The wrapper must match the callsite ABI and
   returns to `callsite + 5`.
2. `0xE9` replaces a complete function. The replacement owns the complete
   function contract.
3. `0xE9` enters a `__declspec(naked)` interceptor inside a function. Record
   overwritten instructions, every continuation, and all machine state that
   must be preserved or reproduced.
4. `0xE9` jumps directly to another native address. Treat it as a control-flow
   patch, not a callable function hook.

The `0xFF` option preserves the existing opcode while replacing the relative
operand. It has no validated use in this project and requires a dedicated
finding before introduction.

`VirtualizeOffset` is legacy code with no known current callsite. Do not use it
for new work without rechecking instruction relocation, a minimum five-byte
size, relative instructions, executable trampoline memory under DEP, allocation
failure, and lifetime.

## Implementation safety

- Do not split instructions or overwrite fewer bytes than the transfer requires.
- Relocate overwritten instructions only after proving their semantics and
  relative-address behavior.
- Preserve live registers, flags, FPU, and SIMD state. `Pushad/Popad` does not
  preserve all of them.
- Prevent duplicate installation, recursion, and competing patch owners.
- Avoid allocation, blocking I/O, locks, and exceptions in sensitive paths
  unless failure and reentrancy behavior are established.
- Validate pointers before accessing memory owned by `main.exe`.
- Keep visual presentation separate from native buffers or protocol data.
- Render extensions in the native render pass when practical and restore global
  render state.
- WndProc hooks must consume only events owned by the custom control. Test
  startup, scene transitions, and shutdown.

Use `.agents/skills/client-hook-change/SKILL.md` for the complete implementation
and validation workflow.
