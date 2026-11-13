#pragma once

#include <cstdint>

// Las tablas de apilado y precios pertenecen a la conexión con el GameServer.
void ItemServerValue_ResetSession();

// Secciones F3:F8 (apilado) y F3:F9 (precios) del catálogo.  Una sección con un
// registro inválido no se aplica y queda la tabla anterior.
bool ItemServerValue_SetStacks(const std::uint8_t* records, int recordSize, int count);
bool ItemServerValue_SetValues(const std::uint8_t* records, int recordSize, int count);

// Máximo de apilado del item (F3:F8), o 0 si no se apila.
int __cdecl ItemStack_GetMaxStack(int index, int level);
