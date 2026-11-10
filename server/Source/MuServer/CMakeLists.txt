cmake_minimum_required(VERSION 3.16)
project(MuServer LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(MuServerPlatform)
include(MuServerDependencies)
mu_server_initialize()

foreach(service ConnectServer JoinServer DataServer GameServer)
  mu_add_server(${service} "${CMAKE_CURRENT_SOURCE_DIR}/${service}")
endforeach()
mu_link_mysql(JoinServer DataServer)

# Texto UTF-8 de los nombres (validación y comparación); utf8proc va como C.
# En Windows cada servidor usa su propio runtime de C (GameServer estático,
# DataServer dinámico, ver mu_add_server) y una biblioteca estática tiene que
# coincidir con el ejecutable que la enlaza: por eso hay una copia por runtime.
function(mu_add_text_library name crt_suffix)
  add_library(${name} STATIC Common/TextName.cpp Common/utf8proc/utf8proc.c)
  target_compile_definitions(${name} PRIVATE UTF8PROC_STATIC)
  target_compile_features(${name} PRIVATE cxx_std_17)
  if(MSVC)
    set_property(TARGET ${name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${crt_suffix}")
  endif()
endfunction()
mu_add_text_library(MuText "")
mu_add_text_library(MuTextDLL "DLL")
target_link_libraries(GameServer PRIVATE MuText)
target_link_libraries(DataServer PRIVATE MuTextDLL)

# Datos por carpeta de mapa (Fase 5): spawns en TOML. Lo usan el GameServer y
# la herramienta mu-data (importar desde MonsterSetBase.txt y comparar).
add_library(MuData STATIC Common/MapSpawns.cpp)
target_include_directories(MuData PUBLIC Common)
target_compile_features(MuData PUBLIC cxx_std_17)
if(MSVC)
  set_property(TARGET MuData PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
target_link_libraries(GameServer PRIVATE MuData)

add_executable(mu-data MuData/mu-data.cpp)
target_link_libraries(mu-data PRIVATE MuData)
if(MSVC)
  set_property(TARGET mu-data PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  set_target_properties(mu-data PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  if(MINGW)
    target_link_options(mu-data PRIVATE -static)
  endif()
endif()

# Lector de la traza de paquetes del GameServer (Fase 5, sección 5c). Sólo Linux:
# el anillo vive en /dev/shm.
if(NOT WIN32)
  add_executable(mu-trace MuTrace/mu-trace.cpp)
  target_include_directories(mu-trace PRIVATE Common MuTrace)
  target_compile_features(mu-trace PRIVATE cxx_std_17)
  target_link_libraries(mu-trace PRIVATE rt)
  set_target_properties(mu-trace PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
