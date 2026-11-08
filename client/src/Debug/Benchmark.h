#pragma once

// Modo benchmark de render — DESVIACION, no existe en el binario.
//
// Pone N personajes de prueba alrededor del héroe con clase, set, armas,
// alas, pet y nivel de items al azar, los hace caminar y, fuera de la zona
// segura, les hace lanzar skills de su clase.  Sirve para medir cuánto cuesta
// dibujar muchos personajes a la vez sin necesitar cuentas ni server.
//
// Los personajes entran por los MISMOS handlers de red que usa el server
// (0x12 viewport, 0x10 movimiento, 0x19 y 0x1E skills, 0x14 baja), así que
// se dibujan y animan por el camino real.  Son locales: el server no los ve
// y no se les puede pegar.
//
// Sólo se compila con MU_BENCHMARK definido: siempre en Debug, y en Release
// únicamente configurando con -DMU_BENCHMARK=ON.  Las Releases publicadas no
// lo traen.
//
// Uso, ya dentro del mundo:
//   F9        pone o saca los personajes de prueba.
//   Shift+F9  activa o desactiva los skills (vienen activados).
// Variables de entorno opcionales:
//   MU_BENCH_COUNT   cantidad de personajes (por defecto 100, tope 350).
//   MU_BENCH_RADIUS  radio en casillas alrededor del héroe (por defecto 10).

#if defined(MU_BENCHMARK)
namespace Benchmark {
void Tick();                    // una vez por paso de simulación, en el mundo
bool Active();
const char* Status();           // línea para el contador de FPS
}
#endif
