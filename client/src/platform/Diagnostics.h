#pragma once
#include "platform/IDiagnostics.h"
#ifdef _WIN32
#include "platform/Win32/Win32Diagnostics.h"
#else
#include "platform/Posix/PosixDiagnostics.h"
#endif
namespace Platform {
inline IDiagnostics& GetDiagnostics() {
#ifdef _WIN32
    static Win32Diagnostics diagnostics;
#else
    static PosixDiagnostics diagnostics;
#endif
    return diagnostics;
}
}
