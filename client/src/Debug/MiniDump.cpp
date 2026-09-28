// MiniDump.cpp — volcado de crash (.dmp).  Ver la nota larga en MiniDump.h:
// esto NO esta en el binario original, se agrega tomando el enfoque de
// Source/MuServer/ConnectServer/MiniDump.cpp.

#include "stdafx.h"
#include "Debug/MiniDump.h"

#include <dbghelp.h>

// Firma de MiniDumpWriteDump, para resolverla por GetProcAddress y no tener que
// enlazar dbghelp.lib (ver la nota del header).
typedef BOOL(WINAPI* PFN_MiniDumpWriteDump)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

bool CMiniDump::Write(_EXCEPTION_POINTERS* info, char* outPath, unsigned int outPathSize)
{
    if (outPath && outPathSize) outPath[0] = '\0';

    HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
    if (!hDbgHelp) return false;

    PFN_MiniDumpWriteDump pWrite =
        (PFN_MiniDumpWriteDump)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (!pWrite) { FreeLibrary(hDbgHelp); return false; }

    // Nombre con fecha y hora, como los del server.  Se le antepone "MuClient_"
    // para poder distinguirlos de los del GameServer cuando se juntan los dos
    // en la misma carpeta al reportar un problema.
    SYSTEMTIME st;
    GetLocalTime(&st);

    char path[MAX_PATH];
    wsprintfA(path, "MuClient_%04d-%02d-%02d_%02dh%02dm%02ds.dmp",
              st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { FreeLibrary(hDbgHelp); return false; }

    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId          = GetCurrentThreadId();
    mdei.ExceptionPointers = (PEXCEPTION_POINTERS)info;
    mdei.ClientPointers    = FALSE;

    // Mismos flags que el server: con MiniDumpScanMemory el stack se puede
    // recorrer, y MiniDumpWithIndirectlyReferencedMemory trae la memoria a la
    // que apuntan sus variables — que es lo que hace falta para ver el
    // contenido de los pools y structs del juego en el momento del crash.
    const MINIDUMP_TYPE type = (MINIDUMP_TYPE)(
        MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory);

    BOOL ok = pWrite(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                     type, info ? &mdei : NULL, NULL, NULL);

    CloseHandle(hFile);
    FreeLibrary(hDbgHelp);

    if (!ok) {
        // Sin dump util: no dejar un archivo vacio dando vueltas.
        DeleteFileA(path);
        return false;
    }

    if (outPath && outPathSize) lstrcpynA(outPath, path, (int)outPathSize);
    return true;
}
