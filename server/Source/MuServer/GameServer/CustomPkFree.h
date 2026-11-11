#pragma once

#include "MapZones.h"

struct MOVE_PKFREE_INFO
{
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
};

class CPKFree
{
public:

	CPKFree();

	~CPKFree();

	void Load(const char* path);

	// Fase 5: los rectángulos de los Data/Maps/NN-Nombre/Zones.toml.
	void SetRects(const std::vector<ZoneRect>& rects);

	bool CheckPKFree(int map, int x, int y);

private:

	std::vector<MOVE_PKFREE_INFO> m_PKFreeInfo;
};

extern CPKFree gPKFree;
