cmake_minimum_required(VERSION 3.16)
project(MuServer LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
include(MuServerPlatform)
include(MuServerDependencies)
mu_server_initialize()

foreach(service ConnectServer JoinServer DataServer GameServer)
  mu_add_server(${service} "${CMAKE_CURRENT_SOURCE_DIR}/${service}")
endforeach()
mu_link_mysql(JoinServer DataServer)
