// Timer.cpp
// IDA: FUN_0043fd70 @ 0x0043fd70 — Timer_FrameUpdate
//
// Called once per frame to update timing globals:
//   DAT_05826e08  — current absolute time (ms, as float)
//   FrameTimeCurrentMs  — last frame timestamp (ms)
//   _DAT_0055979c — delta time (seconds per frame, clamped to 0.0001 min)
//   _DAT_05826df8 — smoothed FPS (frames per second)
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
  local_8 = (double)(ulonglong)DVar2;
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
  _DAT_0055979c = (float)(int)(FrameTimeCurrentMs - FrameTimePreviousMs) * _DAT_00552890;
  if (FrameTimeCurrentMs == FrameTimePreviousMs) {
    _DAT_0055979c = 0.000100000005;
  }
  FrameTimePreviousMs = FrameTimeCurrentMs;
  _DAT_05826df8 = (float)local_8;
  return;
}
