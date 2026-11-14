#pragma once

#include <bitset>
#include <string>
#include <vector>

// Propiedades de cada mapa (Content/map/maps.toml). Un mapa sin sección usa
// los valores por defecto de abajo.

enum class MapMiniMap : unsigned char { None = 0, MiniMap = 1, FullMap = 2 };

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

    bool IsIndoor(int tile) const { return tile >= 0 && tile < 256 && indoorTiles.test(tile); }
};

// Propiedades del mapa; los mapas sin sección devuelven las de por defecto.
const MapInfo& MapInfo_Get(int map);

// Todos los sonidos que algún mapa declara en `ambient`: los que hay que cortar
// cuando el mapa actual no los usa.
const std::vector<int>& MapInfo_AllAmbient();
