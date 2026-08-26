// UI_LegacyGameHelpers.cpp
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


// UI/game helpers
// FUN_004cba60 @ 0x004CBA60 — CharPreview_Reset
// Resets the char-select / second-password UI state:
//   - Clears DAT_07eaa118 (2nd-pass hover flag), DAT_07eaa132, DAT_07eaa134
//   - Clears DAT_07eaa119, DAT_00559f5f, DAT_07eaa14c, DAT_07eaa11a
//   - Resets two entity-slot arrays (stride 0x44, reset id/select fields to 0xffff/0)
//   - Clears DAT_07e11d28, calls FUN_00404bc0(0x19,0,0) and FUN_00404bc0(0x1c,0,0)
void __cdecl FUN_004cba60(void) {
    // Reset second-password hover/state flags
    DAT_07eaa118 = 0;
    DAT_07eaa132 = 0;
    DAT_07eaa134 = 0;
    DAT_07eaa119 = 0;
    DAT_00559f5f  = 0;
    DAT_07eaa14c  = 0;
    DAT_07eaa11a  = 0;
    DAT_07eaa11b  = 0;
    DAT_07eaa11c  = 0;
    DAT_07eaa128  = 0;
    DAT_07eaa12c  = 0;
    DAT_07eaa130  = 0;
    *((char*)&DAT_07eaa130 + 1) = 0; // DAT_07eaa131 (adjacent byte, not separately declared)

    // Reset char-slot entry arrays.
    // 2026-04-30 BUG-FIX: bound by array size not hardcoded original-binary
    // address.  Previously loops compared `(int)p < 0x7EA7B48` against our
    // build's globals which sit at OS-allocated addresses (~0x01F90000).
    // The condition was always true → walk overran into random memory →
    // write-AV at arbitrary address (we observed p=0x1F9E040 crashing).
    //
    // IDA original walks pointer +0x44 each step.  IDA decomp `p + 0x11`
    // is `p + 17 dwords = +68 bytes = +0x44`, but it accesses `p - 0x38`
    // (= -56 bytes from p).  In IDA the array layout is interpreted as
    // (record-base + 0x38) = `p`, so `p - 0x38` is the record base id field.
    {
        BYTE* base = DAT_07ea5b68;
        BYTE* end  = base + sizeof(DAT_07ea5b68);   // 0x1FE0 = 8160 bytes
        UINT* p    = (UINT*)(base + 0x38);          // start at offset 0x38 within first record
        while ((BYTE*)p < end) {
            *(unsigned short*)((char*)p - 0x38) = 0xffff;
            *p = 0;
            p = (UINT*)((BYTE*)p + 0x44);
        }
    }
    {
        BYTE* base = DAT_07ea9880;
        BYTE* end  = base + sizeof(DAT_07ea9880);   // 0x880 = 2176 bytes
        UINT* p    = (UINT*)(base + 0x38);
        while ((BYTE*)p < end) {
            *(unsigned short*)((char*)p - 0x38) = 0xffff;
            *p = 0;
            p = (UINT*)((BYTE*)p + 0x44);
        }
    }
    // Reset char-name index arrays
    int i = 0;
    while (i < 0x880) {
        *(unsigned short*)((int)&DAT_07ea7b88 + i) = 0xffff;
        *(unsigned short*)((int)&DAT_07ea5298 + i) = 0xffff;
        *(UINT*)((int)&DAT_07ea7bc0 + i) = 0;
        *(UINT*)((int)&DAT_07ea52d0 + i) = 0;
        if (DAT_07eaa0e8 == '\x01') {
            *(unsigned short*)((int)&DAT_07e11f78 + i) = 0xffff;
            *(UINT*)((int)&DAT_07e11fb0 + i) = 0;
        }
        i += 0x44;
    }
    DAT_07e11d28 = 0;
    FUN_00404bc0(0x19, 0, 0);
    FUN_00404bc0(0x1c, 0, 0);
}

// FUN_004cd3b0 @ 0x004CD3B0 — UI_ItemGrid_Fill
// Fills 2D grid buffers with current item slot data (DAT_07e91350) for equipment display.
// Dispatches by DAT_07ea9800; each grid entry = 0x11 dwords, selection flag at offset 0x38.
void __cdecl FUN_004cd3b0(void)
{
    if ((int)DAT_07e91388 < 1) return;

    int   iVar8 = *(int*)DAT_07e91350;
    int   iVar3 = iVar8 * 0x40 + (int)DAT_07d78068;
    unsigned int uVar5 = (unsigned int)*(unsigned char *)(iVar3 + 0x20); // item width
    unsigned int uVar9 = (unsigned int)*(unsigned char *)(iVar3 + 0x21); // item height
    unsigned int selCol = (unsigned int)DAT_07e9138e;
    unsigned int selRow = (unsigned int)DAT_07e9138f;

    // Position packed as col (bits 0-2) + row (bits 3+).
    // IDA usa Inventory[32].Type acá.  El comentario anterior decía que
    // DAT_07ea9844 era "our mirror" de eso, pero NO lo es: en el binario esa
    // direccion es un byte-flag aparte (bSell).  Ahora usamos el campo real.
    int posVal  = ItemPickedPos;
    if ((void*)DAT_07ea9800 == (void*)&OffsetInventoryItems[0] && posVal >= 12) {
        posVal -= 12;
    }
    int colBits = posVal & 7;
    int rowBits = posVal >> 3;

    // Helper lambda-style inline: fill grid block at gridBase (stride 8 cols)
    #define FILL_GRID(gridBase) do { \
        int *_gb = (int *)(gridBase); \
        for (unsigned int row = (unsigned int)rowBits; row < (unsigned int)rowBits + uVar9; row++) { \
            for (unsigned int col = (unsigned int)colBits; col < (unsigned int)colBits + uVar5; col++) { \
                int *cell = _gb + (col + row * 8) * 0x11; \
                int *src  = (int *)DAT_07e91350; \
                for (int k = 0; k < 0x11; k++) cell[k] = src[k]; \
                cell[0xe] = (row == selRow && col == selCol) ? 1 : 0; \
            } \
        } \
    } while(0)

    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea7b88) { FILL_GRID(&DAT_07ea7bc0); return; }
    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea5b30) { FILL_GRID(&DAT_07ea5b68); return; }
    if ((void *)DAT_07ea9800 == (void *)&DAT_07ea9848) { FILL_GRID(&DAT_07ea9880); return; }

    {
        BYTE* base = DAT_07ea8448;
        BYTE* end  = base + sizeof(DAT_07ea8448);
        while (base < end) {
            memset(base, 0, 0x44);
            *(short*)base = (short)0xFFFF;
            base += 0x44;
        }
    }

    if (posVal > 0xb) {
        FILL_GRID(&DAT_07ea8448);
        if (iVar8 == 0x1a0 || iVar8 == 0x1a2 || iVar8 == 0x1a3) {
            int spawnType = (iVar8 == 0x1a0) ? 0x330 : (iVar8 == 0x1a2) ? 0xc3 : 0x10b;
            FUN_004fffd0(spawnType, (void *)(DAT_07abf5d8 + 0x10), (void *)DAT_07abf5d8, 0);
        }
    }
    #undef FILL_GRID
    // (HashTable obfuscation blocks skipped — anti-tamper)
}

// FUN_004b0e80 @ 0x004B0E80 — Hotkey_ClassSync(void)
// Iterates hotkey table (DAT_07cf1ff4 +0x57+i, 0x14 entries):
//   compares entry's class byte at (classType*0x40 + DAT_07cf1ff4 + 0xd7 + i)
//   against classType (= 1, the pushed ESI at call site in Chat_InputTick).
//   On match, writes entity[0x391] = slot index (i).
// Also: if DAT_00559c5c set and sub-state != 6 and current hotkey class is 0x06 or 0x0f:
//   clears SelectedCharacter / DAT_00559c58 to -1.
// unaff_EBP = GetAsyncKeyState ptr (HashTable only), unaff_retaddr = 1 (classType).
void __cdecl FUN_004b0e80(void)
{
    // classType = 1 (the pushed ESI value from Chat_InputTick)
    int classType = (int)DAT_005616ac;  // slot class index (confirmed: DAT_005616ac used as iVar7)
    char* charData = (char*)DAT_07cf1ff4; // CharData sub-pointer
    char* playerEnt = DAT_07abf5d8;      // player entity

    for (int i = 0; i < 0x14; i++) {
        // Check if slot is active and matches class
        if (*(charData + 0x57 + i) != '\0' &&
            (unsigned char)*(charData + classType * 0x40 + 0xd7 + i) == (unsigned char)1) {
            // Set entity hotkey slot index
            *(playerEnt + 0x391) = (char)i;
        }

        // If hover enabled, sub-state != 6, and current hotkey class is mage/elf
        if (DAT_00559c5c != '\0' && DAT_0055a7ac != 6) {
            int slotIdx = (unsigned char)*(playerEnt + 0x391);
            char hotkeyCls = *(charData + 0x57 + slotIdx);
            if (hotkeyCls == '\x06' || hotkeyCls == '\x0f') {
                SelectedCharacter = 0xffffffff;
                DAT_00559c58 = 0xffffffff;
            }
        }
    }
}
