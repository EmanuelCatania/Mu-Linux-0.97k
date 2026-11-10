// Puente para MinGW (triplet x64-mingw-mu): Dawn incluye <Windows.h> con
// mayúscula y, compilando desde Linux, el header de MinGW es <windows.h>.
// include_next sigue buscando después de esta carpeta, así que en un sistema
// de archivos que no distingue mayúsculas (MSYS2 en Windows) tampoco se
// incluye a sí mismo. Va como header de sistema: Dawn compila con -pedantic,
// que sin esto marca include_next como error.
#pragma once
#pragma GCC system_header
#include_next <windows.h>
