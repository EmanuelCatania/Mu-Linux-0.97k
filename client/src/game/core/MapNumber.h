#pragma once

// Números de mapa del 0.97k (el `World` del binario, hoy
// gMapManager.GetCurrentMap()). Coinciden con los del server.
enum eMapNumber
{
    MAP_LORENCIA      = 0,
    MAP_DUNGEON       = 1,
    MAP_DEVIAS        = 2,
    MAP_NORIA         = 3,
    MAP_LOST_TOWER    = 4,
    MAP_EXILE         = 5,
    MAP_ARENA         = 6,
    MAP_ATLANS        = 7,
    MAP_TARKAN        = 8,
    MAP_DEVIL_SQUARE1 = 9,
    MAP_ICARUS        = 10,
    MAP_BLOOD_CASTLE1 = 11,
    MAP_BLOOD_CASTLE2 = 12,
    MAP_BLOOD_CASTLE3 = 13,
    MAP_BLOOD_CASTLE4 = 14,
    MAP_BLOOD_CASTLE5 = 15,
    MAP_BLOOD_CASTLE6 = 16,
};

// Los seis niveles de Blood Castle comparten World12 y Object12.
inline bool IsBloodCastleMap(int map)
{
    return map >= MAP_BLOOD_CASTLE1 && map <= MAP_BLOOD_CASTLE6;
}
