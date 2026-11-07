#pragma once

// Las tablas F3/E3 y F3/E4 pertenecen a la conexion con el GameServer.
void ItemServerValue_ResetSession();

// Máximo de apilado del item (F3/E3), o 0 si no se apila.
int __cdecl ItemStack_GetMaxStack(int index, int level);
