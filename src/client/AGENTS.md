# Client - Agent Instructions

These rules extend the root `AGENTS.md` for `src/client/`.

- The client must remain MSVC Win32/x86. Do not remove or bypass this restriction.
- `Main/` builds `Main.dll`; `InfoEncoder/` builds `InfoEncoder.exe` and generates `ClientInfo.bmd`.
- CMake/Ninja is the primary build; MSBuild is the supported fallback and must not drift.
- Addresses, signatures, calling conventions, registers, sizes, and layouts must be verified against the supported executable.
- Use Ghidra with `bethington/ghidra-mcp` as the primary workflow for locating and validating offsets, functions, signatures, and cross-references in `main.exe`.
- Do not copy offsets or prototypes from another version, region, Season, or packed client build.
- Packet, encoded-structure, item, model, or configuration changes must consider the server and runtime.
- Do not edit `ClientInfo.bmd` manually; generate it through the encoder.
- Preserve PE resources, output names, packing, and existing precompiled-header exceptions.

## Read as needed

- Offset discovery: `../../.agents/skills/ghidra-offset-analysis/SKILL.md`.
- Hook implementation: `../../.agents/skills/client-hook-change/SKILL.md`.
- Packet changes: `../../.agents/skills/protocol-change/SKILL.md`.
- Custom entities: `../../.agents/skills/runtime-entity-change/SKILL.md`.
- Binary analysis: `../../docs/client-reverse-engineering.md`.
- Binary patching: `../../docs/client-patching.md`.
- Encoder and data: `../../docs/runtime-data.md`.
- General patterns: `../../docs/coding-patterns.md`.
- Validation: `../../docs/testing.md`.

## Quick start

```powershell
pwsh -File ./scripts/client-workflow.ps1 -Action BuildDeploy -Configuration Debug
```

Run `-Action Encode` when changing encoder inputs or behavior. Use Release and MSBuild when required by `docs/testing.md`.
