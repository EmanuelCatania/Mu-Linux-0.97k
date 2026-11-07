#pragma once

#include "ContentCatalog.h"
#include "User.h"

// CustomPet (0.97.20): pets definidos por datos (Data/Custom/Pets/*.json).
// El cliente recibe en el catálogo cómo se mueve y se ve el pet; el server
// ejecuta sus habilidades (levantar zen, etc.), que son las que tienen
// consecuencias de juego.

struct CUSTOM_PET_PICKUP
{
	bool Enabled;
	int Range;          // en casillas alrededor del dueño
	DWORD Interval;     // ms entre pasadas
	DWORD Delay;        // ms desde que el pet elige un zen hasta levantarlo
};

struct CUSTOM_PET_INFO
{
	int Item;
	CATALOG_PET Catalog;
	CUSTOM_PET_PICKUP Pickup;
};

class CCustomPet
{
public:

	CCustomPet();

	void Load(const char* folder);

	const CUSTOM_PET_INFO* GetInfo(int item) const;

	// Por segundo, para cada jugador conectado.
	void MainProc(LPOBJ lpObj);

	const std::vector<CATALOG_PET>& GetCatalog() const { return this->m_Catalog; }

private:

	void PickupMoney(LPOBJ lpObj, const CUSTOM_PET_PICKUP& pickup);

	bool CanTakeMoney(LPOBJ lpObj, int index, int range) const;

	std::map<int, CUSTOM_PET_INFO> m_Info;

	std::vector<CATALOG_PET> m_Catalog;

	DWORD m_LastPickup[MAX_OBJECT];

	// Zen que el pet de cada jugador está yendo a buscar, y desde cuándo.
	int m_Target[MAX_OBJECT];

	DWORD m_TargetTime[MAX_OBJECT];
};

extern CCustomPet gCustomPet;
