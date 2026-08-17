#pragma once
// Monster.h — Entity_FindById/Spawn, Monster_LoadModel, BMD_Load, status effects.
int  __cdecl Entity_FindById(int key);
DWORD __cdecl Monster_Spawn(int type, int x, int y, int key);
void __cdecl Entity_Init(DWORD entity);
