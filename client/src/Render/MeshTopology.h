#pragma once
#include "Render/MeshDraw.h"
#include "Render/Skinning.h"
#include <cstdint>
#include <vector>

// Topología de las mallas BMD, común a los backends: las esquinas únicas
// (posición, normal, UV) con sus índices, y los vértices fijos en el espacio
// del hueso para dibujarlas residentes en la GPU. Cada backend crea sus
// buffers a partir de esto y los rehace cuando cambia la revisión.
namespace MeshTopology {
struct Corner { int position,normal,uv; };
struct Mesh {
    std::vector<unsigned char> faces;
    // Origen y muestra de las caras ya verificadas: si coinciden, el modelo
    // no cambió y se evita comparar todas las caras en cada envío.
    const unsigned char* facesSource=nullptr;std::uint32_t facesSample=0;
    std::vector<Corner> corners;
    std::vector<unsigned> indices;
    Corner minimum{},maximum{};
    std::vector<int> positions,normals;
    std::uint64_t revision=0;           // cambia cada vez que se rearma
    // Malla residente (Skinning::StaticVertex), armada a pedido.
    const unsigned char* residentVertices=nullptr;
    bool residentTried=false;
    int maxBone=-1;
    std::vector<Skinning::StaticVertex> resident;
    std::uint64_t residentRevision=0;
};
// La malla de input.key, rearmada si las caras cambiaron; nula si no sirve.
Mesh* Get(const MeshDraw::Input& input);
// Vértices residentes de la malla (Mesh::resident); false si no admite el camino.
bool Resident(Mesh& mesh,const MeshDraw::Input& input);
void Release();
}
