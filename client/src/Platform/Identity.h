#pragma once
#include "Platform/IIdentity.h"
#ifdef _WIN32
#include "Platform/Win32/Win32Identity.h"
#else
#include "Platform/Posix/PosixIdentity.h"
#endif
namespace Platform {
inline IIdentity& GetIdentity() {
#ifdef _WIN32
    static Win32Identity identity;
#else
    static PosixIdentity identity;
#endif
    return identity;
}
}
