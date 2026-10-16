# Biblioteca portable usada también por el cliente Windows SDL3.
add_library(mu_platform_sdl STATIC
    SDLNativeBridge.cpp SDLWindow.cpp SDLInput.cpp SDLGLContext.cpp SDLTimer.cpp SDLFileSystem.cpp SDLAudio.cpp SDLSocket.cpp SDLFont.cpp)
target_include_directories(mu_platform_sdl PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../..")
target_link_libraries(mu_platform_sdl PUBLIC SDL3::SDL3-static miniaudio freetype)
target_compile_features(mu_platform_sdl PUBLIC cxx_std_17)
if(WIN32)
    target_link_libraries(mu_platform_sdl PUBLIC opengl32 imm32 ws2_32)
endif()

if(MSVC)
    target_compile_options(mu_platform_sdl PRIVATE /EHsc /W3)
endif()
