---
name: client-hook-change
description: Add, replace, or modify a Main.dll hook, detour, trampoline, patch site, function-pointer call, memory patch, or other integration with the supported x86 main.exe.
---

# Client hook change

Implement the smallest safe patch while preserving the original x86 ABI,
control flow, and supported executable contract.

## Read first

Read only:

1. `docs/client-patching.md`;
2. the target record under `docs/reverse-engineering/findings/`;
3. `docs/coding-patterns.md`;
4. the client section of `docs/testing.md`;
5. affected source and initialization code.

Read `docs/client-reverse-engineering.md` only when the target must be located or
revalidated. Read `docs/protocol.md` or `docs/runtime-data.md` only when the patch
changes those contracts.

## Preconditions

- Confirm that the analyzed and deployed `main.exe` matches
  `docs/reverse-engineering/main-exe.md`.
- Require an accepted Medium- or High-confidence finding for the target. A
  `Medium — source-backed` finding may support maintenance of the existing patch
  only while its exact bytes, owner, and continuation still match. Run
  `ghidra-offset-analysis` before relocation, ABI changes, or reuse elsewhere.
- Search the whole project for the address, symbol, and nearby patch sites.
  One patch site must have one owner.
- Classify the patch using the primitives and hook categories documented in
  `docs/client-patching.md`.

## Verify the contract

From the finding and current disassembly, confirm:

- exact VA/RVA and expected original bytes or value;
- instruction boundaries, overwrite length, destination, and continuation;
- calling convention, argument locations, stack cleanup, and return behavior;
- registers, flags, FPU, and SIMD state that may be live;
- pointer ownership, lifetime, nullability, and reentrancy;
- whether original instructions or the original callee must still execute;
- whether another patch owns or reaches the same region.

Do not hide uncertainty with a cast, varargs call, or naked wrapper. Stop when
the target, ABI, continuation, or ownership is unresolved.

## Implement narrowly

- Reuse the established patch mechanism when it satisfies the verified contract.
- Centralize verified addresses in `Offsets.h` or the existing address owner.
- Compare expected bytes or values before writing. On mismatch, leave the
  executable unmodified and report the failure.
- Preserve original behavior outside the requested condition.
- Reproduce or relocate overwritten instructions only after proving their
  semantics and relative-address behavior.
- Prevent recursive entry, duplicate installation, and competing patch owners.
- Keep sensitive hooks bounded; avoid allocation, blocking I/O, locks, and
  exceptions unless their failure and reentrancy behavior are established.
- Add layout assertions when verified structures cross the native boundary.
- Update the finding with the final owner, replacement target, original bytes,
  preflight behavior, continuation, and remaining uncertainty.

For render or input hooks, use the native render pass when practical, restore
global render state, and consume only events owned by the custom control.

## Validate

At minimum:

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Debug
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Release
```

Also:

- run MSBuild parity when project or compiler settings changed;
- manually test installation, normal execution, the changed path, transitions,
  and shutdown;
- verify the expected-byte mismatch path does not patch;
- inspect crashes, stack imbalance, changed flags, recursion, rendering
  corruption, and neighboring patches;
- run repository validation and `git diff --check`.

## Output

Report:

- finding and executable fingerprint used;
- patch category, site, owner, overwrite length, and destination;
- expected bytes or value and the preflight result;
- ABI, preserved machine state, and continuation behavior;
- original behavior retained, wrapped, or replaced;
- affected files, validation performed, and remaining runtime risks.

## Stop conditions

Stop rather than implement when the fingerprint differs, the finding is Low
confidence, expected bytes do not match, instruction boundaries are unclear, the
ABI is unresolved, the target is not unique, the continuation or trampoline is
unsafe, or another patch owns the site.
