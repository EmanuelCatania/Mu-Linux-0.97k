#pragma once
// UI.h — Inventory, equipment, stats, Item_GetType, CharData_RecalcStats.
void __cdecl UI_Render(void);
void __cdecl CharData_RecalcStats(DWORD entity);
int  __cdecl Item_GetType(BYTE* item_data);
