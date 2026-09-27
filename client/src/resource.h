#pragma once

// IDs de los recursos de src/resource.rc.
//
// IDI_MAIN_ICON TIENE que estar #definido aca: un simbolo sin definir en un
// .rc no es un error, el compilador de recursos lo toma como NOMBRE DE CADENA
// y el grupo de icono termina llamandose "IDI_MAIN_ICON" en vez de tener un
// ordinal.  Asi estaba hasta 2026-09-27, y por eso LoadIcon con
// MAKEINTRESOURCE devolvia NULL (busca ordinales) y la ventana salia con el
// icono generico.  Se puede comprobar en el .exe ya linkeado: el directorio
// de recursos muestra RT_GROUP_ICON -> "IDI_MAIN_ICON" en vez de -> #101.
#define IDI_MAIN_ICON   101
