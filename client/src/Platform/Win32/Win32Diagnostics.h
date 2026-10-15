#pragma once
#include "Platform/IDiagnostics.h"
#include "Platform/IWindow.h"
#include "Debug/MiniDump.h"
#include <windows.h>
#include <cstdio>
namespace Platform {
namespace Win32DiagnosticsDetail {
inline DiagnosticLog logger = nullptr;
inline void CrashLog(const char* text) { if (logger) logger(text); }
inline LONG WINAPI DbgUnhandledException(EXCEPTION_POINTERS* ep)
{
    // Anti-recursion guard: Si DbgLog crashea (e.g., WriteFile AV),
    // el filter se reentra → recursion infinita → otra AV en KernelBase.
    static __declspec(thread) int s_handlerDepth = 0;
    if (s_handlerDepth > 0) return EXCEPTION_CONTINUE_SEARCH;
    s_handlerDepth++;

    // El .dmp se escribe PRIMERO, antes de loguear nada.  Todo lo que sigue
    // (recorrido de frames, escaneo de stack) puede fallar por su cuenta si la
    // memoria quedo hecha un desastre, y en ese caso igual queremos el volcado.
    // Ver Debug/MiniDump.h: no esta en el binario original, se agrega con el
    // enfoque del MuServer.
    char dumpPath[MAX_PATH] = {};
    bool dumpOk = false;
    __try {
        dumpOk = CMiniDump::Write(ep, dumpPath, sizeof(dumpPath));
    } __except (EXCEPTION_EXECUTE_HANDLER) { dumpOk = false; }

    char line1[256], line2[256], full[600];
    __try {
        _snprintf_s(line1, sizeof(line1), _TRUNCATE,
            "CRASH: code=0x%08X  addr=0x%p",
            ep->ExceptionRecord->ExceptionCode,
            ep->ExceptionRecord->ExceptionAddress);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        line1[0] = 0;
    }
    line2[0] = 0;
    __try {
        if (ep && ep->ExceptionRecord && ep->ExceptionRecord->NumberParameters >= 2) {
            _snprintf_s(line2, sizeof(line2), _TRUNCATE,
                "param0=%u  param1=0x%p",
                (unsigned)ep->ExceptionRecord->ExceptionInformation[0],
                (void*)ep->ExceptionRecord->ExceptionInformation[1]);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { line2[0] = 0; }
    __try { CrashLog(line1); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    if (line2[0]) __try { CrashLog(line2); } __except (EXCEPTION_EXECUTE_HANDLER) {}
    // ── Stack trace desde el contexto del crash ──────────────────────────────
    __try {
        if (ep && ep->ContextRecord) {
            CONTEXT* ctx = ep->ContextRecord;
            char trace[800];
            int n = 0;
            n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                "STACK_PC: 0x%p (esp=0x%p ebp=0x%p)",
                (void*)ctx->Eip, (void*)ctx->Esp, (void*)ctx->Ebp);
            CrashLog(trace);

            // Volcado de registros — los valores de los argumentos suelen ir en eax/ecx/edx
            _snprintf_s(trace, sizeof(trace), _TRUNCATE,
                "REGS: eax=%08X ecx=%08X edx=%08X ebx=%08X esi=%08X edi=%08X",
                (unsigned)ctx->Eax, (unsigned)ctx->Ecx, (unsigned)ctx->Edx,
                (unsigned)ctx->Ebx, (unsigned)ctx->Esi, (unsigned)ctx->Edi);
            CrashLog(trace);

            // ── Recorrido de frames por EBP (direcciones de retorno reales) ─────
            // Por cada frame: prev_ebp = *ebp, return_addr = *(ebp+4),
            // then ebp = prev_ebp.
            n = _snprintf_s(trace, sizeof(trace), _TRUNCATE, "STACK_FRAMES:");
            DWORD ebp = ctx->Ebp;
            DWORD prev_ebp_value = 0;
            for (int frame = 0; frame < 12 && n < (int)sizeof(trace) - 16; ++frame) {
                __try {
                    if (ebp < 0x10000 || ebp > 0x7FFFFFFF) break;
                    if (ebp & 3) break; // misaligned
                    DWORD ret = *(DWORD*)(ebp + 4);
                    DWORD next = *(DWORD*)(ebp);
                    if (next != 0 && next <= ebp) break; // sanity: ebp must grow upward
                    n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                        " 0x%X", ret);
                    prev_ebp_value = ebp;
                    ebp = next;
                    if (ebp == prev_ebp_value) break;
                } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            }
            CrashLog(trace);

            // ── Escaneo crudo del stack (250 dwords desde esp): atrapa frames FPO/sin-EBP ──
            // El loop emite hasta ~25 direcciones candidatas (filtradas al código del juego).
            int dumped = 0;
            n = _snprintf_s(trace, sizeof(trace), _TRUNCATE, "STACK_SCAN:");
            DWORD esp = ctx->Esp;
            for (int i = 0; i < 250 && n < (int)sizeof(trace) - 16 && dumped < 25; ++i) {
                __try {
                    DWORD val = *(DWORD*)(esp + i * 4);
                    if (val >= 0x401000 && val < 0x600000) {
                        n += _snprintf_s(trace + n, sizeof(trace) - n, _TRUNCATE,
                            " %X:%X", esp + i*4, val);
                        dumped++;
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
            }
            CrashLog(trace);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    __try {
        if (dumpOk) {
            char b[MAX_PATH + 32];
            _snprintf_s(b, sizeof(b), _TRUNCATE, "MINIDUMP: %s", dumpPath);
            CrashLog(b);
        } else {
            CrashLog("MINIDUMP: no se pudo escribir (dbghelp.dll?)");
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    // Antes del cartel: si estabamos en pantalla completa hay que devolverle el
    // escritorio al usuario, o el MessageBox sale en la resolucion del juego y,
    // peor, queda asi despues de que el proceso muere.
    __try { Platform::GetWindow().RestoreDisplay(); } __except (EXCEPTION_EXECUTE_HANDLER) {}

    __try {
        if (dumpOk) {
            _snprintf_s(full, sizeof(full), _TRUNCATE,
                        "%s\n%s\n\nVolcado guardado en:\n%s", line1, line2, dumpPath);
        } else {
            _snprintf_s(full, sizeof(full), _TRUNCATE, "%s\n%s", line1, line2);
        }
        Platform::GetWindow().ShowError(full, "Crash", false, true);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    s_handlerDepth--;
    return EXCEPTION_EXECUTE_HANDLER;
}

}
class Win32Diagnostics final : public IDiagnostics {
public:
    bool Install(DiagnosticLog log) override {
        Win32DiagnosticsDetail::logger = log;
        SetErrorMode(SEM_FAILCRITICALERRORS);
        SetUnhandledExceptionFilter(Win32DiagnosticsDetail::DbgUnhandledException);
        CMiniDump::Preload();
        return true;
    }
};
}
