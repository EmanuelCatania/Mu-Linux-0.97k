#pragma once

#include <bitset>
#include <string>
#include <utility>
#include <vector>

// Propiedades de cada mapa (Content/map/maps.toml). Un mapa sin sección usa
// los valores por defecto de abajo.

enum class MapMiniMap : unsigned char { None = 0, MiniMap = 1, FullMap = 2 };

// Una carga de [[map.N.load]], en el orden del archivo (OpenWorldModels).
struct MapLoad {
    enum class Kind : unsigned char { Model, Texture, Jpg } kind = Kind::Model;
    int         slot = 0;
    std::string path;      // Model: carpeta "Data/ObjectN/"; Texture: carpeta; Jpg: archivo
    std::string name;      // Model: nombre del .bmd sin número
    int         index = 0; // Model: número del archivo (-1 = sin número)
    bool        repeat = false;   // Jpg: GL_REPEAT en vez de GL_CLAMP
};

struct MapInfo {
    std::string name;                 // vacío = GlobalText, como GetMapName
    std::string music;                // archivo en Data/Music; vacío = sin música fija
    bool        musicSafeZoneOnly = false;  // la música arranca sólo en zona segura
    bool        swimmable = false;    // se nada en vez de caminar (Atlans)
    bool        mountFlies = false;   // el Dinorant vuela alto (Tarkan, Icarus)
    bool        autoAttack = true;    // el ataque automático funciona (no en la arena)
    bool        weather = false;      // hojas, nieve o lluvia (WeatherParticles_Update)
    std::vector<int> ambient;         // sonidos ambientales que el mapa deja sonar
    std::bitset<256> indoorTiles;     // HeroTile bajo techo: sin clima, techos ocultos
    MapMiniMap  miniMap = MapMiniMap::None;
    int         objectFolder = -1;    // Data/Object<N>; -1 = número de mapa + 1
    // [map.N.objects]: nombre de cada modelo de objeto, en el orden del archivo.
    // Sin esta sección los slots 0..159 cargan Object01..Object160.
    std::vector<std::pair<int, std::string>> objects;
    std::vector<MapLoad> loads;       // [[map.N.load]]
    int         loadFrom = -1;        // usa las cargas de otro mapa (load_from)

    bool IsIndoor(int tile) const { return tile >= 0 && tile < 256 && indoorTiles.test(tile); }
};

// Carpeta de objetos del mapa (Data/Object<N>).
int MapInfo_ObjectFolder(int map);

// Propiedades del mapa; los mapas sin sección devuelven las de por defecto.
const MapInfo& MapInfo_Get(int map);

// Todos los sonidos que algún mapa declara en `ambient`: los que hay que cortar
// cuando el mapa actual no los usa.
const std::vector<int>& MapInfo_AllAmbient();
