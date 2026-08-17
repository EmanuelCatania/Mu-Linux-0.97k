#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0047b020 @ 0x0047B020 — Dialog_LoadBMD(path) ────────────────────────
// Reads binary .bmd dialog data file into DAT_07cf5608.
// Allocates 0x400-byte scratch buffer, reads and decrypts records in blocks:
//   - Each iteration: fread 0x400 bytes, XOR-decrypt via FUN_00479910,
//     then copy 0x100 DWORD entries into the dialog table.
// Loop runs while the write pointer < 0x7d27608 (upper bound of dialog array).
// Starting base: DAT_07cf5608 (dialog data array; 4 bytes per entry × 0x100
//   entries per block, multiple blocks).
void __cdecl FUN_0047b020(const char *path)
{
    CHAR msg[256];
    FILE *fp = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!fp) {
        crt_sprintf(msg, (const char *)s__s___File_not_exist__00558094);
        FUN_00405540(&DAT_055c9bf0, msg);
        MessageBoxA(DAT_055c9ffc, msg, nullptr, 0);
        SendMessageA(DAT_055c9ffc, 2, 0, 0);
        return;
    }
    char *buf  = (char *)operator_new(0x400);
    DWORD *dst = DAT_07cf5608;
    DWORD *end = DAT_07cf5608 + (0x32000 / 4);   // 200KB array end (era literal 0x7d27608 en binario original)
    do {
        FUN_00541597(buf, 0x400, 1, (int *)fp);
        FUN_00479910((int)buf, 0x400);
        DWORD *next = dst + 0x100;
        const char *src = buf;
        for (int i = 0x100; i != 0; i--) {
            *dst++ = *(DWORD *)src;
            src += 4;
        }
        dst = next;
    } while (dst < end);
    operator_delete(buf);
    FUN_0054150f(fp);
}
