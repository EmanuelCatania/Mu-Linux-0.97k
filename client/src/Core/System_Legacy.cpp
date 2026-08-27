// System_Legacy.cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_004041e0 @ 0x004041E0 — HashTable::GetIndex — find slot for 4-byte key
// __thiscall(this=hashTableObj, param_1=key). Linear probe hash table lookup.
// Returns slot index or 0xFFFFFFFF if not found / table full.
// Vtable call at this+0xc for hash function, entries at this+8.
UINT __fastcall HashTable_GetIndex(void* ecx, void* /*edx*/, DWORD param_1) {
    (void)ecx; (void)param_1;
    // hash = vtable[3](param_1) — compute initial slot
    // Linear probe: compare 4-byte key at (this+8)[slot*4]
    //   if slot == {0,0,0,0}: return -1 (empty)
    //   if slot matches param_1: return slot
    //   advance slot = (slot+1) % capacity
    // If full: CErrorReport__Write("Hash table full — GetIndex"), return -1
    return 0xFFFFFFFF;
}

// InitDirectSound @ 0x004044A0 — real implementation in src/Sound/Sound.cpp.

// FUN_00405340 @ 0x00405340 — CErrorReport_RotateLog (215 bytes IDA, port FIEL).
// __thiscall(this). Reads error log file (this+4 = HANDLE), decrypts with
// Xor_ConvertBuffer (this+0x10c = key state), keeps last 4 entries, rewrites file.
//
// Layout (per IDA):
//   this+4   HANDLE (file)
//   this+8   LPCSTR file path
//   this+0x10c  XOR key state DWORD
// Xor_ConvertBuffer is declared in functions.h
char* __stdcall CErrorReport_FindLogMarker(char* param_1);
extern "C" void CErrorReport_RotateLog_impl(DWORD This) {
    static char Buffer[0x1FFFF + 1];
    DWORD numRead = 0;
    HANDLE hFile = *(HANDLE*)(This + 4);
    if (hFile == INVALID_HANDLE_VALUE || hFile == nullptr) return;
    if (!ReadFile(hFile, Buffer, 0x1FFFF, &numRead, nullptr)) return;
    *(DWORD*)(This + 0x10C) = (DWORD)Xor_ConvertBuffer(Buffer, numRead, 0);
    Buffer[numRead] = '\0';
    char* trimAt = CErrorReport_FindLogMarker(Buffer);
    if (numRead < 0x7FFF && trimAt == Buffer) return;
    char* keepFrom = (trimAt == Buffer) ? Buffer : trimAt;
    DWORD keepLen = numRead - (DWORD)(keepFrom - Buffer);
    CloseHandle(hFile);
    LPCSTR path = (LPCSTR)(This + 8);
    DeleteFileA(path);
    HANDLE newH = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                              nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    *(HANDLE*)(This + 4) = newH;
    *(DWORD*)(This + 0x10C) = 0;
    if (newH != INVALID_HANDLE_VALUE) {
        DWORD wr = 0;
        WriteFile(newH, keepFrom, keepLen, &wr, nullptr);
    }
}

// IDA: FUN_00405420 @ 0x00405420 — CErrorReport_FindLogMarker (143 bytes IDA, port FIEL).
// Scans buffer for "###Log Begin###" markers, returns pointer to 4th-from-last marker
// (so newer log can keep the last 4 entries), or original buffer if < 5 markers found.
// Note: IDA shows __stdcall(char*, int) but a2 is a phantom param; only the buffer is used.
char* __stdcall CErrorReport_FindLogMarker(char* param_1) {
    static const char kLogMarker[] = "###Log Begin###";
    if (!param_1) return param_1;
    char* markers[128];
    int count = 0;
    char* p = param_1;
    while ((p = strchr(p, '#')) != nullptr) {
        if (strncmp(p, kLogMarker, sizeof(kLogMarker) - 1) == 0) {
            if (count < 128) markers[count++] = p;
            p += sizeof(kLogMarker) - 1;
        } else {
            ++p;
        }
    }
    if (count > 4) return markers[count - 4];
    return param_1;
}




// GetOSVersion @ 0x00405A20 (~461 lines) — Detect Windows version string
// Calls GetVersionExA, then giant switch on major/minor version to build OS name string.
// Detects Win95/98/ME/NT3.51/NT4/2000/XP/Server2003 + service pack info via registry.
// Writes result to si+0x80 (128-byte OS name buffer).
void __cdecl GetOSVersion(DWORD si) {
    // OSVERSIONINFOA ovi; ovi.dwOSVersionInfoSize = 0x94; GetVersionExA(&ovi);
    // Switch on ovi.dwMajorVersion (3,4,5):
    //   3.51 -> "Windows NT 3.51"
    //   4.0+platformId1 -> "Windows 95" (+ "OSR2" if CSD='C'|'B')
    //   4.10 -> "Windows 98" (+ "SE" if CSD='A')
    //   4.90 -> "Windows Me"
    //   4.0+platformId2 -> "Windows NT 4.0"
    //   5.0 -> "Windows 2000" + registry ProductType (Server/Professional)
    //   5.1 -> "Windows XP" + registry ProductType
    //   5.2 -> "Windows Server 2003"
    // Appends service pack string from ovi.szCSDVersion
    // stub: 461 lines of string copy + version detection
}

// IDA: FUN_00405e20 @ 0x00405E20 — Measure CPU speed via RDTSC + QueryPerformanceCounter
// Returns the number of TSC ticks per second of QPC time (= CPU MHz × 1e6 if TSC = clock).
// Used by GetCPUInfo to format the "%d MHz" suffix on the CPU name string.
//
// Port faithful from IDA sub_405E20: cpuid(1) check TSC bit, QPF, boost priority to
// realtime, pin to CPU 0, measure RDTSC across Sleep(ms), restore state, compute ratio.
long long __cdecl Cpu_MeasureClockRate(DWORD param_1) {
    // CPUID(1) — check TSC support (EDX bit 4)
    int regs[4] = {0,0,0,0};
    __cpuid(regs, 1);
    if ((regs[3] & 0x10) == 0) return 0;  // no TSC

    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) return 0;

    HANDLE hProc = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    DWORD priClass = GetPriorityClass(hProc);
    int   threadPri = GetThreadPriority(hThread);
    DWORD_PTR procAff = 0, sysAff = 0;
    GetProcessAffinityMask(hProc, &procAff, &sysAff);

    SetPriorityClass(hProc, REALTIME_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
    SetProcessAffinityMask(hProc, 1);

    // cpuid as serializing barrier before RDTSC
    __cpuid(regs, 0);
    LARGE_INTEGER qpcStart, qpcEnd;
    QueryPerformanceCounter(&qpcStart);
    unsigned __int64 tscStart = __rdtsc();
    Sleep(param_1);
    QueryPerformanceCounter(&qpcEnd);
    unsigned __int64 tscEnd = __rdtsc();

    SetProcessAffinityMask(hProc, procAff);
    SetThreadPriority(hThread, threadPri);
    SetPriorityClass(hProc, priClass);

    double tscDelta = (double)(__int64)(tscEnd - tscStart);
    double qpcDelta = (double)(qpcEnd.QuadPart - qpcStart.QuadPart);
    double seconds  = qpcDelta / (double)freq.QuadPart;
    if (seconds <= 0.0) return 0;
    return (long long)(tscDelta / seconds);
}

// GetCPUInfo @ 0x00405F40 (~336 lines) — Detect CPU vendor + model string
// cpuid(0) -> vendor string (GenuineIntel / AuthenticAMD / etc)
// cpuid(1) -> family/model, then giant switch to map to human-readable name
// (Pentium Pro/2/3/4, AMD K5/K6/K7/Athlon/Duron, Cyrix, etc)
// Writes result to si (128-byte CPU name buffer) + appends " %dMHz" via FUN_00405e20.
void __cdecl GetCPUInfo(DWORD si) {
    (void)si;
    // cpuid(0) -> vendor ID at si[0..11]
    // cpuid(1) -> family = bits 8-11, model = bits 4-7
    // Switch on vendor:
    //   "AuthenticAMD": family 4->486, family 5->K5/K6, family 6->K7/Athlon/Duron
    //   "GenuineIntel": family 3/4/5->486/Pentium, family 6->PentiumPro/2/3, family 15->Pentium4
    //   "CyrixInstead": Cyrix models
    // Append " %dMHz" from FUN_00405e20(500)
    // stub: 336 lines of CPU identification
}

// GetDXVersion @ 0x00406270 (~154 lines) — Detect DirectX version installed
// Probes DDRAW.DLL, DINPUT.DLL, D3D8.DLL, D3D9.DLL via LoadLibrary/GetProcAddress.
// Tests DirectDrawCreate, DirectDrawCreateEx, CoCreateInstance for DirectMusic.
// Returns version code: 0=none, 0x100=DDraw1, 0x200=DInput, 0x300=DDraw2, 0x500=DDraw4,
// 0x600=DMusic, 0x601=DDraw7, 0x700=D3D8, 0x800=D3D8only, 0x900=D3D9.
DWORD __stdcall GetDXVersion_stub(void) {
    // LoadLibrary("DDRAW.DLL") -> GetProcAddress("DirectDrawCreate")
    // Create DDraw1, QI DDraw2 -> 0x100
    // LoadLibrary("DINPUT.DLL") -> GetProcAddress("DirectInputCreateA") -> 0x200
    // DDraw2->SetCooperativeLevel + CreateSurface -> 0x300
    // QI DDraw4 -> 0x500, QI DDraw7 -> 0x601
    // CoCreateInstance(CLSID_DirectMusicPerformance) -> 0x600
    // GetProcAddress("DirectDrawCreateEx") -> 0x601
    // LoadLibrary("D3D8.DLL") -> 0x700
    // LoadLibrary("D3D9.DLL") -> 0x900
    return 0x900;
}

// IDA: FUN_00406660 @ 0x00406660 — CErrorReport_DumpCallStack (port FIEL).
// __thiscall(this). Dumps CPU registers (CONTEXT_FULL) and walks EBP chain to
// log return addresses (up to 1024 frames) on crash. The function.h decl is
// __stdcall(void); IDA shows it's __thiscall(this) — we accept the void
// declaration since the call sites (vtable dispatch via CErrorReport) are not
// active in our build.
void __stdcall CErrorReport_DumpCallStack(void) {
    // No-op: CErrorReport vtable not wired up in our build. The implementation
    // logic is preserved as CErrorReport_DumpCallStack_impl below for future use.
}
extern "C" void CErrorReport_DumpCallStack_impl(DWORD This) {
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(GetCurrentThread(), &ctx)) return;

    CErrorReport__Write(This, (char*)"<Register>\r\n");
    CErrorReport__Write(This, (char*)"EAX = 0x%08X  EBX = 0x%08X  ECX = 0x%08X  EDX = 0x%08X\r\n",
                        ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx);
    CErrorReport__Write(This, (char*)"ESI = 0x%08X  EDI = 0x%08X  EBP = 0x%08X  EIP = 0x%08X\r\n",
                        ctx.Esi, ctx.Edi, ctx.Ebp, ctx.Eip);
    CErrorReport__Write(This, (char*)"<Call Stack>\r\n");

    DWORD* ebp = (DWORD*)(uintptr_t)ctx.Ebp;
    int count = 0;
    while (ebp && !IsBadReadPtr(ebp, sizeof(DWORD) * 2) && *ebp != 0 && count < 1024) {
        DWORD ret = ebp[1];  // return address sits at [ebp+4]
        CErrorReport__Write(This, (char*)"0x%08X\r\n", ret);
        ebp = (DWORD*)(uintptr_t)*ebp;
        ++count;
    }
}

// IDA: FUN_00406bd0 @ 0x00406BD0 — Packet cipher init (constructor)
// __fastcall(ecx=cipherObj). Sets vtable, calls FUN_00406d20 (key schedule),
// allocates random-sized obfuscation buffer at obj+6, allocates encode/decode buffers
// (0x400 entries * 4 bytes each), zeroes them, sets obj+8/+9 as read/write cursors.
void __fastcall PacketCipher_Initialize(void* param_1) {
    // *(int*)param_1 = &PTR_FUN_005524c8 (vtable)
    // FUN_00406d20(param_1) — key schedule init
    // delete old buffer at param_1[6], alloc rand()%0xCC7+0x159 obfuscation bytes
    // param_1[3] = 0x400 (capacity)
    // param_1[1] = new byte[0x1000] (encode buffer)
    // param_1[2] = new byte[0x1000] (decode buffer)
    // Zero both buffers, set param_1[8]=param_1[1], param_1[9]=param_1[2]
    (void)param_1;
}

// IDA: FUN_00406e90 @ 0x00406E90 — Hash function A: hash 4-byte key with multiplier 0x83
// __fastcall(ecx=hashObj). Hashes 4 stack bytes: h = h*0x83 + byte[i], returns h % capacity.
UINT __fastcall PacketCipher_HashKey83(int param_1) {
    (void)param_1;
    // uint h = 0;
    // for (i = 0; i < 4; i++) h = h * 0x83 + key_bytes[i];
    // return h % *(uint*)(param_1 + 0xc);
    return 0;
}

// IDA: FUN_00406ef0 @ 0x00406EF0 — Hash function B: hash 4-byte key with configurable multiplier
// __fastcall(ecx=hashObj). Like FUN_00406e90 but uses multiplier from obj+0x1c.
UINT __fastcall PacketCipher_HashKeyConfigured(int param_1) {
    (void)param_1;
    // uint h = 0;
    // int mult = *(int*)(param_1 + 0x1c);
    // for (i = 0; i < 4; i++) h = key_bytes[i] + mult * h;
    // return h % *(uint*)(param_1 + 0xc);
    return 0;
}
