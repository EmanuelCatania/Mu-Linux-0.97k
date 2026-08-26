// SMD_Legacy.cpp
// Extracted from stubs_bulk_misc.cpp.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// ParseNodes @ 0x0040AB90 (~207 lines) — SMD parser: parse "nodes" section
// Reads SMD file tokens via FUN_0040b350. Scans for "nodes" keyword, then parses each node entry:
// copies node name (stride 0x22) to DAT_00590c12 array, reads parent bone index as short.
// Then scans for "skeleton" keyword for next section. Increments DAT_00590c10 (node count).
void __cdecl ParseNodes(void) {
    // Skip tokens until "nodes" keyword found (compare with DAT_0055908c)
    // For each node until '}' (token type 2):
    //   If token type 0 and != '}': increment DAT_00590c10
    //   If token type 1: copy name to DAT_00590c12[nodeIdx*0x22], read parent index
    // Then scan for "skeleton" keyword
    // stub: 207 lines of token parsing
}

// ParseSkeleton @ 0x0040AE10 (~118 lines) — SMD parser: parse "skeleton" section
// Reads skeleton frames. Scans for "skeleton" keyword, then reads frame data:
// DAT_00794490 = frame count. For each frame: reads time value, then per-node
// reads 6 floats (3 position + 3 rotation) into bone arrays at DAT_0079449c (stride 0x4B0).
void __cdecl ParseSkeleton(void) {
    // Skip tokens until "skeleton"
    // DAT_00794490 = 0
    // For each frame until '}':
    //   If "Ctime" token: read time, then per node (DAT_00590c10 nodes):
    //     Read 6 values via FUN_0040b350 -> DAT_00809798
    //     Store position[3] + rotation[3] per bone into DAT_0079449c + frame*0x4B0
    //   DAT_00794490++
}

// ParseTriangles @ 0x0040AFB0 (~156 lines) — SMD parser: parse "triangles" section
// Scans for "triangles" keyword. For each triangle: reads material name (stride 0x20)
// into DAT_0059396a, then reads 3 vertices (each: bone index as short + 8 floats for
// pos/normal/UV). Supports Flip parameter to reverse vertex winding.
// DAT_00593968 = triangle count.
void __cdecl ParseTriangles(bool Flip) {
    (void)Flip;
    // Skip tokens until "triangles"
    // DAT_00593968 = 0
    // For each triangle until '}':
    //   Copy material name to DAT_0059396a[triIdx*0x20]
    //   For each of 3 vertices (order depends on Flip):
    //     Read bone index (short), 3 pos floats, 3 normal floats, 2 UV floats
    //     Store into DAT_00608c6c arrays (stride 0x12 shorts)
    //   DAT_00593968++
}

// FUN_0040b350 @ 0x0040B350 (~136 lines) — SMD tokenizer: read next token from SMD file
// Reads from global SMDFile (FILE*). Skips whitespace and // comments.
// Token types: 0=string (stored in DAT_00590b10), 1=number (stored in DAT_00809798 as float),
// 2=EOF, 0x23='#', 0x2c=',', 0x3b=';', 0x7b='{', 0x7d='}'.
// Handles quoted strings (delimited by '"'), numbers with sign/decimal.
int __stdcall FUN_0040b350(void) {
    // Read char from SMDFile via FUN_0054218a
    // Skip whitespace (FUN_005420dd), skip // comments
    // Switch on char:
    //   '"': read quoted string into DAT_00590b10, return 0
    //   '#',',',';','{','}': return char code
    //   '-','.',0-9: read number, parse via FUN_00542133 -> DAT_00809798, return 1
    //   else: read identifier into DAT_00590b10, return 0
    // EOF: return 2
    return 0;
}

// FixupSMD @ 0x0040B630 (~386 lines) — SMD: post-process skeleton + triangle data
// Builds bone matrices from parsed skeleton data. For root bones (parent==-1): AngleMatrix
// from rotation, scale position by _DAT_0055259c, copy to bone transform.
// For child bones: AngleMatrix + R_ConcatTransforms with parent matrix.
// Then processes triangle vertices: transforms by bone matrix, computes normals.
// Large function — summary stub only.
void __stdcall FixupSMD_stub(void) {
    // For each node (DAT_00590c10):
    //   if parent == -1: AngleMatrix(rotation, matrix), FUN_004f9e90(matrix, output), copy position
    //   else: AngleMatrix(rotation, local), R_ConcatTransforms(parent, local, result)
    // For each triangle vertex:
    //   Transform position by bone matrix
    //   Transform normal by bone rotation
    //   Scale by _DAT_0055259c
    // stub: 386 lines of matrix math + vertex transformation
}
