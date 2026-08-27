#include "stdafx.h"
#include "../globals.h"
#include "../functions.h"

// BMD_SkinUpdate — 0x0040b630
// Per-frame mesh skinning:
//   1. Transforms each vertex position and normal using the current bone matrices.
//   2. Re-derives per-face normals for triangle faces.
//   3. Builds de-duplicated vertex, UV, and normal index tables consumed by the
//      GL render path (material-grouped, stride 0x637d2 per mesh group).
void BMD_SkinUpdate(void)
{
    return;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    // ── Phase 1: bone vertex transform ───────────────────────────────────────
    // For each skinned vertex: scale position, map through parent-bone matrix,
    // then write world position.  Also writes vertex normal from bone frame.
    int iVar10 = 0;
    if (0 < DAT_00590c10) {
        float *pfVar14 = &DAT_055c4068;          // output world vertex array (stride 0x1b)
        float *pfVar11 = &DAT_00593008;          // source vertex array       (stride 3)
        short *psVar16 = &DAT_00590c32;          // parent bone index array   (stride 0x11)
        do {
            float local_48 = pfVar11[-1] * _DAT_0055259c;
            float local_44 = pfVar11[ 0] * _DAT_0055259c;
            float local_40 = pfVar11[ 1] * _DAT_0055259c;
            float local_3c, local_38, local_34;
            float local_30[12];

            if (*psVar16 == -1) {
                // Root bone: apply global rotation only
                Matrix_BuildFromEuler(&local_48, pfVar14 - 0xc);
                FUN_004f9e90(&local_48, pfVar14);
                // Copy pre-baked normal from source stream
                float fVar2 = pfVar11[-600];
                float fVar3 = pfVar11[-599];
                pfVar14[0xc] = pfVar11[-0x259];
                pfVar14[0xd] = fVar2;
                pfVar14[0xe] = fVar3;
            } else {
                // Child bone: build local matrix then combine with parent
                int iBone = (int)*psVar16;
                Matrix_BuildFromEuler(&local_48, local_30);
                FUN_004f9f70((float *)(&DAT_055c4038 + iBone * 0x6c), local_30, pfVar14 - 0xc);
                FUN_004f9e90(&local_48, local_30);
                FUN_004f9f70(local_30, (float *)(&DAT_055c4068 + iBone * 0x6c), pfVar14);
                // Transform normal by bone matrix, add bone world-space origin
                Vector_Transform(pfVar11 - 0x259, (float *)(&DAT_055c4038 + iBone * 0x6c), &local_3c);
                pfVar14[0xc] = local_3c + (&DAT_055c4098)[iBone * 0x1b];
                pfVar14[0xd] = local_38  + (&DAT_055c409c)[iBone * 0x1b];
                pfVar14[0xe] = local_34  + (&DAT_055c40a0)[iBone * 0x1b];
            }

            iVar10++;
            psVar16 += 0x11;   // next parent-bone entry
            pfVar11 += 3;      // next source vertex (3 floats)
            pfVar14 += 0x1b;   // next output vertex (0x1b floats)
        } while (iVar10 < DAT_00590c10);
    }

    // ── Phase 2: per-triangle normal reconstruction ───────────────────────────
    // Each triangle has 3 vertices × 9 floats.  Offset -2 bytes from pfVar11
    // holds the bone index (short).  Subtract bone origin, transform by bone
    // rotation matrix, normalize the result normal vector.
    iVar10 = 0;
    if (0 < DAT_00593968) {
        float *pfVar11 = &DAT_00608c70;
        do {
            int iVar12 = 3;   // 3 vertices per triangle
            do {
                int iVar5 = (int)*(short *)((BYTE*)pfVar11 - 2);  // bone index for this vertex
                float local_48 = pfVar11[0] - (&DAT_055c4098)[iVar5 * 0x1b];
                float local_44 = pfVar11[1] - (&DAT_055c409c)[iVar5 * 0x1b];
                float local_40 = pfVar11[2] - (&DAT_055c40a0)[iVar5 * 0x1b];
                // Transform position by bone rotation matrix
                Vector_Transform(&local_48, (float *)(&DAT_055c4068 + iVar5 * 0x6c), pfVar11);
                // Transform normal (stored at +3..+5) by same bone rotation
                local_48 = pfVar11[3];
                local_44 = pfVar11[4];
                local_40 = pfVar11[5];
                Vector_Transform(&local_48,
                             (float *)(&DAT_055c4068 + *(short *)((BYTE*)pfVar11 - 2) * 0x6c),
                             pfVar11 + 3);
                // Normalize the output normal
                FUN_004f9d60(pfVar11 + 3);
                pfVar11 += 9;   // 9 floats per vertex: bone,x,y,z,nx,ny,nz,u,v
                iVar12--;
            } while (iVar12 != 0);
            iVar10++;
        } while (iVar10 < DAT_00593968);
    }

    // ── Phase 3: build material-group vertex/UV/normal index tables ───────────
    // Reset per-mesh counters (stride 0x637d2 * sizeof(short))
    short sVar4 = (short)DAT_00593968;
    short *puVar6 = &DAT_0080a428;
    DAT_008097a0 = 0;
    do {
        puVar6[-1] = 0;   // vert count
        puVar6[ 0] = 0;   // UV count
        puVar6[ 1] = 0;   // normal count
        puVar6[ 2] = 0;   // face count
        puVar6 += 0x637d2;
    } while ((int)puVar6 < 0x55c4038);

    int local_54 = 0;
    if (0 < (int)sVar4) {
        int   local_4c = 0x3416;
        float *local_50 = &DAT_00608c70;
        BYTE  *local_58 = (BYTE *)&DAT_0059396a;

        do {
            // --- Find or create material group for this triangle's material name ---
            int iVar12 = (int)DAT_008097a0;
            int iVar10_inner = 0;
            if (0 < iVar12) {
                BYTE *pbVar13  = &DAT_008097a2;
                BYTE *pbVar7   = local_58;
                BYTE *pbVar17  = pbVar13;
            do_strcmp:
                do {
                    BYTE bVar1 = *pbVar7;
                    bool bVar18 = bVar1 < *pbVar13;
                    if (bVar1 == *pbVar13) {
                        if (bVar1 != 0) {
                            bVar1   = pbVar7[1];
                            bVar18  = bVar1 < pbVar13[1];
                            if (bVar1 != pbVar13[1]) goto cmp_result;
                            pbVar13 = pbVar13 + 2;
                            pbVar7  = pbVar7  + 2;
                            if (bVar1 != 0) goto do_strcmp;
                        }
                        goto found_group; // strings equal → group match
                    } else {
                    cmp_result:;
                        int iVar5 = (1 - (unsigned int)bVar18) - (unsigned int)(bVar18 != 0);
                        if (iVar5 == 0) goto found_group;
                    }
                    iVar10_inner++;
                    pbVar13  = pbVar17 + 0x20;
                    pbVar7   = local_58;
                    pbVar17  = pbVar13;
                } while (iVar10_inner < iVar12);
            }
            // New group — store material name into table
            *(short *)(&DAT_0080a424 + iVar12 * 0x31be9) = DAT_008097a0;
            // strcpy material name into DAT_008097a2[iVar12 * 0x20]
            {
                unsigned int uLen = 0xffffffff;
                BYTE *pbSrc = local_58;
                BYTE *pbEnd;
                BYTE bv;
                do {
                    pbEnd = pbSrc;
                    if (uLen == 0) break;
                    uLen--;
                    pbEnd = pbSrc + 1;
                    bv = *pbSrc;
                    pbSrc = pbEnd;
                } while (bv != 0);
                uLen = ~uLen; // strlen(name)+1
                BYTE *pDst = &DAT_008097a2 + iVar12 * 0x20;
                BYTE *pData = pbEnd + 1 - uLen; // = local_58
                for (unsigned int u = uLen >> 2; u != 0; u--) {
                    *(DWORD*)pDst = *(DWORD*)pData;
                    pData += 4; pDst += 4;
                }
                for (unsigned int u = uLen & 3; u != 0; u--) {
                    *pDst++ = *pData++;
                }
            }
            DAT_008097a0 = DAT_008097a0 + 1;
            iVar10_inner = iVar12;

        found_group:
            // --- For each of the 3 triangle vertices, add to index tables ---
            iVar12 = iVar10_inner * 0xc6fa4;
            int iVar5 = 0;
            do {
                // --- Vertex position lookup / insert ---
                int iVar15 = *(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2) - 1;
                if (-1 < iVar15) {
                    float *pfVar11 = (float *)(&DAT_0080a424 + iVar10_inner * 0x31be9 + iVar15 * 4 + 5);
                    do {
                        if ((*local_50 == pfVar11[-1]) &&
                            (local_50[1] == *pfVar11) &&
                            (local_50[2] == pfVar11[1])) {
                            // Found existing vertex
                            (&DAT_00879588)[iVar10_inner * 0x637d2 + iVar5 +
                                (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] = (short)iVar15;
                            goto done_vert;
                        }
                        iVar15--;
                        pfVar11 -= 4;
                    } while (-1 < iVar15);
                }
                // Vertex not found — append to buffer
                {
                    (&DAT_00879588)[iVar10_inner * 0x637d2 + iVar5 +
                        (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] =
                        *(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2);
                    *(short *)(&DAT_0080a424 + iVar10_inner * 0x31be9 +
                        *(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2) * 4 + 3) =
                        *(short *)((BYTE*)local_50 - 2); // bone index (2 bytes before x)
                    (&DAT_0080a424)[iVar10_inner * 0x31be9 +
                        (*(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2) + 1) * 4] = local_50[0];
                    (&DAT_0080a424)[iVar10_inner * 0x31be9 +
                        *(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2) * 4 + 5] = local_50[1];
                    (&DAT_0080a424)[iVar10_inner * 0x31be9 +
                        *(short *)((BYTE*)&DAT_0080a424 + iVar12 + 2) * 4 + 6] = local_50[2];
                    short *pVCnt = (short *)((BYTE*)&DAT_0080a424 + iVar12 + 2);
                    *pVCnt = *pVCnt + 1;
                }
            done_vert:

                // --- UV lookup / insert ---
                iVar15 = (short)(&DAT_0080a428)[iVar10_inner * 0x637d2] - 1;
                if (-1 < iVar15) {
                    float *pfVar11 = (float *)(&DAT_0080a424 + iVar10_inner * 0x31be9 + iVar15 * 5 + 0x9c45);
                    do {
                        if ((local_50[3] == pfVar11[-1]) &&
                            (local_50[4] == *pfVar11) &&
                            (local_50[5] == pfVar11[1])) {
                            (&DAT_00896a48)[iVar10_inner * 0x637d2 + iVar5 +
                                (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] = (short)iVar15;
                            goto done_uv;
                        }
                        iVar15--;
                        pfVar11 -= 5;
                    } while (-1 < iVar15);
                }
                // UV not found — append
                {
                    short uvIdx = (&DAT_0080a428)[iVar10_inner * 0x637d2];
                    (&DAT_00896a48)[iVar10_inner * 0x637d2 + iVar5 +
                        (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] = uvIdx;
                    (&DAT_00831530)[iVar10_inner * 0x637d2 + uvIdx * 10] =
                        *(short *)((BYTE*)local_50 - 2);
                    (&DAT_00831534)[iVar10_inner * 0x31be9 + uvIdx * 5] = local_50[3];
                    (&DAT_0080a424)[iVar10_inner * 0x31be9 + (uvIdx + 0x1f41) * 5] =
                        *(float *)((BYTE*)&DAT_00593968 + (local_4c + iVar5) * 0x12);
                    (&DAT_0083153c)[iVar10_inner * 0x31be9 + uvIdx * 5] = local_50[5];
                    (&DAT_00831540)[iVar10_inner * 0x637d2 + uvIdx * 10] =
                        (&DAT_00879588)[iVar10_inner * 0x637d2 + iVar5 +
                            (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4];
                    (&DAT_0080a428)[iVar10_inner * 0x637d2] = uvIdx + 1;
                }
            done_uv:

                // --- Normal lookup / insert ---
                iVar15 = (short)(&DAT_0080a42a)[iVar10_inner * 0x637d2] - 1;
                if (-1 < iVar15) {
                    float *pfVar11 = (float *)(&DAT_00862270 + iVar10_inner * 0x31be9 + iVar15 * 2);
                    do {
                        if ((local_50[6] == *pfVar11) && (local_50[7] == pfVar11[1])) {
                            (&DAT_008b3f08)[iVar10_inner * 0x637d2 + iVar5 +
                                (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] = (short)iVar15;
                            goto done_norm;
                        }
                        iVar15--;
                        pfVar11 -= 2;
                    } while (-1 < iVar15);
                }
                // Normal not found — append
                {
                    short nIdx = (&DAT_0080a42a)[iVar10_inner * 0x637d2];
                    (&DAT_008b3f08)[iVar10_inner * 0x637d2 + iVar5 +
                        (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] * 4] = nIdx;
                    (&DAT_00862270)[iVar10_inner * 0x31be9 + nIdx * 2] = local_50[6];
                    (&DAT_00862274)[iVar10_inner * 0x31be9 + nIdx * 2] = local_50[7];
                    (&DAT_0080a42a)[iVar10_inner * 0x637d2] = nIdx + 1;
                }
            done_norm:
                iVar5++;
                local_50 += 9;   // next vertex (9 floats)
            } while (iVar5 < 3);

            local_4c += 3;
            // Mark this face as having 3 vertices in the face count byte
            *((BYTE *)&DAT_0080a424 +
              (short)(&DAT_0080a42c)[iVar10_inner * 0x637d2] + 0x6b6cc + iVar12) = 3;
            (&DAT_0080a42c)[iVar10_inner * 0x637d2] =
                (&DAT_0080a42c)[iVar10_inner * 0x637d2] + 1;

            local_58 += 0x20;   // next material name slot
            local_54++;
        } while (local_54 < (int)sVar4);
    }
}
