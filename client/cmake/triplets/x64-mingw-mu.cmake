# Triplet de vcpkg del cliente con MinGW-w64 (Windows x64): todo estático,
# compilado con la misma toolchain que el cliente (cmake/toolchains/mingw-x64,
# variante posix). Sólo Release: es lo único que arman los presets de MinGW.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../toolchains/mingw-x64.cmake")
if(PORT STREQUAL "dawn")
    # Con MinGW, Dawn compila sólo con el backend Vulkan (vcpkg.json): los de
    # Direct3D piden headers que MinGW no trae (XAML/WinRT, PIX, constantes
    # nuevas de D3D11/D3D12).
    # - DAWN_USE_WINDOWS_UI=OFF: la opción que Dawn trae para MinGW; sin ella
    #   incluye windows.ui.*.h.
    # - Los mismos dos ajustes que x64-windows-mu (componentes del sistema y sin
    #   DXC propio).
    # - mingw-include: Tint pide <Windows.h> con mayúscula y, compilando desde
    #   Linux, los headers de MinGW están en minúscula.
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DDAWN_USE_WINDOWS_UI=OFF
        -DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON
        -DDAWN_USE_BUILT_DXC=OFF)
    set(VCPKG_C_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}/mingw-include")
    set(VCPKG_CXX_FLAGS "-I${CMAKE_CURRENT_LIST_DIR}/mingw-include")
endif()
