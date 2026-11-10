# Triplet de vcpkg del cliente con MinGW-w64 (Windows x64): todo estático. Sólo
# Release: es lo único que arman los presets de MinGW.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_BUILD_TYPE release)

# La toolchain de MinGW de vcpkg usa x86_64-w64-mingw32-gcc, que en
# Debian/Ubuntu es la variante win32 (sin std::thread). Si está la posix, la
# misma que usa el cliente (cmake/toolchains/mingw-x64.cmake), va esa.
find_program(MU_MINGW_CC NAMES x86_64-w64-mingw32-gcc-posix)
find_program(MU_MINGW_CXX NAMES x86_64-w64-mingw32-g++-posix)
if(MU_MINGW_CC AND MU_MINGW_CXX)
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        "-DCMAKE_C_COMPILER=${MU_MINGW_CC}" "-DCMAKE_CXX_COMPILER=${MU_MINGW_CXX}")
endif()

if(PORT STREQUAL "dawn")
    # Con MinGW, Dawn se compila sólo con el backend Vulkan (vcpkg.json): los
    # de Direct3D piden headers que MinGW no trae (XAML/WinRT, PIX, constantes
    # nuevas de D3D11/D3D12).
    # - DAWN_USE_WINDOWS_UI=OFF: la opción que Dawn trae para MinGW; sin ella
    #   incluye windows.ui.*.h.
    # - Los mismos dos ajustes que x64-windows-mu (componentes del sistema y sin
    #   DXC propio).
    # - mingw-include: Tint pide <Windows.h> con mayúscula y, compilando desde
    #   Linux, los headers de MinGW están en minúscula.
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DDAWN_USE_WINDOWS_UI=OFF
        -DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON
        -DDAWN_USE_BUILT_DXC=OFF)
    set(VCPKG_C_FLAGS "-isystem ${CMAKE_CURRENT_LIST_DIR}/mingw-include")
    set(VCPKG_CXX_FLAGS "-isystem ${CMAKE_CURRENT_LIST_DIR}/mingw-include")
endif()
