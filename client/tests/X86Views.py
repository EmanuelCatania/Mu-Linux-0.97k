"""Vistas x86 de los structs del juego para los harnesses de punteros.

Los harnesses arman memoria cruda con los offsets de IDA (layout x86), aunque
compilen en x64.  El código de producción ya accede por campos
(AsObject(o)->Alpha); estas vistas ponen cada campo pedido en su offset x86,
leído del comentario `// 0xNNN` del header del struct.  Los punteros quedan
como enteros de 32 bits, igual que en la memoria del harness.
"""
import re

from SceneObjectPointer import read_source

_POINTER_SIZES = {"BYTE": 1, "char": 1, "short": 2, "WORD": 2, "int": 4, "DWORD": 4, "float": 4,
                  "std::uint8_t": 1, "std::int8_t": 1, "std::uint16_t": 2, "std::int16_t": 2,
                  "std::uint32_t": 4, "std::int32_t": 4}


def _fields(struct, revision=None, header="src/Entity/EntityTypes.h"):
    source = read_source(header, revision)
    body = re.search(r"struct " + struct + r"\s*\{(.*?)\n\};", source, re.S).group(1)
    fields = {}
    pattern = r"^\s*([\w:]+\*?)\s+(\w+)\s*(\[\s*(\w+)\s*\])?\s*;\s*//\s*0x([0-9A-Fa-f]+)"
    for kind, name, suffix, count, offset in re.findall(pattern, body, re.M):
        pointer = kind.endswith("*")
        width = 4 if pointer else _POINTER_SIZES.get(kind, 4)
        n = int(count, 0) if count else 1
        fields[name] = (int(offset, 16), "std::uint32_t" if pointer else kind, width * n, suffix)
    return fields


def view(struct, names, size, revision=None, header="src/Entity/EntityTypes.h"):
    """struct con los campos `names` en su offset x86 y tamaño total `size`."""
    fields = _fields(struct, revision, header)
    out, cursor, pad = [f"struct {struct} {{"], 0, 0
    for offset, kind, width, suffix, name in sorted((*fields[n], n) for n in names):
        if offset > cursor:
            out.append(f"    unsigned char _pad{pad}[{offset - cursor}];")
            pad += 1
        out.append(f"    {kind} {name}{suffix};")
        cursor = offset + width
    if size > cursor:
        out.append(f"    unsigned char _pad{pad}[{size - cursor}];")
    out.append("};")
    out.append(f'static_assert(sizeof({struct}) == {size}, "{struct} x86");')
    for name in names:
        out.append(f'static_assert(offsetof({struct}, {name}) == {fields[name][0]}, "{struct}::{name}");')
    return "\n".join(out) + "\n"


def _packed(struct, names, size, revision, header, accessor):
    accessors = (f"inline {struct}* {accessor}(void* p) {{ return static_cast<{struct}*>(p); }}\n"
                 f"inline {struct}* {accessor}(std::uintptr_t p) {{ return reinterpret_cast<{struct}*>(p); }}\n"
                 f"inline {struct}* {accessor}(const void* p) {{ return const_cast<{struct}*>(static_cast<const {struct}*>(p)); }}\n")
    return "\n#pragma pack(push, 1)\n" + view(struct, names, size, revision, header) + "#pragma pack(pop)\n" + accessors


def object_view(names, revision=None):
    return _packed("OBJECT", names, 0x1BC, revision, "src/Entity/EntityTypes.h", "AsObject")


CLOTH_MODEL_MESH = """
#pragma pack(push, 1)
struct ClothModelMesh {  // malla del modelo con el layout x86 (40 bytes, punteros de 32 bits)
    std::uint8_t Unknown0[2];
    std::int16_t TextureIndex, VertexCount, NormalCount, TexCoordCount, TriangleCount;
    std::uint8_t Unknown12[4];
    std::uint32_t Vertices, Normals, TexCoords, Triangles;
    std::uint8_t Unknown32[4];
    std::uint32_t TextureFlags;
};
#pragma pack(pop)
static_assert(sizeof(ClothModelMesh) == 40, "ClothModelMesh x86");
"""


def model_view(names, revision=None):
    return _packed("BMD", names, 0xBC, revision, "src/Model/ModelTypes.h", "AsModel")
