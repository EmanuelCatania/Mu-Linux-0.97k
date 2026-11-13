#pragma once

namespace TextEngine { struct ShapedText; }

// Atlas dinámico de glifos (C2 del plan Unicode). Los glifos se rasterizan la
// primera vez que se usan y se suben a páginas de textura por la capa Gfx, así
// el texto se dibuja como quads del lote igual con cualquier backend.
namespace TextAtlas {
// Dibuja un texto ya armado en el sistema de coordenadas actual de la UI
// (x a la derecha, y hacia arriba), con la línea base en `baseline`.
// El estado de mezcla lo pone el caller; esto sólo activa la textura.
void Draw(const TextEngine::ShapedText& text, float x, float baseline,
          unsigned char r, unsigned char g, unsigned char b, unsigned char a);
// Bytes de textura ocupados por las páginas (para el perfil de render).
unsigned long long Bytes();
// Libera las páginas (cambio de fuente o de contexto).
void Release();
}
