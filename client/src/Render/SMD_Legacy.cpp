#include "stdafx.h"
#include "Model/SmdImport.h"
#include "globals.h"
#include "functions.h"
#include "Render/ResourceTables.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace Smd {
namespace {
constexpr std::size_t MaxNodes = 200, MaxFrames = 100, MaxTriangles = 15000, MaxMaterials = 100,
                      MaxElements = 10000;
Document document;
std::string error;
int ready = -1;
bool fixed = false;
struct Failure : std::runtime_error {
    using std::runtime_error::runtime_error;
};
void Check(bool ok, const char *reason) {
    if (!ok)
        throw Failure(reason);
}
struct Token {
    int Kind;
    std::string Text;
    float Number = 0;
};
class Reader {
    FILE *file;

  public:
    explicit Reader(FILE *value) : file(value) {}
    Token Next() {
        int c = std::fgetc(file);
        while (true) {
            while (c != EOF && std::isspace(static_cast<unsigned char>(c)))
                c = std::fgetc(file);
            if (c != '/')
                break;
            c = std::fgetc(file);
            if (c == '/') {
                while (c != EOF && c != '\n')
                    c = std::fgetc(file);
            } else
                break;
        }
        if (c == EOF)
            return {2, {}};
        Token t{0, {}};
        if (c == '"') {
            while ((c = std::fgetc(file)) != EOF && c != '"') {
                Check(t.Text.size() < 255, "Cadena SMD demasiado larga");
                t.Text += static_cast<char>(c);
            }
            Check(c == '"', "Cadena SMD sin cierre");
            return t;
        }
        if (c == '#' || c == ',' || c == ';' || c == '{' || c == '}')
            return {c, {}};
        if (c == '-' || c == '.' || std::isdigit(static_cast<unsigned char>(c))) {
            t.Kind = 1;
            do {
                Check(t.Text.size() < 99, "Número SMD demasiado largo");
                t.Text += static_cast<char>(c);
                c = std::fgetc(file);
            } while (c != EOF && (c == '.' || c == '-' || std::isdigit(static_cast<unsigned char>(c))));
            // IDA consume el delimitador numérico y no reconoce exponentes.
            char *end = nullptr;
            t.Number = std::strtof(t.Text.c_str(), &end);
            Check(end == t.Text.c_str() + t.Text.size() && std::isfinite(t.Number), "Número SMD inválido");
            return t;
        }
        if (std::isalpha(static_cast<unsigned char>(c))) {
            do {
                Check(t.Text.size() < 255, "Identificador SMD demasiado largo");
                t.Text += static_cast<char>(c);
                c = std::fgetc(file);
            } while (c != EOF && (c == '.' || c == '_' || std::isalnum(static_cast<unsigned char>(c))));
            if (c != EOF)
                std::ungetc(c, file);
            return t;
        }
        return {60, {}};
    }
    float Number() {
        auto t = Next();
        Check(t.Kind == 1, "Se esperaba un número SMD");
        return t.Number;
    }
    int Integer() {
        float n = Number();
        Check(std::trunc(n) == n && n >= -32768 && n <= 32767, "Índice SMD inválido");
        return static_cast<int>(n);
    }
    Vector Vector3() { return {Number(), Number(), Number()}; }
    void Section(const char *name) {
        while (true) {
            auto t = Next();
            Check(t.Kind != 2, "Falta una sección SMD");
            if (t.Kind == 0 && t.Text == name)
                return;
        }
    }
    void PoseValues(Pose &pose, std::size_t count) {
        pose.Positions.resize(count);
        pose.Angles.resize(count);
        for (std::size_t i = 0; i < count; ++i) {
            Integer();
            pose.Positions[i] = Vector3();
            pose.Angles[i] = Vector3();
        }
    }
    void Nodes(Document &out) {
        Section("nodes");
        while (true) {
            auto id = Next();
            Check(id.Kind != 2, "Nodes incompleto");
            if (id.Kind == 0 && id.Text == "end")
                break;
            Check(id.Kind == 1 && out.Nodes.size() < MaxNodes, "Nodo SMD inválido");
            auto name = Next();
            Check(name.Kind == 0 && name.Text.size() < 32, "Nombre de hueso inválido");
            int parent = Integer();
            Check(parent >= -1 && parent < static_cast<int>(out.Nodes.size()), "Jerarquía SMD inválida");
            out.Nodes.push_back({name.Text, static_cast<std::int16_t>(parent)});
        }
        Section("skeleton");
        bool pose = false;
        while (true) {
            auto t = Next();
            Check(t.Kind != 2, "Pose incompleta");
            if (t.Kind == 0 && t.Text == "end")
                break;
            Check(t.Kind == 0 && t.Text == "time", "Falta time");
            Integer();
            PoseValues(out.Bind, out.Nodes.size());
            pose = true;
        }
        Check(pose || out.Nodes.empty(), "Falta pose de referencia");
    }
    void Skeleton(Document &out) {
        Section("skeleton");
        while (true) {
            auto t = Next();
            Check(t.Kind != 2, "Animación incompleta");
            if (t.Kind == 0 && t.Text == "end")
                break;
            Check(t.Kind == 0 && t.Text == "time", "Falta time");
            int frame = Integer();
            Check(frame >= 0 && frame < static_cast<int>(MaxFrames) && out.FrameCount < MaxFrames,
                  "Demasiados frames");
            if (out.Frames.size() <= std::size_t(frame))
                out.Frames.resize(frame + 1);
            PoseValues(out.Frames[frame], out.Nodes.size());
            ++out.FrameCount;
        }
        // IDA cuenta time pero indexa por su valor; rechazar huecos evita poses viejas.
        for (std::size_t i = 0; i < out.FrameCount; ++i)
            Check(i < out.Frames.size() && out.Frames[i].Positions.size() == out.Nodes.size(),
                  "Frames no contiguos");
    }
    void Triangles(Document &out, bool flip) {
        Section("triangles");
        while (true) {
            auto t = Next();
            Check(t.Kind != 2, "Triángulos incompletos");
            if (t.Kind == 0 && t.Text == "end")
                break;
            Check(t.Kind == 0 && !t.Text.empty() && t.Text.size() < 32 && out.Triangles.size() < MaxTriangles,
                  "Material inválido");
            Triangle tri{};
            tri.Material = t.Text;
            for (int j = 0; j < 3; ++j) {
                auto &c = tri.Corners[flip ? 2 - j : j];
                int bone = Integer();
                Check(bone >= 0 && bone < static_cast<int>(out.Nodes.size()), "Hueso de vértice inválido");
                c.Bone = static_cast<std::int16_t>(bone);
                c.Position = Vector3();
                c.Normal = Vector3();
                c.UV = {Number(), 0};
                c.UV[1] = static_cast<float>(1.0 - Number());
            }
            out.Triangles.push_back(std::move(tri));
        }
    }
};
using Matrix = std::array<std::array<float, 4>, 3>;
Matrix Rotation(const Vector &radians) {
    // FixupSMD convierte a grados float antes de AngleMatrix/AngleIMatrix.
    Vector a{};
    for (int i = 0; i < 3; ++i)
        a[i] = radians[i] * 57.295776f;
    const double z = a[2] * 0.017453292, y = a[1] * 0.017453292, x = a[0] * 0.017453292;
    const float sy = static_cast<float>(std::sin(z)), sp = static_cast<float>(std::sin(y)),
                cp = static_cast<float>(std::cos(y)), sr = static_cast<float>(std::sin(x)),
                cr = static_cast<float>(std::cos(x));
    const double cy = std::cos(z);
    Matrix r{};
    r[0][0] = static_cast<float>(cp * cy);
    r[1][0] = cp * sy;
    r[2][0] = -sp;
    const double ss = sr * sp, cs = cr * sp;
    r[0][1] = static_cast<float>(ss * cy - cr * sy);
    r[1][1] = static_cast<float>(ss * sy + cr * cy);
    r[2][1] = sr * cp;
    r[0][2] = static_cast<float>(cs * cy + sr * sy);
    r[1][2] = static_cast<float>(cs * sy - sr * cy);
    r[2][2] = cr * cp;
    return r;
}
Matrix Multiply(const Matrix &a, const Matrix &b) {
    Matrix r{};
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x)
            r[y][x] = a[y][0] * b[0][x] + a[y][1] * b[1][x] + a[y][2] * b[2][x];
    return r;
}
Vector Transform(const Matrix &m, const Vector &v) {
    Vector r{};
    for (int i = 0; i < 3; ++i)
        r[i] = v[0] * m[i][0] + v[1] * m[i][1] + v[2] * m[i][2];
    return r;
}
template <class T, class Equal> std::int16_t FindOrAdd(std::vector<T> &values, const T &value, Equal equal) {
    // IDA busca desde el último y compara coordenadas sin incluir el hueso.
    for (std::size_t i = values.size(); i > 0; --i)
        if (equal(values[i - 1], value))
            return static_cast<std::int16_t>(i - 1);
    Check(values.size() < MaxElements, "Demasiados elementos en una malla");
    values.push_back(value);
    return static_cast<std::int16_t>(values.size() - 1);
}
struct Arena {
    std::vector<void *> blocks;
    bool committed = false;
    ~Arena() {
        if (!committed)
            for (auto p : blocks)
                std::free(p);
    }
    template <class T> T *Array(std::size_t count) {
        void *p = std::calloc(count ? count : 1, sizeof(T));
        if (!p)
            throw std::bad_alloc();
        try {
            blocks.push_back(p);
        } catch (...) {
            std::free(p);
            throw;
        }
        return static_cast<T *>(p);
    }
};
struct Capacity {
    void *Actions;
    void *Bones;
    int Count;
};
std::unordered_map<void *, Capacity> capacities;
void Quaternion(const float *a, float *q) {
    const float sz = static_cast<float>(std::sin(a[2] * 0.5)), sy = static_cast<float>(std::sin(a[1] * 0.5)),
                sx = static_cast<float>(std::sin(a[0] * 0.5));
    const double cz = std::cos(a[2] * 0.5), cy = std::cos(a[1] * 0.5), cx = std::cos(a[0] * 0.5);
    const double x = sx * cy, y = cx * sy, z = cy * cx, w = sx * sy;
    q[0] = static_cast<float>(x * cz - y * sz);
    q[1] = static_cast<float>(y * cz + x * sz);
    q[2] = static_cast<float>(z * sz - w * cz);
    q[3] = static_cast<float>(w * sz + z * cz);
}
void Initialize(BMD &model) {
    // IDA BMD::Init 0x442E00 y CreateBoundingBox 0x442E60; temporales propios.
    auto *bones = reinterpret_cast<Bone *>(model.Bones);
    auto *meshes = reinterpret_cast<Mesh *>(model.Meshs);
    std::vector<Vector> minimum(model.NumBones, Vector{9999, 9999, 9999}),
        maximum(model.NumBones, Vector{-9999, -9999, -9999});
    std::vector<unsigned> count(model.NumBones);
    for (int m = 0; m < model.NumMeshs; ++m)
        for (int v = 0; v < meshes[m].VertexCount; ++v) {
            const auto &vertex = meshes[m].Vertices[v];
            ++count[vertex.Bone];
            for (int a = 0; a < 3; ++a) {
                minimum[vertex.Bone][a] = (std::min)(minimum[vertex.Bone][a], vertex.Position[a]);
                maximum[vertex.Bone][a] = (std::max)(maximum[vertex.Bone][a], vertex.Position[a]);
            }
        }
    for (int i = 0; i < model.NumBones; ++i) {
        auto &bone = bones[i];
        bone.Dummy = bone.Name[0] == 'D' && bone.Name[1] == 'u';
        bone.BoundsEnabled = count[i] != 0;
        for (int corner = 0; corner < 8; ++corner)
            for (int a = 0; a < 3; ++a)
                bone.Bounds[corner][a] = (corner & (1 << (2 - a))) ? minimum[i][a] : maximum[i][a];
    }
    model.BoneHead = -1;
    model.StreamMesh = -1;
}
} // namespace
const Document &Current() { return document; }
const char *LastError() { return error.c_str(); }
void Forget(void *model) { capacities.erase(model); }
bool SelectModel(void *value) {
    ready = -1;
    auto *model = static_cast<BMD *>(value);
    auto entry = capacities.find(model);
    if (!model || entry == capacities.end() || entry->second.Actions != model->Actions ||
        entry->second.Bones != model->Bones || model->NumBones < 0 ||
        model->NumBones > static_cast<short>(MaxNodes))
        return false;
    try {
        std::vector<Node> nodes;
        auto *bones = reinterpret_cast<Bone *>(model->Bones);
        for (int i = 0; i < model->NumBones; ++i) {
            std::size_t length = 0;
            while (length < 32 && bones[i].Name[length])
                ++length;
            Check(length < 32, "Nombre de hueso sin cierre");
            nodes.push_back({std::string(bones[i].Name, length), bones[i].Parent});
        }
        document.Nodes = std::move(nodes);
        return true;
    } catch (const std::exception &e) {
        error = e.what();
        return false;
    }
}
bool Read(FILE *file, int type, bool flip) {
    ready = -1;
    fixed = false;
    error.clear();
    if (!file) {
        error = "Archivo SMD ausente";
        return false;
    }
    try {
        Reader reader(file);
        auto version = reader.Next();
        Check(version.Kind == 0 && version.Text == "version" && reader.Integer() == 1,
              "Versión SMD no soportada");
        Document next;
        if (type == 0) {
            reader.Nodes(next);
            reader.Triangles(next, flip);
        } else if (type == 1) {
            Check(!document.Nodes.empty(), "Animación sin modelo");
            next.Nodes = document.Nodes;
            next.Bind = document.Bind;
            reader.Skeleton(next);
        } else
            throw Failure("Tipo SMD inválido");
        document = std::move(next);
        ready = type;
        return true;
    } catch (const std::exception &e) {
        error = e.what();
        return false;
    }
}
bool Fixup() {
    if (ready != 0)
        return false;
    if (fixed)
        return true;
    try {
        std::vector<Matrix> forward(document.Nodes.size()), inverse(document.Nodes.size());
        std::vector<Vector> origins(document.Nodes.size());
        for (std::size_t i = 0; i < document.Nodes.size(); ++i) {
            auto r = Rotation(document.Bind.Angles[i]);
            Matrix inv{};
            for (int y = 0; y < 3; ++y)
                for (int x = 0; x < 3; ++x)
                    inv[y][x] = r[x][y];
            int parent = document.Nodes[i].Parent;
            if (parent < 0) {
                forward[i] = r;
                inverse[i] = inv;
                origins[i] = document.Bind.Positions[i];
            } else {
                forward[i] = Multiply(forward[parent], r);
                inverse[i] = Multiply(inv, inverse[parent]);
                origins[i] = Transform(forward[parent], document.Bind.Positions[i]);
                for (int a = 0; a < 3; ++a)
                    origins[i][a] += origins[parent][a];
            }
        }
        std::vector<BuiltMesh> built;
        for (const auto &tri : document.Triangles) {
            auto found = std::find_if(built.begin(), built.end(),
                                      [&](const BuiltMesh &m) { return m.Material == tri.Material; });
            if (found == built.end()) {
                Check(built.size() < MaxMaterials, "Demasiados materiales");
                built.push_back({tri.Material, {}, {}, {}, {}});
                found = built.end() - 1;
            }
            auto &mesh = *found;
            Face face{};
            face.Count = 3;
            std::fill(std::begin(face.Adjacent), std::end(face.Adjacent), std::int16_t(-1));
            for (int j = 0; j < 3; ++j) {
                const auto &c = tri.Corners[j];
                Vector p{};
                for (int a = 0; a < 3; ++a)
                    p[a] = c.Position[a] - origins[c.Bone][a];
                p = Transform(inverse[c.Bone], p);
                auto n = Transform(inverse[c.Bone], c.Normal);
                double length = std::sqrt(double(n[0] * n[0]) + double(n[1] * n[1]) + double(n[2] * n[2]));
                if (length != 0)
                    for (auto &v : n)
                        v = static_cast<float>(v / length);
                Vertex vertex{};
                vertex.Bone = c.Bone;
                std::copy(p.begin(), p.end(), vertex.Position);
                face.Vertices[j] = FindOrAdd(mesh.Vertices, vertex, [](const Vertex &a, const Vertex &b) {
                    return std::equal(std::begin(a.Position), std::end(a.Position), b.Position);
                });
                Normal normal{};
                normal.Bone = c.Bone;
                normal.Vertex = face.Vertices[j];
                std::copy(n.begin(), n.end(), normal.Direction);
                face.Normals[j] = FindOrAdd(mesh.Normals, normal, [](const Normal &a, const Normal &b) {
                    return std::equal(std::begin(a.Direction), std::end(a.Direction), b.Direction);
                });
                face.UVs[j] = FindOrAdd(mesh.UVs, UV{c.UV[0], c.UV[1]},
                                        [](const UV &a, const UV &b) { return a.U == b.U && a.V == b.V; });
            }
            mesh.Faces.push_back(face);
        }
        document.Meshes = std::move(built);
        fixed = true;
        return true;
    } catch (const std::exception &e) {
        error = e.what();
        ready = -1;
        return false;
    }
}

// Los conversores publican el grafo sólo después de completar sus reservas.
bool BuildModel(void *value, int capacity) {
    auto *model = static_cast<BMD *>(value);
    if (!model || capacity < 0 || capacity > 32767 || !Fixup())
        return false;
    try {
        Arena arena;
        auto *bones = arena.Array<Bone>(document.Nodes.size());
        auto *meshes = arena.Array<Mesh>(document.Meshes.size());
        auto *names = arena.Array<char>(document.Meshes.size() * 32);
        auto *indices = arena.Array<short>(document.Meshes.size());
        auto *actions = arena.Array<Action>(capacity);
        for (std::size_t i = 0; i < document.Nodes.size(); ++i) {
            const auto &node = document.Nodes[i];
            std::memcpy(bones[i].Name, node.Name.c_str(), node.Name.size() + 1);
            bones[i].Parent = node.Parent;
            bones[i].Actions = arena.Array<Keys>(capacity);
        }
        for (std::size_t i = 0; i < document.Meshes.size(); ++i) {
            const auto &in = document.Meshes[i];
            auto &out = meshes[i];
            out.Texture = static_cast<std::int16_t>(i);
            out.VertexCount = static_cast<std::int16_t>(in.Vertices.size());
            out.NormalCount = static_cast<std::int16_t>(in.Normals.size());
            out.UVCount = static_cast<std::int16_t>(in.UVs.size());
            out.FaceCount = static_cast<std::int16_t>(in.Faces.size());
            out.Vertices = arena.Array<Vertex>(in.Vertices.size());
            out.Normals = arena.Array<Normal>(in.Normals.size());
            out.UVs = arena.Array<UV>(in.UVs.size());
            out.Faces = arena.Array<Face>(in.Faces.size());
            std::copy(in.Vertices.begin(), in.Vertices.end(), out.Vertices);
            std::copy(in.Normals.begin(), in.Normals.end(), out.Normals);
            std::copy(in.UVs.begin(), in.UVs.end(), out.UVs);
            std::copy(in.Faces.begin(), in.Faces.end(), out.Faces);
            std::memcpy(names + i * 32, in.Material.c_str(), in.Material.size() + 1);
            auto us = in.Material.find('_');
            if (us != std::string::npos) {
                std::uint8_t flags[4]{};
                bool valid = true, any = false;
                for (std::size_t k = us + 1; k < in.Material.size() && k <= us + 4 && in.Material[k] != '.';
                     ++k) {
                    auto index = std::string("RHSN").find(in.Material[k]);
                    if (index == std::string::npos) {
                        valid = false;
                        break;
                    }
                    flags[index] = 1;
                    any = true;
                }
                if (valid && any) {
                    out.Script = arena.Array<std::uint8_t>(4);
                    std::copy(std::begin(flags), std::end(flags), out.Script);
                }
            }
        }
        BMD converted = *model;
        converted.NumBones = static_cast<short>(document.Nodes.size());
        converted.NumMeshs = static_cast<short>(document.Meshes.size());
        converted.NumActions = 0;
        converted.Bones = reinterpret_cast<BYTE *>(bones);
        converted.Meshs = reinterpret_cast<BYTE *>(meshes);
        converted.Textures = reinterpret_cast<BYTE *>(names);
        converted.IndexTexture = indices;
        converted.Actions = reinterpret_cast<BYTE *>(actions);
        converted.NumLightMaps = 0;
        Initialize(converted);
        capacities[model] = {actions, bones, capacity};
        *model = converted;
        arena.committed = true;
        ready = -1;
        return true;
    } catch (const std::exception &e) {
        error = e.what();
        return false;
    }
}
bool AddAnimation(void *value, bool lock) {
    auto *model = static_cast<BMD *>(value);
    auto entry = capacities.find(model);
    if (ready != 1 || !model || entry == capacities.end() || entry->second.Actions != model->Actions ||
        entry->second.Bones != model->Bones || model->NumActions < 0 ||
        model->NumActions >= entry->second.Count ||
        model->NumBones != static_cast<short>(document.Nodes.size()))
        return false;
    try {
        Arena arena;
        auto frames = document.FrameCount;
        auto *bones = reinterpret_cast<Bone *>(model->Bones);
        std::vector<Keys> keys(model->NumBones);
        for (int i = 0; i < model->NumBones; ++i) {
            auto &k = keys[i];
            k.Positions = arena.Array<float>(frames * 3);
            k.Angles = arena.Array<float>(frames * 3);
            k.Quaternions = arena.Array<float>(frames * 4);
            for (std::size_t f = 0; f < frames; ++f) {
                std::copy(document.Frames[f].Positions[i].begin(), document.Frames[f].Positions[i].end(),
                          k.Positions + f * 3);
                std::copy(document.Frames[f].Angles[i].begin(), document.Frames[f].Angles[i].end(),
                          k.Angles + f * 3);
                Quaternion(k.Angles + f * 3, k.Quaternions + f * 4);
            }
        }
        Action action{};
        action.Lock = lock;
        action.Frames = static_cast<std::int16_t>(frames);
        const float speed = 0.3f;
        std::memcpy(action.Flags + 4, &speed, 4);
        action.Positions = arena.Array<float>(frames * 3);
        if (!keys.empty())
            for (std::size_t f = 0; f < frames; ++f)
                for (int a = 0; a < 3; ++a)
                    action.Positions[f * 3 + a] = keys[0].Positions[(std::min)(f + 1, frames - 1) * 3 + a] -
                                                  keys[0].Positions[f * 3 + a];
        for (int i = 0; i < model->NumBones; ++i)
            bones[i].Actions[model->NumActions] = keys[i];
        reinterpret_cast<Action *>(model->Actions)[model->NumActions] = action;
        ++model->NumActions;
        arena.committed = true;
        ready = -1;
        return true;
    } catch (const std::exception &e) {
        error = e.what();
        return false;
    }
}
} // namespace Smd

extern "C" void __cdecl FixupSMD() { Smd::Fixup(); }
extern "C" void __cdecl SMD2BMDModel(int id, int actions) {
    if (id >= 0 && std::size_t(id) < ResourceTables::ModelCount())
        Smd::BuildModel(AsModel(DAT_05828d58) + id, actions);
}
extern "C" void __cdecl SMD2BMDAnimation(int id, char lock) {
    if (id >= 0 && std::size_t(id) < ResourceTables::ModelCount())
        Smd::AddAnimation(AsModel(DAT_05828d58) + id, lock != 0);
}
