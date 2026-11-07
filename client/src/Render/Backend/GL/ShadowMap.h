#pragma once
#include "Render/BufferedDraw.h"
#include <cstddef>
// DESVIACION: sombras proyectadas en OpenGL (opción). GL dibuja en el momento,
// así que durante la escena se guarda la geometría que proyecta (en el mundo)
// y al cerrarla se arma el mapa de sombras y se oscurece la imagen con un pase
// de pantalla completa que reconstruye cada píxel desde la profundidad.
namespace ShadowMapGL {
struct CasterVertex { float position[3], uv[2]; };
// Escena abierta, con casting activo, y el estado actual proyecta sombra
// (depth test y escritura de profundidad, sin mezcla aditiva).
bool Capturing();
// Triángulos en el espacio del modelView actual; se pasan al mundo al guardar.
void Capture(const CasterVertex* triangles, std::size_t count);
// Envío de BufferedDraw con la numeración de primitivas de OpenGL 1.1.
void CaptureImmediate(unsigned mode, const BufferedDraw::Vertex* vertices, std::size_t count);
}
