#pragma once
#include "Render/MeshTopology.h"
#include "Render/Skinning.h"

namespace WebGPU {
// Envía lo grabado en el cuadro (pasadas y lotes de BufferedDraw) y presenta.
void SubmitFrame();
// Graba una malla residente con la pose capturada; false si la pose no
// corresponde (el caller dibuja por el camino de referencia).
bool DrawResidentMesh(const void* key, MeshTopology::Mesh& topology, const Skinning::ResidentDraw& draw);
void SetResident(bool enabled);
bool ResidentEnabled();
void ReleaseMeshes();
}
