include(MuMySQLPackages)

function(mu_link_mysql)
  if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      mu_fetch_mysql(winx64)
      set(mysql_lib lib64)
      set(ssl_suffix -x64)
    else()
      mu_fetch_mysql(win32)
      set(mysql_lib lib)
      set(ssl_suffix "")
    endif()
    set(mysql_root "$<IF:$<CONFIG:Debug>,${MU_MYSQL_DEBUG_ROOT},${MU_MYSQL_RELEASE_ROOT}>")
    set(mysql_debug "$<$<CONFIG:Debug>:/debug>")
    file(GLOB mysql_runtime "${MU_MYSQL_RELEASE_ROOT}/bin/*.dll")
    foreach(target IN LISTS ARGN)
      target_include_directories(${target} PRIVATE "${mysql_root}/include/jdbc")
      target_link_libraries(${target} PRIVATE "${mysql_root}/${mysql_lib}/vs14${mysql_debug}/mysqlcppconn.lib")
      target_compile_definitions(${target} PRIVATE MYSQL)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${mysql_root}/${mysql_lib}${mysql_debug}/mysqlcppconn-9-vs14.dll"
          "${mysql_root}/${mysql_lib}/libcrypto-1_1${ssl_suffix}.dll"
          "${mysql_root}/${mysql_lib}/libssl-1_1${ssl_suffix}.dll"
          ${mysql_runtime}
          "$<TARGET_FILE_DIR:${target}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${mysql_root}/LICENSE.txt" "$<TARGET_FILE_DIR:${target}>/LICENSE-MySQL.txt"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${mysql_root}/${mysql_lib}/plugin" "$<TARGET_FILE_DIR:${target}>/plugin"
        VERBATIM)
    endforeach()
  else()
    # API en C de MySQL: libmysqlclient-dev (Ubuntu/Debian) o la de MariaDB
    # (libmariadb-dev, mariadb-libs en Arch). Las dos dan mysql.h y la misma API.
    find_path(MYSQL_CLIENT_INCLUDE_DIR mysql.h PATH_SUFFIXES mysql mariadb)
    find_library(MYSQL_CLIENT_LIBRARY NAMES mysqlclient mariadb PATH_SUFFIXES mysql mariadb)
    if(NOT MYSQL_CLIENT_INCLUDE_DIR OR NOT MYSQL_CLIENT_LIBRARY)
      message(FATAL_ERROR "Se necesita libmysqlclient-dev (o libmariadb-dev) para JoinServer y DataServer")
    endif()
    foreach(target IN LISTS ARGN)
      target_include_directories(${target} PRIVATE "${MYSQL_CLIENT_INCLUDE_DIR}")
      target_link_libraries(${target} PRIVATE "${MYSQL_CLIENT_LIBRARY}")
      target_compile_definitions(${target} PRIVATE MYSQL)
    endforeach()
  endif()
endfunction()
