#pragma once
#include <string>
#include <vector>

// Capas y alias de las rutas de datos (Fase 5, E2). Ver doc del plan de
// orden de Data: el código portado sigue pidiendo "Data\Object8\Object01.bmd" y
// esta capa decide de dónde sale el archivo, sin tocar a los que lo piden.
//
// Capas, de la que gana a la que pierde:
//   1. Servers/<id>/   contenido del server al que se está conectado
//   2. Packs/<nombre>/ packs opcionales del jugador, en orden alfabético
//   3. Content/        contenido del juego
//   4. Data/           el orden de hoy (la ruta tal cual se pidió)
// Las capas 1 a 3 reflejan el interior de Data: "Data\Object8\x.bmd" se busca
// como "<capa>/Object8/x.bmd". Content/alias.toml traduce además carpetas
// viejas a las nuevas ("Object8" = "map/atlans/objects"), y en las capas 1 a 3
// se prueba primero la ruta traducida y después la vieja.
//
// Sólo afecta a las lecturas de rutas relativas que empiezan por Data/. Las
// escrituras y el resto de las rutas (User/, Config.ini...) no pasan por acá.
// Sin Servers/, Packs/ ni Content/ no cambia nada.
namespace Platform::ContentLayers {
// Arma las capas desde el directorio actual (Packs/*, Content/ y su alias.toml).
void Init();
// Capa del server ("" = ninguna). El id es el mismo de User/Cache/<id>.
void SetServer(const std::string& id);
// Un lugar donde buscar `path`: la ruta y su capa (0 = la que más gana; Data
// es la última).
struct Candidate { std::string path; int layer; };
// Candidatos para leer `path`, en orden. Vacío si la ruta no pasa por las
// capas (se usa tal cual).
std::vector<Candidate> Candidates(const char* path);
// Resumen para el log: capas activas y cantidad de alias.
std::string Describe();
}
