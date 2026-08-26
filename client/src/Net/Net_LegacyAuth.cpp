// Net_LegacyAuth.cpp
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


// FUN_004e3d60 @ 0x004E3D60 — Connection_Check(ctx, cols, rows)
// Scans a 2D grid of short-based structs (stride 0x22 each element).
// Returns 1 (clear) if every entry is -1 or has ≤0 at offset+0x1c.
// Returns 0 (busy) if any entry has ≠-1 AND int at +0x1c > 0.
char __cdecl FUN_004e3d60(void *ctx, int p1, int p2) {
    char uVar1 = 1;
    short *psVar2 = (short*)ctx;
    for (int row = 0; row < p2; row++) {
        short *cur = psVar2;
        for (int col = 0; col < p1; col++) {
            if (*cur != -1 && *(int*)(cur + 0x1c) > 0)
                uVar1 = 0;
            cur += 0x22;
        }
        psVar2 += p1 * 0x22;
    }
    return uVar1;
}

// FUN_00494520 @ 0x00494520 — Auth_XorEncode(key_ctx, buf, flag)
// __thiscall in original (this=key_ctx). XOR-encodes buf[] using a key schedule derived
// from this->key_table (256-byte S-box from RC4 variant). flag=0: encode only; flag!=0: also validates.
// Uses SEH frame + 256-byte S-box + HashTable obfuscation. 3500+ lines in binary.
// STUB: SEH + unaff_ESI pattern prevent safe implementation.
unsigned long __cdecl FUN_00494520(void*, unsigned char*, char) { return 0; } // STUB: Auth_XorEncode — SEH+RC4

// FUN_00513440 @ 0x00513440 — Chat_Validate(buf)
// Returns 1 if buf (after stripping spaces) matches any word in the
// banned-keyword table at DAT_07d73104 (stride 0x14, count DAT_07d78070).
// Returns 0 if buf starts with '/' or no match found.
char __cdecl FUN_00513440(char *param_1) {
    if (*param_1 == '/') return 0;
    // strip spaces into local buf
    int len = (int)strlen(param_1);
    char local_100[256];
    int out = 0;
    for (int i = 0; i < len; i++) {
        if (param_1[i] != ' ') local_100[out++] = param_1[i];
    }
    local_100[out] = '\0';
    // search banned word table
    int count = (int)DAT_07d78070;
    char *entry = DAT_07d73104;
    for (int i = 0; i < count; i++) {
        if (FUN_004977f0(local_100, entry, '\0')) return 1;
        entry += 0x14;
    }
    return 0;
}
// FUN_00497c70 @ 0x00497C70 — Auth_KeySchedule(void)
// Initializes the RC4-variant S-box used by FUN_00494520.
// Reads seed data from DAT_07cf1ffc (CharData), permutes 256-byte key table.
// Called once during login handshake. Many unreachable blocks (dead code).
// NOP confirmed safe — key schedule runs in original binary only; stub here has no effect.
void __cdecl FUN_00497c70(void) {} // NOP — Auth_KeySchedule (no external side effects needed)
// FUN_004e9250 @ 0x004E9250 — SecondPassword_Shuffle(mode)
// Initializes a 10-element short array at DAT_07e91394 with values 0..9,
// then performs 20 random XOR swaps. Stores mode in DAT_07eaa14c.
// Returns last randomly-computed iVar4 (ignore value — callers discard).
int __cdecl FUN_004e9250(int mode) {
    // Fill 0..9
    short* arr = (short*)&DAT_07e91394;
    for (int i = 0; i < 10; i++) arr[i] = (short)i;
    // 20 random swaps
    int iVar4 = 0;
    for (int k = 0; k < 20; k++) {
        int a = rand() % 10;
        int b = rand() % 10;
        iVar4 = b / 10; // mirrors original (iVar3 = rand() % 10; iVar4 = iVar3/10)
        if (a != b) {
            arr[a] ^= arr[b];
            arr[b] ^= arr[a];
            arr[a] ^= arr[b];
        }
    }
    _DAT_07ea9814 = 0.0f;
    DAT_07ea9818  = 0;
    DAT_07eaa14c  = (DWORD)mode;
    DAT_07ea981c  = 0;
    DAT_07ea981e  = 0;
    return iVar4;
}
