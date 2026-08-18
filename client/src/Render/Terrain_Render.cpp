#include "stdafx.h"
#include "Render/Terrain_Render.h"

// External helpers
extern unsigned short __cdecl FUN_004f8ff0(float x, float y, float z);
extern void __cdecl FUN_004fc030(unsigned char *entity, unsigned int slot, int flag, char mode);
extern void __cdecl FUN_00405540(void *buf, const char *msg);
extern void __cdecl FUN_00403f80(void *ctx, void *obj, void *key);


// ── Mejora INTENCIONAL del cliente final (cámara mejorada del DLL) — NO es 1:1 ─
// El 0.97k culea objetos del mundo con CollisionRange (obj[+0xD0] ≈ -30 cross-
// units), margen pensado para la cámara estrecha original.  La cámara final del
// proyecto (DLL) muestra más área, lo que expone "popping" de estructuras
// grandes (paredes, pilares, puente de Lorencia): su anchor sale del trapezoide
// 2D mientras la parte alta del modelo sigue visible → el objeto entero
// desaparece.  Ampliamos el margen de cull (Range más negativo = más lenient)
// a un Range efectivo ≈ -300, mismo criterio que Mu 5.2 usa para objetos grandes
// (ZzzObject.cpp: CollisionRange -300 para estructuras).  Verificado contra IDA:
// el cull -30 es 1:1 con el 0.97k; esto es una desviación deliberada alineada a
// la cámara final, NO un fix de bug del port.
static const float OBJECT_CULL_EXTRA_MARGIN = 270.0f;   // -30 - 270 = Range ≈ -300

void FUN_004fd800(void)
{
    float z_offset = 0.0f;
    if (DAT_0055a7ac == 10) {
        z_offset = -10.0f;
    }

    DWORD *chunk_ptr = &DAT_083a021c;

    int chunk_y = 8;
    do
    {
        float chunk_yf = (float)chunk_y;
        int chunk_x = 8;
        do
        {
            unsigned short vis = FUN_004f8ff0(chunk_yf, (float)chunk_x, -180.0f);
            *((char*)chunk_ptr + 8) = (char)vis;

            if ((char)vis != '\0' || DAT_083a42e9 != '\0')
            {
                char *entity = (char*)*chunk_ptr;
                __try {
                    int chunkIter = 0;
                    while (entity != nullptr &&
                           chunkIter++ < 4096 &&
                           (uintptr_t)entity >= 0x10000u &&
                           (uintptr_t)entity < 0x80000000u)
                    {
                        if (*entity == '\0')
                        {
                            entity = *(char**)(entity + 0x1b8);
                            continue;
                        }

                        if (DAT_005615c0 != 5 && *(short*)(entity + 2) == 0)
                        {
                            entity = *(char**)(entity + 0x1b8);
                            continue;
                        }

                        float ex = *(float*)(entity + 0x10) * _DAT_005524f8;
                        float ey = *(float*)(entity + 0x14) * _DAT_005524f8;
                        // Range = z_offset + CollisionRange(obj[+0xD0]) - margen ampliado
                        // (mejora intencional, ver OBJECT_CULL_EXTRA_MARGIN arriba).
                        float ez = z_offset + *(float*)(entity + 0xd0) - OBJECT_CULL_EXTRA_MARGIN;
                        vis = FUN_004f8ff0(ex, ey, ez);
                        entity[0x160] = (char)vis;

                        if ((char)vis != '\0' || DAT_083a42e9 != '\0')
                        {
                            // Terrain_Render in the original only prepares and draws.
                            // Per-frame object animation/update belongs to MoveObjects.
                            if (DAT_0055a7ac == 2 && *(short*)(entity + 2) == 100)
                            {
                                void *pvSlot = operator_new(0x585);
                                *(unsigned char*)((char*)pvSlot + 0x584) = 1;
                                FUN_00403f80(&DAT_055c9bc8, pvSlot, DAT_07cf1ffc);
                            }

                            FUN_004fc030((unsigned char*)entity, 0, 0, '\0');
                            FUN_004fc070((int)entity);
                        }

                        entity = *(char**)(entity + 0x1b8);
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    // Corrupt node in chunk linked list; abandon this chunk.
                }
            }

            chunk_ptr += 4;
            chunk_x += 0x10;
        }
        while (chunk_x < 0x108);

        chunk_y += 0x10;
    }
    while (chunk_y < 0x108);

    // Login/char-select fallback: only seed the frustum-visible flag and let
    // Entity_RenderAll_3D drive the actual draw path.
    if (DAT_005615c0 != 5)
    {
        int iVar = 0;
        do {
            char *entity = (char*)(DAT_07abf5d0 + iVar);
            if (*entity != '\0') {
                float ex = *(float*)(entity + 0x10) * _DAT_005524f8;
                float ey = *(float*)(entity + 0x14) * _DAT_005524f8;
                float ez = z_offset + *(float*)(entity + 0xd0);
                unsigned short vis = FUN_004f8ff0(ex, ey, ez);
                entity[0x160] = (char)vis;
            }
            iVar += 0x394;
        } while (iVar < 0x59740);
    }
}
