cmake_minimum_required(VERSION 3.21)

project(mu_097k LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory(src/server)
elseif(WIN32)
  message(FATAL_ERROR
    "The Windows client CMake targets are not available yet. "
    "Use scripts/client-workflow.ps1 with MSBuild until issue #22 is implemented.")
else()
  message(FATAL_ERROR "Unsupported platform '${CMAKE_SYSTEM_NAME}'.")
endif()
