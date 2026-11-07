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

private:
    const MapInfo* Get(int map) const;
    const char* SelectZoneMusic(int map) const;
    bool IsMapTrack(int map, const char* track) const;
    bool IsAnyMapTrack(const char* track) const;

    int m_CurrentMap = 0; // IDA: World (0x0055A7AC)
    MapInfo m_Maps[MaxMaps];
};

extern CMapManager gMapManager;
