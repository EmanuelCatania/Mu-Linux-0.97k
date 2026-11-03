#pragma once
// OpenGL heredado (perfil de compatibilidad) para el texto Win32 por listas
// de glifos y la creación del contexto. El resto del juego usa Render/Gfx.h y
// BufferedDraw, sin tipos ni constantes de OpenGL.
#include "Platform/PlatformHeaders.h"
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
