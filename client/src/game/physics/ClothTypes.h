#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

struct OBJECT;
// Contratos de ejecución de la tela 0.97k, sin representación en disco/red.
// IDA: 0x407950, 0x407E50, 0x407FE0, 0x408130, 0x4093E0 y 0x449840.
#pragma pack(push, 1)
struct ClothNode {
    void** VTable;
    float Acceleration[3];
    float Velocity[3];
    float Position[3];
    std::uint8_t Flags;
    std::uint8_t Padding[3];
    std::uint32_t CorrectionCount;
    float Correction[3];
};
struct ClothAnchor {
    void** VTable;
    float LocalPosition[3];
    std::int32_t Bone;
    float WorldPosition[3];
    float Radius;
};
struct ClothLink {
    ClothAnchor* Value;
    ClothLink* Previous;
    ClothLink* Next;
};
struct ClothSpring {
    std::int16_t A;
    std::int16_t B;
    float Minimum;
    float Rest;
    std::uint8_t Flags;
    std::uint8_t Padding[3];
};
struct ClothWidget {
    void** VTable;
    OBJECT* Owner;
    std::int32_t Bone;
    std::int32_t FrontTexture;
    std::int32_t BackTexture;
    std::uint32_t Flags;
    float OriginU;
    float OriginV;
    float Width;
    float Height;
    std::int32_t Columns;
    std::int32_t Rows;
    std::int32_t NodeCount;
    ClothNode* Nodes;
    std::int32_t SpringCount;
    ClothSpring* Springs;
    float StepU;
    float StepV;
    std::int32_t AnchorCount;
    ClothLink* Head;
    ClothLink* Tail;
};
struct ClothMeshWidget {
    ClothWidget Base;
    std::int32_t MeshIndex;
    std::uint8_t Unknown[4];
    std::int32_t ModelType;
};
// Vista de ejecución que consume SpringMesh_Create (IDA 0x4093E0).
// No se usa para leer bytes de .bmd; el lector de assets tiene contrato propio.
struct ClothModelMesh {
    std::uint8_t Unknown0[2];
    std::int16_t TextureIndex;
    std::int16_t VertexCount;
    std::int16_t NormalCount;
    std::int16_t TexCoordCount;
    std::int16_t TriangleCount;
    std::uint8_t Unknown12[4];
    std::uint8_t* Vertices;
    void* Normals;
    void* TexCoords;
    std::uint8_t* Triangles;
    std::uint8_t Unknown32[4];
    void* TextureFlags;
};
#pragma pack(pop)

static_assert(sizeof(ClothNode) == sizeof(void*) + 56);
static_assert(sizeof(ClothAnchor) == sizeof(void*) + 32);
static_assert(sizeof(ClothLink) == 3 * sizeof(void*));
static_assert(sizeof(ClothSpring) == 16);
static_assert(sizeof(ClothWidget) == 60 + 6 * sizeof(void*));
static_assert(sizeof(ClothMeshWidget) == sizeof(ClothWidget) + 12);
static_assert(sizeof(ClothModelMesh) == 20 + 5 * sizeof(void*));
static_assert(offsetof(ClothModelMesh, TextureIndex) == 2);
static_assert(offsetof(ClothModelMesh, Vertices) == 16);
static_assert(offsetof(ClothModelMesh, Triangles) == 16 + 3 * sizeof(void*));
static_assert(offsetof(ClothNode, Position) == sizeof(void*) + 24);
static_assert(offsetof(ClothAnchor, Bone) == sizeof(void*) + 12);
static_assert(offsetof(ClothWidget, Owner) == sizeof(void*));
static_assert(offsetof(ClothWidget, Nodes) == 44 + 2 * sizeof(void*));
static_assert(offsetof(ClothWidget, Springs) == 48 + 3 * sizeof(void*));
static_assert(offsetof(ClothWidget, Head) == 60 + 4 * sizeof(void*));
static_assert(offsetof(ClothMeshWidget, ModelType) == sizeof(ClothWidget) + 8);
static_assert(alignof(ClothWidget) == 1 && alignof(ClothNode) == 1);
static_assert(std::is_standard_layout_v<ClothWidget> && std::is_trivially_copyable_v<ClothWidget>);
static_assert(std::is_standard_layout_v<ClothNode> && std::is_trivially_copyable_v<ClothNode>);

inline ClothWidget& ClothSystem(void* p) { return *static_cast<ClothWidget*>(p); }
inline const ClothWidget& ClothSystem(const void* p) { return *static_cast<const ClothWidget*>(p); }
inline ClothWidget& ClothSystem(std::uintptr_t p) { return *reinterpret_cast<ClothWidget*>(p); }
inline ClothNode& ClothPoint(void* p) { return *static_cast<ClothNode*>(p); }
inline ClothNode& ClothPoint(std::uintptr_t p) { return *reinterpret_cast<ClothNode*>(p); }
inline ClothAnchor& ClothBody(void* p) { return *static_cast<ClothAnchor*>(p); }
inline ClothAnchor& ClothBody(std::uintptr_t p) { return *reinterpret_cast<ClothAnchor*>(p); }

// Mantener el tipo real del constructor: nodos (void) y widgets (this).
template<class T, class Constructor>
inline void ClothConstruct(T* elements, int count, Constructor constructor)
{
    for (int i=0;i<count;++i) constructor(elements+i);
}
