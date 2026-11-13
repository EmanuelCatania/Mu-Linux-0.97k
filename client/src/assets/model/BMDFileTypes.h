#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Cabeceras de archivo .bmd normalizado, independientes de los punteros nativos.
// IDA BMD::Open 0x004423E0; v12 conserva el mismo payload después de descifrar.
namespace BmdFile {
#pragma pack(push, 1)
struct Header {
    char Signature[3];
    std::uint8_t Version;
    char Name[32];
    std::int16_t MeshCount;
    std::int16_t BoneCount;
    std::int16_t ActionCount;
};
struct MeshHeader {
    std::int16_t VertexCount, NormalCount, TexCoordCount, TriangleCount, TextureIndex;
};
struct ActionHeader {
    std::int16_t FrameCount;
    std::uint8_t LockPositions;
};
struct BoneHeader {
    char Name[32];
    std::int16_t Parent;
};
#pragma pack(pop)
static_assert(sizeof(Header) == 42 && offsetof(Header, Name) == 4);
static_assert(offsetof(Header, MeshCount) == 36 && offsetof(Header, BoneCount) == 38 && offsetof(Header, ActionCount) == 40);
static_assert(sizeof(MeshHeader) == 10 && offsetof(MeshHeader, TextureIndex) == 8);
static_assert(sizeof(ActionHeader) == 3 && offsetof(ActionHeader, LockPositions) == 2);
static_assert(sizeof(BoneHeader) == 34 && offsetof(BoneHeader, Parent) == 32);
static_assert(alignof(Header) == 1 && alignof(MeshHeader) == 1 && alignof(ActionHeader) == 1 && alignof(BoneHeader) == 1);
static_assert(std::is_standard_layout_v<Header> && std::is_trivially_copyable_v<Header>);
static_assert(std::is_standard_layout_v<MeshHeader> && std::is_trivially_copyable_v<MeshHeader>);
static_assert(std::is_standard_layout_v<ActionHeader> && std::is_trivially_copyable_v<ActionHeader>);
static_assert(std::is_standard_layout_v<BoneHeader> && std::is_trivially_copyable_v<BoneHeader>);
inline constexpr std::size_t VectorBytes = 12, VertexBytes = 16, NormalBytes = 20, TexCoordBytes = 8, TriangleBytes = 64;
}
