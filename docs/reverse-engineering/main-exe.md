# Supported main.exe

This file identifies the exact closed-source executable supported by the current
client hooks. Do not fill fields from memory or from another MU Online
distribution.

## Executable fingerprint

| Property | Value |
|---|---|
| Repository path | `runtime/client/main.exe` |
| Product/version | MU Online 0.97k |
| Architecture | PE32 / x86 |
| SHA-256 | `A888AF27FCAC53DB177E377DFC98D163277B26490E488A4D458EE2BA3DD1782F` |
| File size | `1486848` bytes |
| Git blob | `edb44cd4f756d81af056cd577e6421702e8e0d65` |
| PE timestamp | `0x3FBC1903` (`2003-11-20 01:29:39 UTC`) |
| Preferred image base | `0x00400000` |
| Entry point RVA | `0x00143555` |
| Ghidra language | `x86:LE:32:default` |
| Analysis date | `2026-08-06` |
| Ghidra version | `12.1.2` |
| Ghidra MCP implementation | `bethington/ghidra-mcp` |
| Ghidra MCP release | `v6.0.0` |

## Verify the fingerprint

Compare the repository and runtime executables:

```powershell
pwsh -File ./.agents/skills/ghidra-offset-analysis/scripts/fingerprint-main.ps1 `
  -Path ./runtime/client/main.exe `
  -ComparePath C:\Dev\runtime\mu-097k\client\main.exe
```

Proceed only when the fingerprint matches this document and `MatchesPrimary` is
`True`. Use `-AsJson` for automated consumption.

## Verification rules

- Calculate SHA-256 from the exact `main.exe` being analyzed and executed.
- Stop analysis when the fingerprint does not match this record.
- Record a new executable as a separate supported target; do not silently
  replace this fingerprint.
- Revalidate every hard-coded address, signature, and hook when the executable
  changes.
- Do not commit another executable solely to support this record.

## Accepted findings

Only High- or Medium-confidence records belong in this table.
`Medium — source-backed` identifies an existing source-derived record whose
fingerprint, bytes, owner, target, and continuation are known, while persisted
Ghidra identity, signature uniqueness, or runtime confirmation remains
incomplete.

| Symbol or behavior | VA | RVA | Kind | Confidence | Record |
|---|---:|---:|---|---|---|
| Login account render callsite | `0x00521778` | `0x00121778` | `CALL` | Medium — source-backed | [login-account-render-call.md](findings/login-account-render-call.md) |
| Login password render callsite | `0x005217A0` | `0x001217A0` | `CALL` | Medium — source-backed | [login-password-render-call.md](findings/login-password-render-call.md) |
| Login scene render callsite | `0x0052698A` | `0x0012698A` | `CALL` | Medium — source-backed | [login-scene-render-call.md](findings/login-scene-render-call.md) |

## Candidate findings

Low-confidence candidates document current source assumptions but must be
revalidated before modification or reuse.

| Symbol or behavior | VA | RVA | Kind | Confidence | Record |
|---|---:|---:|---|---|---|
| Reconnect account capture site | `0x00520428` | `0x00120428` | `JMP` / `naked` | Low | [reconnect-account-capture.md](findings/reconnect-account-capture.md) |
| Login panel vertical global | `0x005616A4` | `0x001616A4` | `DWORD` | Low | [login-panel-y.md](findings/login-panel-y.md) |
