#pragma once
#include <cstddef>
#include <cstdint>

// Nodo del árbol de PATH::FindPath (CBTree, 20 bytes en x86).
struct PathNode {
    int Data;
    int Cost;
    PathNode* Left;
    PathNode* Right;
    PathNode* Parent;
};

struct PathQueue {
    int Count;
    PathNode* Root;
};

// IDA: contexto de PATH (0x424 bytes, DAT_05826df4). Los mapas se reservan en
// PathFinder_ResetContext. La vtable de la cola no se conserva: sólo tenía GetCount.
struct PathContext {
    int Width;
    int Height;
    int Size;
    unsigned char* Walk;
    int Length;
    unsigned char PathX[500];
    unsigned char PathY[500];
    unsigned char* Flags;
    int DirtyMin;
    int DirtyMax;
    int* Cost;
    int* ParentX;
    int* ParentY;
    PathQueue Queue;
};
