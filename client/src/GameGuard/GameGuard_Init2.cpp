// GameGuard_Init2.cpp — FUN_0053d890 (entry point FUN_0053de60) @ 0x0053d890
// GameGuard initializer — __thiscall on GameGuard context object.
//
// Responsibilities:
//   1. Create/open the GameGuard log file in the GG directory.
//      - If new file: XOR-encrypt a version header with PRNG(seed=0x58a).
//      - If existing: use different XOR stream.
//      - Write encrypted header via WriteFile; initialize CRITICAL_SECTION.
//   2. Phase 1: locate first GameGuard binary (FUN_00540b30 to find/verify),
//      then CreateProcessA + WaitForSingleObject(INFINITE) for that process.
//      Exit codes: 0x755=success, 0x80=retry path, others=error.
//   3. Phase 2 (if Phase 1 succeeded with 0x755 or 0x80):
//      CreateEventA for synchronization, locate second GG binary,
//      CreateProcessA with CREATE_SUSPENDED (dwCreationFlags=4),
//      ResumeThread, WaitForSingleObject(event, timeout_ms).
//      - If event signals before timeout: success → returns 0x755.
//      - If timeout (WAIT_TIMEOUT=0x102): returns 0x72.
//      - Secondary check via FUN_0053efa0 + nProtectFlag (this+0x1d).
//
// Return codes (stored in caller + DAT_083bbaf0):
//   0x755 = success
//   0x6e  = Phase1 process not found
//   0x6f  = can't create GG log directory
//   0x72  = event timeout (Phase2)
//   0x78  = INI parse error (Phase1 extract)
//   0x82  = INI section read failed
//   0x8e  = INI signature verify failed
//   0x99  = secondary binary file not found
//   0x9a  = secondary binary file not found (alt path)
//   0x9b  = nProtect certificate error
//   0xa0  = 32-bit setup path unavailable
//   0xaa  = CreateProcess failed
//   0xab  = CreateEvent failed (Phase1)
//   0xac  = WaitForSingleObject precondition failed (event not set)
//   0xdc  = nProtect post-launch validate failed
//
// ── Anti-tamper / obfuscation note ───────────────────────────────────────────
// Several string lookups via FUN_0053e8c0(&DAT_005632xx) decode GameGuard
// binary paths and log messages at runtime — not reimplemented here.
// FUN_00540b30 (CryptHashData?), FUN_00543efe (verify), FUN_00541210 (INI read)
// FUN_0053f4f0, FUN_00540520, FUN_00540660 — GG internal checks.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>

// ── Forward declarations for GG-internal helpers ─────────────────────────────
// These are GameGuard-private and not part of the game logic layer.
extern "C" {
    int   __cdecl FUN_0053fc50(int this_int);                    // GG_GetVersion
    void  __cdecl FUN_0054414b(void *dst, void *src, int len);   // GG_memcpy
    int   __cdecl FUN_0053f410(void *buf, int size);             // GG_PathInit (for log dir)
    int   __cdecl FUN_0053f460(void *buf, int size);             // GG_PathInit2 (for binary dir)
    int   __cdecl FUN_00544105(const char *path, int flags);     // GG_CheckFileAttr
    void* __cdecl FUN_0053e8c0(void *key);                       // GG_DecodeStr
    void  __cdecl FUN_0053eba0(int *log, void *msg);             // GG_WriteLog
    int   __cdecl FUN_0053f4f0(void *ctx, int offset);           // GG_CheckState
    void  __cdecl FUN_0053fdd0(void);                            // GG_Setup1
    void  __cdecl FUN_0053feb0(void);                            // GG_Setup2
    int   __cdecl FUN_00540b30(void *ctx, const char *path, char flag); // GG_LoadBinary
    int   __cdecl FUN_00540f40(const char *src, char *out, int max, int offset); // GG_ExtractSection
    void  __cdecl FUN_00541280(int a, int b, void *c, void *d, int e, char *f);  // GG_INI_Parse
    int   __cdecl FUN_00543efe(void *ctx, void *sig, void *key); // GG_VerifySig
    int   __cdecl FUN_00541210(int ctx, int key, int def, char *out);            // GG_INI_ReadInt
    int   __cdecl FUN_00540520(void *ctx);                       // GG_Check32Bit
    int   __cdecl FUN_00540660(void *ctx);                       // GG_Check64Bit
    int   __cdecl FUN_0053efa0(void *ctx, int val);              // GG_ValidateNP
    void  __cdecl FUN_005404a0(int a, int b, char *c);           // GG_Report
    char  __cdecl FUN_00544093(char *str, char ch);              // GG_StrRChr
}

// GG process ID / handle stored globally (written at Phase2 launch)
extern DWORD  DAT_083bbb0c;   // GG child process ID
extern HANDLE DAT_083bbb10;   // GG child process handle
extern DWORD  DAT_083bbaf0;   // GG error state flag (2=error)
extern DWORD  DAT_083bbaf4;   // main thread ID (stored at init)
extern DWORD  DAT_083bbaf8;   // event timeout (ms) read from INI
extern DWORD  DAT_083bbb14;
extern char   DAT_00562e5c;   // INI integer result A
extern char   DAT_00562e58;   // INI key pointer B

// External string constants used by GG
extern char   lpString1_083bb9e0[];  // GG log/data directory path
extern char   lpSubBlock_005592d0[]; // path separator / sub-block
extern char   DAT_005633dc;
extern char   DAT_005633cc;
extern char   DAT_00563338;
extern char   DAT_00563308;
extern char   DAT_005632fc;
extern char   DAT_005632e0;
extern char   DAT_005632d0;
extern char   DAT_005632ac;
extern char   DAT_0056329c;
extern char   DAT_00563298;
extern char   DAT_00563284;
extern char   DAT_00563270;
extern char   DAT_00563254;
extern char   DAT_0056323c;
extern char   DAT_0056322c;
extern char   DAT_0056321c;
extern char   DAT_00563218;
extern char   DAT_00563208;
extern char   DAT_005631fc;
extern char   DAT_005631f8;
extern char   DAT_00563204;
extern char   DAT_00563200;
extern char   DAT_005631e0;
extern char   DAT_005631dc;
extern char   DAT_005631d0;
extern char   DAT_005631b4;
extern char   DAT_00563198;
extern char   DAT_0056318c;
extern char   PTR_DAT_00563180;
extern char   DAT_00563170;
extern char   DAT_00563158;
extern char   DAT_0056314c;
extern char   DAT_0056313c;
extern char   DAT_00563128;
extern char   PTR_DAT_00563110;
extern char   DAT_005630f8;
extern char   DAT_005630e4;
extern char   DAT_005630d4;
extern char   DAT_005630d0;
extern char   DAT_005630c0;
extern char   DAT_005630a4;
extern char   DAT_005630a0;
extern char   DAT_0056308c;
extern char   DAT_00563088;
extern char   DAT_00563064;
extern char   DAT_00563040;
extern char   DAT_0056301c;
extern char   DAT_00563018;
extern char   DAT_00562ff8;
extern char   DAT_00562ff0[];  // format string "i5  %lu"
extern char   DAT_00562fdc;
extern char   DAT_00562fc4;
extern char   DAT_00562fc0;
extern char   DAT_00562fa8;
extern char   DAT_00562f98;
extern char   DAT_00562f94;
extern char   DAT_00562f7c;
extern char   DAT_00562f60;
extern char   DAT_00562f5c;
extern char   DAT_00562f3c;
extern char   DAT_0056337c;

// ─────────────────────────────────────────────────────────────────────────────

// FUN_0053de60 is a jump stub that calls the thiscall below.
// The actual function is FUN_0053d890 (int __thiscall).
// Stub in stubs.cpp is voided; callers that use the return value call directly.

int __cdecl FUN_0053d890(void *gg_ctx, unsigned char *param_1)
{
    // ── Already-initialized guard ─────────────────────────────────────────────
    if (*(char *)gg_ctx != '\0')
        return 0x755;

    *(char *)((int)gg_ctx + 0x331) = (char)FUN_0053fc50((int)gg_ctx);
    *(DWORD *)((int)gg_ctx + 0x2c4) = GetCurrentProcessId();
    DAT_083bbaf4 = GetCurrentThreadId();
    FUN_0054414b((char *)((int)gg_ctx + 0x2c8), param_1, 0x20);
    *(unsigned char *)((int)gg_ctx + 0x2e7) = 0;

    // ── Create/verify GG data directory ──────────────────────────────────────
    FUN_0053f410(lpString1_083bb9e0, 0x104);
    int dirExist = FUN_00544105(lpString1_083bb9e0, 0);
    if (dirExist == -1) {
        if (!CreateDirectoryA(lpString1_083bb9e0, NULL)) {
            GetLastError();
            GetLastError();
            unsigned char *errStr = (unsigned char *)FUN_0053e8c0(&DAT_005633dc);
            FUN_0053eba0((int *)((int)gg_ctx + 0x20), errStr);
            return 0x6f;
        }
    }

    // Append log filename
    lstrcatA(lpString1_083bb9e0, lpSubBlock_005592d0);
    const char *logNameStr = (const char *)FUN_0053e8c0(&DAT_005633cc);
    lstrcatA(lpString1_083bb9e0, logNameStr);

    // ── Open/create log file; XOR-encrypt header ──────────────────────────────
    int *piLog = (int *)((int)gg_ctx + 0x20);
    if (*piLog == -1) {
        *(int *)((int)gg_ctx + 0x250) = 0;
        *(int *)((int)gg_ctx + 0x254) = 300;

        // If path is relative (< 4 chars or no drive letter), use module dir
        char logPath[264] = {0};
        int  pathLen = (int)lstrlenA(lpString1_083bb9e0);
        if (pathLen < 4 || lpString1_083bb9e0[1] == ':') {
            // absolute path — use as-is
            memcpy(logPath, lpString1_083bb9e0, pathLen + 1);
        } else {
            GetModuleFileNameA(NULL, logPath, 0x104);
            char *lastSlash = (char *)FUN_00544093(logPath, '\\');
            if (lastSlash) *(lastSlash + 1) = '\0';
            lstrcatA(logPath, lpString1_083bb9e0);
        }

        HANDLE hFile = CreateFileA(logPath, 0xc0000000, 3, NULL, 2, 0x80000080, NULL);
        *piLog = (int)hFile;
        if (hFile != INVALID_HANDLE_VALUE) {
            GetLocalTime((LPSYSTEMTIME)((int)gg_ctx + 0x23c));

            // Format version string into this+0x3c
            unsigned char *headerBuf = (unsigned char *)((int)gg_ctx + 0x3c);
            unsigned char *fmtStr    = (unsigned char *)FUN_0053e8c0(&DAT_0056337c);
            crt_sprintf((char *)headerBuf, (char *)fmtStr);
            DWORD headerLen = (DWORD)lstrlenA((char *)headerBuf);

            if (*(int *)((int)gg_ctx + 0x254) == 999)
                headerLen = 0;

            // XOR-encrypt header
            if (*(int *)((int)gg_ctx + 0x250) == 0) {
                *(int *)((int)gg_ctx + 0x24c) = 0x58a;
                for (DWORD i = 0; i < headerLen; i++) {
                    int prng = *(int *)((int)gg_ctx + 0x24c) * 3 + 1;
                    *(int *)((int)gg_ctx + 0x24c) = prng;
                    headerBuf[i] ^= (unsigned char)(prng + 0x46);
                }
            } else {
                for (DWORD i = 0; i < headerLen; i++) {
                    int prng = *(int *)((int)gg_ctx + 0x250) + 2;
                    *(int *)((int)gg_ctx + 0x250) = prng;
                    headerBuf[i] ^= (unsigned char)(prng + 0x43);
                }
            }

            DWORD written = 0;
            WriteFile((HANDLE)*piLog, headerBuf, headerLen, &written, NULL);
            InitializeCriticalSection((LPCRITICAL_SECTION)((int)gg_ctx + 0x24));
        }
    }

    // ── Phase 1: locate and launch first GG binary ────────────────────────────
    {
        unsigned char *s;
        s = (unsigned char *)FUN_0053e8c0(&DAT_00563338); FUN_0053eba0(piLog, s);
        s = (unsigned char *)FUN_0053e8c0(&DAT_00563308); FUN_0053eba0(piLog, s);

        if (FUN_0053f4f0(gg_ctx, 0) == 0) {
            // Notify any existing GG instance via event
            const char *evtName = (const char *)FUN_0053e8c0(&DAT_005632fc);
            HANDLE hEvt = OpenEventA(0x100002, 0, evtName);
            if (hEvt) { SetEvent(hEvt); CloseHandle(hEvt); }
            s = (unsigned char *)FUN_0053e8c0(&DAT_005632e0); FUN_0053eba0(piLog, s);

            if (FUN_0053f4f0(gg_ctx, 0x251c) == 0)
                return 0x6e;
        }

        FUN_0053fdd0();
        FUN_0053feb0();

        // Check for file A
        {
            DWORD tick = GetTickCount();
            const char *fileA = (const char *)FUN_0053e8c0(&DAT_005632d0);
            HANDLE hTest = CreateFileA(fileA, 0xc0000000, 3, NULL, 3, 0x80, NULL);
            if (hTest != INVALID_HANDLE_VALUE) {
                CloseHandle(hTest);
                *(unsigned char *)((int)gg_ctx + 0x1c) = 1;
            }

            _SECURITY_ATTRIBUTES sa;
            sa.nLength              = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle       = TRUE;
            HANDLE hEvent1 = CreateEventA(&sa, TRUE, FALSE, NULL);
            *(HANDLE *)((int)gg_ctx + 0x33c) = hEvent1;
            if (!hEvent1) {
                GetLastError(); GetLastError();
                s = (unsigned char *)FUN_0053e8c0(&DAT_005632ac);
                FUN_0053eba0(piLog, s);
                DAT_083bbaf0 = 2;
                return 0xab;
            }
        }

        // Check for file B
        {
            const char *fileB = (const char *)FUN_0053e8c0(&DAT_0056329c);
            HANDLE hTest2 = CreateFileA(fileB, 0xc0000000, 3, NULL, 3, 0x80, NULL);
            if (hTest2 != INVALID_HANDLE_VALUE) {
                CloseHandle(hTest2);
                *(unsigned char *)((int)gg_ctx + 0x1c) = 1;
            }
        }

        // Build path for Phase1 binary (concatenate install dir + name)
        char binPath1[260] = {0};
        FUN_0053eba0(piLog, &DAT_00563298);
        FUN_0053f460(binPath1, 0x104);
        // ... (path construction via multiple lstrcat + FUN_0053e8c0 calls omitted;
        //      result is the Phase1 GG binary path in binPath1)

        if (FUN_00544105(binPath1, 0) == -1) {
            s = (unsigned char *)FUN_0053e8c0(&DAT_00563270);
            FUN_0053eba0(piLog, s);
            return 0x99;
        }

        // Cryptographic load of Phase1 binary
        if (!FUN_00540b30((void *)((int)gg_ctx + 0x278), binPath1, '\x01')) {
            bool bCertErr = (*(int *)((int)gg_ctx + 700) == 8) &&
                            (GetLastError() == 0x80090006);
            GetLastError(); GetLastError();
            s = (unsigned char *)FUN_0053e8c0(&DAT_00563254);
            FUN_0053eba0(piLog, s);
            DAT_083bbaf0 = 2;
            // Read tail of file for diagnostics
            HANDLE hDiag = CreateFileA(binPath1, 0x80000000, 3, NULL, 3, 0x80, NULL);
            if (hDiag != INVALID_HANDLE_VALUE) {
                GetFileSize(hDiag, NULL);
                SetFilePointer(hDiag, -4, NULL, FILE_END);
                DWORD tail = 0, rd = 0;
                ReadFile(hDiag, &tail, 4, &rd, NULL);
                CloseHandle(hDiag);
            }
            if (bCertErr) return 0x9b;
        }

        // Parse INI section; create command line; launch Phase1
        char cmdLine[256] = {0};
        // (INI parse via FUN_00541280 + FUN_00543efe + FUN_00541210 omitted)
        // Verification of signature
        // ... (FUN_00543efe call omitted)

        // Prepare STARTUPINFO / PROCESS_INFORMATION and launch
        STARTUPINFOA        si = {0}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {0};
        DWORD tick = GetTickCount() & 0xfff00000;
        DWORD evtHandle  = (DWORD)*(HANDLE *)((int)gg_ctx + 0x33c);
        int   keyPtrInt  = (int)gg_ctx + 0x2c8;
        const char *launchFmt = (const char *)FUN_0053e8c0(&DAT_00563208);
        wsprintfA(cmdLine, launchFmt, tick, keyPtrInt, evtHandle);

        FUN_0053eba0(piLog, &DAT_00563218);
        if (!CreateProcessA(binPath1, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            s = (unsigned char *)FUN_0053e8c0(&DAT_00562ff8);
            FUN_0053eba0(piLog, s);
            DAT_083bbaf0 = 2;
            return 0xaa;
        }

        FUN_0053eba0(piLog, &DAT_00563204);
        WaitForSingleObject(pi.hProcess, INFINITE);
        FUN_0053eba0(piLog, &DAT_00563200);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        FUN_0053eba0(piLog, &DAT_005631fc);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        FUN_0053eba0(piLog, &DAT_005631f8);

        if (exitCode != 0x755 &&
            !(*(char *)((int)gg_ctx + 0x340) != '\0' && exitCode == 0x80))
        {
            s = (unsigned char *)FUN_0053e8c0(&DAT_005631e0);
            FUN_0053eba0(piLog, s);
            return (int)exitCode;
        }
        FUN_0053eba0(piLog, &DAT_005631dc);
    }

    // ── Phase 2: locate and launch second GG binary (CREATE_SUSPENDED) ────────
    {
        char binPath2[260] = {0};
        // (Path construction via FUN_0053f410 + multiple lstrcat calls omitted;
        //  result is the Phase2 GG binary path in binPath2.)

        if (FUN_00544105(binPath2, 0) == -1) {
            unsigned char *s = (unsigned char *)FUN_0053e8c0(&DAT_005630a4);
            FUN_0053eba0(piLog, s);
            DAT_083bbaf0 = 2;
            return 0x9a;
        }

        if (!FUN_00540b30((void *)((int)gg_ctx + 0x278), binPath2, '\x01')) {
            unsigned char *s2 = (unsigned char *)FUN_0053e8c0(&DAT_00563254);
            FUN_0053eba0(piLog, s2);
            DAT_083bbaf0 = 2;
            return 0x7a;
        }

        FUN_0053eba0(piLog, &DAT_005630a0);

        if (!FUN_00540520((void *)((int)gg_ctx + 0x25c))) {
            unsigned char *s3 = (unsigned char *)FUN_0053e8c0(&DAT_0056308c);
            FUN_0053eba0(piLog, s3);
            DAT_083bbaf0 = 2;
            return 0xa0;
        }

        FUN_0053eba0(piLog, &DAT_00563088);
        DWORD tick2 = GetTickCount();
        DWORD tick2masked = tick2 & 0xfff00000;

        // Create synchronization event for Phase2
        _SECURITY_ATTRIBUTES sa2;
        sa2.nLength              = sizeof(sa2);
        sa2.lpSecurityDescriptor = NULL;
        sa2.bInheritHandle       = TRUE;
        HANDLE hEvent2 = CreateEventA(&sa2, TRUE, FALSE, NULL);
        *(HANDLE *)((int)gg_ctx + 0x338) = hEvent2;
        if (!hEvent2) {
            GetLastError(); GetLastError();
            unsigned char *s4 = (unsigned char *)FUN_0053e8c0(&DAT_00563064);
            FUN_0053eba0(piLog, s4);
            DAT_083bbaf0 = 2;
            return 0xab;
        }

        // Check Phase1 event still clear
        if (WaitForSingleObject(*(HANDLE *)((int)gg_ctx + 0x33c), 0) != 0) {
            unsigned char *s5 = (unsigned char *)FUN_0053e8c0(&DAT_00563040);
            FUN_0053eba0(piLog, s5);
            return 0xac;
        }
        ResetEvent(*(HANDLE *)((int)gg_ctx + 0x33c));

        // Build Phase2 command line and launch CREATE_SUSPENDED
        char cmdLine2[256] = {0};
        DWORD ev1    = (DWORD)*(HANDLE *)((int)gg_ctx + 0x33c);
        DWORD ev2    = (DWORD)hEvent2;
        DWORD pid    = *(DWORD *)((int)gg_ctx + 0x2c4);
        int   keyPtr = (int)gg_ctx + 0x2c8;
        const char *fmt2 = (const char *)FUN_0053e8c0(&DAT_0056301c);
        wsprintfA(cmdLine2, fmt2, tick2masked, pid, 0, 0, 0, keyPtr, ev2, ev1);

        FUN_0053eba0(piLog, &DAT_00563018);
        STARTUPINFOA        si2 = {0}; si2.cb = sizeof(si2);
        PROCESS_INFORMATION pi2 = {0};
        if (!CreateProcessA(binPath2, cmdLine2, NULL, NULL, TRUE, 4, NULL, NULL, &si2, &pi2)) {
            unsigned char *se = (unsigned char *)FUN_0053e8c0(&DAT_00562ff8);
            FUN_0053eba0(piLog, se);
            DAT_083bbaf0 = 2;
            return 0xaa;
        }

        DAT_083bbb0c = pi2.dwProcessId;
        DAT_083bbb10 = pi2.hProcess;

        // 64-bit mode path
        if (FUN_00540660((void *)((int)gg_ctx + 0x25c))) {
            unsigned char *sd = (unsigned char *)FUN_0053e8c0(&DAT_00562fc4);
            FUN_0053eba0(piLog, sd);
            FUN_0053eba0(piLog, &DAT_00562fc0);
            ResumeThread(pi2.hThread);

            DWORD waitResult = WaitForSingleObject(hEvent2, DAT_083bbaf8);
            if (waitResult == WAIT_TIMEOUT) {
                unsigned char *st = (unsigned char *)FUN_0053e8c0(&DAT_00562fa8);
                FUN_0053eba0(piLog, st);
                return 0x72;
            }

            unsigned char *sg = (unsigned char *)FUN_0053e8c0(&DAT_00562f98);
            FUN_0053eba0(piLog, sg);
            FUN_0053eba0(piLog, &DAT_00562f94);

            if (WaitForSingleObject(*(HANDLE *)((int)gg_ctx + 0x33c), 0) != WAIT_TIMEOUT) {
                // nProtect validation
                if (*(char *)((int)gg_ctx + 0x1d) != '\0') {
                    if (!FUN_0053efa0(gg_ctx, *(DWORD *)((int)gg_ctx + 0x334))) {
                        unsigned char *snp = (unsigned char *)FUN_0053e8c0(&DAT_00562f60);
                        FUN_0053eba0(piLog, snp);
                        DAT_083bbaf0 = 2;
                        return 0xdc;
                    }
                }
                FUN_0053eba0(piLog, &DAT_00562f5c);
                FUN_005404a0(0x8b1, 0x611, *(char **)((int)gg_ctx + 0x2c4));
                FUN_0053fdd0();
                FUN_0053feb0();
                unsigned char *sok = (unsigned char *)FUN_0053e8c0(&DAT_00562f3c);
                FUN_0053eba0(piLog, sok);
                *(unsigned char *)gg_ctx = 1;
                return 0x755;
            }

            unsigned char *stt = (unsigned char *)FUN_0053e8c0(&DAT_00562f7c);
            FUN_0053eba0(piLog, stt);
            return 0x72;
        }

        // 32-bit unavailable path
        unsigned char *s32 = (unsigned char *)FUN_0053e8c0(&DAT_00562fdc);
        FUN_0053eba0(piLog, s32);
        DAT_083bbaf0 = 2;
        return 0xa0;
    }
}
