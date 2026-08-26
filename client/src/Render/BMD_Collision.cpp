// BMD_Collision.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// BMD::CollisionDetectLineToMesh @ 0x00440BE0 (~101 lines) — line-mesh collision test
// Iterates all meshes and triangles. Per tri: FaceNormalize, CollisionDetectLineToFace.
bool __cdecl BMD__CollisionDetectLineToMesh_stub(DWORD pThis, float Position[3], float Target[3], bool Collision, int Mesh, int Triangle) {
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
    //   +0x00: char  textureIndex
    //   +0x02: short vertIndex0
    //   +0x04: short vertIndex1
    //   +0x06: short vertIndex2
    //   +0x08: short normalIndex
    // Vertex buffer: DAT_0584621c (float[3] per entry, stride 12 bytes)
    // Each mesh adds 15000 to the vertex base index offset.

    (void)Collision; (void)Mesh; (void)Triangle;  // phantom params from Ghidra

    int meshIdx = 0;
    int vertBaseOffset = 0;  // accumulates +15000 per mesh
    float meshCounter = 0.0f;

    short numMeshes = *(short*)((int)pThis + 0x24);
    if (numMeshes <= 0) return false;

    int meshArrayOffset = 0;  // offset into mesh array (stride 0x28)
    do {
        int meshPtr = *(int*)((int)pThis + 0x28) + meshArrayOffset;
        short numTris = *(short*)(meshPtr + 0x0A);
        float triCounter = 0.0f;

        if (numTris > 0) {
            float triByteOffset = 0.0f;
            float prevMeshCounter = meshCounter;

            do {
                // Skip degenerate check: if both mesh and tri counters match Target, skip
                if (meshCounter != Target[0] || triCounter != Target[1]) {
                    int triPtr = *(int*)(meshPtr + 0x1C) + (int)triByteOffset;

                    short vi0 = *(short*)(triPtr + 2);
                    short vi1 = *(short*)(triPtr + 4);
                    short vi2 = *(short*)(triPtr + 6);
                    short ni  = *(short*)(triPtr + 8);

                    float* v0 = (float*)(&DAT_0584621c + (vi0 + vertBaseOffset) * 3);
                    float* v1 = (float*)(&DAT_0584621c + (vi1 + vertBaseOffset) * 3);
                    float* v2 = (float*)(&DAT_0584621c + (vi2 + vertBaseOffset) * 3);
                    float* nv = (float*)(&DAT_0584621c + (ni  + vertBaseOffset) * 3);

                    // Compute face normal
                    float normal[3];
                    float verts[3] = { *(float*)v0, *(float*)v1, *(float*)v2 };  // placeholder
                    FaceNormalize(v0, v1, v2, normal);

                    // Test line-to-face collision
                    float localC[3];
                    bool hit = CollisionDetectLineToFace(
                        Position,                       // line start
                        v0,                             // face verts packed
                        (int)nv,                        // normal vertex index
                        localC,                         // output collision point
                        &Position[2],                   // Z component
                        v0, v1,                         // additional vert ptrs
                        normal,                         // face normal
                        (char)*(char*)triPtr             // texture index as flag
                    );

                    meshCounter = prevMeshCounter;
                    if (hit) {
                        return true;
                    }
                }
                triCounter = (float)((int)triCounter + 1);
                triByteOffset = (float)((int)triByteOffset + 0x24);
            } while ((int)triCounter < (int)numTris);
        }

        meshCounter = (float)((int)meshCounter + 1);
        meshArrayOffset += 0x28;
        vertBaseOffset += 15000;
    } while ((int)meshCounter < (int)*(short*)((int)pThis + 0x24));

    return false;
}
