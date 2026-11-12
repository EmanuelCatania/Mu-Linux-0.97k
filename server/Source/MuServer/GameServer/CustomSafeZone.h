#pragma once

#include "MapZones.h"

struct SAFE_ZONE_INFO
{
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
};

class CSafeZone
{
public:

	CSafeZone();

	~CSafeZone();

	void Load(const char* path);

	// Fase 5: los rectángulos de los Data/Maps/NN-Nombre/Zones.toml.
	void SetRects(const std::vector<ZoneRect>& rects);

	bool CheckSafeZone(int map, int x, int y);

private:

	std::vector<SAFE_ZONE_INFO> m_SafeZoneInfo;
};

extern CSafeZone gSafeZone;
