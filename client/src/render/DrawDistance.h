#pragma once
// DESVIACION: distancia de dibujado (opción). Con un alcance mayor al normal,
// el terreno se dibuja hasta el borde pero los objetos del mapa y el pasto
// sólo dentro del radio de detalle alrededor del héroe; más allá queda el
// mapa plano bajo la niebla. Lo fija MoveMainCamera en cada cuadro.
namespace DrawDistance {
inline float heroX = 0.0f, heroY = 0.0f, detailRange = 0.0f;   // 0 = sin límite
inline void Set(float x, float y, float range) { heroX = x; heroY = y; detailRange = range; }
// true si el punto del mundo (x, y) va con todo el detalle.
inline bool Detailed(float x, float y) {
    if (detailRange <= 0.0f) return true;
    const float dx = x - heroX, dy = y - heroY;
    return dx * dx + dy * dy <= detailRange * detailRange;
}
}
