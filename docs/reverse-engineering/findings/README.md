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

- Patch primitive: typed write/block write/`CALL` callsite/complete `JMP`/naked
  interceptor/direct control-flow patch
- Hook type:
- Required patch length:
- Overwritten instructions:
- Resume VA:
- Trampoline requirements:
- Patch owner:
- Original values or bytes:
- Preflight verification:

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
- A patch site must have one owner. Search the project before recording a new
  hook or memory write at an existing address.
- For typed or block writes, record width, original value/bytes, replacement
  value/bytes, and whether the target is code or data.
- For naked interceptors, record every reproduced instruction and continuation;
  `Pushad/Popad` does not by itself preserve flags, FPU, or SIMD state.
