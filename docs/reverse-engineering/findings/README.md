# Reverse-engineering findings

Create one Markdown file per verified function, global, structure, packet handler, or hook site. Use kebab-case names, for example `render-item.md` or `main-window-handle.md`.

Add each accepted finding to the index in `../main-exe.md`.

## Finding template

```md
# <Symbol or behavior>

## Target

- Executable SHA-256:
- File size:
- PE timestamp:
- Preferred image base:
- VA:
- RVA:
- File offset, if needed:
- Function or data size:

## Identification

- Requested behavior:
- Search strategy:
- Strings/imports/constants:
- Callers:
- Callees:
- Data references:
- Related source files:

## ABI or layout

- Kind: function/global/structure/hook site
- Calling convention:
- Return type:
- Parameters:
- Register use:
- Stack cleanup:
- Preserved registers:
- Ownership/lifetime:
- Packing/offsets:

## Hook details, when applicable

- Hook type:
- Required patch length:
- Overwritten instructions:
- Resume VA:
- Trampoline requirements:

## Signature

- Original bytes:
- Byte signature:
- Wildcard rationale:
- Expected match count:

## Evidence

- Ghidra program:
- Ghidra function or symbol:
- Decompiler observations:
- Disassembly evidence:
- Cross-reference evidence:
- Debugger evidence:
- Contradictory evidence considered:

## Assessment

- Confidence: High/Medium/Low/Rejected
- Verified facts:
- Inferences:
- Remaining uncertainty:
- Revalidation procedure:
```

## Recording rules

- Separate verified facts from inferences.
- Include enough evidence for another analyst to reproduce the result.
- Do not paste large decompiler listings; quote only the relevant instructions or pseudocode fragments.
- A byte signature must explain wildcards and expected uniqueness.
- A Low-confidence candidate must not be presented as an established offset.
