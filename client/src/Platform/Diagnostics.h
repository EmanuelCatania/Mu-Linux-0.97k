#pragma once
#include "Platform/IDiagnostics.h"
#ifdef _WIN32
#include "Platform/Win32/Win32Diagnostics.h"
#else
#include "Platform/Posix/PosixDiagnostics.h"
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
