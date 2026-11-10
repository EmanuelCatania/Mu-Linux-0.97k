# Compilación cruzada a Windows x86 (32 bits) con MinGW-w64 desde Linux.
#   cmake -S server -B build-mingw -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=server/cmake/toolchains/mingw-x86.cmake
#
# Usa la variante "posix" del toolchain (hilos de winpthreads), que es la que
# trae std::thread y std::mutex. En Debian/Ubuntu: g++-mingw-w64-i686-posix.
# Para compilar desde Windows con MSYS2 (UCRT64) no hace falta este archivo.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(MU_MINGW_PREFIX i686-w64-mingw32)
find_program(CMAKE_C_COMPILER NAMES ${MU_MINGW_PREFIX}-gcc-posix ${MU_MINGW_PREFIX}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${MU_MINGW_PREFIX}-g++-posix ${MU_MINGW_PREFIX}-g++ REQUIRED)
find_program(CMAKE_RC_COMPILER NAMES ${MU_MINGW_PREFIX}-windres REQUIRED)

set(CMAKE_FIND_ROOT_PATH /usr/${MU_MINGW_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
