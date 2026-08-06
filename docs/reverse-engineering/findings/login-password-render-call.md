# Login password render callsite

## Target

- Executable SHA-256: `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F`
- File size: `1486848`
- PE timestamp: `0x3FBC1903`
- Preferred image base: `0x00400000`
- VA: `0x005217A0`
- RVA: `0x001217A0`
- Function or data size: callsite, 5-byte instruction

## Identification

- Requested behavior: native password field rendering.
- Search strategy: source `LoginInputPasswordCall`, raw x86 callsite, and cross-reference to the shared text renderer.
- Related source files: `src/client/Main/Input.cpp`, `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `CALL` hook site.
- Calling convention: callsite-compatible native text-render function.
- Parameters: established by the surrounding native login renderer.
- Ownership/lifetime: native login scene owns the input state.

## Hook details

- Hook type: `0xE8` callsite replacement.
- Required patch length: 5 bytes.
- Overwritten instruction: `E8 0B D9 F5 FF` (`CALL 0x0047F0B0`).
- Resume VA: `0x005217A5`.
- Trampoline requirements: none for the callsite itself; a wrapper must preserve the call ABI.

## Signature

- Original bytes: `E8 0B D9 F5 FF A1 7C 42 3A 08 83 C4 38 8D 7E 5F`
- Expected match count: one at the supported executable VA.

## Evidence

- Evidence: matching source constant, raw bytes, relative target calculation, and native login rendering behavior.
- Verified facts: the callsite targets `0x0047F0B0` and resumes at `0x005217A5`.
- Inferences: the surrounding code loads and cleans the native input state after rendering.

## Assessment

- Confidence: Medium.
- Revalidation procedure: confirm fingerprint, bytes, target, and surrounding stack setup in Ghidra before changing the hook.
