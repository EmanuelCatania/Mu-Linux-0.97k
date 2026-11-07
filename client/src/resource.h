#pragma once

// IDs de los recursos de src/resource.rc.
//
// IDI_MAIN_ICON TIENE que estar #definido aca: un simbolo sin definir en un
// .rc no es un error, el compilador de recursos lo toma como NOMBRE DE CADENA
// y el grupo de icono termina llamandose "IDI_MAIN_ICON" en vez de tener un
// ordinal, asi que LoadIcon con MAKEINTRESOURCE devuelve NULL (busca ordinales).
// Se puede comprobar en el .exe ya linkeado: el directorio de recursos tiene que
// mostrar RT_GROUP_ICON -> #101 y no -> "IDI_MAIN_ICON".
#define IDI_MAIN_ICON   101
