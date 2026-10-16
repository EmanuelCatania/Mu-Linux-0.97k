#pragma once
#include <cstdio>
#include <cstdint>

namespace Platform {
// Aperturas CRT: el llamador conserva fread/fseek/fclose y sus modos originales.
// El backend resuelve las rutas; Win32 conserva su semántica nativa.
class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual FILE* Open(const char* path, const char* mode) = 0;
    virtual bool SetWorkingDirectory(const char* path) = 0;
    virtual uint32_t WorkingDirectory(uint32_t capacity, char* output) = 0;
    // Crea un directorio de salida; acepta un directorio ya existente.
    virtual bool EnsureDirectory(const char* path) = 0;
    virtual bool Exists(const char* path) = 0;
    virtual uint32_t ExecutablePath(char* output, uint32_t capacity) = 0;
    virtual uint32_t ReadIniString(const char* section, const char* key,
        const char* fallback, char* output, uint32_t capacity, const char* path) = 0;
    // Lista de pares terminados en NUL y NUL final; conserva el truncado nativo.
    virtual uint32_t ReadIniSection(const char* section, char* output,
        uint32_t capacity, const char* path) = 0;
    // key/value nulos conservan la eliminación de sección/clave de Win32.
    virtual bool WriteIniString(const char* section, const char* key,
        const char* value, const char* path) = 0;
};
IFileSystem& GetFileSystem();
} // namespace Platform
