#pragma once

// El caller valida los rangos y la vida de sus datos antes de ejecutar.
// Windows conserva la defensa SEH adicional de los sitios heredados.
// POSIX no intenta capturar accesos invalidos como excepciones C++.
#if defined(_WIN32)
#include "Platform/PlatformHeaders.h"
#endif

namespace Platform {
namespace Detail {
inline bool InvokeGuarded(void (*operation)(void*), void* context)
{
#if defined(_WIN32)
    __try {
        operation(context);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
#else
    operation(context);
    return true;
#endif
}
}

template<class Operation>
bool RunValidatedOperation(Operation operation)
{
    return Detail::InvokeGuarded([](void* context) {
        (*static_cast<Operation*>(context))();
    }, &operation);
}
}
