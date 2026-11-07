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
