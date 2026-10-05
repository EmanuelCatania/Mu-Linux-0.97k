// 0x00405E20 FUN_00405e20 — nunca activado: IDA_PORT_00405E20 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00405e20 (IDA-only, gated) ──
#if defined(IDA_PORT_00405E20)
int __cdecl FUN_00405e20(DWORD dwMilliseconds)
{
  __int64 v6; // rax
  HANDLE CurrentProcess; // esi
  HANDLE CurrentThread; // edi
  unsigned __int64 v15; // [esp+4h] [ebp-38h]
  unsigned __int64 v16; // [esp+Ch] [ebp-30h]
  LARGE_INTEGER Frequency; // [esp+14h] [ebp-28h] BYREF
  LARGE_INTEGER PerformanceCount; // [esp+1Ch] [ebp-20h] BYREF
  LARGE_INTEGER v19; // [esp+24h] [ebp-18h] BYREF
  DWORD dwPriorityClass; // [esp+2Ch] [ebp-10h]
  int nPriority; // [esp+30h] [ebp-Ch]
  ULONG_PTR SystemAffinityMask; // [esp+34h] [ebp-8h] BYREF
  ULONG_PTR ProcessAffinityMask; // [esp+38h] [ebp-4h] BYREF

  _EAX = 1;
  __asm { cpuid }
  nPriority = _EDX;
  if ( (_EDX & 0x10) != 0 )
  {
    LODWORD(v6) = QueryPerformanceFrequency(&Frequency);
    if ( (DWORD)v6 )
    {
      CurrentProcess = GetCurrentProcess();
      CurrentThread = GetCurrentThread();
      dwPriorityClass = GetPriorityClass(CurrentProcess);
      nPriority = GetThreadPriority(CurrentThread);
      GetProcessAffinityMask(CurrentProcess, &ProcessAffinityMask, &SystemAffinityMask);
      SetPriorityClass(CurrentProcess, 0x100u);
      SetThreadPriority(CurrentThread, 15);
      _EAX = SetProcessAffinityMask(CurrentProcess, 1u);
      __asm { cpuid }
      QueryPerformanceCounter(&PerformanceCount);
      v15 = __rdtsc();
      Sleep(dwMilliseconds);
      QueryPerformanceCounter(&v19);
      v16 = __rdtsc();
      SetProcessAffinityMask(CurrentProcess, ProcessAffinityMask);
      SetThreadPriority(CurrentThread, nPriority);
      SetPriorityClass(CurrentProcess, dwPriorityClass);
      return (__int64)((double)(__int64)(v16 - v15)
                     / ((double)(v19.QuadPart - PerformanceCount.QuadPart)
                      / (double)Frequency.QuadPart));
    }
  }
  else
  {
    LODWORD(v6) = 0;
  }
  return v6;
}
#endif
