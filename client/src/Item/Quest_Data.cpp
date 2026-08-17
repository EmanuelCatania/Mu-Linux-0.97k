#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_00401040 @ 0x00401040 — Quest_LoadBMD(this, path) ────────────────────
// __thiscall: reads binary .bmd quest data into a quest manager object.
// Parameters:
//   this  = pointer to quest manager object
//   path  = file path (e.g. "Data/Local/Quest.bmd")
// Returns: 1 on success (high 24 bits from fclose result, low byte = 1),
//          0 on failure (file not found).
//
// Layout:
//   this+8 onward = 0x7210 DWORDs (zeroed first), then filled with quest data.
//   File contains 200 records × 0x248 bytes each.
//   Each record is XOR-decrypted by FUN_00401120(buf, 0x248) before copy.
//   Copy: 0x92 DWORDs (0x248 bytes) per record into this+8 (sequential).
uint __cdecl FUN_00401040(int handle, const char *path)
{
    CHAR msg[256];
    FILE *fp = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!fp) {
        crt_sprintf(msg, (const char *)s__s___File_not_exist__00558094);
        // Note: original doesn't show a MessageBox here, just crt_sprintf
        return 0;
    }
    // zero out quest array: this+8, 0x7210 DWORDs
    DWORD *arr = (DWORD *)(handle + 8);
    for (int i = 0x7210; i != 0; i--)
        *arr++ = 0;

    char *buf = (char *)operator_new(0x248);
    int   cnt = 200;
    arr = (DWORD *)(handle + 8);
    do {
        FUN_00541597(buf, 0x248, 1, (int *)fp);
        FUN_00401120((int)buf, 0x248);
        const char *src = buf;
        DWORD      *dst = arr;
        for (int j = 0x92; j != 0; j--) {
            *dst++ = *(DWORD *)src;
            src += 4;
        }
        arr += 0x92;
        cnt--;
    } while (cnt != 0);
    operator_delete(buf);
    FUN_0054150f(fp);
    // Ghidra shows: return CONCAT31((int3)((uint)fclose_result >> 8), 1)
    // i.e. success = low byte 1, upper bytes from fclose. Simplified:
    return 1;
}
