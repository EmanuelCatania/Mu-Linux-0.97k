# Backend de render WebGPU (Fase 4): Dawn enlazado estático, sin DLL propias.
#
# Dawn llega por vcpkg (vcpkg.json, feature "webgpu"); los presets de Windows
# configuran el toolchain, el triplet (cmake/triplets/*-mu) y la feature. Linux
# x86 queda sólo con OpenGL.
option(MU_RENDER_WEBGPU "Compilar el backend de render WebGPU (Dawn vía vcpkg)" OFF)
if(NOT MU_RENDER_WEBGPU)
    return()
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT WIN32)
    message(FATAL_ERROR "WebGPU no está soportado en Linux x86: usar el backend OpenGL")
endif()
find_package(Dawn CONFIG REQUIRED)
if(WIN32 AND VCPKG_APPLOCAL_DEPS)
    # La feature d3d12 instala dxcompiler.dll/dxil.dll como dependencia aunque
    # Dawn no los use (el triplet desactiva DXC): que no se copien al exe.
    message(FATAL_ERROR "Configurar con -DVCPKG_APPLOCAL_DEPS=OFF (lo fijan los presets)")
endif()
