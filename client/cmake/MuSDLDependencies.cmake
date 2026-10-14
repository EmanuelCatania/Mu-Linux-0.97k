# Versiones aprobadas; no se busca SDL3/miniaudio instalados en el sistema.
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
# Primera validación Linux: X11. Wayland se verifica en una tanda propia.
set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
FetchContent_Declare(mu_sdl
    URL https://github.com/libsdl-org/SDL/archive/7f3ae3d57459e59943a4ecfefc8f6277ec6bf540.tar.gz)
FetchContent_MakeAvailable(mu_sdl)
set(MINIAUDIO_NO_EXTRA_NODES ON CACHE BOOL "" FORCE)
FetchContent_Declare(mu_miniaudio
    URL https://github.com/mackron/miniaudio/archive/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d.tar.gz)
FetchContent_MakeAvailable(mu_miniaudio)
# SDL enlaza la biblioteca miniaudio; Win32 conserva la implementación de CSound.
set(MU_MINIAUDIO_INCLUDE_DIR "${mu_miniaudio_SOURCE_DIR}")
