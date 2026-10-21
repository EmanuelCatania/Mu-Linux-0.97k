#pragma once

// DESVIACION: conservar el bitmap y el avance de la fuente de cada backend,
// pero enviar sus glifos como quads del batcher durante el dibujo del HUD.
namespace GlyphAtlas {
bool Build(unsigned base, int height, int maxWidth);
bool Draw(unsigned base, const char* text, int count, int width, int height);
void Forget(unsigned base);
void Release();
}
