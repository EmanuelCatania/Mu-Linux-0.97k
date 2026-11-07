#pragma once

// Backend de render activo. Se elige una vez al arrancar (Config.ini,
// [Window] RenderBackend) y no cambia en caliente: el menú de opciones guarda
// la preferencia y se aplica al reiniciar.
namespace RenderBackend {
enum class Kind : unsigned char { OpenGL, WebGPU };
extern Kind active;
inline bool IsWebGPU() {
#ifdef MU_RENDER_WEBGPU
    return active == Kind::WebGPU;
#else
    return false;
#endif
}
}
