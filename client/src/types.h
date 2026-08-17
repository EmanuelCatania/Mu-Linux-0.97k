#pragma once
// types.h — Primitive type aliases for the Mu Online 0.97k client reconstruction.
// Names follow the 5.2 source conventions (vec3_t, BYTE, etc.) where possible.
// The Ghidra decompiler pseudo-types are mapped to Win32/standard equivalents.
// Windows.h is assumed to already be included via stdafx.h.

// ── Ghidra pseudo-types ───────────────────────────────────────────────────────
typedef unsigned char   undefined;    // generic 1-byte unknown (bare Ghidra type)
typedef unsigned char   undefined1;   // 1-byte unknown
typedef unsigned short  undefined2;   // 2-byte unknown
typedef unsigned int    undefined4;   // 4-byte unknown
typedef unsigned __int64 undefined8;  // 8-byte unknown

typedef long double     float10;      // x87 80-bit extended precision (fsin etc.)
typedef long long       longlong;
typedef unsigned long long ulonglong;
typedef unsigned int    uint3;        // 3-byte uint (rare Ghidra artifact)
typedef unsigned int    uint;         // shorthand used by Ghidra decompile output
typedef unsigned short  ushort;       // shorthand used by Ghidra decompile output

// Function TYPE (not pointer type) — matches Ghidra semantics.
// Callers use `code*` for a function pointer; `**(code**)X` performs ONE
// memory read (at X, yielding a function pointer) and then calls it.
// Defining `code` as a pointer type would double the indirection and crash.
typedef int code(...);

// ── CONCAT macros (Ghidra byte-concatenation helpers) ─────────────────────────
// CONCAT<hi_bytes><lo_bytes>(hi, lo)
#define CONCAT11(a,b)  ((unsigned short)(((unsigned char)(a) << 8) | (unsigned char)(b)))
#define CONCAT12(a,b)  ((unsigned int)(((unsigned char)(a) << 16) | (unsigned short)(b)))
#define CONCAT13(a,b)  ((unsigned int)(((unsigned char)(a) << 24) | ((unsigned int)(b) & 0x00ffffff)))
#define CONCAT21(a,b)  ((unsigned int)(((unsigned short)(a) << 8) | (unsigned char)(b)))
#define CONCAT22(a,b)  ((unsigned int)(((unsigned short)(a) << 16) | (unsigned short)(b)))
#define CONCAT31(a,b)  ((unsigned int)(((unsigned int)(a) << 8) | (unsigned char)(b)))
#define CONCAT41(a,b)  ((unsigned __int64)(((unsigned int)(a) << 8) | (unsigned char)(b)))
#define CONCAT44(a,b)  ((__int64)(((unsigned __int64)(unsigned int)(a) << 32) | (unsigned int)(b)))

// int3: Ghidra 3-byte int — treated as 32-bit here (x86 has no 3-byte registers)
typedef unsigned int int3;

// ── 5.2-compatible vector types ───────────────────────────────────────────────
typedef float  vec_t;
typedef vec_t  vec2_t[2];
typedef vec_t  vec3_t[3];
typedef vec_t  vec4_t[4];
typedef vec_t  vec34_t[3][4];
