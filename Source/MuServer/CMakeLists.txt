cmake_minimum_required(VERSION 3.16)

project(MuServerLinux CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

function(add_mu_server target dir)
  file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp")

  if (WIN32)
    list(REMOVE_ITEM SRC_FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${target}Linux.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/SocketManagerLinux.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/SocketManagerUdpLinux.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/ConnectionLinux.cpp")
  else()
    list(REMOVE_ITEM SRC_FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${target}.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/SocketManager.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/SocketManagerUdp.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/Connection.cpp")
  endif()

  add_executable(${target} ${SRC_FILES})
  target_include_directories(${target} PRIVATE "${COMMON_DIR}")

  if (UNIX)
    target_link_libraries(${target} PRIVATE pthread)
  endif()
endfunction()

add_mu_server(ConnectServer ConnectServer)
add_mu_server(JoinServer JoinServer)
add_mu_server(DataServer DataServer)
add_mu_server(GameServer GameServer)

if (UNIX)
  find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_connection.h
    PATHS /usr/include /usr/include/mysql /usr/local/include /usr/include/cppconn)
  find_library(MYSQLCPPCONN_LIBRARY mysqlcppconn
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

  if (MYSQLCPPCONN_INCLUDE_DIR)
    target_include_directories(JoinServer PRIVATE ${MYSQLCPPCONN_INCLUDE_DIR})
    target_include_directories(DataServer PRIVATE ${MYSQLCPPCONN_INCLUDE_DIR})
  endif()

  if (MYSQLCPPCONN_LIBRARY)
    target_link_libraries(JoinServer PRIVATE ${MYSQLCPPCONN_LIBRARY})
    target_link_libraries(DataServer PRIVATE ${MYSQLCPPCONN_LIBRARY})
  endif()

  target_compile_definitions(JoinServer PRIVATE MYSQL)
  target_compile_definitions(DataServer PRIVATE MYSQL)
endif()
