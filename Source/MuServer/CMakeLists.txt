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
add_library(MuText STATIC Common/TextName.cpp Common/utf8proc/utf8proc.c)
target_compile_definitions(MuText PRIVATE UTF8PROC_STATIC)
target_compile_features(MuText PRIVATE cxx_std_17)
target_link_libraries(GameServer PRIVATE MuText)
target_link_libraries(DataServer PRIVATE MuText)
