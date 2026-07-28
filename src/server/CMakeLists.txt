if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "The server CMake build supports Linux only.")
endif()

find_package(Threads REQUIRED)
find_package(MySQLConnectorCpp REQUIRED)

function(mu_add_server target)
  add_executable(${target} ${ARGN})
  target_compile_features(${target} PRIVATE cxx_std_17)
  target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Common")
  target_link_libraries(${target} PRIVATE Threads::Threads)
  set_target_properties(${target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endfunction()

add_subdirectory(ConnectServer)
add_subdirectory(JoinServer)
add_subdirectory(DataServer)
add_subdirectory(GameServer)
