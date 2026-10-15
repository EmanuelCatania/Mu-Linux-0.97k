#pragma once
#include <unordered_set>

// Solo nodos de 0x1bc creados por CreateObject y aun no liberados.
// La escena se crea, recorre y destruye en el hilo principal.
namespace SceneObjects {
inline std::unordered_set<const void*>& LiveNodes()
{
    static std::unordered_set<const void*> nodes;
    return nodes;
}
inline void Register(const void* node) { LiveNodes().insert(node); }
inline void Unregister(const void* node) { LiveNodes().erase(node); }
inline bool Contains(const void* node) { return LiveNodes().find(node) != LiveNodes().end(); }
}
