// ghidra_compat.h
//
// Macros que el decompile de Ghidra / Hex-Rays da por existentes y que el port
// necesita para compilar sus salidas tal cual.
//
// Antes cada archivo las redefinía por su cuenta con su propio `#ifndef`: había
// 15 copias de `qmemcpy`, 14 de `delete__` y 11-12 de cada accesor de palabra,
// repartidas en 17 archivos. Eso rompía el refactor B3: al mover una función a
// otro módulo dejaba de ver las macros de su archivo de origen.
//
// Se incluye desde `stdafx.h`, así que está disponible en todo el proyecto.
// Los `#ifndef` se conservan para no chocar con los `#define` locales que aún
// queden en los .cpp mientras se van limpiando.

#pragma once

// ── Accesores de palabra/byte dentro de un valor ─────────────────────────────
// Hex-Rays los emite para leer partes de un registro. Son lvalues: se usan
// tanto para leer como para escribir (p. ej. `LOBYTE(v) = 3;`).
#ifndef LODWORD
#define LODWORD(x)  (*((DWORD*)&(x)))
#define HIDWORD(x)  (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)  (*((char*)&(x)))
#define SLOWORD(x)  (*((short*)&(x)))
#define SLODWORD(x) (*((int*)&(x)))
#endif

#ifndef LOBYTE
#define LOBYTE(x)   (*((unsigned char*)&(x)))
#define HIBYTE(x)   (*(((unsigned char*)&(x))+1))
#define LOWORD(x)   (*((unsigned short*)&(x)))
#define HIWORD(x)   (*(((unsigned short*)&(x))+1))
#endif

// ── Helpers del decompile ────────────────────────────────────────────────────
#ifndef qmemcpy
#define qmemcpy(dst, src, sz)  memcpy((dst), (src), (size_t)(sz))
#endif

// `delete__` es el `operator delete` del binario original (0x0054158C).
#ifndef delete__
extern void __cdecl FUN_0054158c(void* ptr);
#define delete__(p)  FUN_0054158c((unsigned char*)(p))
#endif
