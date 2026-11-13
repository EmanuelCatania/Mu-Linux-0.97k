#pragma once

// Utilidades iguales en los cuatro servers.

#include "ServerDisplayer.h"

void ErrorMessageBox(const char* message, ...);

void LogAdd(eLogColor color, const char* text, ...);
