# Ghidra MCP tool selection

Read this file only when the required capability is not already available, more
than one bridge instance exists, or the installed catalog differs from the
validated baseline.

## Baseline

- `bethington/ghidra-mcp` `v6.0.0` with Ghidra `12.1.2`.
- Core groups normally loaded: `listing`, `function`, and `program`.
- Program-aware calls require an explicit program selector.

## Discovery sequence

1. `list_instances`, then `connect_instance` to the intended bridge.
2. `check_tools` for a known capability.
3. `search_tools` using narrow action and subject terms.
4. `list_tool_groups` only when search is insufficient.
5. `load_tool_group` only for the current task, then inspect the selected schema.

Do not preserve generated non-management tool names as permanent instructions.

## Capability hints

| Need | Start with |
|---|---|
| Program metadata or memory map | `program` |
| Bytes, instructions, or data ranges | `listing` |
| Functions, callers, callees, prototypes | `function` |
| Strings, constants, imports, references, or patterns | `search_tools` with the specific subject |
| Runtime state | debugger capabilities only after explicit approval |

Prefer bounded reads and narrow queries over whole-program exports or broad group
loading.

## Mutation boundary

Capability discovery does not authorize writes. Renaming, retyping, patching,
script execution, emulation, or debugger control requires explicit task scope.
Use `dry_run` when available and report any approved mutation separately from the
analysis finding.
