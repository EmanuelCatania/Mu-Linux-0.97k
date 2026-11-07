#pragma once

// Lee en segundo plano todos los archivos de `root` para dejarlos en la caché
// del sistema (ver DataPrefetch.cpp).
void StartDataPrefetch(const char* root);
