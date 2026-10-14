#pragma once
#include <cstdint>
namespace Platform {
// Efectos del cliente; la música continúa en CSound/miniaudio.
class IAudio {
public:
    virtual ~IAudio() = default;
    virtual int32_t Initialize(void* window) = 0;
    virtual int32_t CreateBuffer(int slot, const char* path, int channels, bool spatial) = 0;
    virtual int32_t Play(int slot, uint32_t channel, bool loop) = 0;
    virtual void Stop(int slot) = 0;
    virtual int32_t ReleaseBuffer(int slot) = 0;
    virtual void SetVolume(int slot, int channel, int32_t hundredthsDb) = 0;
    // Espacio del oyente: (-X rotada, 0, -Y rotada), escala 0.004; aplicar inmediatamente.
    virtual void SetPosition(int slot, int channel, float x, float y, float z) = 0;
    virtual void Shutdown() = 0;
};
IAudio& GetAudio();
}
