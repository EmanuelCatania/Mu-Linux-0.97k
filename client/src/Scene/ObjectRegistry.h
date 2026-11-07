#pragma once
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

// Sólo nodos creados por CreateObject y aún no liberados.
// La escena se crea, recorre y destruye en el hilo principal.
namespace SceneObjects {
inline bool IsSlot(const void* pointer, const void* base, std::size_t bytes, std::size_t stride)
{
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto start = reinterpret_cast<std::uintptr_t>(base);
    return start && address >= start && address - start < bytes &&
           (address - start) % stride == 0;
}
struct BoneAllocation { const void* buffer; std::size_t bytes; };
inline std::unordered_map<const void*, BoneAllocation>& Bones()
{
    static std::unordered_map<const void*, BoneAllocation> allocations;
    return allocations;
}
inline void RegisterBones(const void* object, const void* buffer, std::size_t bytes)
{
    Bones()[object] = {buffer, bytes};
}
inline void UnregisterBones(const void* object) { Bones().erase(object); }
inline bool HasBoneRange(const void* object, const void* buffer, std::size_t offset, std::size_t bytes)
{
    const auto found = Bones().find(object);
    return found != Bones().end() && buffer && found->second.buffer == buffer &&
           offset <= found->second.bytes && bytes <= found->second.bytes - offset;
}
inline std::unordered_set<const void*>& LiveNodes()
{
    static std::unordered_set<const void*> nodes;
    return nodes;
}
inline std::unordered_map<const void*, std::uint64_t>& Generations()
{
    static std::unordered_map<const void*, std::uint64_t> generations;
    return generations;
}
inline std::uint64_t Generation(const void* node)
{
    const auto found = Generations().find(node);
    return found != Generations().end() ? found->second : 0;
}
inline void Register(const void* node)
{
    // DESVIACION: identidad externa para las muestras, sin ampliar OBJECT.
    static std::uint64_t nextGeneration = 0;
    Generations()[node] = ++nextGeneration;
    LiveNodes().insert(node);
}
inline void Unregister(const void* node) { LiveNodes().erase(node); Generations().erase(node); }
inline bool Contains(const void* node) { return LiveNodes().find(node) != LiveNodes().end(); }
}
