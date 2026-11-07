#pragma once
#include <vector>

// DESVIACION: el texto heredado (wglUseFontBitmaps + glCallLists) se dibuja
// como quads del batcher sobre un atlas, con la misma semántica de glBitmap:
// posición raster, color fijado al tomarla y avance por glifo.
namespace GlyphAtlas {
// Un glifo como lo emitía glBitmap: máscara 0/255 con las filas de abajo hacia
// arriba, desplazamiento del bitmap respecto del origen y avance en píxeles.
struct Bitmap {
    int left=0,bottom=0,width=0,height=0;
    float advanceX=0,advanceY=0;
    std::vector<unsigned char> coverage;
};
// Arma el atlas de `count` glifos (código de carácter = índice) bajo `key`.
bool Build(unsigned key,const Bitmap* glyphs,unsigned count);
// glRasterPos2f(x, y) con el color actual + glCallLists(text) sobre un
// viewport de width x height. Devuelve false si no hay atlas para `key`.
bool Draw(unsigned key,float x,float y,const char* text,int count,int width,int height);
void Forget(unsigned key);
void Release();
}
