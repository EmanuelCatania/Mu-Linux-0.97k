#pragma once

// MSVC conserva sus palabras clave. GCC/Clang x86 usan los atributos ABI
// equivalentes; no borrar convenciones en callbacks ni punteros a funciones.
#if defined(__GNUC__) && !defined(_MSC_VER)
#if !defined(__i386__)
#error "El cliente requiere GCC/Clang de 32 bits (-m32)."
#endif
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif
#ifndef __cdecl
#define __cdecl __attribute__((cdecl))
#endif
#ifndef __stdcall
#define __stdcall __attribute__((stdcall))
#endif
#ifndef __fastcall
#define __fastcall __attribute__((fastcall))
#endif
#ifndef __thiscall
#define __thiscall __attribute__((thiscall))
#endif
#endif
