// Cloth_MeshDivine.cpp -- tela por malla para la falda de los pants Divine.
//
// DESVIACION DOCUMENTADA (no existe en el 0.97k).
//
// El 0.97k no trae el set Divine: el modelo `Data/Player/PantMale20.bmd`
// (MODEL_PANTS + 19 = 707) que se usa en este cliente es el del 0.99, que
// agrega una cuarta malla con la falda.  Sin simulacion esa malla se mueve
// pegada al cuerpo y atraviesa las piernas.  El modelo original (0.95/0.97)
// tiene solo 3 mallas y no trae falda, asi que ahi no se hace nada.
//
// Se porta el CPhysicsClothMesh del source de MU 5.2 en su modo "por malla"
// (sin PCT_OPT_MESHPROG), tal como lo usa ZzzObject.cpp::RenderPartObject
// para MODEL_PANTS + 19:
//
//     pCloth->Create(o, 3, 2, PCT_OPT_CORRECTEDFORCE | PCT_HEAVY, Type);
//     AddCollisionSphere(0, 0, -15, 22, 2);  ... (-27, 23) (-40, 24)
//                                                (-54, 25) (-69, 26)
//     if (!pCloth->Move2(0.005f, 5)) DeleteCloth(...); else pCloth->Render();
//
// La tela no dibuja nada propio: los vertices de la malla 3 son las
// particulas (los del hueso 2 quedan fijos a la cadera), los resortes salen
// de las aristas de los triangulos, y Render() escribe las posiciones
// simuladas sobre los vertices ya transformados de la malla antes de que se
// dibuje el cuerpo.  Asi la falda conserva su textura y su luz.
//
// Se guarda en una tabla propia (no en part+0x14) porque DeleteCloth del
// 0.97k libera ese campo asumiendo el layout de su propia tela.

#include "stdafx.h"
#include <math.h>

// g_BoneVertexBuf = VertexTransform[mesh][vertex]; _DAT_00590af0 = viento
// (CPhysicsManager::s_fWind); DAT_083a7c00 = MoveSceneFrame.  Vienen de
// globals.h via stdafx.h.

namespace {

const int   kDivinePantsModel = 707;             // MODEL_PANTS + 19
const int   kSkirtMesh        = 3;
const int   kFixedBone        = 2;

// Flags de 5.2 que usa este caso.
const unsigned PCT_HEAVY              = 0x00000400;
const unsigned PCT_OPT_CORRECTEDFORCE = 0x20000000;

const unsigned char PVS_FIXEDPOS = 0x01;
const unsigned char PLS_LOOSEDISTANCE  = 0x01;
const unsigned char PLS_SPRING         = 0x02;
const unsigned char PLS_STRICTDISTANCE = 0x04;

const float kGravity   = 9.8f;
const float kMass      = 0.0025f;
const float kInvOfMass = 400.0f;

struct Vec3 { float x, y, z; };

struct Vertex {
    Vec3 force, vel, pos, oneTimeMove;
    int  countOneTimeMove;
    unsigned char state;
};

struct Link {
    short v[2];
    float dist[2];
    unsigned char style;
};

struct Sphere {
    Vec3  centerLocal;   // antes de transformar (ejes del source)
    Vec3  center;
    float radius;
    int   bone;
};

struct MeshCloth {
    int     part;            // clave: puntero a la pieza (PART_t)
    int     lastFrame;
    int     numVertices;
    Vertex *vertices;
    int     numLinks;
    Link   *links;
    Sphere  spheres[5];
    int     numSpheres;
    unsigned type;
};

const int kSlots = 16;
MeshCloth g_Slots[kSlots];

inline float* VT(int mesh, int v) {
    return (float *)(g_BoneVertexBuf + (mesh * 15000 + v) * 12);
}

inline float Length(const Vec3 &a) { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }

float GetDistance(const Vertex &a, const Vertex &b, Vec3 *d) {
    d->x = a.pos.x - b.pos.x; d->y = a.pos.y - b.pos.y; d->z = a.pos.z - b.pos.z;
    return Length(*d);
}

void Destroy(MeshCloth &c) {
    delete[] c.vertices;
    delete[] c.links;
    memset(&c, 0, sizeof(c));
}

// Mesh_t en memoria (BMD_Load): +4 NumVertices, +10 NumTriangles,
// +0x10 Vertices (stride 0x10, Node en +0), +0x1c Triangles (stride 0x24,
// VertexIndex[4] en +2).
inline char* MeshPtr(void *model, int mesh) {
    return (char *)(*(int *)((int)model + 0x28) + mesh * 0x28);
}
inline short TriVertex(char *mesh, int tri, int i) {
    return *(short *)(*(int *)(mesh + 0x1c) + tri * 0x24 + 2 + i * 2);
}

int FindMatchVertex(char *mesh, int v1, int v2, int v3) {
    const int nTri = *(short *)(mesh + 10);
    for (int t = 0; t < nTri; ++t) {
        for (int i = 0; i < 3; ++i) {
            const int c1 = TriVertex(mesh, t, i);
            const int c2 = TriVertex(mesh, t, (i + 1) % 3);
            const int c3 = TriVertex(mesh, t, (i + 2) % 3);
            if (v1 == c2 && v2 == c1 && v3 != c3) return c3;
        }
    }
    return -1;
}

bool FindInLink(const MeshCloth &c, int count, int v1, int v2) {
    for (int i = 0; i < count; ++i)
        if (c.links[i].v[0] == v1 && c.links[i].v[1] == v2) return true;
    return false;
}

void SetLink(MeshCloth &c, int i, int v1, int v2, float dMin, float dMax, unsigned char style) {
    c.links[i].v[0] = (short)v1; c.links[i].v[1] = (short)v2;
    c.links[i].dist[0] = dMin;  c.links[i].dist[1] = dMax;
    c.links[i].style = style;
}

// CPhysicsClothMesh::Create(o, iMesh, iBone, dwType)
bool Create(MeshCloth &c, void *model) {
    char *mesh = MeshPtr(model, kSkirtMesh);
    const int nV   = *(short *)(mesh + 4);
    const int nTri = *(short *)(mesh + 10);
    if (nV <= 0 || nTri <= 0 || nV > 15000) return false;

    c.type        = PCT_OPT_CORRECTEDFORCE | PCT_HEAVY;
    c.numVertices = nV;
    c.vertices    = new Vertex[nV];
    memset(c.vertices, 0, sizeof(Vertex) * nV);
    c.links       = new Link[nTri * 3 * 2];

    bool anyFixed = false;
    const char *verts = (const char *)*(int *)(mesh + 0x10);
    for (int v = 0; v < nV; ++v) {
        const float *p = VT(kSkirtMesh, v);
        c.vertices[v].pos.x = p[0]; c.vertices[v].pos.y = p[1]; c.vertices[v].pos.z = p[2];
        if (*(short *)(verts + v * 0x10) == kFixedBone) {
            c.vertices[v].state |= PVS_FIXEDPOS;
            anyFixed = true;
        }
    }
    if (!anyFixed) return false;   // no es la falda del 0.99

    int link = 0;
    Vec3 tmp;
    for (int t = 0; t < nTri; ++t) {
        for (int i = 0; i < 3; ++i) {
            const int v1 = TriVertex(mesh, t, i);
            const int v2 = TriVertex(mesh, t, (i + 1) % 3);
            if (v1 < 0 || v1 >= nV || v2 < 0 || v2 >= nV) continue;
            const float d = GetDistance(c.vertices[v1], c.vertices[v2], &tmp);
            unsigned char style = PLS_STRICTDISTANCE;
            if (fabsf(c.vertices[v1].pos.x - c.vertices[v2].pos.x) > 10.0f)
                style = PLS_LOOSEDISTANCE;
            SetLink(c, link++, v1, v2, d * 0.5f, d, PLS_SPRING | style);

            const int v3 = TriVertex(mesh, t, (i + 2) % 3);
            const int match = FindMatchVertex(mesh, v1, v2, v3);
            if (match > 0 && match < nV && v3 >= 0 && v3 < nV) {
                const float d2 = GetDistance(c.vertices[v3], c.vertices[match], &tmp);
                if (d2 < d * 1.2f && !FindInLink(c, link, v3, match))
                    SetLink(c, link++, v3, match, d2 * 0.5f, d2, PLS_SPRING | style);
            }
        }
    }
    c.numLinks = link;

    // AddCollisionSphere(0, 0, z, r, 2) x5
    const float z[5] = { -15.0f, -27.0f, -40.0f, -54.0f, -69.0f };
    const float r[5] = {  22.0f,  23.0f,  24.0f,  25.0f,  26.0f };
    c.numSpheres = 5;
    for (int i = 0; i < 5; ++i) {
        c.spheres[i].centerLocal.x = 0.0f;
        c.spheres[i].centerLocal.y = 0.0f;
        c.spheres[i].centerLocal.z = z[i];
        c.spheres[i].radius = r[i];
        c.spheres[i].bone   = kFixedBone;
    }
    return true;
}

// CPhysicsVertex::UpdateForce
void UpdateForce(Vertex &v, unsigned key, unsigned type, const Vec3 &wind) {
    if (v.state & PVS_FIXEDPOS) { v.force.x = v.force.y = v.force.z = 0.0f; return; }
    int t = 5 - (int)key; if (t < 0) t = 0; if (t > 4) t = 4;
    const float fRand = (float)(t == 0 ? 0 : t + 2);
    v.force.x = fRand * wind.x - v.vel.x * 0.01f;
    v.force.y = fRand * wind.y - v.vel.y * 0.01f;
    v.force.z = fRand * wind.z - v.vel.z * 0.01f;
    if (type & PCT_HEAVY) v.force.z -= kGravity * kMass * 180.0f;
    else                  v.force.z -= kGravity * kMass * 100.0f;
}

void AddOneTimeMove(Vertex &v, const Vec3 &m) {
    v.oneTimeMove.x += m.x; v.oneTimeMove.y += m.y; v.oneTimeMove.z += m.z;
    v.countOneTimeMove++;
}

void DoOneTimeMove(Vertex &v) {
    if (v.state & PVS_FIXEDPOS) {
        v.oneTimeMove.x = v.oneTimeMove.y = v.oneTimeMove.z = 0.0f;
        return;
    }
    if (v.countOneTimeMove > 0) {
        const float n = (float)v.countOneTimeMove;
        v.pos.x += v.oneTimeMove.x / n; v.pos.y += v.oneTimeMove.y / n; v.pos.z += v.oneTimeMove.z / n;
        v.oneTimeMove.x = v.oneTimeMove.y = v.oneTimeMove.z = 0.0f;
        v.countOneTimeMove = 0;
    }
}

bool KeepLength(Vertex &v, const Vertex &other, const float *len) {
    if (v.state & PVS_FIXEDPOS) return true;
    Vec3 d;
    float dist = GetDistance(v, other, &d);
    if (dist < 0.001f) dist = 0.001f;
    if (dist > len[1] * 20.0f) return false;
    float k = 0.0f;
    if (dist > len[1])      k = (dist - len[1]) / dist;
    else if (dist < len[0]) k = (dist - len[0]) / dist;
    v.pos.x -= d.x * k; v.pos.y -= d.y * k; v.pos.z -= d.z * k;
    return true;
}

// CPhysicsCloth::Move (una iteracion)
bool Step(MeshCloth &c, void *model, const float *boneBase, float angleZ, float fTime) {
    const float wind = _DAT_00590af0;
    const float a = (180.0f + angleZ) * 3.14159265f / 180.0f;
    const Vec3 vWind = { wind * sinf(a), -wind * cosf(a), 0.0f };

    // InitForces (modo malla): la misma clave para todos los vertices.
    const int seed = (((int)DAT_083a7c00 / 10) * 101) % c.numVertices;
    const unsigned key = (unsigned)abs(seed % 10);
    for (int i = 0; i < c.numVertices; ++i) UpdateForce(c.vertices[i], key, c.type, vWind);

    // Resortes.
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_SPRING)) continue;
        Vertex &v1 = c.vertices[lk.v[0]];
        Vertex &v2 = c.vertices[lk.v[1]];
        Vec3 d;
        float dist = GetDistance(v1, v2, &d);
        if (dist < 0.001f) dist = 0.001f;
        if (dist > lk.dist[1] + 0.01f) {
            float k = (dist - lk.dist[1]) / dist;
            if (c.type & PCT_OPT_CORRECTEDFORCE) k *= lk.dist[1] / 32.0f;
            const Vec3 f = { d.x * k, d.y * k, d.z * k };
            v1.force.x -= f.x; v1.force.y -= f.y; v1.force.z -= f.z;
            v2.force.x += f.x; v2.force.y += f.y; v2.force.z += f.z;
        }
    }

    // SetFixedVertices (modo malla): los del hueso fijo siguen a la cadera.
    {
        char *mesh = MeshPtr(model, kSkirtMesh);
        const char *verts = (const char *)*(int *)(mesh + 0x10);
        for (int v = 0; v < c.numVertices; ++v) {
            if (*(short *)(verts + v * 0x10) == kFixedBone) {
                const float *p = VT(kSkirtMesh, v);
                c.vertices[v].pos.x = p[0]; c.vertices[v].pos.y = p[1]; c.vertices[v].pos.z = p[2];
                c.vertices[v].state |= PVS_FIXEDPOS;
            }
        }
    }

    // Integracion.
    for (int i = 0; i < c.numVertices; ++i) {
        Vertex &v = c.vertices[i];
        if (v.state & PVS_FIXEDPOS) continue;
        v.vel.x += v.force.x * kInvOfMass * fTime;
        v.vel.y += v.force.y * kInvOfMass * fTime;
        v.vel.z += v.force.z * kInvOfMass * fTime;
        v.pos.x += v.vel.x * fTime; v.pos.y += v.vel.y * fTime; v.pos.z += v.vel.z * fTime;
    }

    // PreventFromStretching: esferas de colision en el hueso.
    for (int s = 0; s < c.numSpheres; ++s) {
        Sphere &sp = c.spheres[s];
        // Cambio de ejes del source: (x, y, z) -> (z, -y, x).
        float in[3]  = { sp.centerLocal.z, -sp.centerLocal.y, sp.centerLocal.x };
        float out[3] = { 0.0f, 0.0f, 0.0f };
        BMD_TransformPosition(model, (float *)(boneBase + sp.bone * 12), in, out, '\x01');
        sp.center.x = out[0]; sp.center.y = out[1]; sp.center.z = out[2];
    }
    if (c.numSpheres > 0) {
        for (int s = 0; s < c.numSpheres; ++s) {
            const Sphere &sp = c.spheres[s];
            for (int i = 0; i < c.numVertices; ++i) {
                Vec3 d = { c.vertices[i].pos.x - sp.center.x,
                           c.vertices[i].pos.y - sp.center.y,
                           c.vertices[i].pos.z - sp.center.z };
                float len = Length(d);
                if (len < 0.01f) { len = 0.01f; d.x = len; d.y = 0.0f; d.z = 0.0f; }
                if (len < sp.radius) {
                    const float k = (sp.radius - len) / len;
                    const Vec3 m = { d.x * k, d.y * k, d.z * k };
                    AddOneTimeMove(c.vertices[i], m);
                }
            }
        }
        for (int i = 0; i < c.numVertices; ++i) DoOneTimeMove(c.vertices[i]);
    }

    // Distancia "suelta": se acumula y se aplica en la proxima pasada.  En 5.2
    // el DoOneTimeMove que sigue recorre m_iNumVer * m_iNumHor, que en el modo
    // por malla es 0; se conserva ese comportamiento.
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_LOOSEDISTANCE)) continue;
        Vertex &v1 = c.vertices[lk.v[0]];
        Vertex &v2 = c.vertices[lk.v[1]];
        Vec3 d;
        float dist = GetDistance(v1, v2, &d);
        if (dist < 0.001f) dist = 0.001f;
        const float k = (dist - lk.dist[1]) * 0.5f / dist;
        const Vec3 m = { d.x * k, d.y * k, d.z * k };
        v1.oneTimeMove.x -= m.x; v1.oneTimeMove.y -= m.y; v1.oneTimeMove.z -= m.z;
        v2.oneTimeMove.x += m.x; v2.oneTimeMove.y += m.y; v2.oneTimeMove.z += m.z;
        v1.countOneTimeMove++; v2.countOneTimeMove++;
    }

    // Distancia "estricta" (m_iNumHor = 0: aplica a todos los enlaces).
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_STRICTDISTANCE)) continue;
        if (!KeepLength(c.vertices[lk.v[1]], c.vertices[lk.v[0]], lk.dist)) return false;
    }
    return true;
}

MeshCloth* FindSlot(int part) {
    MeshCloth *freeSlot = nullptr, *oldest = &g_Slots[0];
    for (int i = 0; i < kSlots; ++i) {
        if (g_Slots[i].part == part) return &g_Slots[i];
        if (!g_Slots[i].part && !freeSlot) freeSlot = &g_Slots[i];
        if (g_Slots[i].lastFrame < oldest->lastFrame) oldest = &g_Slots[i];
    }
    MeshCloth *s = freeSlot ? freeSlot : oldest;
    if (s->part) Destroy(*s);
    s->part = part;
    return s;
}

} // namespace

// Llamada desde RenderPartObject (FUN_00505a10) entre la transformacion de
// los vertices (FUN_004404e0) y el dibujado (FUN_00504b50).
void __cdecl DivineSkirt_Apply(int entity, int modelType, int part, void *model)
{
    if (modelType != kDivinePantsModel || !part || !model || !entity) return;
    // El modelo viejo (0.95/0.97) tiene 3 mallas y no trae falda.
    if (*(short *)((int)model + 0x24) <= kSkirtMesh) return;

    const float *boneBase = *(const float **)(entity + 0x114);   // BoneTransform
    if (!boneBase) return;

    MeshCloth *c = FindSlot(part);
    // Si la pieza no se dibujo en los ultimos frames (cambio de mapa,
    // entidad reusada), se vuelve a armar desde la pose actual.
    const int frame = (int)DAT_083a7c00;
    if (c->vertices && (frame - c->lastFrame > 3 || frame < c->lastFrame)) {
        Destroy(*c);
        c->part = part;
    }
    c->lastFrame = frame;
    if (!c->vertices && !Create(*c, model)) { Destroy(*c); return; }

    const float angleZ = *(float *)(entity + 0x24);
    for (int i = 0; i < 5; ++i) {                      // Move2(0.005f, 5)
        if (!Step(*c, model, boneBase, angleZ, 0.005f)) {
            Destroy(*c);                               // DeleteCloth
            return;
        }
    }

    // Render(): las posiciones simuladas pasan a VertexTransform[malla 3].
    for (int v = 0; v < c->numVertices; ++v) {
        float *p = VT(kSkirtMesh, v);
        p[0] = c->vertices[v].pos.x; p[1] = c->vertices[v].pos.y; p[2] = c->vertices[v].pos.z;
    }
}
