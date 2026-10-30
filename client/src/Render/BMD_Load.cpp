// BMD_Load.cpp
//
// BMD (Mu Online 3D model format) loaders:
//   BMD__Open (BMD::Open)              — load compressed BMD file into model slot
//   BMD__FindTriangleForEdge (BMD_BuildAdjacentFaceTable)
//   BMD__FindNearTriangle (BMD_InitAdjFaceTable)
//   BMD_CreateBoundingBox (BMD_ComputeBounds)
//   BMD__Init (BMD_ResetAnimState)
//   BMD__Save (BMD_SaveToFile)

#include "stdafx.h"
#include "Platform/Text.h"
#include "globals.h"
#include "functions.h"
#include "Model/BMDFileTypes.h"

extern "C" void DbgLogPublic(const char* msg);

// ── BMD loaders ──────────────────────────────────────────────────────────────
// BMD::Open @0x004423E0: archivo normalizado con mallas(+36), huesos(+38), acciones(+40).
// Cabeceras de disco sin punteros; tablas de ejecución con tipos nativos.
// Se conservan v10/v12 y las inicializaciones defensivas del port existente.
void __cdecl BMD__Open(BMD* param_1, const char* param_2, const char* param_3, int param_4)
{
    (void)param_4;
    BMD* model = param_1;
    void* thisPtr = model;
    const char *path     = param_2;
    const char *filename = param_3;

    // VERY-EARLY defensive BodyLight init: done BEFORE fopen so even if file
    // can't be opened the slot has sane (1,1,1) instead of the pre-existing
    // garbage pattern (0xF5F5F5F5 from the random model pool offset).
    model->BodyLight[0] = 1.0f;
    model->BodyLight[1] = 1.0f;
    model->BodyLight[2] = 1.0f;

    char local_4c[64];
    crt_sprintf(local_4c, "%s%s", path, filename);

    // Unconditional entry log for Logo/Ship so we can confirm we actually
    // reach this function for those slots and know what thisPtr they use.
    {
        if (strstr(local_4c, "Logo") || strstr(local_4c, "Ship")) {
        }
    }

    FILE *fp = crt_fopen(local_4c, DAT_005580ac);
    if (!fp) {
        return;
    }

    fseek(fp, 0, SEEK_END);
    int fileSize = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fileSize < 0x2a) {
        char diag[200];
        Platform::TextFormatTruncated(diag, sizeof(diag), Platform::TruncateText, "BMD::Open too-small: %s size=%d", local_4c, fileSize);
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
        Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText,
            "BMD skip (unsupported): %s ver=0x%02x size=%d",
            local_4c, verByte, fileSize);
        DbgLogPublic(line);
        operator_delete(Buffer);
        return;
    }

    if (verByte == 0x0C) {
        if (fileSize < 0x2a + 4) {
            char line[160];
            Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText,
                "BMD v12 too-small: %s size=%d", local_4c, fileSize);
            DbgLogPublic(line);
            operator_delete(Buffer);
            return;
        }
        std::int32_t encSize;
        memcpy(&encSize, Buffer + 4, sizeof(encSize));
        if (encSize <= 0 || encSize > fileSize - 8) {
            char line[160];
            Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText,
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

    BmdFile::Header fileHeader;
    memcpy(&fileHeader, Buffer, sizeof(fileHeader));
    model->Version = fileHeader.Version;
    // El nombre se lee de Buffer+4 (Ghidra: puVar7+1 antes del body).
    memcpy(model->Name, fileHeader.Name, sizeof(fileHeader.Name));

    // EARLY defensive BodyLight init: write (1,1,1) BEFORE any parsing path that
    // could early-return. Any subsequent type-specific caller will overwrite as
    // needed (BMD_SetupRender, Entity_DrawByType), but the slot never holds the
    // uninitialized CD/FD pattern that manifested as -1.695e+37 garbage on Logo01.
    model->BodyLight[0] = 1.0f;
    model->BodyLight[1] = 1.0f;
    model->BodyLight[2] = 1.0f;

    const short numMeshes = fileHeader.MeshCount;
    const short numBones = fileHeader.BoneCount;
    const short numActions = fileHeader.ActionCount;

    // Sanity check: counts razonables
    if (numMeshes < 0 || numMeshes > 512 ||
        numBones < 0 || numBones > 512 ||
        numActions  < 0 || numActions  > 512) {
        model->NumBones = 0;
        model->NumMeshs = 0;
        model->NumActions = 0;
        model->Meshs = nullptr;
        model->Bones = nullptr;
        model->Actions = nullptr;
        model->Textures = nullptr;
        model->IndexTexture = nullptr;
        {
            char line[200];
            Platform::TextFormatTruncated(line, sizeof(line), Platform::TruncateText,
                "BMD_SanityFail: %s nMesh=%d nAction=%d nBone=%d  thisPtr=%p",
                local_4c, (int)numMeshes, (int)numBones, (int)numActions, thisPtr);
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
        if (!isPlayer && s_parsed < 200) {
            s_parsed++;
        }
    }

    model->NumMeshs = numMeshes;
    model->NumBones = numBones;
    model->NumActions = numActions;

    // Reservas de ejecución, separadas de las cabeceras de archivo.
    const int nM = numMeshes > 0 ? numMeshes : 1;
    const int nB = numBones > 0 ? numBones : 1;
    const int nA = numActions > 0 ? numActions : 1;
    model->Meshs = static_cast<BYTE*>(operator_new(nM * sizeof(ClothModelMesh)));
    model->Bones = static_cast<BYTE*>(operator_new(nB * sizeof(ModelAnimationBone)));
    model->Actions = static_cast<BYTE*>(operator_new(nA * sizeof(ModelAnimationAction)));
    model->Textures = static_cast<BYTE*>(operator_new(nM * 32));
    model->IndexTexture = static_cast<short*>(operator_new(nM * sizeof(std::int16_t)));
    int cursor = sizeof(BmdFile::Header);

    // Caras de archivo: 64 bytes; vista de ejecución: 36 bytes.
    for (int m = 0; m < numMeshes; ++m) {
        auto* mesh = reinterpret_cast<ClothModelMesh*>(model->Meshs) + m;
        BmdFile::MeshHeader header;
        memcpy(&header, Buffer + cursor, sizeof(header)); cursor += sizeof(header);
        mesh->VertexCount = header.VertexCount;
        mesh->NormalCount = header.NormalCount;
        mesh->TexCoordCount = header.TexCoordCount;
        mesh->TriangleCount = header.TriangleCount;
        mesh->TextureIndex = header.TextureIndex;
        mesh->Unknown0[0] = 0;
        const int vertexBytes = header.VertexCount * BmdFile::VertexBytes;
        const int normalBytes = header.NormalCount * BmdFile::NormalBytes;
        const int texCoordBytes = header.TexCoordCount * BmdFile::TexCoordBytes;
        mesh->Vertices = static_cast<BYTE*>(operator_new(vertexBytes));
        mesh->Normals = operator_new(normalBytes);
        mesh->TexCoords = operator_new(texCoordBytes);
        mesh->Triangles = static_cast<BYTE*>(operator_new(header.TriangleCount * sizeof(ModelRuntimeTriangle)));
        memcpy(mesh->Vertices, Buffer + cursor, vertexBytes); cursor += vertexBytes;
        memcpy(mesh->Normals, Buffer + cursor, normalBytes); cursor += normalBytes;
        memcpy(mesh->TexCoords, Buffer + cursor, texCoordBytes); cursor += texCoordBytes;
        for (int f = 0; f < header.TriangleCount; ++f) {
            memcpy(mesh->Triangles + f * sizeof(ModelRuntimeTriangle), Buffer + cursor, sizeof(ModelRuntimeTriangle));
            cursor += BmdFile::TriangleBytes;
        }
        char* texSlot = reinterpret_cast<char*>(model->Textures) + m * 32;
        memcpy(texSlot, Buffer + cursor, 32); cursor += 32;
        // TextureScript parse (BMD::Open + parsingTScript @ 0x40C190, fiel a IDA).
        // El nombre de textura puede llevar un marcador `_XYZ` (tras '_') con chars
        // R/H/S/N que activan flags de render por-mesh:
        //   R→script[0] (blend aditivo/glow)   H→script[1] (hidden)
        //   S→script[2] (wave/scroll)          N→script[3] (no-depth)
        // parsingTScript aborta (sin script) si tras '_' hay algún char no R/H/S/N.
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
                mesh->TextureFlags = strObj;
            } else {
                mesh->TextureFlags = nullptr;
            }
        }
    }

    BMD__FindNearTriangle(thisPtr);

    // Acciones: cabeceras de archivo de tres bytes, sin punteros.
    auto* actions = reinterpret_cast<ModelAnimationAction*>(model->Actions);
    for (int action = 0; action < numActions; ++action) {
        BmdFile::ActionHeader header;
        memcpy(&header, Buffer + cursor, sizeof(header)); cursor += sizeof(header);
        actions[action].Unknown0[0] = 0;
        actions[action].FrameCount = header.FrameCount;
        actions[action].LockPositions = header.LockPositions;
        if (header.LockPositions != 0) {
            const int bytes = header.FrameCount * BmdFile::VectorBytes;
            actions[action].Positions = static_cast<float*>(operator_new(bytes));
            memcpy(actions[action].Positions, Buffer + cursor, bytes); cursor += bytes;
        }
    }
    // Huesos y claves: mismo orden de lectura y de reservas que en x86.
    auto* bones = reinterpret_cast<ModelAnimationBone*>(model->Bones);
    for (int bone = 0; bone < numBones; ++bone) {
        bones[bone].Dummy = Buffer[cursor++];
        if (bones[bone].Dummy == 0) {
            BmdFile::BoneHeader header;
            memcpy(&header, Buffer + cursor, sizeof(header)); cursor += sizeof(header);
            memcpy(bones[bone].Name, header.Name, sizeof(header.Name));
            bones[bone].Parent = header.Parent;
            bones[bone].Keys = static_cast<ModelAnimationKeys*>(operator_new(numActions * sizeof(ModelAnimationKeys)));
            for (int action = 0; action < numActions; ++action) {
                auto& entry = bones[bone].Keys[action];
                const int frames = actions[action].FrameCount;
                const int bytes = frames * BmdFile::VectorBytes;
                entry.Positions = static_cast<float*>(operator_new(bytes));
                entry.EulerAngles = static_cast<float*>(operator_new(bytes));
                entry.Quaternions = static_cast<float*>(operator_new(frames * 4 * sizeof(float)));
                memcpy(entry.Positions, Buffer + cursor, bytes); cursor += bytes;
                memcpy(entry.EulerAngles, Buffer + cursor, bytes); cursor += bytes;
                for (int frame = 0; frame < frames; ++frame)
                    EulerToQuat(entry.EulerAngles + frame * 3, entry.Quaternions + frame * 4, 0, 0);
            }
        }
    }

    // Defensive BodyLight init: BMD_SetupRenderByType's default path calls
    // BMD__RenderBody without writing +0x48..+0x50, so they'd read whatever was
    // left in memory (observed: R=1.0, G/B=-1.69e37 garbage → negative alpha
    // triangles). Default to white (1,1,1) so BMD_DrawMesh reads sane color
    // for models that never go through a type-specific colorization branch
    // (ships, logos, UI items, etc.). Callers that need colored models will
    // overwrite these fields before BMD__RenderBody as in BMD_SetupRender.cpp.
    model->BodyLight[0] = 1.0f;
    model->BodyLight[1] = 1.0f;
    model->BodyLight[2] = 1.0f;

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
            float r = model->BodyLight[0];
            float g = model->BodyLight[1];
            float b = model->BodyLight[2];
            s_end++;
        }
    }

    operator_delete(Buffer);
    BMD__Init(thisPtr, '\0');
}

// BMD__FindTriangleForEdge @ 0x004422F0 — BMD_BuildAdjacentFaceTable
// For each face in bone param_1, and each edge param_3 (0-2): finds the adjacent face sharing
// the flipped edge and stores the adjacency index in psVar1[faceIdx*0x12 + edgeIdx + 0xd].
void __cdecl BMD__FindTriangleForEdge(void *pThis, int meshIndex, int faceIndex, int edge)
{
    auto* model = AsModel(pThis);
    auto& mesh = reinterpret_cast<ClothModelMesh*>(model->Meshs)[meshIndex];
    auto* triangles = reinterpret_cast<ModelRuntimeTriangle*>(mesh.Triangles);
    auto& face = triangles[faceIndex];
    if (face.Adjacent[edge] != -1) return;
    for (int other = 0; other < mesh.TriangleCount; ++other) {
        if (other == faceIndex) continue;
        auto& candidate = triangles[other];
        for (int candidateEdge = 0; candidateEdge < 3; ++candidateEdge) {
            if (candidate.Adjacent[candidateEdge] == -1 &&
                face.VertexIndex[edge] == candidate.VertexIndex[(candidateEdge + 1) % 3] &&
                face.VertexIndex[(edge + 1) % 3] == candidate.VertexIndex[candidateEdge]) {
                face.Adjacent[edge] = static_cast<short>(other);
                candidate.Adjacent[candidateEdge] = static_cast<short>(faceIndex);
                return;
            }
        }
    }
}

// BMD__FindNearTriangle @ 0x00442260 — BMD_InitAdjFaceTable
// For each action/bone, resets adjacency table to -1, then calls BMD__FindTriangleForEdge on each face×edge.
void __cdecl BMD__FindNearTriangle(void *pThis)
{
    auto* model = AsModel(pThis);
    auto* meshes = reinterpret_cast<ClothModelMesh*>(model->Meshs);
    for (int m = 0; m < model->NumMeshs; ++m) {
        if (meshes[m].TriangleCount <= 0) continue;
        auto* triangles = reinterpret_cast<ModelRuntimeTriangle*>(meshes[m].Triangles);
        for (int f = 0; f < meshes[m].TriangleCount; ++f)
            for (int edge = 0; edge < 3; ++edge) triangles[f].Adjacent[edge] = -1;
        for (int f = 0; f < meshes[m].TriangleCount; ++f)
            for (int edge = 0; edge < 3; ++edge) BMD__FindTriangleForEdge(pThis, m, f, edge);
    }
}

// BMD_CreateBoundingBox @ 0x00442E60 — BMD_ComputeBounds
// Computes per-bone bounding boxes by scanning vertex positions; stores into bbox arrays.
void __cdecl BMD_CreateBoundingBox(BMD* model)
{
    // IDA 0x00442E60: conservar centinelas, comparaciones y contador de 16 bits.
    auto* counts = reinterpret_cast<std::uint16_t*>(DAT_077d87fc);
    float* maximum = DAT_05827a98;
    float* minimum = DAT_06f42a5c;
    constexpr std::uint32_t lowerBits = 0xc61c3c00, upperBits = 0x461c3c00;
    for (int b = 0; b < model->NumBones; ++b) {
        counts[b] = 0;
        for (int axis = 0; axis < 3; ++axis) {
            memcpy(maximum + b * 3 + axis, &lowerBits, sizeof(float));
            memcpy(minimum + b * 3 + axis, &upperBits, sizeof(float));
        }
    }
    const ClothModelMesh* meshes = model->NumMeshs > 0 ? reinterpret_cast<const ClothModelMesh*>(model->Meshs) : nullptr;
    for (int m = 0; m < model->NumMeshs; ++m) {
        if (meshes[m].VertexCount <= 0) continue;
        const auto* vertices = reinterpret_cast<const ModelRuntimeVertex*>(meshes[m].Vertices);
        for (int v = 0; v < meshes[m].VertexCount; ++v) {
            const auto& vertex = vertices[v];
            for (int axis = 0; axis < 3; ++axis) {
                const int index = vertex.Bone * 3 + axis;
                if (vertex.Position[axis] < minimum[index]) minimum[index] = vertex.Position[axis];
                if (maximum[index] < vertex.Position[axis]) maximum[index] = vertex.Position[axis];
            }
            ++counts[vertex.Bone];
        }
    }
    auto* bones = model->NumBones > 0 ? reinterpret_cast<ModelAnimationBone*>(model->Bones) : nullptr;
    constexpr unsigned corners[8] = {7, 3, 5, 1, 6, 2, 4, 0};
    for (int b = 0; b < model->NumBones; ++b) {
        bones[b].BoundingBoxEnabled = counts[b] == 0 ? 0 : 1;
        for (int corner = 0; corner < 8; ++corner)
            for (int axis = 0; axis < 3; ++axis) {
                const float* source = (corners[corner] & (1u << axis)) ? maximum : minimum;
                // El original copia DWORD: conservar cargas NaN y ceros con signo.
                memcpy(&bones[b].BoundingBox[corner][axis], source + b * 3 + axis, sizeof(float));
            }
    }
}

// BMD__Init @ 0x00442E00 — BMD_ResetAnimState
// If flag!=0: scan action array (stride 0x8c), mark entries Du/non-Du;
// then reset frame index and call BMD_CreateBoundingBox (BMD_ComputeBounds).
void __cdecl BMD__Init(void *pThis, char flag)
{
    auto* model = AsModel(pThis);
    if (flag != 0 && model->NumBones > 0) {
        auto* bones = reinterpret_cast<ModelAnimationBone*>(model->Bones);
        for (int b = 0; b < model->NumBones; ++b)
            bones[b].Dummy = bones[b].Name[0] == 'D' && bones[b].Name[1] == 'u' ? 1 : 0;
    }
    model->BoneHead = -1;
    const BYTE unset = 0xff;
    memcpy(&model->StreamMesh, &unset, sizeof(unset));
    BMD_CreateBoundingBox(model);
}

// BMD__Save @ 0x00442A60 — BMD_SaveToFile
// Writes BMD model structure to binary file param_1+param_2 (concatenated paths).
// Writes header 'B'/'M'/'D', then mesh/bone/action data via crt_fwrite (fwrite).
undefined4 __cdecl BMD__Save(int thisModel, char *param_1, char *param_2)
{
    char local_40[64];
    // concatenate param_1 + param_2
    strncpy(local_40, param_1, sizeof(local_40)-1); local_40[63] = '\0';
    size_t len1 = strlen(local_40);
    strncpy(local_40 + len1, param_2, sizeof(local_40)-len1-1);
    local_40[63] = '\0';

    FILE *pFVar4 = (FILE *)crt_fopen(local_40, &DAT_005597d4);
    if (!pFVar4) return 0;

    putc(0x42, (int *)pFVar4);  // 'B'
    putc(0x4d, (int *)pFVar4);  // 'M'
    putc(0x44, (int *)pFVar4);  // 'D'
    crt_fwrite((char *)(thisModel + 0x20), 1, 1, (int *)pFVar4);
    crt_fwrite((char *)thisModel, 0x20, 1, (int *)pFVar4);
    crt_fwrite((char *)(thisModel + 0x24), 2, 1, (int *)pFVar4);
    crt_fwrite((char *)(thisModel + 0x22), 2, 1, (int *)pFVar4);
    crt_fwrite((char *)(thisModel + 0x26), 2, 1, (int *)pFVar4);

    // Write mesh data
    int param_1i = 0;
    if (0 < AsModel(thisModel)->NumMeshs) {
        int local_50 = 0, local_48 = 0;
        do {
            int iVar8 = *(int *)&AsModel(thisModel)->Meshs + local_50;
            crt_fwrite((char *)(iVar8 + 4), 2, 1, (int *)pFVar4);
            crt_fwrite((char *)(iVar8 + 6), 2, 1, (int *)pFVar4);
            crt_fwrite((char *)(iVar8 + 8), 2, 1, (int *)pFVar4);
            short *psVar1 = (short *)(iVar8 + 10);
            crt_fwrite((char *)psVar1, 2, 1, (int *)pFVar4);
            crt_fwrite((char *)(iVar8 + 2), 2, 1, (int *)pFVar4);
            crt_fwrite(*(char **)(iVar8 + 0x10), (int)*(short *)(iVar8 + 4) << 4, 1, (int *)pFVar4);
            crt_fwrite(*(char **)(iVar8 + 0x14), *(short *)(iVar8 + 6) * 0x14, 1, (int *)pFVar4);
            crt_fwrite(*(char **)(iVar8 + 0x18), (int)*(short *)(iVar8 + 8) << 3, 1, (int *)pFVar4);
            int local_4c = 0;
            if (0 < *psVar1) {
                char *param_2p = (char *)0;
                do {
                    crt_fwrite(param_2p + *(int *)(iVar8 + 0x1c), 0x40, 1, (int *)pFVar4);
                    param_2p += 0x24;
                    local_4c++;
                } while (local_4c < *psVar1);
            }
            crt_fwrite((char *)(*(int *)&AsModel(thisModel)->Textures + local_48), 0x20, 1, (int *)pFVar4);
            local_50 += 0x28;
            param_1i++;
            local_48 += 0x20;
        } while (param_1i < (int)AsModel(thisModel)->NumMeshs);
    }
    // Write action data
    param_1i = 0;
    if (0 < AsModel(thisModel)->NumActions) {
        char *param_2p = (char *)0;
        do {
            int iVar8 = *(int *)&AsModel(thisModel)->Actions;
            crt_fwrite(param_2p + iVar8 + 8, 2, 1, (int *)pFVar4);
            crt_fwrite(param_2p + iVar8 + 10, 1, 1, (int *)pFVar4);
            if (*(param_2p + iVar8 + 10) != '\0')
                crt_fwrite(*(char **)(param_2p + iVar8 + 0xc), *(short *)(param_2p + iVar8 + 8) * 0xc, 1, (int *)pFVar4);
            param_1i++;
            param_2p += 0x10;
        } while (param_1i < (int)AsModel(thisModel)->NumActions);
    }
    // Write bone data
    param_1i = 0;
    if (0 < AsModel(thisModel)->NumBones) {
        int local_50 = 0;
        do {
            char *pcVar11 = (char *)(local_50 + *(int *)&AsModel(thisModel)->Bones);
            crt_fwrite(pcVar11 + 0x22, 1, 1, (int *)pFVar4);
            if (pcVar11[0x22] == '\0') {
                crt_fwrite(pcVar11, 0x20, 1, (int *)pFVar4);
                crt_fwrite(pcVar11 + 0x20, 2, 1, (int *)pFVar4);
                int iVar8 = 0, local_48 = 0;
                if (0 < AsModel(thisModel)->NumActions) {
                    char *param_2p = (char *)0;
                    do {
                        int iVar3 = *(int *)(pcVar11 + 0x24);
                        crt_fwrite(*(char **)(param_2p + iVar3), *(short *)(*(int *)&AsModel(thisModel)->Actions + 8 + iVar8) * 0xc, 1, (int *)pFVar4);
                        crt_fwrite(*(char **)(param_2p + iVar3 + 4), *(short *)(*(int *)&AsModel(thisModel)->Actions + 8 + iVar8) * 0xc, 1, (int *)pFVar4);
                        param_2p += 0xc;
                        local_48++;
                        iVar8 += 0x10;
                    } while (local_48 < AsModel(thisModel)->NumActions);
                }
            }
            local_50 += 0x8c;
            param_1i++;
        } while (param_1i < (int)AsModel(thisModel)->NumBones);
    }
    crt_fclose(pFVar4);
    return 1;
}
