cmake_minimum_required(VERSION 3.21)
project(MuMySQLLegacy LANGUAGES NONE)
# Sólo descarga/verifica dependencias; no compila los servicios ni requiere un compilador.
include("${CMAKE_CURRENT_LIST_DIR}/../MuMySQLPackages.cmake")
mu_fetch_mysql(win32)
