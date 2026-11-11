#pragma once

#include "User.h"

struct GATE_INFO
{
	int Index;
	int Flag;
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
	int TargetGate;
	int Dir;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int AccountLevel;
};

class CGate
{
public:

	CGate();

	~CGate();

	void Load(const char* path);

	// Fase 5: desde Data/Maps/NN-Nombre/Gates.toml (`path` es Data/). Llena la
	// misma tabla que Load con Gate.txt.
	void LoadToml(const char* path);

	bool GetInfo(int index, GATE_INFO* lpInfo);

	int GetGateMap(int index);

	int GetMoveLevel(LPOBJ lpObj, int map, int level);

	bool IsGate(int index);

	bool IsInGate(LPOBJ lpObj, int index);

	bool GetGate(int index, int* gate, int* map, int* x, int* y, int* dir, int* level);

	const std::map<int, GATE_INFO>& GetAll() const { return this->m_GateInfo; }

private:

	std::map<int, GATE_INFO> m_GateInfo;
};

extern CGate gGate;
