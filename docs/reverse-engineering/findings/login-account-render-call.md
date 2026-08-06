# Login account render callsite

## Target

- Executable SHA-256: `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F`
- File size: `1486848`
- PE timestamp: `0x3FBC1903`
- Preferred image base: `0x00400000`
- VA: `0x00521778`
- RVA: `0x00121778`
- Function or data size: callsite, 5-byte instruction

## Identification

- Requested behavior: native account field rendering.
- Search strategy: source constant `LoginInputAccountCall`, exact executable
  bytes, relative call-target calculation, and the current hook owner.
- Strings/imports/constants: no string anchor recorded; source constant is
  `0x00521778`.
- Callers: the containing native login-render function has not been named in
  the persisted Ghidra record.
- Callees: native input renderer at `0x0047F0B0`.
- Data references: surrounding pushes establish the native renderer arguments.
- Related source files: `src/client/Main/Input.cpp`,
  `src/client/Main/Offsets.h`.

## ABI or layout

- Kind: `CALL rel32` hook site.
- Calling convention: callsite-compatible native text-render function.
- Return type: `void`.
- Parameters: established by the surrounding native login renderer.
- Register use: unresolved beyond the surrounding callsite contract.
- Stack cleanup: performed by the surrounding native flow.
- Ownership/lifetime: native login scene owns the input state.

## Hook details

- Patch primitive: `CALL` callsite replacement.
- Hook type: `0xE8`.
- Replacement target: `CInput::RenderLoginInputTextHook`.
- Required patch length: 5 bytes.
- Overwritten instruction: `E8 33 D9 F5 FF` (`CALL 0x0047F0B0`).
- Resume VA: `0x0052177D`.
- Trampoline requirements: none for the callsite; the wrapper must preserve the
  call ABI.
- Patch owner: `CInput::Init`.
- Original values or bytes: `E8 33 D9 F5 FF`.
- Preflight verification: absent in the current `SetCompleteHook`; future
  changes must compare the expected bytes before writing.

## Signature

- Original bytes: `E8 33 D9 F5 FF 6A 00 6A 00 8D 7E 48 6A 00 68 A8`
- Byte signature: `E8 ?? ?? ?? ?? 6A 00 6A 00 8D 7E 48 6A 00 68 A8`
- Wildcard rationale: the `CALL rel32` displacement is address-dependent.
- Expected match count: not measured; require exactly one match before using the
  signature for relocation or rediscovery.

## Evidence

- Ghidra program: `main.exe` matching the recorded fingerprint.
- Ghidra function or symbol: containing function name not persisted.
- Decompiler observations: not relied upon for this record.
- Disassembly evidence: exact `CALL rel32` bytes resolve to `0x0047F0B0` and
  execution resumes at `0x0052177D`.
- Cross-reference evidence: `CInput::Init` installs this one callsite and
  `Offsets.h` centralizes the address.
- Debugger evidence: not recorded.
- Contradictory evidence considered: no conflicting patch owner found; runtime
  ABI verification remains absent.

## Assessment

- Confidence: Medium.
- Verified facts: fingerprint, VA/RVA, original bytes, target calculation,
  resume address, source owner, and replacement target.
- Inferences: the surrounding native stack and registers satisfy the wrapper's
  current parameters.
- Remaining uncertainty: persisted Ghidra function identity, measured signature
  uniqueness, and runtime ABI validation.
- Revalidation procedure: confirm fingerprint, exact bytes, containing function,
  surrounding argument setup, and unique signature match in Ghidra before
  changing the hook.
