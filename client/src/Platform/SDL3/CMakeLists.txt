# Biblioteca portable usada también por el cliente Windows SDL3.
add_library(mu_platform_sdl STATIC
    SDLNativeBridge.cpp SDLWindow.cpp SDLInput.cpp SDLGLContext.cpp SDLTimer.cpp SDLFileSystem.cpp SDLAudio.cpp SDLSocket.cpp SDLFont.cpp ../../Text/TextShaper.cpp)
target_include_directories(mu_platform_sdl PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../..")
target_link_libraries(mu_platform_sdl PUBLIC SDL3::SDL3-static miniaudio freetype mu_harfbuzz mu_sheenbidi)
target_compile_features(mu_platform_sdl PUBLIC cxx_std_17)
# Icono de la ventana: SDL no lee el recurso .rc, así que los bytes de
# src/icon.ico se embeben en un header generado (la única fuente sigue siendo
# el .ico). Lo decodifica SDLWindow.cpp.
set(MU_ICON_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../icon.ico")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${MU_ICON_SOURCE}")
file(READ "${MU_ICON_SOURCE}" MU_ICON_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," MU_ICON_BYTES "${MU_ICON_HEX}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/MuIconData.h"
    "// Generado por CMake desde src/icon.ico; no editar.\n#pragma once\nstatic const unsigned char kMuIconIco[] = {${MU_ICON_BYTES}};\n")
target_include_directories(mu_platform_sdl PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
if(WIN32)
    target_link_libraries(mu_platform_sdl PUBLIC opengl32 imm32 ws2_32)
endif()

if(MSVC)
    target_compile_options(mu_platform_sdl PRIVATE /EHsc /W3)
endif()

# El cliente completo ya compila el cargador en MU_SOURCES.
if(MU_PLATFORM_ONLY)
    target_sources(mu_platform_sdl PRIVATE ../GLFunctions.cpp ../../Text/TextCore.cpp ../../Text/Utf8Proc.c)
endif()
