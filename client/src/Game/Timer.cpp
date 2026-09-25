// Timer.cpp
// IDA: CalcFPS @ 0x0043fd70 — Timer_FrameUpdate
//
// Called once per frame to update timing globals:
//   DAT_05826e08  — current absolute time (ms, as float)
//   FrameTimeCurrentMs  — last frame timestamp (ms)
//   DeltaT — delta time (seconds per frame, clamped to 0.0001 min)
//   FPS — smoothed FPS (frames per second)
//
// Also maintains a 5-second sliding FPS average window.
//
// Globals:
//   DAT_05826e00/04/10/14 — FPS accumulator ring buffer
//   _DAT_005528a8 — reciprocal of 1000 (ms→s)
//   FloatZero — float 0.0 (guard for division)
//   _DAT_00552898 — FPS window interval (5.0 seconds)
//   _DAT_00552890 — delta-time scale (1/1000)

#include "stdafx.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// Base de tiempo de WorldTime: ms desde el arranque del proceso (ver abajo).
static DWORD WorldTimeEpochMs = 0;

void Timer_UpdateFrameTiming(void)
{
  double dVar1;
  DWORD DVar2;
  undefined8 local_8;

  if (FpsTimerInitialized == 0) {
    DAT_05826e10 = 0;
    FpsWindowStartTimeMs = timeGetTime();
    FpsTimerInitialized = 1;
  }
  DAT_05826e10 = DAT_05826e10 + 1;
  DAT_05826e14 = DAT_05826e14 + 1;
  DVar2 = timeGetTime();
  // DESVIACION DELIBERADA (2026-09-08).  El binario hace
  // `fild qword [timeGetTime()]; fstp dword WorldTime`, o sea guarda los ms
  // DESDE EL ARRANQUE DE WINDOWS en un float de 4 bytes.  Pasadas ~13 horas de
  // uptime el ULP del float supera 1 ms y WorldTime deja de poder representar
  // cada milisegundo; con 34 dias encendido el ULP es de 256 ms, asi que
  // WorldTime se queda quieto ~6 frames y despues salta de golpe.  Eso se ve en
  // el reloj de Blood Castle (`sub_4BF2D0` imprime `WorldTime % 60`), y tambien
  // en el cursor, el agua y las UV animadas -- todos consumidores del mismo
  // global (ver la nota de `(int)WorldTime` que satura a los 24.8 dias).
  //
  // El cliente de referencia NO tiene el problema porque el DLL de inyeccion
  // reemplaza CalcFPS entero (`SetCompleteHook(0xE9, 0x0043FD70, ...)`) y ahi
  // hace `WorldTime = (float)clock()`, o sea ms desde que arranco el PROCESO.
  // Se adopta la misma base: contando desde el arranque del cliente el float
  // conserva precision de 1 ms durante 4.6 horas y de 2 ms hasta las 9.3.
  if (WorldTimeEpochMs == 0) WorldTimeEpochMs = DVar2;
  local_8 = (double)(ulonglong)(DWORD)(DVar2 - WorldTimeEpochMs);
  DAT_05826e08 = (float)(longlong)local_8;
  FrameTimeCurrentMs = timeGetTime();
  dVar1 = (double)(int)(FrameTimeCurrentMs - FpsWindowStartTimeMs) * _DAT_005528a8;
  if (dVar1 == FloatZero) {
    local_8 = -1.0;
  }
  else {
    local_8 = (double)DAT_05826e10 / dVar1;
  }
  if ((_DAT_00552898 < dVar1) && (10 < DAT_05826e10)) {
    FpsWindowStartTimeMs = DAT_05826e00;
    DAT_05826e10 = DAT_05826e14;
    DAT_05826e00 = timeGetTime();
    DAT_05826e14 = 0;
  }
  DeltaT = (float)(int)(FrameTimeCurrentMs - FrameTimePreviousMs) * _DAT_00552890;
  if (FrameTimeCurrentMs == FrameTimePreviousMs) {
    DeltaT = 0.000100000005;
  }
  FrameTimePreviousMs = FrameTimeCurrentMs;
  FPS = (float)local_8;
  return;
}
