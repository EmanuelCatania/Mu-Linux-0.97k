#pragma once

// IDA: Skeleton_Transform (0x004404E0), MoveCharacterVisual (0x004520C0).
// Caja usada por el picking del port: esquina mínima y tres semiejes alineados.
// No aplica escala ni rotación: ambos callers conservan la geometría del binario.
// Sólo escribe el destino; permite preparar la caja SIM sin transformar mallas,
// tocar el BMD compartido ni utilizar los buffers de vértices del render.
inline void ObjectBounds_Build(const float* minimum, const float* maximum,
                               const float* origin, float* bounds)
{
    bounds[0] = minimum[0];
    bounds[1] = minimum[1];
    bounds[2] = minimum[2];
    bounds[3] = maximum[0] - minimum[0];
    bounds[7] = maximum[1] - minimum[1];
    bounds[11] = maximum[2] - minimum[2];
    bounds[0] += origin[0];
    bounds[1] += origin[1];
    bounds[2] += origin[2];
    bounds[4] = bounds[5] = bounds[6] = 0.0f;
    bounds[8] = bounds[9] = bounds[10] = 0.0f;
}
