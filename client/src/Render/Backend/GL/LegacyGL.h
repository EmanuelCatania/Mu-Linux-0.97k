#pragma once
// OpenGL heredado (perfil de compatibilidad) y las barreras del lote. Sólo
// para archivos del backend GL: el resto del juego usa Render/Gfx.h y
// BufferedDraw, sin tipos ni constantes de OpenGL.
#include "Platform/PlatformHeaders.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "Render/GLCallBarrier.h"
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
