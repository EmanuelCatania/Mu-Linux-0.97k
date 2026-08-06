# Login scene render callsite

## Target

- Executable SHA-256: `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F`
- File size: `1486848`
- PE timestamp: `0x3FBC1903`
- Preferred image base: `0x00400000`
- VA: `0x0052698A`
- RVA: `0x0012698A`
- Function or data size: callsite, 5-byte instruction

## Identification

- Requested behavior: native login scene render entry used by `Interface::RenderLogInScene`.
- Search strategy: source callsite, raw x86 call instruction, and target calculation.
- Related source files: `src/client/Main/Interface.cpp`, `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `CALL` hook site.
- Calling convention: `__cdecl` with `HDC` argument.
- Parameters: `HDC` is passed by the caller.
- Ownership/lifetime: native scene render owns the device context and scene transition.

## Hook details

- Hook type: `0xE8` callsite replacement.
- Required patch length: 5 bytes.
- Overwritten instruction: `E8 A1 AC FF FF` (`CALL 0x00521630`).
- Resume VA: `0x0052698F`.
- Trampoline requirements: wrapper must call the native scene renderer exactly once when preserving the scene.

## Signature

- Original bytes: `E8 A1 AC FF FF 83 C4 04 88 45 BC 83 3D C0 15 56`
- Expected match count: one at the supported executable VA.

## Evidence

- Evidence: matching source hook, raw bytes, relative target calculation, and native login scene behavior.
- Verified facts: the callsite targets `0x00521630` and resumes at `0x0052698F`.
- Inferences: the caller cleans the four-byte argument after the call.

## Assessment

- Confidence: Medium.
- Revalidation procedure: confirm fingerprint, bytes, target ABI, and scene-transition behavior in Ghidra and runtime.
