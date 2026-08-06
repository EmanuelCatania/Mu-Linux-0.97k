# Login panel vertical global

## Target

- Executable SHA-256: `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F`
- File size: `1486848`
- PE timestamp: `0x3FBC1903`
- Preferred image base: `0x00400000`
- VA: `0x005616A4`
- RVA: `0x001616A4`
- Function or data size: 4-byte signed integer

## Identification

- Requested behavior: native login panel vertical position used by custom login
  UI alignment.
- Search strategy: source macro, raw value, and current custom consumers.
- Strings/imports/constants: value `480`.
- Callers: not applicable to a global.
- Callees: not applicable.
- Data references: native readers and writers have not yet been persisted from
  Ghidra.
- Related source files: `src/client/Main/Offsets.h`,
  `src/client/Main/LoginCredentials.cpp`.

## ABI or layout

- Kind: global `int`.
- Value: `0x000001E0` (`480`).
- Width and signedness: 4-byte signed integer in current source usage.
- Ownership/lifetime: native login layout is presumed to own the value; custom
  UI code reads it.

## Signature

- Original bytes: `E0 01 00 00`
- Byte signature: none. The value `480` alone is not a unique signature.
- Wildcard rationale: not applicable until native data-reference context is
  recorded.
- Expected match count: not measured.

## Evidence

- Ghidra program: `main.exe` matching the recorded fingerprint.
- Ghidra function or symbol: not persisted.
- Decompiler observations: not recorded.
- Disassembly evidence: only the four-byte value at the recorded VA is known.
- Cross-reference evidence: custom source reads the macro; native references
  have not been documented.
- Debugger evidence: not recorded.
- Contradictory evidence considered: the same value can occur throughout the
  executable and does not establish semantic identity.

## Assessment

- Confidence: Low.
- Verified facts: fingerprint, VA/RVA, raw value, width used by current custom
  source, and custom consumers.
- Inferences: this address is the authoritative native login panel Y coordinate.
- Remaining uncertainty: native readers, native writers, semantic ownership,
  mutability, and runtime behavior.
- Revalidation procedure: identify all native data references in Ghidra, name
  the owning functions, confirm read/write behavior during the login scene, and
  observe the value at runtime before accepting it as a permanent global.
