// Runtime_Medium.cpp
//
// Extracted from stubs_bulk_med.cpp (B3: stubs.cpp lines 14828-15881).
//
// BATCH 2-4: Functions 56-200+ bytes, sorted by size.
//   BATCH 2 — generic 56-200 byte functions
//   BATCH 3 — Game functions + BST/STL + Codec
//   BATCH 4 — JPEG/libjpeg codec (statically linked) + IAT thunks
//
// Mostly:
//   - Game state setters/getters
//   - BST/STL container ops
//   - libjpeg trampolines (no-op codec hooks)
//   - IAT thunk landing pads

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif

// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 2: Functions 56-200 bytes, sorted by size
// ═══════════════════════════════════════════════════════════════════════════════

// SaveIME_Status @ 0x0047ED45 (56 bytes) — save IME conversion status
void __cdecl SaveIME_Status(void) {
    HIMC hImc = ImmGetContext(g_hWnd);
    DWORD dwConv, dwSent;
    ImmGetConversionStatus(hImc, &dwConv, &dwSent);
    ImmReleaseContext(g_hWnd, hImc);
}

// FUN_004c3dd0 @ 0x004C3DD0 (56 bytes) — color value from gold amount
int __cdecl FUN_004c3dd0(int param_1) {
    if (9999999 < param_1) return (int)0xFF0000FF;
    if (999999 < param_1) return (int)0xFF0096FF;
    return ((param_1 < 100000) - 1 & (int)0xFF81EC01) - (int)0x692301;
}

// RenderInformation @ 0x0051E200 (58 bytes) — render all HUD info layers
void __cdecl RenderInformation(void) {
    FUN_0051af50(); // RenderErrorMessage / stats panel
    FUN_004f64d0(); // Scene_MapTick
    UI_RenderNotices(); // RenderNotices
    if ((DAT_005590ac == 1) || (DAT_005615c0 != 5)) {
        UI_RenderChatLogOverlay(); // SystemText / chat list
    }
    FUN_004c14e0(); // RenderDebugWindow / FPS reset
    FUN_004c3530(); // RenderHelpWindow / item info
    FUN_004bffa0(); // RenderCursor
    FUN_0051e0c0(); // RenderInfomation3D
}

// GetMapName @ 0x004EF120 (64 bytes) — return map display name
//
// BUG-FIX 2026-04-28: el original devolvía punteros directos a strings dentro de
// GlobalText[] usando direcciones absolutas del binario (0x07d2c04c etc.). En
// nuestro proceso esas addrs no existen → AV si se desreferencia. Usamos una
// tabla estática con los nombres de mapa estándar de Mu 0.97k.
char *__cdecl GetMapName(int iMap) {
    static char s_mapNames[18][32] = {
        "Lorencia", "Dungeon", "Devias", "Noria", "LostTower", "Exile",
        "Stadium", "Atlans", "Tarkan", "Devil Square", "Icarus", "BloodCastle",
        "BloodCastle", "BloodCastle", "BloodCastle", "BloodCastle", "BloodCastle",
        "ChaosCastle"
    };
    if (iMap < 0 || iMap >= 18) iMap = 0;
    return s_mapNames[iMap];
}

// CErrorReport::WriteFile @ 0x004054B0 (65 bytes) — XOR + write to log
int __fastcall CErrorReport__WriteFile(int ecx, int /*edx*/,
    HANDLE hFile, void *lpBuffer, DWORD nBytes, LPDWORD lpWritten, LPOVERLAPPED lpOvl) {
    // Xor_ConvertBuffer: XOR each byte with running key
    int iVar1 = *(int *)(ecx + 0x10c); // stub — actual XOR omitted
    *(int *)(ecx + 0x10c) = iVar1;
    return WriteFile(hFile, lpBuffer, nBytes, lpWritten, lpOvl);
}

// LevelConvert @ 0x0045C850 (66 bytes) — wing level to display level
int __cdecl LevelConvert(BYTE Level) {
    switch (Level) {
        case 1: return 3; case 2: return 5; case 3: return 7;
        case 4: return 8; case 5: return 9; case 6: return 10; case 7: return 11;
        default: return 0;
    }
}



// OpenMacro @ 0x0050F750 (72 bytes) — load macro hotkey file
//
// BUG-FIX 2026-04-28: usaba dirección absoluta 0x07e0ffc8 con bound 0x07e109c8.
// Ahora indexa el array DAT_07e0ffc8[10][0x100] (10 slots × 256 bytes).
void __cdecl OpenMacro(char *FileName) {
    FILE *fp = fopen(FileName, "rb");
    if (fp != NULL) {
        for (int i = 0; i < 10; ++i) {
            fscanf(fp, "%s", DAT_07e0ffc8 + i * 0x100);
        }
        fclose(fp);
    }
}

// CSQuest::setQuestLists @ 0x00401160 (73 bytes) — set quest list from packet
void __fastcall CSQuest__setQuestLists(int ecx, int /*edx*/, BYTE *byList, int num, int Class) {
    if (Class != -1) {
        *(BYTE *)(ecx + 4) = (BYTE)Class & 7;
        *(char *)(ecx + 5) = (char)(Class >> 3);
    }
    memset((void *)(ecx + 0x1c848), 0, 0x32);
    memcpy((void *)(ecx + 0x1c848), byList, num);
}


// FUN_005404c0 @ 0x005404C0 (71 bytes) — GameGuard: seed RNG from system time
int __fastcall FUN_005404c0(int param_1) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    // DAT_083bbb74 = (st.wSecond | ((int)st.wMinute << 16)) * st.wMonth * st.wYear ^ 0xdad53a;
    return param_1;
}



// FUN_00540a70 @ 0x00540A70 (75 bytes) — GameGuard: init crypto context
int *__fastcall FUN_00540a70(int *param_1) {
    param_1[5] = 0; param_1[3] = 0; *param_1 = 0; param_1[1] = 0; param_1[2] = 0;
    param_1[9] = 0; param_1[0xe] = 0; param_1[0x10] = 0; param_1[0xf] = 0;
    param_1[6] = 0; *(BYTE *)(param_1 + 7) = 0; param_1[8] = 0; param_1[0x11] = 0;
    BOOL r = CryptAcquireContextA((HCRYPTPROV *)param_1, NULL, MS_DEF_PROV_A, PROV_RSA_FULL, 0xf0000000);
    if (!r) *param_1 = 0;
    return param_1;
}

// CheckMouseIn @ 0x0040C490 (110 bytes) — check if mouse is within rectangle
int __cdecl CheckMouseIn(int x, int y, int width, int height, int direction) {
    if (direction == 2) {
        if (x <= MouseX && MouseX < x + width && y - height <= MouseY && MouseY < y)
            return 1;
    } else {
        if (x <= MouseX && MouseX < x + width && y <= MouseY && MouseY < y + height)
            return 1;
    }
    return 0;
}

// CreateClassAttribute @ 0x0047D240 (97 bytes) — set class base stats
void __cdecl CreateClassAttribute(int Class, int Str, int Dex, int Vit, int Eng,
    int Life, int Mana, int LvLife, int LvMana, int VitToLife, int EngToMana) {
    int off = Class * 0x50;
    *(short *)(off + 0x7d77f28) = (short)Str;
    *(short *)(off + 0x7d77f2a) = (short)Dex;
    *(short *)(off + 0x7d77f2c) = (short)Vit;
    *(short *)(off + 0x7d77f2e) = (short)Eng;
    *(short *)(off + 0x7d77f30) = (short)Life;
    *(short *)(off + 0x7d77f32) = (short)Mana;
    *(BYTE *)(off + 0x7d77f34) = (BYTE)LvLife;
    *(BYTE *)(off + 0x7d77f35) = (BYTE)LvMana;
    *(BYTE *)(off + 0x7d77f36) = (BYTE)VitToLife;
    *(BYTE *)(off + 0x7d77f37) = (BYTE)EngToMana;
}

// SetPlayerColor @ 0x00480D80 (98 bytes) — set name color by PK level
void __cdecl SetPlayerColor(BYTE PK) {
    static DWORD m_dwTextColor_local;
    switch (PK) {
        case 0: m_dwTextColor_local = 0xfff0ff96; break;
        case 1: m_dwTextColor_local = 0xffff7864; break;
        case 2: m_dwTextColor_local = 0xffffb48c; break;
        case 3: m_dwTextColor_local = 0xffffdcc8; break;
        case 4: m_dwTextColor_local = 0xff3c96ff; break;
        case 5: m_dwTextColor_local = 0xff1e50ff; break;
        default: m_dwTextColor_local = 0xff0000ff; break;
    }
}

// CreateClassAttributes @ 0x0047D2B0 (115 bytes) — init all 4 class stat tables
void __cdecl CreateClassAttributes(void) {
    CreateClassAttribute(0, 0x12,0x12,0xf,0x1e,0x50,0x3c, 1,2,1,2);
    CreateClassAttribute(1, 0x1c,0x14,0x19,0x0a,0x6e,0x14, 2,1,2,1);
    CreateClassAttribute(2, 0x32,0x32,0x32,0x1e,0x6e,0x1e, 0x6e,0x1e,6,3);
    CreateClassAttribute(3, 0x1e,0x1e,0x1e,0x1e,0x78,0x50, 1,1,2,2);
}

// UpdateFile @ 0x0041E500 (90 bytes) — copy+delete with retry
BOOL __cdecl UpdateFile(char *lpszOld, char *lpszNew) {
    for (int i = 0; i < 0x14; i++) {
        Sleep(500);
        if (CopyFileA(lpszOld, lpszNew, FALSE)) {
            DeleteFileA(lpszOld);
            return TRUE;
        }
    }
    return FALSE;
}

// DecryptCheckSumKey @ 0x00412D30 (32 bytes) — already implemented above in batch 1

// FUN_00406cd0 @ 0x00406CD0 (79 bytes) — HashWidget deinit (SEH + cleanup)
void __fastcall FUN_00406cd0(void *param_1_raw) {
    int *param_1 = (int *)param_1_raw;
    *param_1 = (int)&PTR_FUN_005524d8; // intermediate vtable
    FUN_00406d40((int)param_1);
    param_1[3] = 0; param_1[1] = 0; param_1[2] = 0;
    *param_1 = (int)&PTR_FUN_005524d8; // final vtable
}

// FUN_00406d40 @ 0x00406D40 (102 bytes) — free all hash table buckets
void __cdecl FUN_00406d40(int param_1) {
    unsigned int uVar1 = 0;
    do {
        void *lpMem = *(void **)(*(int *)(param_1 + 4) + uVar1);
        if (lpMem != NULL) {
            operator_delete(lpMem);
            *(int *)(*(int *)(param_1 + 4) + uVar1) = 0;
        }
        uVar1 += 4;
    } while (uVar1 < 0x1000);
    if (*(void **)(param_1 + 0x18) != NULL)
        operator_delete(*(void **)(param_1 + 0x18));
    *(int *)(param_1 + 4) = *(int *)(param_1 + 0x20);
    *(int *)(param_1 + 8) = *(int *)(param_1 + 0x24);
    operator_delete(*(void **)(param_1 + 0x24));
    operator_delete(*(void **)(param_1 + 4));
    *(int *)(param_1 + 0xc) = 0;
}

// FUN_0040a600 @ 0x0040A600 (82 bytes) — LinkedList: alloc head+tail sentinel nodes
void __fastcall FUN_0040a600(void *param_1_raw) {
    int *param_1 = (int *)param_1_raw;
    void *n1 = operator_new(0xc);
    if (n1 != NULL) { *(int *)((int)n1 + 8) = 0; *(int *)((int)n1 + 4) = 0; }
    else n1 = NULL;
    param_1[1] = (int)n1;
    void *n2 = operator_new(0xc);
    if (n2 != NULL) { *(int *)((int)n2 + 8) = 0; *(int *)((int)n2 + 4) = 0; }
    else n2 = NULL;
    param_1[2] = (int)n2;
    *(int *)(param_1[1] + 8) = (int)n2;
    *(int *)(param_1[2] + 4) = param_1[1];
    *param_1 = 0;
}


// FUN_00408070 @ 0x00408070 (117 bytes) — SoundWidget: destroy linked list
void __fastcall FUN_00408070(void *param_1_raw) {
    int *param_1 = (int *)param_1_raw;
    *param_1 = 0x00552520; // PTR_LAB_00552520
    *(int *)(*(int *)(param_1[0x14] + 4) + 8) = 0;
    void *pvVar1 = *(void **)(param_1[0x13] + 8);
    while (pvVar1 != NULL) {
        void *next = *(void **)((int)pvVar1 + 8);
        operator_delete(pvVar1);
        pvVar1 = next;
    }
    *(int *)(param_1[0x13] + 8) = param_1[0x14];
    *(int *)(param_1[0x14] + 4) = param_1[0x13];
    param_1[0x12] = 0;
    if ((void *)param_1[0x14] != NULL) operator_delete((void *)param_1[0x14]);
    if ((void *)param_1[0x13] != NULL) operator_delete((void *)param_1[0x13]);
}

// FUN_00540ac0 @ 0x00540AC0 (108 bytes) — GameGuard: release crypto resources
void __fastcall FUN_00540ac0(int *param_1) {
    if ((void *)param_1[1] != NULL) FUN_00543c98((void *)param_1[1]);
    if ((void *)param_1[5] != NULL) FUN_00543c98((void *)param_1[5]);
    if (param_1[3] != 0) CryptDestroyHash((HCRYPTHASH)param_1[3]);
    if (param_1[4] != 0) CryptDestroyKey((HCRYPTKEY)param_1[4]);
    if (param_1[0xf] != 0) CryptDestroyKey((HCRYPTKEY)param_1[0xf]);
    if (param_1[0x10] != 0) CryptDestroyHash((HCRYPTHASH)param_1[0x10]);
    if (*param_1 != 0) CryptReleaseContext((HCRYPTPROV)*param_1, 0);
}

// Xor_ConvertBlock @ 0x00405130 (131 bytes) — XOR buffer with 16-byte key
void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey) {
    BYTE byXorKey[16] = {0x7c,0xbd,0x81,0x9f,0x3d,0x93,0xe2,0x56,
                          0x2a,0x73,0xd2,0x3e,0xf2,0x83,0x95,0xbf};
    for (int i = 0; i < iSize; i++)
        lpBuffer[i] ^= byXorKey[i + iKey];
}

// Xor_ConvertBuffer @ 0x004051C0 (126 bytes) — XOR full buffer in 16-byte blocks
// Signature matches functions.h: `int __cdecl(void*, DWORD, int)`.
int __cdecl Xor_ConvertBuffer(void *lpBuffer, DWORD nBytes, int iKey) {
    int iSize = (int)nBytes;
    unsigned int uVar1 = (0x10 - iKey) & 0x0f;
    if (iSize <= (int)uVar1) uVar1 = iSize;
    Xor_ConvertBlock((BYTE *)lpBuffer, uVar1, iKey);
    int rem = iSize - uVar1;
    BYTE *p = (BYTE *)lpBuffer + uVar1;
    if (rem < 1) return uVar1 + iKey;
    if (rem > 0xf) {
        unsigned int blocks = rem >> 4;
        rem += blocks * -0x10;
        do { Xor_ConvertBlock(p, 0x10, 0); p += 0x10; blocks--; } while (blocks != 0);
    }
    Xor_ConvertBlock(p, rem, 0);
    return rem;
}

// FUN_004052b0 @ 0x004052B0 (108 bytes) — CErrorReport: open log file
void __fastcall FUN_004052b0_impl(int ecx, int /*edx*/, char *param_1) {
    // Copy filename to this+8
    char *dst = (char *)(ecx + 8);
    while (*param_1) *dst++ = *param_1++;
    *dst = 0;
    *(int *)(ecx + 0x10c) = 0;
    HANDLE h = CreateFileA((LPCSTR)(ecx + 8), 0xc0000000, 1, NULL, 4, 0x80, NULL);
    *(HANDLE *)(ecx + 4) = h;
    // FUN_00405340(); // log header — skip
    SetFilePointer(h, 0, NULL, 2);
}

// FindTextureByName @ 0x00505BF0 (141 bytes) — search texture table by name
// Escanea slots [TextureBegin .. TextureCurrent) comparando el filename en
// [+0x00..+0x1F] de cada slot de Bitmaps. En hit, escribe el puntero del slot
// en *dwTexture y devuelve el índice; en miss, devuelve -1 y deja *dwTexture=0.
int __cdecl FindTextureByName(char *Name, DWORD *dwTexture) {
    *dwTexture = 0;
    int lo = (int)DAT_083a4104;   // TextureBegin
    int hi = (int)DAT_083a4108;   // TextureCurrent
    for (int i = lo; i < hi; i++) {
        char* slot = &g_BitmapsRaw[i * 0x38];
        if (strncmp(slot, Name, 32) == 0) {
            *dwTexture = (DWORD)(uintptr_t)slot;
            return i;
        }
    }
    return -1;
}

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 2
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 3: Game functions + BST/STL + Codec (56-200+ bytes)
// ═══════════════════════════════════════════════════════════════════════════════

// CSQuest::ShowQuestNpcWindow @ 0x00401910 (75 bytes) — open quest NPC dialog
void __fastcall CSQuest__ShowQuestNpcWindow(void *This, int /*edx*/, int index) {
    if (index != -1) {
        *(char *)((int)This + 0x1c87a) = (char)index;
    }
    CloseInventoryRelatedWindows();
    _g_bEventChipDialogEnable = 0;
    InventoryOpened = 0;
    CharacterOpened = 0;
    *(BYTE *)((int)This + 0x1c87f) = 1;
    CSQuest__CheckQuestState(This, 0xff);
    CSQuest__ShowDialogText((int)*(short *)((int)This + 0x1c880), 0);
}


// FUN_0040e730 @ 0x0040E730 (105 bytes) — text filter: check word against blocklist
int __fastcall FUN_0040e730(void *This, int /*edx*/, char *param_1) {
    char *Token = (char *)((int)This + 200);
    if (*(char *)((int)This + 200) == '\0') return 1;
    int i = 0;
    while (true) {
        if (*Token == '\0') return 0;
        if (FindText(param_1, Token, false)) break;
        i++;
        Token += 0x100;
        if (i > 4) return 0;
    }
    return 1;
}

// FUN_00451ea0 @ 0x00451EA0 (78 bytes) — transform bone position to world coords
void __cdecl FUN_00451ea0(int param_1, void *param_2, int param_3) {
    float Position[3] = {0.0f, 0.0f, 0.0f};
    float WorldPosition[3] = {0.0f, 0.0f, 0.0f};
    Position[2] = 1.4013e-45f;
    BMD__TransformPosition(param_2,
        (float (*)[4])(param_3 * 0x30 + *(int *)(param_1 + 0x114)),
        Position, WorldPosition, false);
}

// FUN_00479540 @ 0x00479540 (120 bytes) — render terrain alpha bitmaps
// FUN_00479540 (IDA-activated, was Ghidra stub)
void FUN_00479540()
{
  float *v0; // esi
  float Rotation; // [esp+0h] [ebp-10h]

  EnableAlphaBlend();
  v0 = (float *)&DAT_07c82cf4;
  do
  {
    if ( *((BYTE *)v0 - 36) )
    {
      if ( Bitmaps[*((DWORD *)v0 - 8)].Components == 3 )
      {
        EnableAlphaBlend();
      }
      else
      {
        EnableAlphaTest(1);
      }
      Rotation = -*v0;
      RenderTerrainAlphaBitmap(*((DWORD *)v0 - 8), *(v0 - 5), *(v0 - 4), *(v0 - 6), *(v0 - 6), v0 + 1, Rotation, v0[4]);
    }
    v0 += 28;
  }
  while ( (int)v0 < (int)&DAT_07c858b4 );
}


// MoveCharacterCamera @ 0x0051E450 (195 bytes) — update camera from character origin
void __cdecl MoveCharacterCamera(float *Origin, float *Position, float *Angle) {
    float angles[3];
    float matrix[3][4];
    float result[3];

    CameraAngle[0] = 0.0f;
    CameraAngle[1] = 0.0f;
    CameraAngle[2] = *(float *)(*(int *)&Origin[2] + 8);
    angles[0] = CameraAngle[0];
    angles[1] = CameraAngle[1];
    angles[2] = CameraAngle[2];
    AngleMatrix(angles, matrix);

    float in1[3];
    in1[0] = Origin[1];
    in1[1] = Origin[1]; // placeholder — Ghidra stack layout unclear
    in1[2] = Origin[1];
    VectorIRotate(in1, matrix, result);

    CameraAngle[0] = *(float *)&Origin[2];
    CameraPosition[0] = result[0] + *(float *)&Origin[0];
    CameraPosition[1] = result[1] + *((float *)&Origin[0] + 1);
    CameraPosition[2] = result[2] + *((float *)&Origin[0] + 2);
}

// OpenSMDFile @ 0x0040B200 (106 bytes) — open and parse SMD model file
// CRITICAL 2026-05-03: ParseNodes/ParseSkeleton/ParseTriangles are EMPTY STUBS
// (lines 19407, 19417, 19432). If we open SMDFile here and call them, the file
// content is never consumed; fclose() leaves the global SMDFile pointing to a
// freed FILE* — any later reader (GetToken from Monster_Data, etc.) crashes
// dereferencing it. Until the SMD parsers are actually implemented, do not
// touch the SMDFile global. Return false so the SMD chain stays a no-op
// (matches the original behaviour: 0.97k ships only .bmd, no .smd files).
bool __cdecl OpenSMDFile(char *FileName, int /*Type*/, bool /*Flip*/) {
    if (FileName == NULL) return false;
    FILE* probe = fopen(FileName, "r");
    if (!probe) return false;
    fclose(probe);
    // SMDFile global intentionally untouched — parsers are stubs and consumers
    // (GetToken/Monster_Data/Item_Data/...) own SMDFile via their own fopen().
    return false;
}




// FUN_004112b0 @ 0x004112B0 (73 bytes) — BST iterator: advance to next (in-order successor)
void __fastcall FUN_004112b0_impl(int *param_1) {
    int *puVar1 = *(int **)(*param_1 + 8);
    if ((void *)puVar1 == DAT_055c9b98) {
        int iVar3 = *(int *)(*param_1 + 4);
        if (*param_1 == *(int *)(iVar3 + 8)) {
            do {
                *param_1 = iVar3;
                iVar3 = *(int *)(iVar3 + 4);
            } while (*param_1 == *(int *)(iVar3 + 8));
        }
        if (*(int *)(*param_1 + 8) != iVar3) {
            *param_1 = iVar3;
        }
        return;
    }
    int *puVar2;
    for (puVar2 = (int *)*puVar1; (void *)puVar2 != DAT_055c9b98; puVar2 = (int *)*puVar2) {
        puVar1 = puVar2;
    }
    *param_1 = (int)puVar1;
}

// FUN_00411420 @ 0x00411420 (37 bytes) — BST recursive post-order delete
void __cdecl FUN_00411420_impl(int *param_1) {
    if ((void *)param_1 != DAT_055c9b98) {
        int *piVar1;
        do {
            FUN_00411420_impl((int *)param_1[2]);
            piVar1 = (int *)*param_1;
            operator_delete(param_1);
            param_1 = piVar1;
        } while ((void *)piVar1 != DAT_055c9b98);
    }
}



// FUN_004117c0 @ 0x004117C0 (95 bytes) — BST iterator: post-increment (return old, advance)
// FUN_004117c0 (IDA-activated, was Ghidra stub)
DWORD *__cdecl FUN_004117c0(int *_this, DWORD *a2, int a3)
{
  int v3; // edi
  DWORD **v4; // edx
  DWORD *j; // eax
  DWORD *result; // eax
  int i; // eax

  v3 = *_this;
  v4 = *(DWORD ***)(*_this + 8);
  if ( v4 == (DWORD **)DAT_055c9b98 )
  {
    for ( i = *(DWORD *)(v3 + 4); *_this == *(DWORD *)(i + 8); i = *(DWORD *)(i + 4) )
    {
      *_this = i;
    }
    if ( *(DWORD *)(*_this + 8) != i )
    {
      *_this = i;
    }
    result = a2;
    *a2 = v3;
  }
  else
  {
    for ( j = *v4; j != (DWORD *)DAT_055c9b98; j = (DWORD *)*j )
    {
      v4 = (DWORD **)j;
    }
    result = a2;
    *_this = (int)v4;
    *a2 = v3;
  }
  return result;
}


// FUN_00411870 @ 0x00411870 (95 bytes) — BST iterator: pre-decrement
void __fastcall FUN_00411870_impl(int *param_1) {
    int *piVar3 = (int *)*param_1;
    if ((piVar3[0x4d] == 0) && (*(int *)(piVar3[1] + 4) == (int)piVar3)) {
        *param_1 = piVar3[2];
        return;
    }
    int iVar1 = *piVar3;
    if ((void *)iVar1 == DAT_055c9b98) {
        piVar3 = (int *)piVar3[1];
        if (*param_1 == *piVar3) {
            do {
                *param_1 = (int)piVar3;
                piVar3 = (int *)piVar3[1];
            } while (*param_1 == *piVar3);
        }
        *param_1 = (int)piVar3;
        return;
    }
    int iVar2;
    for (iVar2 = *(int *)(iVar1 + 8); iVar2 != (int)DAT_055c9b98; iVar2 = *(int *)(iVar2 + 8)) {
        iVar1 = iVar2;
    }
    *param_1 = iVar1;
}

// FUN_004118d0 @ 0x004118D0 (82 bytes) — doubly-linked list: clear all nodes
void __fastcall FUN_004118d0_impl(int param_1) {
    int *piVar1 = *(int **)(param_1 + 0x5c);
    int *lpMem = (int *)*piVar1;
    while (lpMem != piVar1) {
        int *piVar2 = (int *)*lpMem;
        *(int *)lpMem[1] = *lpMem;
        *(int *)(*lpMem + 4) = lpMem[1];
        operator_delete(lpMem);
        *(int *)(param_1 + 0x60) = *(int *)(param_1 + 0x60) - 1;
        lpMem = piVar2;
    }
    *(int *)(param_1 + 0x88) = 0;
}

// FUN_004122c0 @ 0x004122C0 (85 bytes) — list: trim excess entries from tail
void __fastcall FUN_004122c0_impl(int param_1) {
    int iVar1 = *(int *)(param_1 + 0x60);
    int iVar2 = *(int *)(param_1 + 0x84);
    if (iVar2 <= iVar1 && iVar1 != iVar2 && iVar1 - iVar2 > 0) {
        int iVar3 = 0;
        do {
            int *lpMem = *(int **)(*(int *)(param_1 + 0x5c) + 4);
            *(int *)lpMem[1] = *lpMem;
            *(int *)(*lpMem + 4) = lpMem[1];
            operator_delete(lpMem);
            *(int *)(param_1 + 0x60) = *(int *)(param_1 + 0x60) - 1;
            iVar3++;
        } while (iVar3 < iVar1 - *(int *)(param_1 + 0x84));
    }
}

// FUN_00411300 @ 0x00411300 (95 bytes) — doubly-linked list: insert node after position
int *__fastcall FUN_00411300(void *This, int /*edx*/, int *param_1, void *param_2, int *param_3) {
    int *puVar3 = *(int **)((int)param_2 + 4);
    int *puVar1 = (int *)operator_new(0xc);
    int *puVar2 = (int *)param_2;
    if (param_2 == NULL) puVar2 = puVar1;
    *puVar1 = (int)puVar2;
    if (puVar3 == NULL) puVar3 = puVar1;
    puVar1[1] = (int)puVar3;
    *(int **)((int)param_2 + 4) = puVar1;
    *(int *)puVar1[1] = (int)puVar1;
    if (puVar1 + 2 != NULL) {
        puVar1[2] = *param_3;
    }
    *(int *)((int)This + 8) = *(int *)((int)This + 8) + 1;
    *param_1 = (int)puVar1;
    return param_1;
}


// FUN_00410d90 @ 0x00410D90 (78 bytes) — list: destroy all + free sentinel
void __fastcall FUN_00410d90_impl(int param_1) {
    int *piVar1 = *(int **)(param_1 + 4);
    int *lpMem = (int *)*piVar1;
    while (lpMem != piVar1) {
        int *piVar2 = (int *)*lpMem;
        *(int *)lpMem[1] = *lpMem;
        *(int *)(*lpMem + 4) = lpMem[1];
        operator_delete(lpMem);
        *(int *)(param_1 + 8) = *(int *)(param_1 + 8) - 1;
        lpMem = piVar2;
    }
    operator_delete(*(void **)(param_1 + 4));
    *(int *)(param_1 + 4) = 0;
    *(int *)(param_1 + 8) = 0;
}

// FUN_00410270 @ 0x00410270 (88 bytes) — allocate next free slot in pool
// FUN_00410270 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00410270(DWORD *_this)
{
  int v1; // edx
  int v2; // eax

  v1 = _this[51];
  v2 = 0;
  _this[50] = -1;
  if ( v1 >= 0 )
  {
    while ( *((BYTE *)_this + v2 + 208) )
    {
      if ( ++v2 > v1 )
      {
        goto LABEL_6;
      }
    }
    _this[50] = v2;
  }
LABEL_6:
  if ( _this[50] == -1 )
  {
    _this[51] = v1 + 1;
    _this[50] = v1 + 1;
  }
  *((BYTE *)_this + _this[50] + 208) = 1;
  return _this[50] + 10000;
}


// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 3
// ═══════════════════════════════════════════════════════════════════════════════

// FUN_00482dd0 @ 0x00482dd0 — CSQuest::FindQuestItemsInInven
// Counts items in inventory matching nType (param_1), optional level filter (param_3).
// Returns shortage = nCount - found (0 means at least nCount items present).
// Inventory grid: 8 rows × 8 cols at DAT_07EA9328..DAT_07EA9504 (stride 0x44 per cell row,
// 0x11 ints = 68 bytes per outer step). Iterates 7 outer × 8 inner = 56 cells.
//
// BUG-FIX 2026-05-03: original port used `if (piVar4 < 0x7ea9328)` — a hardcoded
// absolute bound from the source binary. In our build &DAT_07ea9504 lives at
// a different address (linker-placed) so the comparison was meaningless: it
// either triggered immediately (early-exit returns wrong shortage) or never
// (infinite loop / heap walk crash). Replaced with explicit iteration count.
int __cdecl FUN_00482dd0(int param_1, int param_2, uint param_3)
{
    int iVar3 = 0;
    int *piVar4 = &DAT_07ea9504;
    // 2026-08-22: eran 7 columnas y son 8.  El bound de IDA es
    // `while (v5 >= &unk_7EA9328)` arrancando en &unk_7EA9504 con paso de -17
    // ints (-68 bytes): (0x7EA9504 - 0x7EA9328) / 68 + 1 = 8.  Con 7 se salteaba
    // una columna entera del inventario, asi que un item de quest que estuviera
    // ahi contaba como faltante: la lista salia en rojo y el boton en gris
    // aunque el personaje lo tuviera.
    for (int outer = 0; outer < 8; ++outer) {
        int *piVar1 = piVar4;
        for (int iVar2 = 7; iVar2 >= 0; --iVar2) {
            if ((((short)piVar1[-0xe] == param_1) && (0 < *piVar1)) &&
                ((param_3 == 0xffffffff || ((uint)(piVar1[-0xd] >> 3) & 0xfU) == param_3))) {
                iVar3++;
                if (param_2 <= iVar3) return 0;
            }
            piVar1 -= 0x88;
        }
        piVar4 -= 0x11;
    }
    return param_2 - iVar3;
}

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH 4: JPEG/libjpeg codec (statically linked) + IAT thunks
// ═══════════════════════════════════════════════════════════════════════════════

// --- IAT thunks (0x0052a0d0-0x0052a100) ---
// These are 6-byte jmp [IAT] stubs. The linker resolves them when linking
// imm32.lib and dsound.lib. No source code needed — they are:
//   0x0052a0d0 = ImmReleaseContext        0x0052a0d6 = ImmGetIMEFileNameA
//   0x0052a0dc = ImmGetDescriptionA       0x0052a0e2 = ImmGetContext
//   0x0052a0e8 = ImmGetConversionStatus   0x0052a0ee = ImmGetDefaultIMEWnd
//   0x0052a0f4 = ImmSetConversionStatus   0x0052a100 = DirectSoundEnumerateA
// Total: 8 thunks × 6 bytes = 48 bytes (linker-generated, skip)

// Forward declarations for codec internal helpers used below
static void __cdecl FUN_00403a30_impl(void);
static int  __cdecl FUN_00543a8c_impl(char *p1, BYTE *p2);
static char*__cdecl FUN_00543ac0_impl(BYTE *p);

static void __cdecl FUN_00403a30_impl(void) { /* nop - error reset */ }

static int __cdecl FUN_00543a8c_impl(char *p1, BYTE *p2) {
    int val = 0;
    sscanf(p1, (const char*)p2, &val);
    return (val > 0) ? 1 : 0;
}

static char *__cdecl FUN_00543ac0_impl(BYTE *name) {
    return getenv((const char*)name);
}

// Forward decl for FUN_0052f4d0

// Forward decl

// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH 9
// ═══════════════════════════════════════════════════════════════════════════════
