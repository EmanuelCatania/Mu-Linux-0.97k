# Supported main.exe

This file identifies the exact closed-source executable supported by the current client hooks. Do not fill fields from memory or from another MU Online distribution.

## Executable fingerprint

| Property | Value |
|---|---|
| Repository path | `runtime/client/main.exe` |
| Product/version | MU Online 0.97k |
| Architecture | PE32 / x86 |
| SHA-256 | `TBD` |
| File size | `1486848` bytes |
| Git blob | `edb44cd4f756d81af056cd577e6421702e8e0d65` |
| PE timestamp | `TBD` |
| Preferred image base | `TBD` |
| Entry point RVA | `TBD` |
| Ghidra language | `x86:LE:32:default` |
| Analysis date | `TBD` |
| Ghidra version | `12.1.2` |
| Ghidra MCP implementation | `bethington/ghidra-mcp` |
| Ghidra MCP release | `v6.0.0` |

## Populate or verify the fingerprint

Run against the same file imported into Ghidra and copied to the external runtime:

```powershell
$MainExe = (Resolve-Path .\runtime\client\main.exe).Path
Get-FileHash -Algorithm SHA256 $MainExe
(Get-Item $MainExe).Length
dumpbin /headers $MainExe
```

Use `dumpbin` or Ghidra program information to record the PE timestamp, preferred image base, and entry-point RVA. Verify the external runtime copy separately:

```powershell
Get-FileHash -Algorithm SHA256 C:\Dev\runtime\mu-097k\client\main.exe
```

Do not accept a permanent offset until both hashes match this record.

## Verification rules

- Calculate SHA-256 from the exact `main.exe` being analyzed and executed.
- Stop analysis when the fingerprint does not match this record.
- Record a new executable as a separate supported target; do not silently replace this fingerprint.
- Revalidate every hard-coded address, signature, and hook when the executable changes.
- Do not commit another executable solely to support this record.

## Findings index

| Symbol or behavior | VA | RVA | Kind | Confidence | Record |
|---|---:|---:|---|---|---|
| _Add verified findings here_ |  |  |  |  |  |
