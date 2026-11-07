include_guard(GLOBAL)
include(FetchContent)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# MariaDB Connector/C compilado desde el código fuente, estático, para Windows.
# Es la API en C de MySQL (mysql.h): su ABI no depende del compilador, así que
# sirve igual con MSVC y con MinGW, y no deja DLL de terceros junto al server.
# El TLS va por Schannel (el de Windows): MySQL 8.4 autentica con
# caching_sha2_password, que fuera del caché del server lo necesita.
#
# Define MU_MARIADB_INCLUDE_DIRS y deja el target `mariadbclient`.
# Compartido por el build de CMake y por los proyectos MSBuild (cmake/mysql).
set(MU_MARIADB_VERSION 3.4.11)
set(MU_MARIADB_SHA256 7cdb35c571dd0c187f806f569285829d31b4aba4feb93957a034b0b1e887e276)

function(mu_fetch_mariadb)
  if(TARGET mariadbclient)
    return()
  endif()

  # Sólo la biblioteca estática y los plugins de autenticación que usa MySQL,
  # metidos dentro de ella. Todo lo demás apagado.
  set(options
    WITH_UNIT_TESTS=OFF WITH_CURL=OFF WITH_EXTERNAL_ZLIB=OFF WITH_MYSQLCOMPAT=OFF
    WITH_DOCS=OFF WITH_MSI=OFF WITH_SIGNCODE=OFF WITH_RTC=OFF WITH_ICONV=OFF
    DEFAULT_SSL_VERIFY_SERVER_CERT=OFF
    CLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC CLIENT_PLUGIN_SHA256_PASSWORD=STATIC
    CLIENT_PLUGIN_DIALOG=OFF CLIENT_PLUGIN_CLIENT_ED25519=OFF CLIENT_PLUGIN_PARSEC=OFF
    CLIENT_PLUGIN_AUTH_GSSAPI_CLIENT=OFF CLIENT_PLUGIN_MYSQL_CLEAR_PASSWORD=OFF
    CLIENT_PLUGIN_REMOTE_IO=OFF CLIENT_PLUGIN_ZSTD=OFF CLIENT_PLUGIN_REPLICATION=OFF
    # pvio_shmem no se puede apagar: dentro de la biblioteca, para que no sea un módulo.
    CLIENT_PLUGIN_PVIO_SHMEM=STATIC
    # Como subproyecto no define los destinos de instalación y los plugins los piden.
    INSTALL_PLUGINDIR=lib/mariadb/plugin INSTALL_LIBDIR=lib/mariadb
    INSTALL_INCLUDEDIR=include/mariadb INSTALL_BINDIR=bin INSTALL_DOCDIR=docs
    INSTALL_LICENSEDIR=docs INSTALL_MANDIR=man INSTALL_PCDIR=lib/pkgconfig)
  if(WIN32)
    list(APPEND options WITH_SSL=SCHANNEL)
  endif()
  foreach(option IN LISTS options)
    string(REPLACE "=" ";" pair "${option}")
    list(GET pair 0 name)
    list(GET pair 1 value)
    set(${name} ${value} CACHE STRING "" FORCE)
  endforeach()

  FetchContent_Declare(mu_mariadb
    URL "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v${MU_MARIADB_VERSION}.tar.gz"
    URL_HASH SHA256=${MU_MARIADB_SHA256})
  FetchContent_GetProperties(mu_mariadb)
  if(NOT mu_mariadb_POPULATED)
    FetchContent_MakeAvailable(mu_mariadb)
  endif()

  # El proyecto de MariaDB agrega varios targets (la DLL, mariadb_config, ...):
  # que no se compilen salvo que alguien los pida.
  set_property(DIRECTORY "${mu_mariadb_SOURCE_DIR}" PROPERTY EXCLUDE_FROM_ALL TRUE)

  # Mismo runtime de C que el ejecutable que la enlaza (DataServer y JoinServer
  # usan el dinámico, ver mu_add_server).
  if(MSVC)
    set_property(TARGET mariadbclient PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()

  set(MU_MARIADB_INCLUDE_DIRS "${mu_mariadb_SOURCE_DIR}/include" "${mu_mariadb_BINARY_DIR}/include" PARENT_SCOPE)
endfunction()
