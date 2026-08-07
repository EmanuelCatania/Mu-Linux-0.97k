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

- Requested behavior: native login scene render entry wrapped by
  `Interface::RenderLogInScene`.
- Search strategy: source hook owner, exact executable bytes, relative
  call-target calculation, and caller stack cleanup.
- Strings/imports/constants: no string anchor recorded.
- Callers: the containing native scene dispatcher has not been named in the
  persisted Ghidra record.
- Callees: native login scene renderer at `0x00521630`.
- Data references: the caller passes an `HDC` and adjusts `ESP` by four bytes
  after the call.
- Related source files: `src/client/Main/Interface.cpp`,
  `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `CALL rel32` hook site.
- Calling convention: `__cdecl`.
- Return type: `void`.
- Parameters: one `HDC` argument supplied by the caller.
- Register use: no additional register contract recorded.
- Stack cleanup: caller cleans four bytes.
- Ownership/lifetime: native scene flow owns the device context and transition.

## Hook details

- Patch primitive: `CALL` callsite replacement.
- Hook type: `0xE8`.
- Replacement target: `Interface::RenderLogInScene`.
- Required patch length: 5 bytes.
- Overwritten instruction: `E8 A1 AC FF FF` (`CALL 0x00521630`).
- Resume VA: `0x0052698F`.
- Trampoline requirements: none for the callsite; the wrapper must call the
  native scene renderer exactly once when preserving original behavior.
- Patch owner: `Interface::Init`.
- Original values or bytes: `E8 A1 AC FF FF`.
- Preflight verification: absent in the current `SetCompleteHook`; future
  changes must compare the expected bytes before writing.

## Signature

- Original bytes: `E8 A1 AC FF FF 83 C4 04 88 45 BC 83 3D C0 15 56`
- Byte signature: `E8 ?? ?? ?? ?? 83 C4 04 88 45 BC 83 3D ?? ?? ?? ??`
- Wildcard rationale: wildcard the relative call displacement and the absolute
  address used by the later comparison.
- Expected match count: not measured; require exactly one match before using the
  signature for relocation or rediscovery.

## Evidence

- Ghidra program: `main.exe` matching the recorded fingerprint.
- Ghidra function or symbol: containing function name not persisted.
- Decompiler observations: not relied upon for this record.
- Disassembly evidence: exact `CALL rel32` bytes resolve to `0x00521630`,
  execution resumes at `0x0052698F`, and `ADD ESP, 4` follows the call.
- Cross-reference evidence: `Interface::Init` installs the patch and
  `Interface::RenderLogInScene` calls the native target.
- Debugger evidence: not recorded.
- Contradictory evidence considered: no conflicting patch owner found; scene
  transition and render-state behavior still require runtime testing.

## Assessment

- Confidence: Medium — source-backed.
- Maintenance scope: accepted for the existing hook while fingerprint, bytes,
  owner, target, and continuation remain unchanged; revalidate before
  relocation, ABI changes, or reuse.
- Verified facts: fingerprint, VA/RVA, original bytes, target calculation,
  caller cleanup, source owner, and replacement target.
- Inferences: the `HDC` contract is complete and no additional machine state is
  required by the wrapper.
- Remaining uncertainty: persisted Ghidra function identity, measured signature
  uniqueness, and runtime scene-transition validation.
- Revalidation procedure: confirm fingerprint, exact bytes, containing function,
  caller cleanup, target prototype, unique signature match, and runtime behavior
  across login and scene transitions.
