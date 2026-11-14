// MapManager.cpp — CMapManager. Ver MapManager.h.

#include "stdafx.h"
#include "platform/Text.h"
#include "platform/IFileSystem.h"
#include "game/core/MapManager.h"
#include "audio/SoundManager.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <vector>

CMapManager gMapManager;

namespace {
// Rutas de la tabla g_lpszMp3 del binario (IDA 0x005615C4). El binario apunta a
// nombres coreanos que el pack no trae; nuestro pack los tiene renombrados:
//   [0] taberna = Pub   [1] MuTheme = Lorencia   [2] catedral = Cathedral
//   [3] Devias          [4] Noria                [5] Dungeon
// La catedral es el Church.mp3 del cliente japonés 0.98.
const char kMusicPub[]       = "Data\\Music\\Pub.mp3";
const char kMusicCathedral[] = "Data\\Music\\Cathedral.mp3";

constexpr int kLorencia = 0;
constexpr int kDevias   = 2;
constexpr int kLostTower = 4;

// HeroTile 4 = el piso de la taberna de Lorencia.
constexpr int kPubTile = 4;

constexpr char kMusicDir[] = "Data\\Music\\";

// Nombre con que se busca el mp3 de cada mapa en Data/Music (los mismos que las
// carpetas Data/Maps del server).  Blood Castle 1..6 comparte uno.
const char* const kMusicFileNames[] = {
    "Lorencia", "Dungeon", "Devias", "Noria", "LostTower", "Exile", "Arena",
    "Atlans", "Tarkan", "DevilSquare", "Icarus",
    "BloodCastle", "BloodCastle", "BloodCastle", "BloodCastle", "BloodCastle", "BloodCastle",
};

// Para comparar nombres: minúsculas y sin '_', '-' ni espacios, así
// "lost_tower_a" y "LostTower" se reconocen igual.
std::string MusicKey(const std::string& name)
{
    std::string key;
    for (unsigned char c : name)
        if (std::isalnum(c)) key += (char)std::tolower(c);
    return key;
}

bool ValidMusicName(const char* name)
{
    const size_t length = name ? strlen(name) : 0;
    if (length == 0 || length > 27) return false;
    for (const char* p = name; *p; ++p)
        if (!std::isalnum((unsigned char)*p) && *p != '_' && *p != '-') return false;
    return true;
}

bool InSafeZone()
{
    return Hero && *(Hero + EntityOffset(0x34E)) != 0;   // c->SafeZone
}

// IDA Game_MainLoop: la catedral de Devias es el rectángulo de celdas
// 205..214 x 13..31 (lee Hero+904 / Hero+908).
bool InDeviasCathedral()
{
    if (!Hero) return false;
    const int x = *(int*)&((CHARACTER*)(uintptr_t)(Hero))->PositionX;
    const int y = *(int*)&((CHARACTER*)(uintptr_t)(Hero))->PositionY;
    return x > 204 && x < 215 && y > 12 && y < 32;
}

// Pisos 5 a 7 de Lost Tower.  Cada piso es un área separada por paredes en
// Terrain5.att: 1 = x 162..247 y 1..138, 2 = x 162..248 y 164..250,
// 3 = x 80..135 y 165..250, 4 = x 80..135 y 85..140, 5 = x 80..135 y 5..60,
// 6 = x 2..57 y 5..60, 7 = x 2..57 y 85..250.
bool InLostTowerUpperFloors()
{
    if (!Hero) return false;
    const int x = *(int*)&((CHARACTER*)(uintptr_t)(Hero))->PositionX;
    const int y = *(int*)&((CHARACTER*)(uintptr_t)(Hero))->PositionY;
    return x < 70 || (x < 140 && y < 70);
}
}

CMapManager::CMapManager() = default;

const CMapManager::MapInfo* CMapManager::Get(int map) const
{
    if (map < 0 || map >= MaxMaps) return nullptr;
    return &MapInfo_Get(map);
}

// IDA: GetMapName (0x004EF120), con el nombre propio de la tabla adelante.
const char* CMapManager::GetName(int map) const
{
    const MapInfo* info = Get(map);
    if (info && !info->name.empty()) return info->name.c_str();

    if (map >= 11 && map <= 16) return GlobalText[56];   // Blood Castle
    if (map == 10)              return GlobalText[55];   // Icarus
    if (map < 17)               return GlobalText[map + 30];
    return GlobalText[map + 40];
}

bool CMapManager::IsSwimmable(int map) const
{
    const MapInfo* info = Get(map);
    return info && info->swimmable;
}

CMapManager::MiniMapMode CMapManager::GetMiniMap(int map) const
{
    const MapInfo* info = Get(map);
    return info ? info->miniMap : MiniMapMode::None;
}

const char* CMapManager::ResolveMusic(int map) const
{
    if (map < 0 || map >= MaxMaps) return nullptr;
    if (!m_ServerMusic[map].empty()) return m_ServerMusic[map].c_str();
    const std::string& configured = MapInfo_Get(map).music;
    if (!configured.empty()) {
        // Se arma una sola vez: el puntero tiene que seguir siendo el mismo.
        if (m_ConfiguredMusic[map].empty()) m_ConfiguredMusic[map] = std::string(kMusicDir) + configured;
        return m_ConfiguredMusic[map].c_str();
    }
    DetectMusicFiles();
    return m_DetectedMusic[map].empty() ? nullptr : m_DetectedMusic[map].c_str();
}

// Busca una vez los mp3 de temas de mapa que llevan el nombre de un mapa: el nombre
// exacto (Atlans.mp3), o con una letra o un número de variante
// (lost_tower_a.mp3).  Si hay varias variantes gana la primera en orden
// alfabético, como 5.2, que en Lost Tower sólo usa lost_tower_a.  Las que llevan
// una palabra después (devil_square_intro.mp3) no cuentan: son cortinas de un
// evento, no el tema del mapa.
void CMapManager::DetectMusicFiles() const
{
    if (m_MusicDetected) return;
    m_MusicDetected = true;
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    // Fase 5 (E2): la carpeta se lista a través de las capas, así cuentan los
    // temas de Content/audio/music/map (alias "Music" de Content/alias.toml),
    // los de un pack o del server y los de Data/Music de una instalación vieja.
    for (const auto& entry : Platform::GetFileSystem().ListFiles("Data/Music")) {
        std::string ext = fs::path(entry.name).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        if (ext == ".mp3") files.push_back(entry.name);
    }
    std::sort(files.begin(), files.end());
    const int count = (int)(sizeof(kMusicFileNames) / sizeof(kMusicFileNames[0]));
    for (int map = 0; map < count && map < MaxMaps; ++map) {
        const std::string want = MusicKey(kMusicFileNames[map]);
        std::string exact;
        std::vector<std::string> variants;
        for (const std::string& file : files) {
            const std::string key = MusicKey(fs::path(file).stem().string());
            if (key == want && exact.empty()) exact = file;
            else if (key.size() == want.size() + 1 && key.compare(0, want.size(), want) == 0)
                variants.push_back(file);
        }
        const std::string& found = !exact.empty() ? exact : (variants.empty() ? exact : variants[0]);
        if (!found.empty()) m_DetectedMusic[map] = std::string(kMusicDir) + found;
        if (exact.empty() && variants.size() > 1) m_DetectedSecond[map] = std::string(kMusicDir) + variants[1];
    }
}

bool CMapManager::SetServerMusic(int map, const char* name)
{
    if (map < 0 || map >= MaxMaps || !ValidMusicName(name)) return false;
    m_ServerMusic[map] = std::string(kMusicDir) + name + ".mp3";
    return true;
}

void CMapManager::ClearServerMusic()
{
    for (std::string& music : m_ServerMusic) music.clear();
}

// Temas de una zona del mapa que reemplazan al del mapa (reglas del binario).
const char* CMapManager::SelectZoneMusic(int map) const
{
    if (map >= 0 && map < MaxMaps && !m_ServerMusic[map].empty()) return nullptr;
    if (map == kLorencia && HeroTile == kPubTile) return kMusicPub;
    if (map == kDevias && InDeviasCathedral())     return kMusicCathedral;
    if (map == kLostTower && InLostTowerUpperFloors()) {
        DetectMusicFiles();
        if (!m_DetectedSecond[map].empty()) return m_DetectedSecond[map].c_str();
    }
    return nullptr;
}

bool CMapManager::IsMapTrack(int map, const char* track) const
{
    const MapInfo* info = Get(map);
    if (!info) return false;
    const char* music = ResolveMusic(map);
    if (music && Platform::TextCompareIgnoreCase(music, track) == 0) return true;
    if (map == kLorencia && Platform::TextCompareIgnoreCase(kMusicPub, track) == 0) return true;
    if (map == kDevias && Platform::TextCompareIgnoreCase(kMusicCathedral, track) == 0) return true;
    if (!m_DetectedSecond[map].empty() && Platform::TextCompareIgnoreCase(m_DetectedSecond[map].c_str(), track) == 0) return true;
    return false;
}

bool CMapManager::IsAnyMapTrack(const char* track) const
{
    for (int map = 0; map < MaxMaps; ++map) {
        if (IsMapTrack(map, track)) return true;
    }
    return false;
}

// IDA Game_MainLoop (0x00526D0C..0x00527475), con g_GameState == 5:
//   - Lorencia, Devias y Noria: en zona segura suena el tema del mapa (o el de
//     la taberna / la catedral); fuera de la zona segura no se toca nada, así
//     que el tema que estaba sonando sigue hasta terminar.
//   - Dungeon (1 y 5): suena siempre.
//   - Cualquier tema de otro mapa se corta (StopMp3 de los 6 de la tabla).
// El último punto también corta el tema del login (MuTheme): en el binario el
// login usa el mismo tema que Lorencia, así que entra en ese corte.
void CMapManager::UpdateMusic() const
{
    if (gSound.GetCurrentTrack()[0] && !IsMapTrack(gMapManager.GetCurrentMap(), gSound.GetCurrentTrack()))
        Music_StopTrack(gSound.GetCurrentTrack(), 0);

    const MapInfo* info = Get(gMapManager.GetCurrentMap());
    const char* music = ResolveMusic(gMapManager.GetCurrentMap());
    if (!info || !music) return;
    if (info->musicSafeZoneOnly && !InSafeZone()) return;

    const char* track = SelectZoneMusic(gMapManager.GetCurrentMap());
    Music_PlayTrack(track ? track : music, 0);
}

// IDA: StopMusic (0x00513420) recorre los 6 temas de g_lpszMp3 con StopMp3; o
// sea corta el que esté sonando si es un tema de mapa.
void CMapManager::StopMusic() const
{
    if (gSound.GetCurrentTrack()[0] && IsAnyMapTrack(gSound.GetCurrentTrack()))
        Music_StopTrack(gSound.GetCurrentTrack(), 0);
}
