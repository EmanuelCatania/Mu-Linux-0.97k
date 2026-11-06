# Backend de render WebGPU (Fase 4): Dawn enlazado estático, sin DLL propias.
#
# Dawn llega por vcpkg (vcpkg.json, feature "webgpu"); los presets de Windows
# x64 configuran el toolchain, el triplet (cmake/triplets/x64-windows-mu) y la
# feature. Sólo 64 bits: el build de 32 bits de Dawn falla dentro de Dawn
# (D3D12 en cada cuadro, D3D11 al cerrar) y no se mantiene.
option(MU_RENDER_WEBGPU "Compilar el backend de render WebGPU (Dawn vía vcpkg)" OFF)
if(NOT MU_RENDER_WEBGPU)
    return()
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "WebGPU sólo se soporta en 64 bits; x86 usa el backend OpenGL")
endif()
find_package(Dawn CONFIG REQUIRED)
if(WIN32 AND VCPKG_APPLOCAL_DEPS)
    # La feature d3d12 instala dxcompiler.dll/dxil.dll como dependencia aunque
    # Dawn no los use (el triplet desactiva DXC): que no se copien al exe.
    message(FATAL_ERROR "Configurar con -DVCPKG_APPLOCAL_DEPS=OFF (lo fijan los presets)")
endif()
