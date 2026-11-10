# Parche de MariaDB Connector/C, aplicado sobre el código bajado (cwd = fuentes).
#
# mysql_load_plugin() es variádica: su declaración (mysql.h, client_plugin.h)
# no lleva STDCALL y su definición sí. MSVC ignora __stdcall en una función
# variádica y en x64 hay una sola convención, así que no se nota; GCC en 32 bits
# (MinGW i686) lo toma como otro tipo y no compila. Se saca de la definición.
set(file "libmariadb/ma_client_plugin.c.in")
file(READ "${file}" text)
set(old "struct st_mysql_client_plugin * STDCALL\nmysql_load_plugin(MYSQL *mysql, const char *name, int type, int argc, ...)")
set(new "struct st_mysql_client_plugin *\nmysql_load_plugin(MYSQL *mysql, const char *name, int type, int argc, ...)")
string(FIND "${text}" "${old}" found)
if(found GREATER -1)
  string(REPLACE "${old}" "${new}" text "${text}")
  file(WRITE "${file}" "${text}")
endif()
