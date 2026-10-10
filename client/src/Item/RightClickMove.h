#pragma once
// RightClickMove.h — click derecho para mover y equipar items.
//
// DESVIACION (DLL RightClickMove.cpp, MU 5.2 LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY):
// el 0.97k sólo mueve con click derecho entre el inventario y el baúl.  Acá
// además:
//   * inventario -> trade / chaos (sin mezcla en curso) / baúl, y de vuelta;
//   * inventario -> casillero de equipo, si no hay ninguna de esas ventanas;
//   * casillero de equipo -> primer hueco libre del inventario.
// Todo usa el mismo 0x24 que el arrastre; el server sigue validando.

#include <windows.h>

struct ITEM;

// Valida si el item puede ir en el casillero `slot` (0..11).  Es la misma
// regla que pinta la casilla de verde/rojo al arrastrar (sub_4CDC70).
bool Equip_CanPlace(const ITEM* item, int slot);

// Casillero de equipo vacío donde va el item, o -1.
int RightClickMove_FindEquipSlot(const ITEM* item);

// Primera celda libre (0..63) del inventario con lugar para el tipo, o -1.
int RightClickMove_FindInventorySlot(int type);
