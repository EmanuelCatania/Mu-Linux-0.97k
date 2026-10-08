#include "stdafx.h"
#include "Game/FrameLimiter.h"

CFrameLimiter gFrameLimiter;

// IDA: Game_MainLoop (0x00525D40), espera posterior al render.
// DESVIACION fase 2d: ceder CPU entre lecturas del reloj; conservar el plazo de 40 ms.
DWORD CFrameLimiter::Wait(DWORD renderStart) const
{
    DWORD elapsed = GetTickCount() - renderStart;
    while (elapsed < 40) {
        Sleep(1);
        elapsed = GetTickCount() - renderStart;
    }
    // Incluir cualquier demora del scheduler para conservar el catch-up original.
    return elapsed;
}
