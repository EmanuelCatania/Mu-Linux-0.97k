// Scene_EntityLifecycle.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0050f700 @ 0x0050F700 — Map_Load(filename)
// Opens map file and reads blocks into the buffer at DAT_07e0ffc8 via the
// format descriptor at DAT_00560694 (10 iterations of 0x100 bytes each).
//
// 2026-05-03: AUTO-SKIP removed. DAT_07e0ffc8 is properly sized
// `char[10 * 0x100]` (2560 bytes) in globals.cpp; only the bound was a
// literal source-binary address. Iteration count is exactly 10.
void __cdecl FUN_0050f700(const char* map_name)
{
    FILE* pFVar1 = (FILE*)fopen(map_name, (const char*)DAT_00559b74);
    if (!pFVar1) return;
    char* puVar2 = (char*)&DAT_07e0ffc8[0];
    for (int i = 0; i < 10; ++i, puVar2 += 0x100) {
        FUN_00543274((int*)pFVar1, (void*)&DAT_00560694);
    }
    fclose(pFVar1);
    (void)puVar2;
}

// FUN_004ff5a0 @ 0x004ff5a0 — Entity_New (scene entity allocator)
// Allocates 0x1BC-byte entity node, zeroes it, links into doubly-linked list
// at DAT_083A021C[cell]/DAT_083A0220[cell] (16x16 grid, 4-DWORD stride).
//
// Cálculo de celda (verbatim contra asm @ 0x4ff5a7):
//   FLD [pos.x]  FMUL [DAT_00552d20]  __ftol  → grid_x
//   FLD [pos.y]  FMUL [DAT_00552d20]  __ftol  → grid_y
//   DAT_00552d20 = 6.25e-4f = 1/1600  (cell = 1600 unidades de mundo)
// __ftol usa el modo de redondeo actual (por defecto nearest-even), no
// truncate-toward-zero. Para los mocks del login todos caen en (0,0)/(0,1).
// Rechaza si grid_x<0 || grid_y<0 || grid_x>15 || grid_y>15.
void* __cdecl FUN_004ff5a0(int param_1, float* param_2, float* param_3, float param_4) {
    // BUG-FIX 2026-04-26 (audit #12): __ftol implementa truncate-toward-zero
    // (semántica de cast C de float→int), no nearest-even. lrintf redondeaba al
    // más cercano y divergía en negativos (lrintf(-1.5)=-2 vs __ftol(-1.5)=-1).
    int igx = (int)(param_2[0] * _DAT_00552d20);
    int igy = (int)(param_2[1] * _DAT_00552d20);
    if (igx < 0 || igy < 0 || igx > 15 || igy > 15) return nullptr;
    BYTE  bVar4 = (BYTE)((char)igx * 0x10 + (char)igy);
    UINT  uVar2 = (UINT)bVar4;

    UINT* puVar3 = (UINT*)operator_new(0x1bc);
    UINT* p = puVar3;
    for (int i = 0x6f; i != 0; i--) *p++ = 0;

    // link into per-cell doubly-linked list (head=DAT_083a021c, tail=DAT_083a0220)
    UINT** headArr = (UINT**)&DAT_083a021c;
    UINT** tailArr = (UINT**)((char*)&DAT_083a021c + 4);
    if (headArr[uVar2 * 4] == nullptr) {
        puVar3[0x6d] = 0; puVar3[0x6e] = 0;
        headArr[uVar2 * 4] = puVar3;
    } else {
        UINT* prevTail = tailArr[uVar2 * 4];
        *(UINT**)((char*)prevTail + 0x1b8) = puVar3;
        puVar3[0x6d] = (UINT)prevTail;
        puVar3[0x6e] = 0;
    }
    tailArr[uVar2 * 4] = puVar3;

    // common initialisation
    *(BYTE*)puVar3                  = 1;           // active flag
    *(BYTE*)(puVar3 + 0x58)         = 0;           // +0x160
    *(BYTE*)((char*)puVar3 + 0x161) = 0;
    *(BYTE*)(puVar3 + 0x37)         = 1;           // +0x0dc
    *(BYTE*)(puVar3 + 0x39)         = 0;           // +0x0e4
    *(BYTE*)(puVar3 + 0x44)         = 0;           // +0x110
    *(short*)((char*)puVar3 + 2)    = (short)param_1;
    puVar3[3]    = *(UINT*)&param_4;               // extra (+0x0c)
    puVar3[0x5a] = 0x3f800000;                     // scale_x = 1.0f  (+0x168)
    puVar3[0x59] = 0x3f800000;                     // scale_y = 1.0f  (+0x164)
    puVar3[0x33] = 0x3e23d70a;                     // speed 0.16f     (+0x0cc)
    puVar3[0x35] = 0x42480000;                     // 50.0f           (+0x0d4)
    puVar3[0x16] = 0xffffffff;                     // RGBA white      (+0x058)
    *(BYTE*)((char*)puVar3 + 0x105) = 0;           // anim_state
    *(BYTE*)((char*)puVar3 + 0x106) = 0;           // anim_state_prev
    puVar3[0x42] = 0; puVar3[0x43] = 0;
    *(BYTE*)(puVar3 + 0x6c) = bVar4;               // grid cell       (+0x1b0)
    puVar3[0x19] = 0xffffffff;                     // flags           (+0x064)
    puVar3[0x1a] = 0x3f800000;                     // entity scale 1.0f (+0x068)
    puVar3[0x1b] = 0; puVar3[0x1c] = 0; puVar3[0x1e] = 0;
    puVar3[0x34] = 0xc1f00000;                     // -30.0f          (+0x0d0)
    puVar3[0x20] = 0;
    puVar3[4] = *(UINT*)&param_2[0];               // world_x         (+0x010)
    puVar3[5] = *(UINT*)&param_2[1];               // world_y         (+0x014)
    puVar3[6] = *(UINT*)&param_2[2];               // world_z         (+0x018)
    puVar3[7] = *(UINT*)&param_3[0];               // target_x        (+0x01c)
    puVar3[8] = *(UINT*)&param_3[1];               // target_y        (+0x020)
    puVar3[9] = *(UINT*)&param_3[2];               // target_z        (+0x024)
    puVar3[0x46] = 0xc2200000;                     // bbox_min_x -40f (+0x118)
    puVar3[0x47] = 0xc2200000;                     // bbox_min_y -40f (+0x11c)
    puVar3[0x3a] = 0; puVar3[0x3b] = 0; puVar3[0x3c] = 0;
    puVar3[0xa]  = 0; puVar3[0xb]  = 0; puVar3[0xc]  = 0;
    puVar3[0x30] = 0; puVar3[0x31] = 0; puVar3[0x32] = 0;
    puVar3[0x48] = 0;
    puVar3[0x49] = 0x42200000;                     // bbox_max_x 40f  (+0x124)
    puVar3[0x4a] = 0x42200000;                     // bbox_max_y 40f  (+0x128)
    puVar3[0x4b] = 0x42a00000;                     // bbox_max_z 80f  (+0x12c)

    // login/char-select scene type overrides (g_GameState 2 or 4)
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4) {
        switch (param_1) {
        case 0x3c:
            puVar3[3]=0x3f4ccccd; puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3e4ccccd;
            *(BYTE*)(puVar3+0x37)=1; break;
        case 0xa0:
            puVar3[3]=0x3d343958; puVar3[0x19]=1; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa1:
            puVar3[3]=0x3f4ccccd; puVar3[0x19]=0; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        case 0xa2:
            // IDA-faithful MODEL_MUGAME (case 3 en byte_4FFAA4[Type-60]):
            //   scale=0.6, [0x64]=1, [0xDC]=0. No escribe offsets 58/59/60
            //   (bodyLight). memset(0) previo deja bodyLight=0, pero lightEnable=0
            //   → nunca se lee. Revertido el "PORT FIX" previo porque el tint
            //   (1,1,1) no tenía efecto visual (lightEnable==0) y divergía de IDA.
            puVar3[3]=0x3f19999a; puVar3[0x19]=1;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa3:
            puVar3[3]=0x40400000; puVar3[0x19]=0;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f000000; break;
        case 0xa4:
            puVar3[0x19]=10;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        }
    }

    // game-substate overrides; some cases return directly (skipping FUN_004ff580),
    // others call it and return; fall-through also calls it.
    switch (DAT_0055a7ac) {
    case 0:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 6: case 0x91: case 0x92: break;
        case 0x34: puVar3[0x19]=1; return puVar3;
        case 0x3b: puVar3[0x33]=0; return puVar3;
        case 0x50: puVar3[0x34]=0xc2480000; return puVar3;
        case 0x5a: case 0x96: puVar3[0x19]=1; [[fallthrough]];
        case 0x60: case 0x61: puVar3[0x33]=0x3e99999a; return puVar3;
        case 0x62: case 0x77: puVar3[0x19]=2; return puVar3;
        case 0x69: puVar3[0x19]=3; return puVar3;
        case 0x75: case 0x7a: puVar3[0x19]=4; return puVar3;
        case 0x76: puVar3[0x19]=8; return puVar3;
        case 0x85:
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        }
        break;
    case 1:
        if (param_1==0x3b) break;
        if (param_1==0x3c) {
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe;
        }
        return puVar3;
    case 2:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 0x13: case 0x5c: case 0x5d: puVar3[0x19]=0; return puVar3;
        case 0x14: case 0x41: case 0x56: case 0x58:
            // copy current pos to previous, compute facing angle mod 360
            puVar3[0xb]=puVar3[8]; puVar3[0xf]=puVar3[6]; puVar3[0xd]=puVar3[4];
            puVar3[0xa]=puVar3[7]; puVar3[0xe]=puVar3[5];
            { UINT ua = puVar3[9];
              float fa = (float)((int)((long long)(unsigned long long)ua % 0x168));
              puVar3[9]=*(UINT*)&fa; puVar3[0xc]=*(UINT*)&fa; }
            return puVar3;
        case 0x16: case 0x19: case 0x28: case 0x2d: case 0x37: case 0x49: break;
        case 0x36: case 0x38: puVar3[0x19]=1; return puVar3;
        case 0x4e: puVar3[0x19]=3; return puVar3;
        case 0x5b:
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        case 100: puVar3[0x16]=0xfffffffe; return puVar3;
        }
        [[fallthrough]];
    case 3:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 1: puVar3[0x19]=1; return puVar3;
        case 8: break;
        case 9: puVar3[0x19]=3; return puVar3;
        case 0x11: case 0x13: case 0x25: puVar3[0x19]=0; return puVar3;
        case 0x12: puVar3[0x19]=2; return puVar3;
        case 0x26: FUN_004ff580(puVar3); puVar3[0x16]=0xfffffffe; return puVar3;
        }
        [[fallthrough]];
    default:
        goto lbl_skip_init;
    case 7:
        if (param_1==0x27) { FUN_004ff580(puVar3); puVar3[0x16]=0xfffffffe; return puVar3; }
        return puVar3;
    case 8:
        if (param_1==0x4e) { FUN_004ff580(puVar3); goto lbl_skip_init; }
        return puVar3;
    }
    FUN_004ff580(puVar3);
lbl_skip_init:
    return puVar3;
}
// FUN_004FFFA0 @ 0x004FFFA0 — DeleteBug(Owner).
// Walk butterfly slot array (10 entries × 0x1BC stride at DAT_083a1218..DAT_083a1218+0x1158),
// clear active flag (slot[0]=0) on every slot whose owner field (slot+0xFC = DWORD index 63)
// matches `Owner`. Each slot occupies 111 DWORDs (= 0x1BC bytes); we step in DWORD units.
// Ported verbatim from IDA reference 0045C8C0_ChangeCharacterExt's helper.
void __cdecl FUN_004fffa0(DWORD Owner) {
    DWORD* Butterflies = (DWORD*)DAT_083a1218;
    DWORD* End         = (DWORD*)(DAT_083a1218 + 0x1158);
    do {
        if (*(BYTE*)Butterflies) {
            if (Butterflies[63] == Owner) {
                *(BYTE*)Butterflies = 0;
            }
        }
        Butterflies += 111;
    } while ((int)(uintptr_t)Butterflies < (int)(uintptr_t)End);
}

// FUN_004FFFD0 @ 0x004FFFD0 — CreateBug(Type, Position[3], Owner, SubType[, LinkBone]).
// Allocates a free slot in butterfly/effect array at DAT_083A1218 (stride 0x1BC, 10 entries up
// to DAT_083A1218+0x1158). Initialises slot from owner entity (param_3) and world_pos (param_2).
// Per IDA: only spawns when owner class==390 OR Type==816.
// Special cases: type 0x330(816)/0x331(817) → randomise spawn XY around owner pos.
//                type 0xC3(195)/0x10B(267)  → set bug-color tint to 0.9 (0x3F666666 = "fff?").
// Note: original IDA signature has 5th `LinkBone` param but it's never read; our 4-arg form
// is functionally identical. The Ghidra decompile labelled this Entity_Spawn — that was wrong.
void __cdecl FUN_004fffd0(int param_1, void *param_2_v, void *param_3_v, int param_4) {
    DWORD *param_2 = (DWORD*)param_2_v;
    int param_3 = (int)(uintptr_t)param_3_v;
    if ((*(short*)(param_3 + 2) == 0x186) || (param_1 == 0x330)) {
        char *pcVar5 = DAT_083a1218;
        while (*pcVar5 != '\0') {
            pcVar5 += 0x1bc;
            if (0x83a236f < (int)pcVar5) return;
        }
        *(DWORD*)(pcVar5 + 4)   = param_4;
        pcVar5[0x58] = pcVar5[0x59] = pcVar5[0x5a] = pcVar5[0x5b] = -1;
        pcVar5[100]  = pcVar5[0x65] = pcVar5[0x66] = pcVar5[0x67] = -1;
        *(short*)(pcVar5 + 2)   = (short)param_1;
        pcVar5[0x68] = pcVar5[0x69] = '\0'; pcVar5[0x6a] = (char)0x80; pcVar5[0x6b] = '?';
        pcVar5[0x164]= pcVar5[0x165]= '\0'; pcVar5[0x166]= (char)0x80; pcVar5[0x167]= '?';
        *pcVar5 = '\x01';
        pcVar5[0x160] = '\0';
        pcVar5[0xdc]  = '\x01';
        pcVar5[0xe4]  = '\0';
        pcVar5[0x161] = '\0';
        pcVar5[0x110] = '\0';
        *(int*)(pcVar5 + 0xfc)  = param_3;
        pcVar5[0x0c]='3'; pcVar5[0x0d]='3'; pcVar5[0x0e]='3'; pcVar5[0x0f]='?';
        pcVar5[0x60]='\x1e'; pcVar5[0x61]=pcVar5[0x62]=pcVar5[63]='\0';
        pcVar5[0x168]=pcVar5[0x169]=pcVar5[0x16a]=pcVar5[0x16b]='\0';
        *(DWORD*)(pcVar5+0x10)=param_2[0]; *(DWORD*)(pcVar5+0x14)=param_2[1]; *(DWORD*)(pcVar5+0x18)=param_2[2];
        *(DWORD*)(pcVar5+0x1c)=*(DWORD*)(param_3+0x1c);
        *(DWORD*)(pcVar5+0x20)=*(DWORD*)(param_3+0x20);
        *(DWORD*)(pcVar5+0x24)=*(DWORD*)(param_3+0x24);
        pcVar5[0x10c]=pcVar5[0x10d]=pcVar5[0x10e]=pcVar5[0x10f]='\0';
        pcVar5[0x108]=pcVar5[0x109]=pcVar5[0x10a]=pcVar5[0x10b]='\0';
        pcVar5[0xe8]=pcVar5[0xe9]='\0'; pcVar5[0xea]=pcVar5[0xeb]='@';
        pcVar5[0xec]=pcVar5[0xed]='\0'; pcVar5[0xee]=pcVar5[0xef]='@';
        pcVar5[0xf0]=pcVar5[0xf1]='\0'; pcVar5[0xf2]=pcVar5[0xf3]='@';
        pcVar5[0xcc]=pcVar5[0xcd]=pcVar5[0xce]='\0'; pcVar5[0xcf]='?';
        short sVar4 = (short)param_1;
        if (sVar4 == 0x330) {
            UINT u; int v;
            u=_rand()&0x800001ff; if((int)u<0)u=(u-1|0xfffffe00)+1; *(float*)(pcVar5+0x10)=(float)(int)(u-0x100)+*(float*)(param_3+0x10);
            u=_rand()&0x800001ff; if((int)u<0)u=(u-1|0xfffffe00)+1; *(float*)(pcVar5+0x14)=(float)(int)(u-0x100)+*(float*)(param_3+0x14);
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x18)=(float)(int)(u+0x80)+*(float*)(param_3+0x18);
        } else if (sVar4 == 0x331) {
            UINT u;
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x10)=(float)(int)(u-0x40)+*(float*)(param_3+0x10);
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x14)=(float)(int)(u-0x40)+*(float*)(param_3+0x14);
            *(DWORD*)(pcVar5+0x18)=*(DWORD*)(param_3+0x18);
            *(float*)(pcVar5+0x18)=(float)(_rand()%100)+FUN_004f7500(*(float*)(pcVar5+0x10), *(float*)(pcVar5+0x14));
        } else if ((sVar4==0xc3)||(sVar4==0x10b)) {
            strncpy(pcVar5+0x0c,"fff?",4);
        }
    }
}
// FUN_00500970 — implemented in src/Render/Entity_Render.cpp
// FUN_00500e80 — implemented in src/Render/Weather.cpp (Weather_Update)
// FUN_00502320 — implemented in src/Render/Ambient_Particles.cpp (Ambient_ParticleUpdate)
// FUN_00503760 — implemented in src/Util/Misc.cpp
// FUN_00503830 — implemented in src/Render/Entity_Render.cpp
// FUN_00504b50 — implemented in src/Render/Entity_DrawSetup.cpp (Entity_SetColorAndRender)
// FUN_00505970 — implemented in src/Render/Entity_Render.cpp
// FUN_00505a10 — implemented in src/Render/Entity_Render.cpp
