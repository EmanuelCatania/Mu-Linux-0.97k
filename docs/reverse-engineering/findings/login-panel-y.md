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

- Requested behavior: native login panel vertical position used by custom login UI alignment.
- Search strategy: source `LoginPanelY` definition, raw data, and login layout references.
- Related source files: `src/client/Main/Offsets.h`, `src/client/Main/LoginCredentials.cpp`.

## ABI or layout

- Kind: global `DWORD`/`int` value.
- Value: `0x000001E0` (`480`).
- Ownership/lifetime: native login layout and client UI extensions read the shared value.

## Signature

- Original bytes: `E0 01 00 00`
- Expected match count: one at the supported executable VA.

## Evidence

- Evidence: matching source macro, raw data bytes, and login layout use.
- Verified facts: the supported executable stores `480` at this address.

## Assessment

- Confidence: Medium.
- Revalidation procedure: confirm the data reference and value usage in Ghidra before changing layout behavior.
