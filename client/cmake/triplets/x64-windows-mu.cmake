# Triplet de vcpkg del cliente (Windows x64): todo estático y con el runtime de
# C++ estático (/MT), igual que el ejecutable (cmake/MuCompilerOptions.cmake).
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
# Mismo toolset que el cliente (v143, CMakePresets.json). vcpkg elige solo el
# Visual Studio más nuevo instalado, y una biblioteca estática compilada con
# un MSVC más nuevo no enlaza contra el STL de uno más viejo.
set(VCPKG_PLATFORM_TOOLSET v143)
if(PORT STREQUAL "dawn")
    # vcpkg agrega estas opciones después de las del portfile, así que pisan:
    # - los componentes del sistema (d3dcompiler_47, vulkan-1) se cargan desde
    #   System32; sin esto fallan con el error 87 de Windows y D3D11/Vulkan
    #   no encuentran adaptador;
    # - D3D12 compila shaders con FXC (del sistema) en vez de DXC, que se
    #   distribuiría como dxcompiler.dll/dxil.dll al lado del exe.
    set(VCPKG_CMAKE_CONFIGURE_OPTIONS
        -DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON
        -DDAWN_USE_BUILT_DXC=OFF)
endif()
