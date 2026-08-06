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

- Requested behavior: reconnect path reads the native account buffer.
- Search strategy: source `ReconnectGetAccountInfo`, raw prologue bytes, and continuation disassembly.
- Related source files: `src/client/Main/Reconnect.cpp`, `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `JMP` to `__declspec(naked)` interceptor.
- Calling convention: machine-state and fall-through contract of the enclosing native function.
- Ownership/lifetime: reconnect state owns the copied account and password snapshots.

## Hook details

- Hook type: `0xE9` mid-function interceptor.
- Required patch length: 5 bytes.
- Overwritten instruction: `68 10 87 DB 07` (`PUSH 0x07DB8710`).
- Instructions reproduced by the interceptor: pushes at `0x0052042D` and `0x00520432`, and stores at `0x00520437` and `0x0052043C`.
- Resume VA: `0x00520442`.
- Trampoline requirements: explicit continuation; preserve the surrounding machine state and reconnect-only ownership.

## Signature

- Original bytes: `68 10 87 DB 07 68 C0 19 56 00 68 F0 9B 5C 05 A3 C8 7A 3A 08 89 1D 20 43 3A 08`
- Expected match count: one at the supported executable VA.

## Evidence

- Evidence: matching source naked wrapper, raw bytes, explicit resume address, and reconnect flow.
- Verified facts: the original first instruction pushes `0x07DB8710`; the wrapper resumes at `0x00520442`.
- Inferences: the two native stores must remain before the resumed call.

## Assessment

- Confidence: Medium.
- Revalidation procedure: verify every reproduced instruction, register/flag effect, and reconnect transition in Ghidra and runtime.
