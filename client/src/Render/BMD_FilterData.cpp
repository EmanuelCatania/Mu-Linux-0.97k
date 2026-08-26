// BMD_FilterData.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// ── Filter BMD helpers ────────────────────────────────────────────────────────
// Checksum is computed by walking the encrypted blob 4 bytes at a time, XOR/ADD
// alternating on a seed pointer, with a rotate-mix every 16th iteration.
// FUN_00479a50 @ 0x00479A50 — Filter_SaveBMD
// Copies 1000 word-filter entries (stride 0x14) from DAT_07d73104 into a
// 20000-byte heap buffer, BuxConvert_0-encrypts each entry, computes the
// checksum starting from &DAT_007cfa00, then writes buffer + checksum to file.
void __cdecl FUN_00479a50(const char* path)
{
    // Port of IDA sub_479A50 (Filter_SaveBMD).  Not called in the .bmd branch
    // (DAT_0055a7c4==1), only in the txt-source branch.  Kept for parity.
    FILE* pFVar1 = fopen(path, "wb");
    if (!pFVar1) return;
    BYTE* lpMem  = (BYTE*)operator_new(20000);
    char* src = DAT_07d73104;
    BYTE* dst = lpMem;
    char* srcEnd = DAT_07d73104 + sizeof(DAT_07d73104);
    while (src < srcEnd) {
        memcpy(dst, src, 0x14);
        FUN_00479910((int)dst, 0x14);
        src += 0x14;
        dst += 0x14;
    }
    uintptr_t acc = (uintptr_t)DAT_007cfa00;
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(lpMem + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 15997) >> (((i >> 2) & 7) + 1);
    }
    fwrite(lpMem, 20000, 1, pFVar1);
    fwrite(&acc,      4, 1, pFVar1);
    operator_delete(lpMem);
    fclose(pFVar1);
}

// FUN_00479b30 @ 0x00479B30 — Filter_LoadBMD (OpenFilterFile)
// Port of IDA sub_479B30 (raw/00479B30_OpenFilterFile.c).
// Reads 20000-byte blob + 4-byte checksum, validates ring checksum
// (seed 0x7cfa00, magic 15997), then BuxConvert_0-decrypts each 20-byte
// entry into DAT_07d73104[]; stops on empty-first-byte sentinel or when
// the 20000-byte target buffer is full; writes count into DAT_07d78070.
void __cdecl FUN_00479b30(const char* path)
{
    char local_100[256];
    FILE* Stream = fopen(path, "rb");
    if (!Stream) {
        sprintf(local_100, "%s - File not exist.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        return;
    }
    BYTE* Buffer = (BYTE*)operator_new(20000);
    uintptr_t Checksum = 0;
    fread(Buffer,     20000, 1, Stream);
    fread(&Checksum,      4, 1, Stream);
    fclose(Stream);
    // Validate ring checksum — IDA-exact.
    uintptr_t acc = (uintptr_t)DAT_007cfa00;  // 0x007cfa00 (literal seed)
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(Buffer + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;   // IDA: "if(k!=0){ if(k==1) add; }"
        if ((i & 0xf) == 0)
            acc ^= (acc + 15997) >> (((i >> 2) & 7) + 1);
    }
    if (Checksum != acc) {
        sprintf(local_100, "%s - File corrupted.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        operator_delete(Buffer);
        return;
    }
    // Decrypt and copy entries until sentinel (empty first byte) or buffer full.
    int count = 0;
    BYTE* src = Buffer;
    char* dst = DAT_07d73104;
    char* end = DAT_07d73104 + sizeof(DAT_07d73104);
    while (dst < end) {
        FUN_00479910((int)src, 0x14);
        memcpy(dst, src, 0x14);
        if (*dst == '\0') break;
        src   += 0x14;
        dst   += 0x14;
        count += 1;
    }
    if (dst < end) DAT_07d78070 = count;
    operator_delete(Buffer);
}

// FUN_00479cf0 @ 0x00479CF0 — FilterName_LoadData
// Reads word tokens from a text file via GetToken(); stores each into
// DAT_07d27610 (stride 0x14), incrementing DAT_07d78074 per token.
void __cdecl FUN_00479cf0(const char* path)
{
    DAT_07d7806c = (FILE*)fopen(path, DAT_005580ac);
    if (!DAT_07d7806c) return;
    int iVar2;
    while ((iVar2 = ParseNextToken()) != 2) {
        char* dst = DAT_07d27610 + DAT_07d78074 * 0x14;
        int   len = (int)strlen(ParserTokenString) + 1;  // TokenString
        memcpy(dst, ParserTokenString, len);
        DAT_07d78074++;
    }
    fclose(DAT_07d7806c);
}

// FUN_00479d70 @ 0x00479D70 — FilterName_SaveBMD
// Mirror of FUN_00479a50 for the name filter (DAT_07d27610, seed DAT_00578200,
// magic 0x2bc1).
void __cdecl FUN_00479d70(const char* path)
{
    // Port of IDA sub_479D70 (FilterName_SaveBMD).  Not called in the .bmd
    // branch (DAT_0055a7c4==1), only in the txt-source branch.  Kept for parity.
    FILE* pFVar1 = fopen(path, "wb");
    if (!pFVar1) return;
    BYTE* lpMem  = (BYTE*)operator_new(20000);
    char* src = DAT_07d27610;
    BYTE* dst = lpMem;
    char* srcEnd = DAT_07d27610 + sizeof(DAT_07d27610);
    while (src < srcEnd) {
        memcpy(dst, src, 0x14);
        FUN_00479910((int)dst, 0x14);
        src += 0x14;
        dst += 0x14;
    }
    uintptr_t acc = (uintptr_t)DAT_00578200;
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(lpMem + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 11201) >> (((i >> 2) & 7) + 1);
    }
    fwrite(lpMem, 20000, 1, pFVar1);
    fwrite(&acc,      4, 1, pFVar1);
    operator_delete(lpMem);
    fclose(pFVar1);
}

// FUN_00479e50 @ 0x00479E50 — FilterName_LoadBMD (OpenNameFilterFile)
// Port of IDA sub_479E50 (raw/00479E50_OpenNameFilterFile.c).
// Mirror of FUN_00479b30 for the name filter (seed 0x578200, magic 11201,
// target DAT_07d27610[], count in DAT_07d78074).
void __cdecl FUN_00479e50(const char* path)
{
    char local_100[256];
    FILE* Stream = fopen(path, "rb");
    if (!Stream) {
        sprintf(local_100, "%s - File not exist.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        return;
    }
    BYTE* Buffer = (BYTE*)operator_new(20000);
    uintptr_t Checksum = 0;
    fread(Buffer,     20000, 1, Stream);
    fread(&Checksum,      4, 1, Stream);
    fclose(Stream);
    uintptr_t acc = (uintptr_t)DAT_00578200;  // 0x00578200 (literal seed)
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(Buffer + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 11201) >> (((i >> 2) & 7) + 1);
    }
    if (Checksum != acc) {
        sprintf(local_100, "%s - File corrupted.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        operator_delete(Buffer);
        return;
    }
    int count = 0;
    BYTE* src = Buffer;
    char* dst = DAT_07d27610;
    char* end = DAT_07d27610 + sizeof(DAT_07d27610);
    while (dst < end) {
        FUN_00479910((int)src, 0x14);
        memcpy(dst, src, 0x14);
        if (*dst == '\0') break;
        src   += 0x14;
        dst   += 0x14;
        count += 1;
    }
    if (dst < end) DAT_07d78074 = count;
    operator_delete(Buffer);
}
// FUN_0047b020 — implemented in src/Item/Dialog_Data.cpp  (Dialog_LoadBMD)
// FUN_00401040 — implemented in src/Item/Quest_Data.cpp   (Quest_LoadBMD)
// FUN_0047d120 — implemented in src/Item/NPC_Data.cpp     (NPCName_Load)
