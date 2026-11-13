#pragma once
#include <cstddef>
// DESVIACION: almacenamiento contiguo que crece sin mover los slots existentes.
// El techo de dirección corresponde al short de los modelos/BMD, no a un
// cupo del catálogo. Las páginas físicas se asignan sólo al solicitar slots.
namespace ResourceTables {
void EnsureModels(std::size_t count);
void EnsureTextures(std::size_t count);
std::size_t ModelCount();
std::size_t TextureCount();
void* ModelBase();
char* TextureBase();
void ResetModels();
}
