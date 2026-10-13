# Sólo se habilita el backend ya implementado; la elección Linux sigue pendiente.
set(MU_PLATFORM_BACKEND "Win32" CACHE STRING "Backend de plataforma del cliente")
set_property(CACHE MU_PLATFORM_BACKEND PROPERTY STRINGS Win32)
if(NOT MU_PLATFORM_BACKEND STREQUAL "Win32" OR NOT WIN32)
    message(FATAL_ERROR "Backend pendiente: esta tanda sólo implementa Win32. La biblioteca Linux aún no fue elegida.")
endif()
enable_language(RC)

if(NOT MSVC)
    message(FATAL_ERROR "mu97k sólo compila con MSVC (Visual Studio 2022, toolset v143).")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "mu97k es Win32 (x86). Configurar con: cmake -A Win32")
endif()

