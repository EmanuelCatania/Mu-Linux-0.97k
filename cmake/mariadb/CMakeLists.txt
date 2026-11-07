# Envoltorio de MariaDB Connector/C (ver ../MuMySQLPackages.cmake).
#
# Como proyecto raíz, MariaDB define WIN32_LEAN_AND_MEAN y NOGDI; como
# subproyecto no, y sin ellos windows.h trae winsock.h, que choca con
# winsock2.h. Se definen en este directorio para que los herede sólo MariaDB:
# NOGDI rompería el ServerDisplayer de los servers.
if(WIN32)
  add_compile_definitions(WIN32_LEAN_AND_MEAN NOGDI)
endif()

mu_fetch_mariadb()

add_library(mu_mariadb INTERFACE)
target_include_directories(mu_mariadb INTERFACE ${MU_MARIADB_INCLUDE_DIRS})
target_link_libraries(mu_mariadb INTERFACE mariadbclient)
