# Versiones aprobadas; no se busca SDL3/miniaudio instalados en el sistema.
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MU_WAYLAND REQUIRED wayland-client>=1.18 wayland-egl wayland-cursor egl xkbcommon>=0.5.0 libdecor-0)
    set(SDL_WAYLAND ON CACHE BOOL "" FORCE)
    set(SDL_X11 ON CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(mu_sdl
    URL https://github.com/libsdl-org/SDL/archive/7f3ae3d57459e59943a4ecfefc8f6277ec6bf540.tar.gz)
FetchContent_MakeAvailable(mu_sdl)
set(MINIAUDIO_NO_EXTRA_NODES ON CACHE BOOL "" FORCE)
FetchContent_Declare(mu_miniaudio
    URL https://github.com/mackron/miniaudio/archive/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d.tar.gz)
FetchContent_MakeAvailable(mu_miniaudio)
# SDL enlaza la biblioteca miniaudio; Win32 conserva la implementación de CSound.
set(MU_MINIAUDIO_INCLUDE_DIR "${mu_miniaudio_SOURCE_DIR}")

# FreeType se construye desde fuentes, también en Linux -m32; sin SDL_ttf.
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
FetchContent_Declare(mu_freetype
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.14.1.tar.gz
    URL_HASH SHA256=174d9e53402e1bf9ec7277e22ec199ba3e55a6be2c0740cb18c0ee9850fc8c34)
FetchContent_MakeAvailable(mu_freetype)
