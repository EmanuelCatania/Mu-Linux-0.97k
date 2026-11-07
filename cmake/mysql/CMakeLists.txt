cmake_minimum_required(VERSION 3.21)
project(MuMySQLLegacy LANGUAGES C)
# Compila sólo MariaDB Connector/C para los proyectos MSBuild (ver MuMySQLLegacy.targets).
include("${CMAKE_CURRENT_LIST_DIR}/../MuMySQLPackages.cmake")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../mariadb" "${CMAKE_BINARY_DIR}/mu_mariadb")

# Target en este directorio: `cmake --build --target` con un target de un
# subdirectorio no encuentra su .vcxproj con el generador de Visual Studio.
add_custom_target(mu_mysql_legacy ALL)
add_dependencies(mu_mysql_legacy mariadbclient)
