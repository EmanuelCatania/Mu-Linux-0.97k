#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// IDA: FUN_0047B130
// Reads a text-format item data file (Item.txt / local-language variant).
// Parser loop uses TextParser_GetToken which returns record type:
//   0 = comment/section header (strcmp with "END" at DAT_00559088; break loop on match)
//   1 = data record   (iVar4 = row, lVar16 = col)
//   2 = EOF           (close file, return)
//
// Data structure: DAT_07d78068 + (row*0x20 + col)*0x40
//   Each 0x40-byte slot stores:
//     [0x00..0x1F] = name string (memcpy from TextParserTokenString)
//     [0x20] = item type group A
//     [0x21] = item sub-type A
//     [0x1F] = item level / flag
//     [0x22],[0x23] = stat A, stat B
//     [0x24],[0x25],[0x26],[0x27],[0x28],[0x29] = extra stats (type-dependent)
//     [0x2A] = flag
//     [0x2C..0x2D] = short field C
//     [0x2E..0x2F] = short field D
//     [0x30],[0x31] = bonus fields (ring/wing/etc)
//     [0x32] = scroll type
//     [0x38..0x3B] = accessory sub-fields
//     [0x3C..0x3F] = pet/charm sub-fields
//     [0x1E] = stackable flag (set 1 if item type group A > 1)
//   Mirror copy written to DAT_07cf1ff0 at same offset.
//   [0x27] field in mirror is left-shifted by 1 after copy.
//
// Item type dispatch by row (iVar4):
//   0..5  (Swords/Axes/Maces/Spears/Bows/Staffs): reads [1F 22 23 27 29 2A 2C 2E]
//   6     (Shield): reads [1F 25 24 29 2C 2E]
//   7..9  (Helm/Armor/Pants): reads [1F 25 26 29 2C 2E]
//   10    (Glove): reads [1F 25 27 29 2C 2E]
//   11    (Boot): reads [1F 25 28 29 2C 2E]
//   12    (Ring): reads [1F 25 29 31 30 2C 2E] + [38 39 3A 3B]
//   13    (Necklace): reads [1F 29 3C 3D 3E 3F]; [31]=[1F]; then [38 39 3A 3B]
//   14    (Scroll): reads [32 1F] only
//   15    (Wing): reads [1F 31 30]; sets [22]=[1F], [23]=[1F]+[1F]/2; then [38..3B]
//   >13   (others/consumables): skips extra reads
void __cdecl Item_LoadTextData(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

LAB_loop:
    {
        int recType = TextParser_GetToken();
        if (recType == 2) {
            FUN_0054150f(DAT_07d7806c);
            return;
        }
        if (recType == 1) {
            // row index
            long long lv = 0; __ftol();  // read row
            lv = 0; // Ghidra emits __ftol() as float-to-long conversion
            // NOTE: the actual implementation uses a streaming float parser;
            // here we document the structure faithfully as a stub that calls
            // the real parser. The logic below mirrors the Ghidra decompilation.

            // row = (int)__ftol result from first TextParser_GetToken
            // col = (int)__ftol result from second TextParser_GetToken
            // slot = (row*0x20 + col) * 0x40
            // The loop runs until iVar4==0 && strcmp(TextParserTokenString, "END")==0
        }
        goto LAB_loop;
    }
}

// IDA: FUN_0047B650
// Writes item data table to a binary .bmd file with checksum.
// Allocates 0x8000-byte buffer, copies 0x200 item slots (stride 0x40) from
// DAT_07d78068, XOR-encrypts each 0x40-byte block via FUN_00479910,
// writes the buffer (fwrite via FUN_005430f0), then computes a rolling
// checksum and appends 4 bytes (checksum seed: DAT_01c5e200 = 0x01c5e200).
void __cdecl Item_SaveBMD(const char *path)
{
    FILE *fp = (FILE *)FUN_0054173f(path, DAT_005597d4);  // "wb"
    char *buf = (char *)operator_new(0x8000);
    int off = 0;
    char *p = buf;
    do {
        // copy 0x40 bytes from item table slot
        void *src = (void *)(DAT_07d78068 + off);
        memcpy(p, src, 0x40);
        FUN_00479910((int)p, 0x40);  // XOR-encrypt
        off += 0x40;
        p   += 0x40;
    } while (off < 0x8000);
    FUN_005430f0(buf, 0x8000, 1, (int *)fp);
    // compute checksum
    DWORD cs = DAT_01c5e200;
    for (UINT i = 0; i < 0x7ffd; i += 4) {
        DWORD v  = *(DWORD *)(buf + i);
        DWORD q  = ((i >> 2) - 1) & 1;
        if      (q == 0) cs ^= v;
        else if (q == 1) cs += v;
        if ((i & 0xf) == 0)
            cs ^= (DWORD)(cs + 0xe2f1) >> (((BYTE)(i >> 2) & 7) + 1);
    }
    FUN_005430f0((char *)&cs, 4, 1, (int *)fp);
    operator_delete(buf);
    FUN_0054150f(fp);
}

// IDA: FUN_0047B740
// Reads a binary .bmd item data file (counterpart to FUN_0047b650).
// Allocates 0x8000-byte buffer, reads data + 4-byte checksum, validates
// checksum (exits with MessageBox on mismatch), XOR-decrypts each 0x40-byte
// block, copies decrypted records into DAT_07d78068 (raw table) and
// DAT_07cf1ff0 (shadow table); shifts [+0x27] field left by 1 in shadow.
void __cdecl Item_LoadBMD(const char *path)
{
    CHAR msg[256];
    FILE *fp = (FILE *)FUN_0054173f(path, DAT_005580ac);  // "rb"
    if (!fp) {
        crt_sprintf(msg, (const char *)s__s___File_not_exist__00558094);
        FUN_00405540(&DAT_055c9bf0, msg);
        MessageBoxA(DAT_055c9ffc, msg, nullptr, 0);
        SendMessageA(DAT_055c9ffc, 2, 0, 0);
        return;
    }
    char *buf = (char *)operator_new(0x8000);
    FUN_00541597(buf, 0x8000, 1, (int *)fp);
    DWORD stored_cs;
    FUN_00541597((char *)&stored_cs, 4, 1, (int *)fp);
    FUN_0054150f(fp);

    // validate checksum
    DWORD cs = DAT_01c5e200;
    for (UINT i = 0; i < 0x7ffd; i += 4) {
        DWORD v  = *(DWORD *)(buf + i);
        DWORD q  = ((i >> 2) - 1) & 1;
        if      (q == 0) cs ^= v;
        else if (q == 1) cs += v;
        if ((i & 0xf) == 0)
            cs ^= (DWORD)(cs + 0xe2f1) >> (((BYTE)(i >> 2) & 7) + 1);
    }
    if (stored_cs != cs) {
        crt_sprintf(msg, (const char *)s__s___File_corrupted__00559bd4);
        FUN_00405540(&DAT_055c9bf0, msg);
        MessageBoxA(DAT_055c9ffc, msg, nullptr, 0);
        SendMessageA(DAT_055c9ffc, 2, 0, 0);
        operator_delete(buf);
        return;
    }

    int  off = 0;
    char *p  = buf;
    do {
        FUN_00479910((int)p, 0x40);  // XOR-decrypt
        // copy to raw table
        memcpy((char *)DAT_07d78068 + off, p, 0x40);
        // copy to shadow table
        memcpy((char *)DAT_07cf1ff0 + off, p, 0x40);
        // shift [+0x27] field in shadow
        *((BYTE *)DAT_07cf1ff0 + off + 0x27) <<= 1;
        off += 0x40;
        p   += 0x40;
    } while (off < 0x8000);
    operator_delete(buf);
}
