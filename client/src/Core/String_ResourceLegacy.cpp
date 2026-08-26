// String_ResourceLegacy.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// CRT / string helpers

// FUN_00541eab @ 0x00541EAB — IsLeadByte(str)
// DBCS lead-byte check using CRT _pctype table at DAT_083bc1a0.
// Bit 2 of table[*str+1] set → double-byte (return 2), else single-byte (return 1).
// Implemented via Win32 IsDBCSLeadByteEx(949) to avoid needing the 256-byte table.
int __cdecl FUN_00541eab(byte *param_1) {
    return IsDBCSLeadByteEx(949, *param_1) ? 2 : 1;
}
// FUN_0053d5a0 @ 0x0053D5A0 — Resource_Load(filename)
// Calls FUN_0053ed30(DAT_083bbb14, filename) if resource manager is initialized.
// Returns non-zero on success. DAT_083bbb14 is the resource manager context pointer.
// FUN_0053ed30 not implemented — returning 0 (no-op stub).
unsigned int  __cdecl FUN_0053d5a0(char *path)
{
    if (DAT_083bbb14 == 0) return 0;
    FUN_0053ed30((void *)(ULONG_PTR)DAT_083bbb14, path);
    return 1;
}
// FUN_0053d5c0 @ 0x0053D5C0 — Pipe_QueryResource
unsigned int  __cdecl FUN_0053d5c0(char *path)
{
    if (DAT_083bbb14 == 0) return 0;
    return FUN_0053ed00((void *)(ULONG_PTR)DAT_083bbb14, path);
}
