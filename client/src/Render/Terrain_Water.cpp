// Terrain_Water.cpp
// FUN_004f95e0 @ 0x004F95E0
//
// World-tick / terrain water wave animation.
// Two responsibilities:
//   1) Copy per-tile lighting data from the "shadow" buffer (DAT_0828b608) to
//      the live buffer (DAT_081cb608) for the current viewport rectangle.
//      Viewport: columns [DAT_0839bc90 .. DAT_0055a774+3],
//                rows    [DAT_0839bc94 .. DAT_0055a778+3]
//      Each tile entry is 12 bytes (3 × float), layout:
//        DAT_081cb608[tile*3+0] = DAT_0828b608[tile*3+0]  (R or X)
//        DAT_081cb608[tile*3+1] = DAT_0828b60c[tile*3+0]  (G or Y)
//        DAT_081cb608[tile*3+2] = DAT_0828b610[tile*3+0]  (B or Z)
//
//   2) Compute sine-wave height offsets into DAT_07eab200 (float table, row*256+col)
//      for each tile in the same viewport rectangle.
//      If DAT_083a3ff0 == 0: fast ocean waves (period 720000 sub-ticks, scale _DAT_005528e0)
//      If DAT_083a3ff0 != 0: slow terrain waves (period 36000, scale _DAT_005524f8)
//      Per-tile wave: sin(col * waveFreq + timePhase) * _DAT_00552488
//      waveFreq = _DAT_00552660 (normal) or _DAT_00552598 (game state 8 = CharSelectInit)
//      col step = _DAT_0055256c
//
// Globals:
//   DAT_0839bc90/94  — viewport top-left (col, row)
//   DAT_0055a774/778 — viewport extents (+3 gives inclusive max)
//   DAT_083a3ff0     — 0=ocean wave mode, else terrain mode
//   DAT_0055a7ac     — g_GameSubState
//   DAT_07eab200     — water height table [row][col] floats
//   DAT_081cb608     — live per-tile lighting buffer (3 floats/tile)
//   DAT_0828b608/0c/10 — shadow/pre-computed lighting buffers

#include "stdafx.h"

int __cdecl FUN_004f95e0(void)
{
    int   iVar4;
    float fVar1;
    int   iVar2, iVar3;
    uint  uVar5, uVar7, uVar8;
    float *pfVar6;
    float10 fVar11;
    float10 fVar12, fVar13;
    ulonglong uVar14;

    // ── 1) Copy lighting data into live buffer for viewport rect ──────────────
    int rowMax = DAT_0055a778 + 3;
    uVar8 = DAT_0839bc94;  // viewport start row
    if ((int)DAT_0839bc94 <= rowMax) {
        int colMax = DAT_0055a774 + 3;
        uVar7 = DAT_0839bc94;
        do {
            if ((int)DAT_0839bc90 <= colMax) {
                uVar5 = DAT_0839bc90;
                do {
                    // tile index = col + row*256 (0x100)
                    iVar2 = (uVar5 & 0xff) + (uVar7 & 0xff) * 0x100;
                    iVar3 = iVar2 * 0xc;  // byte offset (3 floats × 4 bytes)
                    uVar5++;
                    // BUG-FIX: DAT_081cb608 está declarado DWORD, así que
                    // `&DAT_081cb608 + iVar3` hace aritmética DWORD* (= +iVar3*4
                    // = +iVar2*48 bytes). El stride real es 12 bytes (3 floats).
                    // Disasm @ 0x004f9620-23 confirma byte-offset = iVar2*3*4=12.
                    // Castear a char* para que la suma sea aritmética de bytes.
                    // FIX 2026-06-27: el RHS era `DAT_0828b608[iVar2*3]` (float[] → VALOR
                    // float); asignado a `*(unsigned int*)` hacía conversión float→int =
                    // truncación → R quedaba 0 (luz 0.x<1.0). G/B usan el macro DWORD*
                    // (bits). IDA hace PrimaryTerrainLight[i][0]=BackTerrainLight[i][0]
                    // (copia float). Bit-cast del source para preservar el float, igual a G/B.
                    *(unsigned int *)((char*)&DAT_081cb608 + iVar3) = *(unsigned int*)&DAT_0828b608[iVar2 * 3];
                    *(unsigned int *)((char*)&DAT_081cb60c + iVar3) = (&DAT_0828b60c)[iVar2 * 3];
                    *(unsigned int *)((char*)&DAT_081cb610 + iVar3) = (&DAT_0828b610)[iVar2 * 3];
                    uVar8 = DAT_0839bc94;
                } while ((int)uVar5 <= colMax);
            }
            uVar7++;
        } while ((int)uVar7 <= rowMax);
    }

    // ── 2) Compute time phase for water waves ─────────────────────────────────
    if (DAT_083a3ff0 == 0) {
        // Ocean / water mode: long period (720000 units)
        uVar14 = (ulonglong)(unsigned int)__ftol();
        iVar4  = (int)((long long)uVar14 / 720000);
        fVar11 = (float10)(int)((long long)uVar14 % 720000) * (float10)_DAT_005528e0;
    } else {
        // Terrain wave mode: short period (36000 units)
        uVar14 = (ulonglong)(unsigned int)__ftol();
        iVar4  = (int)((long long)uVar14 / 36000);
        fVar11 = (float10)(int)((long long)uVar14 % 36000) * (float10)_DAT_005524f8;
    }

    int iVar2b = DAT_0055a7ac;

    // ── 3) Fill water height table with sine wave ─────────────────────────────
    if ((int)uVar8 <= rowMax) {
        int iVar9 = (int)uVar8 << 8;   // row * 256
        int colMax2 = DAT_0055a774 + 3;
        int rowCount = (rowMax - (int)uVar8) + 1;
        do {
            fVar12 = (float10)(int)DAT_0839bc90;
            if ((int)DAT_0839bc90 <= colMax2) {
                iVar4 = (colMax2 - DAT_0839bc90) + 1;
                // BUG-FIX: DAT_07eab200 es DWORD → &DAT_07eab200 + N*4 hace
                // aritmética DWORD* (=+N*16 bytes). Disasm @ 0x004f96d1 muestra
                // LEA EDX,[EAX*0x4 + 0x7eab200] = byte offset N*4. Castear a char*.
                pfVar6 = (float *)((char*)&DAT_07eab200 + (iVar9 + DAT_0839bc90) * 4);
                do {
                    // Select wave frequency by game sub-state
                    fVar1 = _DAT_00552660;
                    if (iVar2b == 8)
                        fVar1 = _DAT_00552598;

                    iVar4--;
                    // height = sin(col * freq + timePhase) * amplitude
                    fVar13 = (float10)fsin(fVar12 * (float10)fVar1 + fVar11);
                    *pfVar6 = (float)(fVar13 * (float10)_DAT_00552488);

                    fVar12 += (float10)_DAT_0055256c;  // next column
                    pfVar6++;
                } while (iVar4 != 0);
            }
            iVar9 += 0x100;  // next row
            rowCount--;
        } while (rowCount != 0);
    }

    return iVar4;
}
