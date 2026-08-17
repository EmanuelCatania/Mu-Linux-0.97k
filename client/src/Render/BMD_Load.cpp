// BMD_Load.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12147-12807 (661 lines).
//
// BMD (Mu Online 3D model format) loaders:
//   FUN_004423e0 (BMD::Open)              — load compressed BMD file into model slot
//   FUN_004422f0 (BMD_BuildAdjacentFaceTable)
//   FUN_00442260 (BMD_InitAdjFaceTable)
//   FUN_00442e60 (BMD_ComputeBounds)
//   FUN_00442e00 (BMD_ResetAnimState)
//   FUN_00442a60 (BMD_SaveToFile)

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);

// ── BMD loaders ──────────────────────────────────────────────────────────────
// FUN_004423e0 @ 0x004423E0 — BMD::Open (load compressed BMD file into model slot)
// param_1 = this (model object), param_2 = path string, param_3 = filename string, param_4 = unused
// Reescrito contra Ghidra 97k (verbatim de "Main 97K", confirmado). Layout 97k:
//   Buffer: [ver@3][name@4..0x23 (32B)][numMeshes@0x24][numActions@0x26]
//           [numBones@0x28][data@0x2A...]
//
// NOTA 97k vs Main 5.2: el orden en el archivo es Mesh/Actions/Bones (97k)
// pero en Main 5.2 es Mesh/Bones/Actions. El 97k lee this[0x22]=numActions
// desde Buffer[0x26] y this[0x26]=numBones desde Buffer[0x28] — los strides
// de allocación confirman la semántica (stride 0x8c = Action struct,
// stride 0x10 = Bone struct).
//
// NOTA Player.bmd: es sólo-esqueleto (nMesh=0, muchas bones + actions). El
// render compone Player.bmd (bones + anims) con ArmorClass##/HelmClass##/
// PantClass##/GloveClass##/BootClass##.bmd (cada uno: 1 mesh + 56 anims
// matching skeleton). nMesh=0 para Player.bmd NO es bug — es el formato.
//   this  : +0x00..+0x1F modelName (copiado de Buffer+4)
//           +0x20   versionByte (Buffer[3])
//           +0x22 s NumActions  (<- Buffer+0x26)
//           +0x24 s NumMeshes   (<- Buffer+0x24)
//           +0x26 s NumBones    (<- Buffer+0x28)
//           +0x28   Meshes  [nM] stride 0x28 (geom: verts/normals/UVs/faces)
//           +0x2c   Actions [nA] stride 0x8c (matrix + per-bone keyframe)
//           +0x30   Bones   [nB] stride 0x10 (skin weights por hueso)
//           +0x34   TexNames[nM] stride 0x20 (char[32] por mesh)
//           +0x38   TexIdx  [nM] stride 2
// Loop order: Mesh → FindNearTriangle → Bone → Action
void __cdecl FUN_004423e0(int param_1, int param_2, int param_3, int param_4)
{
    (void)param_4;
    void *thisPtr = (void *)param_1;
    const char *path     = (const char *)param_2;
    const char *filename = (const char *)param_3;

    // VERY-EARLY defensive BodyLight init: done BEFORE fopen so even if file
    // can't be opened the slot has sane (1,1,1) instead of the pre-existing
    // garbage pattern (0xF5F5F5F5 from the random model pool offset).
    *(float *)((int)thisPtr + 0x48) = 1.0f;
    *(float *)((int)thisPtr + 0x4c) = 1.0f;
    *(float *)((int)thisPtr + 0x50) = 1.0f;

    char local_4c[64];
    crt_sprintf(local_4c, "%s%s", path, filename);

    // Unconditional entry log for Logo/Ship so we can confirm we actually
    // reach this function for those slots and know what thisPtr they use.
    {
        if (strstr(local_4c, "Logo") || strstr(local_4c, "Ship")) {
            char line[200];
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD::Open ENTER '%s' thisPtr=%p", local_4c, thisPtr);
            DbgLogPublic(line);
        }
    }

    FILE *fp = FUN_0054173f(local_4c, DAT_005580ac);
    if (!fp) {
        char diag[200];
        _snprintf_s(diag, sizeof(diag), _TRUNCATE, "BMD::Open fopen FAIL: %s", local_4c);
        DbgLogPublic(diag);
        return;
    }

    fseek(fp, 0, SEEK_END);
    int fileSize = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSize < 0x2a) {
        char diag[200];
        _snprintf_s(diag, sizeof(diag), _TRUNCATE, "BMD::Open too-small: %s size=%d", local_4c, fileSize);
        DbgLogPublic(diag);
        fclose(fp);
        return;
    }
    unsigned char *Buffer = (unsigned char *)operator_new(fileSize);
    if (!Buffer) { fclose(fp); return; }
    fread(Buffer, 1, fileSize, fp);
    fclose(fp);

    // --- header ---
    // BMD v10: "BMD\x0A" + plain payload (97k original format)
    // BMD v12: "BMD\x0C" + [long encSize] + encrypted payload (XOR+sub).
    //   Clave XOR de 16 bytes + running byte sub; ver MapFileDecrypt abajo.
    //   Al descifrar, el payload queda idéntico a v10: name[32], counts,
    //   mesh/bone/action blobs. Por eso, tras descifrar, normalizamos el
    //   Buffer reconstruyendo el header "BMD\x0C" + payload descifrado,
    //   de modo que el resto del parser (name en Buffer+4, counts en
    //   Buffer+0x24..) funciona sin cambios.
    unsigned char verByte = Buffer[3];
    if (Buffer[0] != 'B' || Buffer[1] != 'M' || Buffer[2] != 'D' ||
        (verByte != 0x0A && verByte != 0x0C))
    {
        char line[160];
        _snprintf_s(line, sizeof(line), _TRUNCATE,
            "BMD skip (unsupported): %s ver=0x%02x size=%d",
            local_4c, verByte, fileSize);
        DbgLogPublic(line);
        operator_delete(Buffer);
        return;
    }

    if (verByte == 0x0C) {
        if (fileSize < 0x2a + 4) {
            char line[160];
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD v12 too-small: %s size=%d", local_4c, fileSize);
            DbgLogPublic(line);
            operator_delete(Buffer);
            return;
        }
        int encSize = *(int *)(Buffer + 4);
        if (encSize <= 0 || encSize > fileSize - 8) {
            char line[160];
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD v12 bad encSize: %s encSize=%d file=%d",
                local_4c, encSize, fileSize);
            DbgLogPublic(line);
            operator_delete(Buffer);
            return;
        }
        unsigned char *decBuf = (unsigned char *)operator_new(encSize + 4);
        if (!decBuf) {
            operator_delete(Buffer);
            return;
        }
        // Reconstruir header para mantener layout v10 en el resto del parser.
        decBuf[0] = 'B'; decBuf[1] = 'M'; decBuf[2] = 'D'; decBuf[3] = 0x0C;
        // MapFileDecrypt: XOR rotativo + substracción con key derivada del
        // byte cifrado anterior. Inicial wMapKey = 0x5E.
        static const unsigned char bmdMapXorKey[16] = {
            0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
            0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2
        };
        unsigned char wMapKey = 0x5E;
        const unsigned char *enc = Buffer + 8;
        for (int i = 0; i < encSize; i++) {
            unsigned char src = enc[i];
            decBuf[4 + i] = (unsigned char)((src ^ bmdMapXorKey[i & 0xF]) - wMapKey);
            wMapKey = (unsigned char)(src + 0x3D);
        }
        operator_delete(Buffer);
        Buffer = decBuf;
        fileSize = encSize + 4;
    }

    *(char *)((int)thisPtr + 0x20) = verByte;
    // El nombre se lee de Buffer+4 (Ghidra: puVar7 = puVar7+1 antes del body).
    memcpy(thisPtr, Buffer + 4, 32);

    // EARLY defensive BodyLight init: write (1,1,1) BEFORE any parsing path that
    // could early-return. Any subsequent type-specific caller will overwrite as
    // needed (BMD_SetupRender, Entity_DrawByType), but the slot never holds the
    // uninitialized CD/FD pattern that manifested as -1.695e+37 garbage on Logo01.
    *(float *)((int)thisPtr + 0x48) = 1.0f;
    *(float *)((int)thisPtr + 0x4c) = 1.0f;
    *(float *)((int)thisPtr + 0x50) = 1.0f;

    short numMeshes  = *(short *)(Buffer + 0x24);
    short numActions = *(short *)(Buffer + 0x26);
    short numBones   = *(short *)(Buffer + 0x28);

    // Sanity check: counts razonables
    if (numMeshes < 0 || numMeshes > 512 ||
        numActions < 0 || numActions > 512 ||
        numBones  < 0 || numBones  > 512) {
        *(short *)((int)thisPtr + 0x22) = 0;
        *(short *)((int)thisPtr + 0x24) = 0;
        *(short *)((int)thisPtr + 0x26) = 0;
        *(int   *)((int)thisPtr + 0x28) = 0;
        *(int   *)((int)thisPtr + 0x2c) = 0;
        *(int   *)((int)thisPtr + 0x30) = 0;
        *(int   *)((int)thisPtr + 0x34) = 0;
        *(int   *)((int)thisPtr + 0x38) = 0;
        {
            char line[200];
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD_SanityFail: %s nMesh=%d nAction=%d nBone=%d  thisPtr=%p",
                local_4c, (int)numMeshes, (int)numActions, (int)numBones, thisPtr);
            DbgLogPublic(line);
        }
        operator_delete(Buffer);
        return;
    }

    {
        // Filter: log only non-Player models (skip the noisy ArmorClass/BootClass
        // spam to reserve counter slots for login-scene models like Logos/Ship).
        bool isPlayer = (strstr(local_4c, "Player\\") != NULL) ||
                        (strstr(local_4c, "Player/") != NULL);
        static int s_parsed = 0;
        // 2026-05-04: temporarily upped limit from 30 to 200 to debug Lorencia
        // BMD load (slots 0..0xa0). Will revert when validated.
        if (!isPlayer && s_parsed < 200) {
            char line[192];
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD_Parsed #%d '%s' v=0x%02x fileSize=%d  nMesh=%d nAction=%d nBone=%d  thisPtr=%p",
                s_parsed, local_4c, (unsigned)verByte, fileSize,
                (int)numMeshes, (int)numActions, (int)numBones, thisPtr);
            DbgLogPublic(line);
            s_parsed++;
        }
    }

    *(short *)((int)thisPtr + 0x24) = numMeshes;
    *(short *)((int)thisPtr + 0x22) = numActions;
    *(short *)((int)thisPtr + 0x26) = numBones;

    // --- allocate (clamp a 1 como el original) ---
    int nM = numMeshes  > 0 ? numMeshes  : 1;
    int nA = numActions > 0 ? numActions : 1;
    int nB = numBones   > 0 ? numBones   : 1;
    *(int *)((int)thisPtr + 0x28) = (int)operator_new(nM * 0x28);
    *(int *)((int)thisPtr + 0x2c) = (int)operator_new(nA * 0x8c);
    *(int *)((int)thisPtr + 0x30) = (int)operator_new(nB * 0x10);
    *(int *)((int)thisPtr + 0x34) = (int)operator_new(nM * 0x20);
    *(int *)((int)thisPtr + 0x38) = (int)operator_new(nM * 2);

    int cursor = 0x2a;

    // ============ MESH loop ============
    // Mesh (stride 0x28):
    //   +0  byte flag=0
    //   +2  short aux        (Buffer[cursor+8])
    //   +4  short numVerts   (Buffer[cursor+0])
    //   +6  short numNormals (Buffer[cursor+2])
    //   +8  short numUVs     (Buffer[cursor+4])
    //   +10 short numFaces   (Buffer[cursor+6])
    //   +0x10 vertBuf  stride 0x10
    //   +0x14 normBuf  stride 0x14
    //   +0x18 uvBuf    stride 0x08
    //   +0x1c faceBuf  stride 0x24 (src stride 0x40)
    //   +0x24 texNameString* (std::string-ish, 4-byte stub)
    for (int m = 0; m < numMeshes; m++) {
        char *mesh = (char *)(*(int *)((int)thisPtr + 0x28) + m * 0x28);
        short nV   = *(short *)(Buffer + cursor + 0);
        short nN   = *(short *)(Buffer + cursor + 2);
        short nT   = *(short *)(Buffer + cursor + 4);
        short nF   = *(short *)(Buffer + cursor + 6);
        *(short *)(mesh + 4)  = nV;
        *(short *)(mesh + 6)  = nN;
        *(short *)(mesh + 8)  = nT;
        *(short *)(mesh + 10) = nF;
        *(short *)(mesh + 2)  = *(short *)(Buffer + cursor + 8);
        *(char  *)(mesh + 0)  = 0;

        *(int *)(mesh + 0x10) = (int)operator_new(nV * 0x10);
        *(int *)(mesh + 0x14) = (int)operator_new(nN * 0x14);
        *(int *)(mesh + 0x18) = (int)operator_new(nT * 0x08);
        *(int *)(mesh + 0x1c) = (int)operator_new(nF * 0x24);

        cursor += 10;
        memcpy((void *)*(int *)(mesh + 0x10), Buffer + cursor, nV * 0x10); cursor += nV * 0x10;
        memcpy((void *)*(int *)(mesh + 0x14), Buffer + cursor, nN * 0x14); cursor += nN * 0x14;
        memcpy((void *)*(int *)(mesh + 0x18), Buffer + cursor, nT * 0x08); cursor += nT * 0x08;

        for (int f = 0; f < nF; f++) {
            memcpy((void *)(*(int *)(mesh + 0x1c) + f * 0x24), Buffer + cursor, 0x24);
            cursor += 0x40;
        }

        // texture name (char[32])
        char *texSlot = (char *)(*(int *)((int)thisPtr + 0x34) + m * 0x20);
        memcpy(texSlot, Buffer + cursor, 0x20);
        cursor += 0x20;

        // TextureScript parse (BMD::Open + parsingTScript @ 0x40C190, fiel a IDA).
        // El nombre de textura puede llevar un marcador `_XYZ` (tras '_') con chars
        // R/H/S/N que activan flags de render por-mesh:
        //   R→script[0] (blend aditivo/glow)   H→script[1] (hidden)
        //   S→script[2] (wave/scroll)          N→script[3] (no-depth)
        // parsingTScript aborta (sin script) si tras '_' hay algún char no R/H/S/N.
        // BUG-FIX: el port previo alocaba un script en cero SIN parsear → todas las
        // meshes aditivas (glows, auras, alas) de todos los modelos renderizaban
        // opacas → recuadros negros. Ahora se parsea el nombre como el binario.
        {
            unsigned char ts0 = 0, ts1 = 0, ts2 = 0, ts3 = 0, ts4 = 0;
            char nameCopy[33];
            memcpy(nameCopy, texSlot, 32);
            nameCopy[32] = '\0';
            char *us = strchr(nameCopy, '_');
            if (us) {
                bool valid = true;
                int count = 0;
                for (int k = 1; k <= 4; k++) {
                    char c = us[k];
                    if (c == '.' || c == '\0') break;
                    if      (c == 'R') ts0 = 1;
                    else if (c == 'H') ts1 = 1;
                    else if (c == 'S') ts2 = 1;
                    else if (c == 'N') ts3 = 1;
                    else { valid = false; break; }   // char inválido → aborta (IDA: return 0)
                    count++;
                }
                if (valid && count > 0) ts4 = 1;
            }
            if (ts4) {
                char *strObj = (char *)operator_new(4);
                if (strObj) { strObj[0] = ts0; strObj[1] = ts1; strObj[2] = ts2; strObj[3] = ts3; }
                *(int *)(mesh + 0x24) = (int)strObj;
            } else {
                *(int *)(mesh + 0x24) = 0;
            }
        }
    }

    FUN_00442260(thisPtr);

    // ============ BONE loop ============
    // Bone (stride 0x10):
    //   +0  byte flag=0
    //   +8  short numVerts   (Buffer[cursor+0])
    //   +10 byte  hasData    (Buffer[cursor+2])
    //   +0xc ptr  vertData (opcional, stride 0xc)
    for (int b = 0; b < numBones; b++) {
        char *bone = (char *)(*(int *)((int)thisPtr + 0x30) + b * 0x10);
        *(char *)(bone + 0) = 0;
        short bV   = *(short *)(Buffer + cursor + 0);
        char  flag = *(char  *)(Buffer + cursor + 2);
        *(short *)(bone + 8)  = bV;
        *(char  *)(bone + 10) = flag;
        cursor += 3;
        if (flag) {
            *(int *)(bone + 0xc) = (int)operator_new(bV * 0xc);
            memcpy((void *)*(int *)(bone + 0xc), Buffer + cursor, bV * 0xc);
            cursor += bV * 0xc;
        }
    }

    // ============ ACTION loop ============
    // Action (stride 0x8c):
    //   +0..0x1F  bone matrix (32 bytes) si flag==0
    //   +0x20 short numKeyframes
    //   +0x22 byte  flag
    //   +0x24 ptr   perBoneAnim[numBones] stride 0xc
    //         (+0 pos, +4 rot, +8 scale) cada uno para mV frames
    for (int a = 0; a < numActions; a++) {
        char *action = (char *)(*(int *)((int)thisPtr + 0x2c) + a * 0x8c);
        char aFlag = *(char *)(Buffer + cursor); cursor += 1;
        *(char *)(action + 0x22) = aFlag;
        if (aFlag == 0) {
            memcpy(action, Buffer + cursor, 32);
            cursor += 32;
            short nKF = *(short *)(Buffer + cursor); cursor += 2;
            *(short *)(action + 0x20) = nKF;

            int perBone = (int)operator_new(numBones * 0xc);
            *(int *)(action + 0x24) = perBone;

            for (int bb = 0; bb < numBones; bb++) {
                int entry = perBone + bb * 0xc;
                short mV = *(short *)(*(int *)((int)thisPtr + 0x30) + bb * 0x10 + 8);
                int posSz = mV * 0xc;
                int rotSz = mV * 0xc;
                int sclSz = mV * 0x10;
                *(int *)(entry + 0) = (int)operator_new(posSz);
                *(int *)(entry + 4) = (int)operator_new(rotSz);
                *(int *)(entry + 8) = (int)operator_new(sclSz);

                memcpy((void *)*(int *)(entry + 0), Buffer + cursor, posSz); cursor += posSz;
                memcpy((void *)*(int *)(entry + 4), Buffer + cursor, rotSz); cursor += rotSz;

                for (int k = 0; k < mV; k++) {
                    FUN_004fa1d0(*(int *)(entry + 4) + k * 0xc,
                                 *(int *)(entry + 8) + k * 0x10, 0, 0);
                }
            }
        }
    }

    // Defensive BodyLight init: BMD_SetupRenderByType's default path calls
    // FUN_00441e00 without writing +0x48..+0x50, so they'd read whatever was
    // left in memory (observed: R=1.0, G/B=-1.69e37 garbage → negative alpha
    // triangles). Default to white (1,1,1) so BMD_DrawMesh reads sane color
    // for models that never go through a type-specific colorization branch
    // (ships, logos, UI items, etc.). Callers that need colored models will
    // overwrite these fields before FUN_00441e00 as in BMD_SetupRender.cpp.
    *(float *)((int)thisPtr + 0x48) = 1.0f;
    *(float *)((int)thisPtr + 0x4c) = 1.0f;
    *(float *)((int)thisPtr + 0x50) = 1.0f;

    // DIAG: log any non-Player BMD that finishes parsing + the actual values
    // persisted at +0x48..+0x50 after defensive init. This tells us if Logo01
    // (v=0x0c) really reaches here — vs being intercepted by some other write.
    {
        bool isPlayer = (strstr(local_4c, "Player\\") != NULL) ||
                        (strstr(local_4c, "Player/") != NULL);
        bool isLoginScene = (strstr(local_4c, "Logo") != NULL) ||
                            (strstr(local_4c, "Ship") != NULL);
        static int s_end = 0;
        // Always log Logo/Ship (login scene) even if 30-cap reached.
        if (isLoginScene || (!isPlayer && s_end < 30)) {
            char line[220];
            float r = *(float *)((int)thisPtr + 0x48);
            float g = *(float *)((int)thisPtr + 0x4c);
            float b = *(float *)((int)thisPtr + 0x50);
            _snprintf_s(line, sizeof(line), _TRUNCATE,
                "BMD_OpenEnd #%d '%s' thisPtr=%p bodyLight=(%.3f,%.3f,%.3f)%s",
                s_end, local_4c, thisPtr, r, g, b,
                isLoginScene ? " [LOGIN]" : "");
            DbgLogPublic(line);
            s_end++;
        }
    }

    operator_delete(Buffer);
    FUN_00442e00(thisPtr, '\0');
}

// FUN_004422f0 @ 0x004422F0 — BMD_BuildAdjacentFaceTable
// For each face in bone param_1, and each edge param_3 (0-2): finds the adjacent face sharing
// the flipped edge and stores the adjacency index in psVar1[faceIdx*0x12 + edgeIdx + 0xd].
void __cdecl FUN_004422f0(void *pThis, int param_1, int param_2, int param_3)
{
    short *psVar1 = *(short **)(*(int *)((int)pThis + 0x28) + 0x1c + param_1 * 0x28);
    if (psVar1[param_2 * 0x12 + param_3 + 0xd] == -1) {
        int iVar3 = (int)*(short *)(*(int *)((int)pThis + 0x28) + param_1 * 0x28 + 10);
        int param_1_iter = 0;
        short *psVar5 = psVar1;
        if (0 < iVar3) {
            do {
                if (param_2 != param_1_iter) {
                    int iVar4 = 0;
                    short *psVar2 = psVar5;
                    do {
                        if ((psVar2[0xd] == -1) &&
                            (psVar1[param_2 * 0x12 + param_3 + 1] == psVar5[(iVar4 + 1) % 3 + 1]) &&
                            (psVar1[param_2 * 0x12 + (param_3 + 1) % 3 + 1] == psVar2[1])) {
                            psVar1[param_2 * 0x12 + param_3 + 0xd] = (short)param_1_iter;
                            psVar5[iVar4 + 0xd] = (short)param_2;
                            return;
                        }
                        iVar4++;
                        psVar2++;
                    } while (iVar4 < 3);
                }
                param_1_iter++;
                psVar5 += 0x12;
                if (iVar3 <= param_1_iter) return;
            } while (true);
        }
    }
}

// FUN_00442260 @ 0x00442260 — BMD_InitAdjFaceTable
// For each action/bone, resets adjacency table to -1, then calls FUN_004422f0 on each face×edge.
void __cdecl FUN_00442260(void *param_1)
{
    int iVar3 = 0;
    if (0 < *(short *)((int)param_1 + 0x24)) {
        int local_4 = 0;
        do {
            int iVar1 = *(int *)((int)param_1 + 0x28) + local_4;
            int iVar4 = (int)*(short *)(iVar1 + 10);
            if (0 < iVar4) {
                // faces + 0x1a bytes (NO *2): 3 shorts de adyacencia al final de cada face 0x24-byte.
                unsigned int *puVar2 = (unsigned int *)(*(int *)(iVar1 + 0x1c) + 0x1a);
                int i = iVar4;
                do {
                    i--;
                    *puVar2 = 0xffffffff;
                    *(unsigned short *)(puVar2 + 1) = 0xffff;
                    puVar2 += 9;
                } while (i != 0);
            }
            int iVar1_ = 0;
            if (0 < iVar4) {
                do {
                    FUN_004422f0(param_1, iVar3, iVar1_, 0);
                    FUN_004422f0(param_1, iVar3, iVar1_, 1);
                    FUN_004422f0(param_1, iVar3, iVar1_, 2);
                    iVar1_++;
                } while (iVar1_ < iVar4);
            }
            iVar3++;
            local_4 += 0x28;
        } while (iVar3 < *(short *)((int)param_1 + 0x24));
    }
}

// FUN_00442e60 @ 0x00442E60 — BMD_ComputeBounds
// Computes per-bone bounding boxes by scanning vertex positions; stores into bbox arrays.
void __cdecl FUN_00442e60(int param_1)
{
    int iVar3 = 0;
    if (0 < *(short *)(param_1 + 0x22)) {
        unsigned short *puVar5 = (unsigned short *)&DAT_077d87fc;
        int iVar2 = 0;
        do {
            *puVar5 = 0;
            iVar3++;
            *(unsigned int *)((char*)&DAT_05827a98 + iVar2) = 0xc61c3c00;
            puVar5++;
            *(unsigned int *)((char*)&DAT_05827a9c + iVar2) = 0xc61c3c00;
            *(unsigned int *)((char*)&DAT_05827aa0 + iVar2) = 0xc61c3c00;
            *(unsigned int *)((char*)&DAT_06f42a5c + iVar2) = 0x461c3c00;
            *(unsigned int *)((char*)&DAT_06f42a60 + iVar2) = 0x461c3c00;
            *(unsigned int *)((char*)&DAT_06f42a64 + iVar2) = 0x461c3c00;
            iVar2 += 0xc;
        } while (iVar3 < *(short *)(param_1 + 0x22));
    }
    int local_8 = 0;
    if (0 < *(short *)(param_1 + 0x24)) {
        int local_4 = 0;
        do {
            int local_c = 0;
            int iVar3_ = local_4 + *(int *)(param_1 + 0x28);
            if (0 < *(short *)(iVar3_ + 4)) {
                int iVar2 = 0;
                do {
                    float *pfVar6 = (float *)(iVar2 + *(int *)(iVar3_ + 0x10));
                    int iVar7 = 0;
                    float *pfVar4 = pfVar6;
                    do {
                        pfVar4++;
                        int iVar1 = iVar7 + *(short *)pfVar6 * 3;
                        if (*pfVar4 < ((float*)&DAT_06f42a5c)[iVar1])
                            ((float*)&DAT_06f42a5c)[iVar1] = *pfVar4;
                        iVar1 = iVar7 + *(short *)pfVar6 * 3;
                        if (((float*)&DAT_05827a98)[iVar1] < *pfVar4)
                            ((float*)&DAT_05827a98)[iVar1] = *pfVar4;
                        iVar7++;
                    } while (iVar7 < 3);
                    iVar2 += 0x10;
                    ((unsigned short*)&DAT_077d87fc)[*(short *)pfVar6]++;
                    local_c++;
                } while (local_c < *(short *)(iVar3_ + 4));
            }
            local_8++;
            local_4 += 0x28;
        } while (local_8 < *(short *)(param_1 + 0x24));
    }
    iVar3 = 0;
    if (0 < *(short *)(param_1 + 0x22)) {
        int iVar7 = 0;
        unsigned short *psVar8 = (unsigned short *)&DAT_077d87fc;
        int iVar2 = 0;
        do {
            int iVar1 = iVar7 + *(int *)(param_1 + 0x2c);
            *(char *)(iVar1 + 0x28) = (*psVar8 == 0) ? '\0' : '\x01';
            iVar3++;
            // Copy bbox max (XYZ) and min (XYZ) — 8 corners total — into bone struct at +0x2c..+0x88
            *(unsigned int *)(iVar1 + 0x2c) = *(unsigned int *)((char*)&DAT_05827a98 + iVar2);
            *(unsigned int *)(iVar1 + 0x30) = *(unsigned int *)((char*)&DAT_05827a9c + iVar2);
            *(unsigned int *)(iVar1 + 0x34) = *(unsigned int *)((char*)&DAT_05827aa0 + iVar2);
            *(unsigned int *)(iVar1 + 0x38) = *(unsigned int *)((char*)&DAT_05827a98 + iVar2);
            *(unsigned int *)(iVar1 + 0x3c) = *(unsigned int *)((char*)&DAT_05827a9c + iVar2);
            *(unsigned int *)(iVar1 + 0x40) = *(unsigned int *)((char*)&DAT_06f42a64 + iVar2);
            *(unsigned int *)(iVar1 + 0x44) = *(unsigned int *)((char*)&DAT_05827a98 + iVar2);
            *(unsigned int *)(iVar1 + 0x48) = *(unsigned int *)((char*)&DAT_06f42a60 + iVar2);
            *(unsigned int *)(iVar1 + 0x4c) = *(unsigned int *)((char*)&DAT_05827aa0 + iVar2);
            *(unsigned int *)(iVar1 + 0x50) = *(unsigned int *)((char*)&DAT_05827a98 + iVar2);
            *(unsigned int *)(iVar1 + 0x54) = *(unsigned int *)((char*)&DAT_06f42a60 + iVar2);
            *(unsigned int *)(iVar1 + 0x58) = *(unsigned int *)((char*)&DAT_06f42a64 + iVar2);
            *(unsigned int *)(iVar1 + 0x5c) = *(unsigned int *)((char*)&DAT_06f42a5c + iVar2);
            *(unsigned int *)(iVar1 + 0x60) = *(unsigned int *)((char*)&DAT_05827a9c + iVar2);
            *(unsigned int *)(iVar1 + 100)  = *(unsigned int *)((char*)&DAT_05827aa0 + iVar2);
            *(unsigned int *)(iVar1 + 0x68) = *(unsigned int *)((char*)&DAT_06f42a5c + iVar2);
            *(unsigned int *)(iVar1 + 0x6c) = *(unsigned int *)((char*)&DAT_05827a9c + iVar2);
            *(unsigned int *)(iVar1 + 0x70) = *(unsigned int *)((char*)&DAT_06f42a64 + iVar2);
            *(unsigned int *)(iVar1 + 0x74) = *(unsigned int *)((char*)&DAT_06f42a5c + iVar2);
            *(unsigned int *)(iVar1 + 0x78) = *(unsigned int *)((char*)&DAT_06f42a60 + iVar2);
            *(unsigned int *)(iVar1 + 0x7c) = *(unsigned int *)((char*)&DAT_05827aa0 + iVar2);
            *(unsigned int *)(iVar1 + 0x80) = *(unsigned int *)((char*)&DAT_06f42a5c + iVar2);
            *(unsigned int *)(iVar1 + 0x84) = *(unsigned int *)((char*)&DAT_06f42a60 + iVar2);
            *(unsigned int *)(iVar1 + 0x88) = *(unsigned int *)((char*)&DAT_06f42a64 + iVar2);
            iVar7 += 0x8c;
            psVar8++;
            iVar2 += 0xc;
        } while (iVar3 < *(short *)(param_1 + 0x22));
    }
}

// FUN_00442e00 @ 0x00442E00 — BMD_ResetAnimState
// If flag!=0: scan action array (stride 0x8c), mark entries Du/non-Du;
// then reset frame index and call FUN_00442e60 (BMD_ComputeBounds).
void __cdecl FUN_00442e00(void *pThis, char param_1)
{
    char *pcVar1;
    int iVar2, iVar3;
    if ((param_1 != '\0') && (iVar3 = 0, 0 < *(short *)((int)pThis + 0x22))) {
        iVar2 = 0;
        do {
            pcVar1 = (char *)(*(int *)((int)pThis + 0x2c) + iVar2);
            pcVar1[0x22] = ((*pcVar1 == 'D') && (pcVar1[1] == 'u')) ? '\x01' : '\0';
            iVar3++;
            iVar2 += 0x8c;
        } while (iVar3 < *(short *)((int)pThis + 0x22));
    }
    *(unsigned int *)((int)pThis + 0x54) = 0xffffffff;
    *(unsigned char *)((int)pThis + 0x88) = 0xff;
    FUN_00442e60((int)pThis);
}

// FUN_00442a60 @ 0x00442A60 — BMD_SaveToFile
// Writes BMD model structure to binary file param_1+param_2 (concatenated paths).
// Writes header 'B'/'M'/'D', then mesh/bone/action data via FUN_005430f0 (fwrite).
undefined4 __cdecl FUN_00442a60(int thisModel, char *param_1, char *param_2)
{
    char local_40[64];
    // concatenate param_1 + param_2
    strncpy(local_40, param_1, sizeof(local_40)-1); local_40[63] = '\0';
    size_t len1 = strlen(local_40);
    strncpy(local_40 + len1, param_2, sizeof(local_40)-len1-1);
    local_40[63] = '\0';

    FILE *pFVar4 = (FILE *)FUN_0054173f(local_40, &DAT_005597d4);
    if (!pFVar4) return 0;

    FUN_00543264(0x42, (int *)pFVar4);  // 'B'
    FUN_00543264(0x4d, (int *)pFVar4);  // 'M'
    FUN_00543264(0x44, (int *)pFVar4);  // 'D'
    FUN_005430f0((char *)(thisModel + 0x20), 1, 1, (int *)pFVar4);
    FUN_005430f0((char *)thisModel, 0x20, 1, (int *)pFVar4);
    FUN_005430f0((char *)(thisModel + 0x24), 2, 1, (int *)pFVar4);
    FUN_005430f0((char *)(thisModel + 0x22), 2, 1, (int *)pFVar4);
    FUN_005430f0((char *)(thisModel + 0x26), 2, 1, (int *)pFVar4);

    // Write mesh data
    int param_1i = 0;
    if (0 < *(short *)(thisModel + 0x24)) {
        int local_50 = 0, local_48 = 0;
        do {
            int iVar8 = *(int *)(thisModel + 0x28) + local_50;
            FUN_005430f0((char *)(iVar8 + 4), 2, 1, (int *)pFVar4);
            FUN_005430f0((char *)(iVar8 + 6), 2, 1, (int *)pFVar4);
            FUN_005430f0((char *)(iVar8 + 8), 2, 1, (int *)pFVar4);
            short *psVar1 = (short *)(iVar8 + 10);
            FUN_005430f0((char *)psVar1, 2, 1, (int *)pFVar4);
            FUN_005430f0((char *)(iVar8 + 2), 2, 1, (int *)pFVar4);
            FUN_005430f0(*(char **)(iVar8 + 0x10), (int)*(short *)(iVar8 + 4) << 4, 1, (int *)pFVar4);
            FUN_005430f0(*(char **)(iVar8 + 0x14), *(short *)(iVar8 + 6) * 0x14, 1, (int *)pFVar4);
            FUN_005430f0(*(char **)(iVar8 + 0x18), (int)*(short *)(iVar8 + 8) << 3, 1, (int *)pFVar4);
            int local_4c = 0;
            if (0 < *psVar1) {
                char *param_2p = (char *)0;
                do {
                    FUN_005430f0(param_2p + *(int *)(iVar8 + 0x1c), 0x40, 1, (int *)pFVar4);
                    param_2p += 0x24;
                    local_4c++;
                } while (local_4c < *psVar1);
            }
            FUN_005430f0((char *)(*(int *)(thisModel + 0x34) + local_48), 0x20, 1, (int *)pFVar4);
            local_50 += 0x28;
            param_1i++;
            local_48 += 0x20;
        } while (param_1i < (int)*(short *)(thisModel + 0x24));
    }
    // Write action data
    param_1i = 0;
    if (0 < *(short *)(thisModel + 0x26)) {
        char *param_2p = (char *)0;
        do {
            int iVar8 = *(int *)(thisModel + 0x30);
            FUN_005430f0(param_2p + iVar8 + 8, 2, 1, (int *)pFVar4);
            FUN_005430f0(param_2p + iVar8 + 10, 1, 1, (int *)pFVar4);
            if (*(param_2p + iVar8 + 10) != '\0')
                FUN_005430f0(*(char **)(param_2p + iVar8 + 0xc), *(short *)(param_2p + iVar8 + 8) * 0xc, 1, (int *)pFVar4);
            param_1i++;
            param_2p += 0x10;
        } while (param_1i < (int)*(short *)(thisModel + 0x26));
    }
    // Write bone data
    param_1i = 0;
    if (0 < *(short *)(thisModel + 0x22)) {
        int local_50 = 0;
        do {
            char *pcVar11 = (char *)(local_50 + *(int *)(thisModel + 0x2c));
            FUN_005430f0(pcVar11 + 0x22, 1, 1, (int *)pFVar4);
            if (pcVar11[0x22] == '\0') {
                FUN_005430f0(pcVar11, 0x20, 1, (int *)pFVar4);
                FUN_005430f0(pcVar11 + 0x20, 2, 1, (int *)pFVar4);
                int iVar8 = 0, local_48 = 0;
                if (0 < *(short *)(thisModel + 0x26)) {
                    char *param_2p = (char *)0;
                    do {
                        int iVar3 = *(int *)(pcVar11 + 0x24);
                        FUN_005430f0(*(char **)(param_2p + iVar3), *(short *)(*(int *)(thisModel + 0x30) + 8 + iVar8) * 0xc, 1, (int *)pFVar4);
                        FUN_005430f0(*(char **)(param_2p + iVar3 + 4), *(short *)(*(int *)(thisModel + 0x30) + 8 + iVar8) * 0xc, 1, (int *)pFVar4);
                        param_2p += 0xc;
                        local_48++;
                        iVar8 += 0x10;
                    } while (local_48 < *(short *)(thisModel + 0x26));
                }
            }
            local_50 += 0x8c;
            param_1i++;
        } while (param_1i < (int)*(short *)(thisModel + 0x22));
    }
    FUN_0054150f(pFVar4);
    return 1;
}
