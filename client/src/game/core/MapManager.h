#pragma once
// MapManager.h — CMapManager: datos por mapa (nombre, nado, música y minimapa).
//
// En el binario esos datos están repartidos en comparaciones contra `World`:
//   IDA: GetMapName (0x004EF120) y la copia inline de RenderParty (0x004EF44F)
//   IDA: la selección de BGM de Game_MainLoop (0x00526D0C..0x00527475)
//   IDA: los `World == 7` (Atlans) que deciden si se nada: SetPlayerStop,
//        SetPlayerWalk, el sonido de pasos (0x00451A90) y el arma a la espalda
//        de RenderCharacter
// El DLL de inyección los reemplazaba con hooks en esas mismas direcciones y una
// tabla por mapa (MapManager.cpp / Encoder/MapManager.txt). Acá la tabla es
// nativa y sus valores por defecto reproducen el comportamiento del binario.
//
// DESVIACION respecto del DLL: el DLL elegía la música sólo por mapa y sonaba en
// todo el mapa. Se conservan las reglas del binario: en Lorencia, Devias y Noria
// la música suena sólo en zona segura, y la taberna de Lorencia y la catedral de
// Devias tienen su propio tema.
//
// DESVIACION (Fase 5): el tema de cada mapa sale, en este orden, de
//   1. el server (`music` en su Map.toml, sección F3:F7 del catálogo);
//   2. la tabla del binario (Lorencia, Devias, Noria y Dungeon);
//   3. un mp3 de Content/audio/music/map (o Data/Music) con el nombre del mapa (Atlans.mp3, tarkan.mp3,
//      lost_tower_a.mp3...), que en el 0.97k no sonaba en ningún mapa.
// Así un tema propio se agrega dejando el archivo, y el server lo puede
// reemplazar por cualquier otro.  Los temas por zona (taberna, catedral y la
// segunda variante en los pisos 5 a 7 de Lost Tower) aplican sólo si el server
// no fijó un tema para ese mapa.

#include <string>

class CMapManager {
public:
    static constexpr int MaxMaps = 32;   // DLL: MAX_MAPS

    // Modo del minimapa (DLL MapManager.txt, columna MiniMap). La imagen no es
    // un asset por mapa: CMiniMap la arma al cargar el mapa a partir de
    // TerrainWall (.att), así que todo mapa integrado la tiene.
    enum class MiniMapMode : unsigned char { None = 0, MiniMap = 1, FullMap = 2 };

    struct MapInfo {
        bool swimmable = false;          // se nada en vez de caminar (Atlans)
        bool musicSafeZoneOnly = false;  // la música arranca sólo en zona segura
        const char* name = nullptr;      // nullptr = GlobalText, como GetMapName
        const char* music = nullptr;     // nullptr = sin música
        MiniMapMode miniMap = MiniMapMode::None;
    };

    CMapManager();

    int GetCurrentMap() const { return m_CurrentMap; }
    void SetCurrentMap(int map) { m_CurrentMap = map; }

    // Nombre del mapa para el chat y el panel de party.
    const char* GetName(int map) const;

    bool IsSwimmable(int map) const;

    MiniMapMode GetMiniMap(int map) const;

    // Selección de la música de fondo; se llama una vez por frame con el héroe
    // en el mundo (IDA Game_MainLoop, g_GameState == 5).
    void UpdateMusic() const;

    // Corta la música del mapa que esté sonando (IDA StopMusic 0x00513420).
    void StopMusic() const;

    // Tema que fija el server para un mapa: nombre del mp3 sin extensión (Data/Music, ver Content/alias.toml)
    // (letras, números, '_' y '-').  false si el nombre no es válido.
    bool SetServerMusic(int map, const char* name);
    // Al desconectar: vuelven los temas del cliente.
    void ClearServerMusic();

private:
    const MapInfo* Get(int map) const;
    const char* SelectZoneMusic(int map) const;
    // Tema del mapa según el orden de arriba; nullptr = sin música.
    const char* ResolveMusic(int map) const;
    void DetectMusicFiles() const;
    bool IsMapTrack(int map, const char* track) const;
    bool IsAnyMapTrack(const char* track) const;

    int m_CurrentMap = 0; // IDA: World (0x0055A7AC)
    MapInfo m_Maps[MaxMaps];
    std::string m_ServerMusic[MaxMaps];            // ruta completa, "" = la del cliente
    mutable std::string m_DetectedMusic[MaxMaps];  // ruta completa, "" = no hay archivo
    mutable std::string m_DetectedSecond[MaxMaps]; // segunda variante (lost_tower_b)
    mutable bool m_MusicDetected = false;
};

extern CMapManager gMapManager;
