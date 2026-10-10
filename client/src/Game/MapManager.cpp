// MapManager.cpp — CMapManager. Ver MapManager.h.

#include "stdafx.h"
#include "Game/MapManager.h"
#include "Sound/SoundManager.h"

CMapManager gMapManager;

namespace {
// Rutas de la tabla g_lpszMp3 del binario (IDA 0x005615C4). El binario apunta a
// nombres coreanos que el pack no trae; nuestro pack los tiene renombrados:
//   [0] taberna = Pub   [1] MuTheme = Lorencia   [2] catedral = Cathedral
//   [3] Devias          [4] Noria                [5] Dungeon
// La catedral es el Church.mp3 del cliente japonés 0.98.
const char kMusicPub[]       = "Data\\Music\\Pub.mp3";
const char kMusicLorencia[]  = "Data\\Music\\Lorencia.mp3";
const char kMusicCathedral[] = "Data\\Music\\Cathedral.mp3";
const char kMusicDevias[]    = "Data\\Music\\Devias.mp3";
const char kMusicNoria[]     = "Data\\Music\\Noria.mp3";
const char kMusicDungeon[]   = "Data\\Music\\Dungeon.mp3";

constexpr int kLorencia = 0;
constexpr int kDungeon  = 1;
constexpr int kDevias   = 2;
constexpr int kNoria    = 3;
constexpr int kDungeon2 = 5;   // el binario le pone la música del Dungeon
constexpr int kAtlans   = 7;
constexpr int kMapCount = 17;  // mapas del 0.97k: 0..16 (Blood Castle 1..6 = 11..16)

// HeroTile 4 = el piso de la taberna de Lorencia.
constexpr int kPubTile = 4;

bool InSafeZone()
{
    return Hero && *(Hero + 0x34E) != 0;   // c->SafeZone
}

// IDA Game_MainLoop: la catedral de Devias es el rectángulo de celdas
// 205..214 x 13..31 (lee Hero+904 / Hero+908).
bool InDeviasCathedral()
{
    if (!Hero) return false;
    const int x = *(int*)(Hero + 904);
    const int y = *(int*)(Hero + 908);
    return x > 204 && x < 215 && y > 12 && y < 32;
}
}

CMapManager::CMapManager()
{
    MapInfo& lorencia = m_Maps[kLorencia];
    lorencia.music = kMusicLorencia;
    lorencia.musicSafeZoneOnly = true;

    MapInfo& devias = m_Maps[kDevias];
    devias.music = kMusicDevias;
    devias.musicSafeZoneOnly = true;

    MapInfo& noria = m_Maps[kNoria];
    noria.music = kMusicNoria;
    noria.musicSafeZoneOnly = true;

    m_Maps[kDungeon].music  = kMusicDungeon;
    m_Maps[kDungeon2].music = kMusicDungeon;

    m_Maps[kAtlans].swimmable = true;

    // Como el MapManager.txt del DLL: minimapa completo en todos los mapas.
    for (int map = 0; map < kMapCount; ++map)
        m_Maps[map].miniMap = MiniMapMode::FullMap;
}

const CMapManager::MapInfo* CMapManager::Get(int map) const
{
    if (map < 0 || map >= MaxMaps) return nullptr;
    return &m_Maps[map];
}

// IDA: GetMapName (0x004EF120), con el nombre propio de la tabla adelante.
const char* CMapManager::GetName(int map) const
{
    const MapInfo* info = Get(map);
    if (info && info->name) return info->name;

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

// Temas de una zona del mapa que reemplazan al del mapa (reglas del binario).
const char* CMapManager::SelectZoneMusic(int map) const
{
    if (map == kLorencia && HeroTile == kPubTile) return kMusicPub;
    if (map == kDevias && InDeviasCathedral())     return kMusicCathedral;
    return nullptr;
}

bool CMapManager::IsMapTrack(int map, const char* track) const
{
    const MapInfo* info = Get(map);
    if (!info) return false;
    if (info->music && _stricmp(info->music, track) == 0) return true;
    if (map == kLorencia && _stricmp(kMusicPub, track) == 0) return true;
    if (map == kDevias && _stricmp(kMusicCathedral, track) == 0) return true;
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
    if (gSound.GetCurrentTrack()[0] && !IsMapTrack(World, gSound.GetCurrentTrack()))
        Music_StopTrack((DWORD)(uintptr_t)gSound.GetCurrentTrack(), 0);

    const MapInfo* info = Get(World);
    if (!info || !info->music) return;
    if (info->musicSafeZoneOnly && !InSafeZone()) return;

    const char* track = SelectZoneMusic(World);
    Music_PlayTrack((DWORD)(uintptr_t)(track ? track : info->music), 0);
}

// IDA: StopMusic (0x00513420) recorre los 6 temas de g_lpszMp3 con StopMp3; o
// sea corta el que esté sonando si es un tema de mapa.
void CMapManager::StopMusic() const
{
    if (gSound.GetCurrentTrack()[0] && IsAnyMapTrack(gSound.GetCurrentTrack()))
        Music_StopTrack((DWORD)(uintptr_t)gSound.GetCurrentTrack(), 0);
}
