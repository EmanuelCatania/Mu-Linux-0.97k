# Reconnect account capture site

## Target

- Executable SHA-256: `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F`
- File size: `1486848`
- PE timestamp: `0x3FBC1903`
- Preferred image base: `0x00400000`
- VA: `0x00520428`
- RVA: `0x00120428`
- Function or data size: mid-function patch site, first instruction is 5 bytes

## Identification

- Requested behavior: capture the native account and password buffers for the
  reconnect flow.
- Search strategy: current naked interceptor, exact executable bytes, explicit
  continuation, and source ownership.
- Strings/imports/constants: native input buffers at `0x07DB8710` and
  `0x07DB8810`.
- Callers: containing native function not named in the persisted Ghidra record.
- Callees: the interceptor calls `memcpy` before reproducing native instructions.
- Data references: pushes `0x07DB8710`, `0x005619C0`, and `0x055C9BF0`; stores
  `EAX` to `0x083A7AC8` and `EBX` to `0x083A4320`.
- Related source files: `src/client/Main/Reconnect.cpp`,
  `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `JMP` to `__declspec(naked)` mid-function interceptor.
- Calling convention: machine-state and fall-through contract of the enclosing
  native function.
- Return type: continuation by explicit jump.
- Register use: `Pushad/Popad` restores general-purpose registers around the
  capture call.
- Stack cleanup: the interceptor reproduces the native pushes before continuing.
- Preserved registers: general-purpose registers only during the `memcpy` block.
- Ownership/lifetime: reconnect state owns plaintext snapshots after capture.

## Hook details

- Patch primitive: naked interceptor.
- Hook type: `0xE9`.
- Replacement target: `CReconnect::ReconnectGetAccountInfo`.
- Required patch length: 5 bytes.
- Overwritten instruction: `68 10 87 DB 07` (`PUSH 0x07DB8710`).
- Resume VA: `0x00520442`.
- Trampoline requirements: explicit continuation after reproducing the native
  pushes and stores.
- Patch owner: `CReconnect::Init`.
- Original values or bytes: `68 10 87 DB 07`.
- Preflight verification: absent in the current `SetCompleteHook`; future
  changes must compare the expected bytes before writing.

## Signature

- Original bytes: `68 10 87 DB 07 68 C0 19 56 00 68 F0 9B 5C 05 A3 C8 7A 3A 08 89 1D 20 43 3A 08`
- Byte signature: not accepted yet.
- Wildcard rationale: most operands are absolute addresses; a useful signature
  requires surrounding stable control-flow bytes and a measured uniqueness
  check.
- Expected match count: not measured.

## Evidence

- Ghidra program: `main.exe` matching the recorded fingerprint.
- Ghidra function or symbol: containing function name not persisted.
- Decompiler observations: not relied upon for this record.
- Disassembly evidence: exact bytes identify the overwritten push and the
  instructions reproduced before continuation at `0x00520442`.
- Cross-reference evidence: `CReconnect::Init` owns the patch and the current
  wrapper copies both native credential buffers.
- Debugger evidence: not recorded.
- Contradictory evidence considered: `Pushad/Popad` does not restore `EFLAGS`,
  while the original pushes and stores do not modify flags. The liveness of
  incoming flags at the continuation has not been established.

## Assessment

- Confidence: Low.
- Verified facts: fingerprint, VA/RVA, original bytes, source owner, reproduced
  instructions, replacement target, and continuation.
- Inferences: changing `EFLAGS` during the capture call does not affect the
  resumed native flow.
- Remaining uncertainty: containing Ghidra function identity, flag liveness,
  FPU/SIMD state, signature uniqueness, runtime reconnect transitions, and
  plaintext snapshot cleanup.
- Revalidation procedure: identify the containing function in Ghidra, verify
  every reproduced instruction and branch, determine flag liveness, inspect
  callers and continuations, and test reconnect success, failure, logout, and
  disconnect before modifying or accepting this site.
