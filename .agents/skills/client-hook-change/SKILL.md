---
name: client-hook-change
description: Add, replace, or modify a Main.dll hook, detour, trampoline, patch site, function-pointer call, or other integration with the supported x86 main.exe.
---

# Client hook change

## Goal

Implement the smallest safe hook while preserving the original x86 ABI, control flow, and supported executable contract.

## Read first

Read only:

1. `docs/client-reverse-engineering.md`;
2. the target record under `docs/reverse-engineering/findings/`;
3. `docs/coding-patterns.md`;
4. the client section of `docs/testing.md`;
5. affected source and initialization code.

Read `docs/protocol.md` or `docs/runtime-data.md` only when the hook changes those contracts.

## Preconditions

- The target `main.exe` fingerprint must match `docs/reverse-engineering/main-exe.md`.
- The target address or function must have an accepted Medium- or High-confidence finding.
- If the target is missing or uncertain, run `.agents/skills/ghidra-offset-analysis/SKILL.md` first.
- Define the intended behavior and whether the original function or overwritten instructions must still execute.

## Workflow

### 1. Inspect existing hook infrastructure

- Locate existing hook helpers, initialization order, address definitions, and nearby hooks.
- Reuse an established mechanism when it satisfies the contract.
- Do not introduce a new detour framework for one change.
- Identify startup, shutdown, enable/disable, and failure behavior.

### 2. Verify the patch site

From the finding and disassembly, confirm:

- exact VA/RVA and expected original bytes;
- complete x86 instruction boundaries;
- transfer instruction size and relative-displacement range;
- total overwrite length;
- fall-through, branch targets, and resume address;
- instructions that must be relocated into a trampoline;
- absolute-address and ASLR assumptions.

Fail closed when expected bytes do not match. Do not patch a merely similar sequence.

### 3. Verify ABI and machine state

Confirm:

- calling convention and stack cleanup;
- argument locations and widths;
- `ECX`/`EDX` meaning;
- return mechanism;
- registers, flags, FPU, or SIMD state that must be preserved;
- object lifetime, nullability, and ownership;
- thread and reentrancy assumptions.

Do not hide an uncertain contract with a cast or naked wrapper.

### 4. Implement narrowly

- Centralize the verified address and use a typed function alias.
- Add `static_assert` or `offsetof` checks for verified layouts when useful.
- Keep work inside the hook bounded; avoid allocation, blocking I/O, locks, and exceptions on sensitive paths.
- Call the original implementation exactly as intended and prevent accidental recursion.
- Preserve original behavior outside the requested condition.
- Provide a safe no-hook path when initialization verification fails.

### 5. Review integration risks

Check:

- initialization before first use;
- shutdown and DLL detach;
- multiple installation attempts;
- concurrent calls;
- recursive rendering or message paths;
- stale pointers after map, character, window, or device transitions;
- compatibility with neighboring patches at the same region.

Update the reverse-engineering finding with final hook type, patch length, overwritten instructions, resume VA, and trampoline details.

### 6. Validate

At minimum:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

Also:

- run MSBuild parity when project or compiler settings changed;
- manually test installation, normal execution, the changed path, and shutdown;
- verify the original-byte mismatch path does not patch;
- inspect crashes, stack imbalance, recursion, rendering corruption, and state transitions;
- run repository validation and `git diff --check`.

## Output

Report:

- finding and executable fingerprint used;
- hook type, site, overwrite length, and resume address;
- ABI and preserved machine state;
- original behavior retained or replaced;
- affected files;
- validation performed;
- remaining runtime risks or unverified scenarios.

## Stop conditions

Stop rather than implement when the fingerprint differs, the finding is Low confidence, instruction boundaries are unclear, the trampoline cannot preserve semantics, the ABI is unresolved, expected bytes do not match, the target site is ambiguous, or the hook conflicts with another patch.
