#pragma once

// Arma el texto de una consulta SQL a partir de un formato estilo printf, con
// los valores de texto escapados. Lo usan los QueryManager del JoinServer y del
// DataServer en lugar de vsprintf.
//
// Conversiones:
//   %d  entero (int)
//   %u  entero sin signo (unsigned int): experiencia, zen y stats al guardar
//       el personaje
//   %s  texto (const char*), pasado por `escape`. Las consultas lo ponen
//       siempre entre comillas simples ('%s'), así que el valor no puede
//       cerrar la comilla y meter SQL propio.
//   %%  un signo %
// Cualquier otra (ancho, precisión, %x, etc.) es un error: la consulta no
// se ejecuta, para que un formato nuevo no se salte el escape sin que se note.

#include <cstdarg>
#include <functional>
#include <string>

// Devuelve el texto escapado; false si no se pudo escapar.
using SqlEscapeFn = std::function<bool(const char* in, std::string& out)>;

bool SqlFormat(std::string& out, const char* format, va_list args, const SqlEscapeFn& escape, std::string& error);
