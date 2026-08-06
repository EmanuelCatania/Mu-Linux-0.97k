---
name: client-hook-change
description: Add, replace, or modify a Main.dll hook, detour, trampoline, patch site, function-pointer call, memory patch, or other integration with the supported x86 main.exe.
---

# Client hook change

Implement the smallest safe patch while preserving the original x86 ABI,
control flow, and supported executable contract.

## Read first

Read only:

1. `docs/client-reverse-engineering.md`;
2. the target record under `docs/reverse-engineering/findings/`;
3. `docs/coding-patterns.md`;
4. the client section of `docs/testing.md`;
5. affected source and initialization code.

Read `docs/protocol.md` or `docs/runtime-data.md` only when the patch changes
those contracts.

## Preconditions

- Confirm that the analyzed and deployed `main.exe` matches
  `docs/reverse-engineering/main-exe.md`.
- Require an accepted Medium- or High-confidence finding for the target. If it
  is missing or uncertain, run `ghidra-offset-analysis` first.
- Search the whole project for the address, symbol, and nearby patch sites.
  One patch site must have one owner; do not install competing hooks from
  independent subsystems.
- Classify the change before implementation: typed write, block write, callsite
  hook, complete replacement, naked interceptor, or direct control-flow patch.

## Patch primitives

`SetByte`, `SetWord`, `SetDword`, `SetFloat`, and `SetDouble` write fixed-width
values. Record the original value, replacement value, width, type, endianness,
and whether the address is data, a pointer, an immediate operand, or an opcode.
Explain expressions such as `address + 1` in terms of the instruction field they
modify.

`MemoryCpy` replaces an exact byte block. `MemorySet` fills a byte block, often
with `0x90`. For code, record complete instructions, incoming branches, and the
effect of removing each instruction. Never choose a NOP size approximately.

`SetCompleteHook` writes exactly five bytes (`opcode + rel32`). It does not
validate original bytes, create a trampoline, preserve overwritten instructions,
verify an ABI, flush the instruction cache, or coordinate with other patches.
Perform those checks before invoking it. The `0xFF` form preserves the existing
opcode while changing the relative operand; it has no validated use here.

Treat `VirtualizeOffset` as legacy and unavailable for new work unless a finding
revalidates instruction relocation, relative operands, executable trampoline
memory under DEP, allocation failure, and lifetime.

## Hook categories

### `0xE8` callsite replacement

- Confirm the address is an existing `CALL rel32` instruction, not the callee's
  entry point.
- Match the callsite calling convention, argument locations, return value, stack
  cleanup, and relevant registers.
- Return normally so execution resumes at `callsite + 5`.
- Call the original implementation explicitly when the behavior is a wrapper.

### `0xE9` complete function replacement

- Confirm the address is the function entry.
- Implement the complete native contract; there is no automatic return to the
  original function body.
- Match calling convention, hidden `ECX`/`EDX` use, return mechanism, object
  lifetime, and stack cleanup. Do not introduce an unverified hidden `this`.

### `0xE9` naked interceptor

- Confirm the patch is in the middle of a function.
- Record every overwritten instruction and every resume/branch address.
- Reproduce original instructions when their effects are still required.
- Preserve registers, flags, stack, FPU, and SIMD state according to the
  surrounding contract. `Pushad/Popad` does not preserve flags, FPU, or SIMD.
- Use explicit jumps to continuations; do not use a normal C++ `return`.

### `0xE9` direct control-flow patch

- Treat a direct jump to another native address as a flow patch, not a callable
  function hook.
- Verify instruction boundaries, destination, skipped side effects, and every
  branch that can reach the patched region.

## ABI and machine state

Confirm `__cdecl`, `__stdcall`, `__fastcall`, or `__thiscall`; stack cleanup;
parameter widths and signedness; `ECX`/`EDX`; return registers or FPU values;
packing and offsets; ownership; nullability; and reentrancy.

Do not hide an uncertain contract with a cast or an unverified naked wrapper.
Avoid allocation, blocking I/O, locks, and exceptions on sensitive paths unless
their reentrancy and failure behavior are established.

## Implementation

- Centralize verified addresses in `Offsets.h` or the established address owner.
- Preserve original behavior outside the requested condition.
- Add layout assertions when a verified structure is shared with native code.
- Preflight expected bytes before patching. If the preflight fails, leave the
  executable unmodified and report the mismatch.
- Prevent recursive entry and duplicate installation.
- Keep visual presentation separate from native buffers and protocol data.
- For render/input hooks, prefer the native render pass, save and restore global
  render state, and consume only owned WndProc events.
- Test initialization, scene transitions, shutdown, and neighboring patches.

## Validation

At minimum:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

Also:

- run MSBuild parity when project or compiler settings changed;
- manually test installation, normal execution, the changed path, transitions,
  and shutdown;
- verify the original-byte mismatch path does not patch;
- inspect crashes, stack imbalance, recursion, rendering corruption, and state
  transitions;
- run repository validation and `git diff --check`.

## Output

Report:

- finding and executable fingerprint used;
- patch category, opcode, site, overwrite length, and destination;
- original bytes or values and the preflight result;
- ABI, preserved machine state, and continuation behavior;
- original behavior retained, wrapped, or replaced;
- affected files and patch owner;
- validation performed and remaining runtime risks.

## Stop conditions

Stop rather than implement when the fingerprint differs, the finding is Low
confidence, instruction boundaries are unclear, expected bytes do not match, the
ABI is unresolved, the target is not uniquely identified, the trampoline or
continuation is unsafe, or another patch owns the site.
