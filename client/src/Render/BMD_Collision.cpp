// BMD_Collision.cpp
// Colisión línea/malla de los modelos BMD (BMD__CollisionDetectLineToMesh).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// BMD::CollisionDetectLineToMesh @ 0x00440BE0 (~101 lines) — line-mesh collision test
// Iterates all meshes and triangles. Per tri: FaceNormalize, CollisionDetectLineToFace.
bool __cdecl BMD__CollisionDetectLineToMesh(BMD* pThis, float Position[3], float Target[3], bool Collision, int Mesh, int Triangle) {
    // 0x00440BE0 — Line-mesh collision test (__thiscall in original, pThis = this pointer)
    // Iterates all meshes in the BMD model. For each mesh, iterates all triangles.
    // Per triangle: compute face normal via FaceNormalize, then test line-to-face intersection
    // via CollisionDetectLineToFace. Returns true on first hit.
    //
    // BMD struct layout (from this pointer):
    //   +0x24: short NumMeshes
    //   +0x28: int*  MeshArray (pointer to mesh data)
    // Each mesh entry (stride 0x28 = 40 bytes):
    //   +0x0A: short NumTriangles
    //   +0x1C: int*  TriangleArray (pointer to triangle data)
    // Each triangle (stride 0x24 = 36 bytes):
    //   +0x00: char  Polygon (cantidad de vértices)
    //   +0x02: short vertIndex0
    //   +0x04: short vertIndex1
    //   +0x06: short vertIndex2
    //   +0x08: short vertIndex3
    // Vertex buffer: DAT_0584621c (float[3] per entry, stride 12 bytes)
    // Each mesh adds 15000 to the vertex base index offset.


    auto* meshes = reinterpret_cast<const ClothModelMesh*>(pThis->Meshs);
    for (int mesh = 0; mesh < pThis->NumMeshs; ++mesh) {
        const auto& entry = meshes[mesh];
        for (int triangle = 0; triangle < entry.TriangleCount; ++triangle) {
            // IDA: omitir sólo la pareja completa, no una malla o cara por separado.
            if (mesh == Mesh && triangle == Triangle) continue;
            const auto* indices = reinterpret_cast<const short*>(entry.Triangles +
                triangle * sizeof(ModelRuntimeTriangle) + 2);
            auto* worldVertices = reinterpret_cast<float*>(&DAT_0584621c);
            float* corners[4];
            for (int corner = 0; corner < 4; ++corner)
                corners[corner] = worldVertices + (mesh * 15000 + indices[corner]) * 3;
            float normal[3];
            FaceNormalize(corners[0], corners[1], corners[2], normal);
            const char polygon = *reinterpret_cast<const char*>(entry.Triangles +
                triangle * sizeof(ModelRuntimeTriangle));
            if (CollisionDetectLineToFace(Position, Target, polygon,
                    corners[0], corners[1], corners[2], corners[3], normal, (char)Collision))
                return true;
        }
    }
    return false;
}
