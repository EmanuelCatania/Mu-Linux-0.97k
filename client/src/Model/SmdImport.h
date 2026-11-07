#pragma once
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <cstddef>

// Registros temporales y de ejecución; nunca se serializan como structs nativos.
// IDA 0x40AB90..0x40C16D. Límites derivados de las tablas originales.
namespace Smd {
using Vector = std::array<float, 3>;
struct Node {
    std::string Name;
    std::int16_t Parent;
};
struct Pose {
    std::vector<Vector> Positions, Angles;
};
struct Corner {
    std::int16_t Bone;
    Vector Position, Normal;
    std::array<float, 2> UV;
};
struct Triangle {
    std::string Material;
    std::array<Corner, 3> Corners;
};
#pragma pack(push, 1)
struct Vertex {
    std::int16_t Bone;
    std::uint8_t Padding[2];
    float Position[3];
};
struct Normal {
    std::int16_t Bone;
    std::uint8_t Padding[2];
    float Direction[3];
    std::int16_t Vertex;
    std::uint8_t Padding18[2];
};
struct UV {
    float U, V;
};
struct Face {
    std::uint8_t Count, Padding;
    std::int16_t Vertices[4], Normals[4], UVs[4], Adjacent[4];
    std::uint8_t Padding34[2];
};
struct Mesh {
    std::uint8_t Unknown0[2];
    std::int16_t Texture, VertexCount, NormalCount, UVCount, FaceCount;
    std::uint8_t Unknown12[4];
    Vertex *Vertices;
    Normal *Normals;
    UV *UVs;
    Face *Faces;
    std::uint8_t Unknown32[4];
    std::uint8_t *Script;
};
struct Keys {
    float *Positions;
    float *Angles;
    float *Quaternions;
};
struct Bone {
    char Name[32];
    std::int16_t Parent;
    std::uint8_t Dummy, Padding35;
    Keys *Actions;
    std::uint8_t BoundsEnabled, Padding[3];
    float Bounds[8][3];
};
struct Action {
    std::uint8_t Flags[8];
    std::int16_t Frames;
    std::uint8_t Lock, Padding11;
    float *Positions;
};
#pragma pack(pop)
static_assert(sizeof(Vertex) == 16 && offsetof(Vertex, Position) == 4);
static_assert(sizeof(Normal) == 20 && offsetof(Normal, Vertex) == 16);
static_assert(sizeof(UV) == 8 && sizeof(Face) == 36 && offsetof(Face, Adjacent) == 26);
static_assert(sizeof(Mesh) == 20 + 5 * sizeof(void *) && offsetof(Mesh, Vertices) == 16);
static_assert(offsetof(Mesh, Script) == 20 + 4 * sizeof(void *));
static_assert(sizeof(Keys) == 3 * sizeof(void *) && sizeof(Bone) == 136 + sizeof(void *));
static_assert(offsetof(Bone, Actions) == 36 && offsetof(Bone, Bounds) == 40 + sizeof(void *));
static_assert(sizeof(Action) == 12 + sizeof(void *) && offsetof(Action, Positions) == 12);
struct BuiltMesh {
    std::string Material;
    std::vector<Vertex> Vertices;
    std::vector<Normal> Normals;
    std::vector<UV> UVs;
    std::vector<Face> Faces;
};
struct Document {
    std::vector<Node> Nodes;
    Pose Bind;
    std::vector<Pose> Frames;
    std::size_t FrameCount = 0;
    std::vector<Triangle> Triangles;
    std::vector<BuiltMesh> Meshes;
};
bool Read(FILE *file, int type, bool flip);
bool Fixup();
const Document &Current();
bool BuildModel(void *model, int capacity);
bool AddAnimation(void *model, bool lock);
bool SelectModel(void *model);
void Forget(void *model);
const char *LastError();
} // namespace Smd
