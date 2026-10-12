#pragma once
// MiniMap.h — mapa del terreno superpuesto (tecla Tab).
//
// DESVIACION (DLL MiniMap.cpp): el 0.97k no tiene mapa.  Se porta el modo
// "mapa completo" del DLL, que es el que CMapManager asigna a los mapas 0-16:
// el terreno rotado 45° sobre la pantalla, con el héroe, jugadores, monstruos,
// NPCs y puertas, más los botones de zoom y transparencia.  La imagen no es un
// asset: se arma con TerrainWall cada vez que cambia el mapa.  El modo de
// minimapa en una esquina no se porta porque ningún mapa lo usa.

#include <windows.h>

class CMiniMap {
public:
    bool IsOpen() const { return m_Open; }
    void Toggle();
    void Close() { m_Open = false; }
    void UpdateMouse();
    void Render();
    // Los dragones del marco inferior quedan debajo de los botones del mapa.
    bool HidesLeftDragon() const { return m_Open; }
    bool HidesRightDragon() const { return m_Open; }

private:
    bool Available() const;
    void BuildTexture();
    void RenderBackground() const;
    void RenderMarkers() const;
    void RenderGates() const;
    void RenderButton(bool left) const;
    void ChangeZoom();
    void ChangeAlpha();

    bool  m_Open = false;
    bool  m_Loaded = false;      // Alpha y ZoomLevel leídos de Config.ini
    unsigned int m_Texture = 0;  // GLuint
    int   m_TextureWorld = -2;   // mapa con el que se armó la textura
    int   m_Zoom = 0;            // 0..6
    int   m_Alpha = 10;          // 3..10 (x10 %)
};

extern CMiniMap gMiniMap;
