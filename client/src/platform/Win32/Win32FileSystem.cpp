#include "stdafx.h"
#include "platform/IFileSystem.h"
#include "platform/ContentLayers.h"
#include <cstring>

namespace Platform {
namespace {
class Win32FileSystem final : public IFileSystem {
public:
    FILE* Open(const char* path, const char* mode) override {
        const bool write = mode && (std::strchr(mode, 'w') || std::strchr(mode, 'a') || std::strchr(mode, '+'));
        // Lecturas de Data/: la primera capa que tenga el archivo (ContentLayers).
        if (!write)
            for (const auto& candidate : ContentLayers::Candidates(path))
                if (FILE* file = fopen(candidate.path.c_str(), mode)) return file;
        return fopen(path, mode);
    }
    bool SetWorkingDirectory(const char* path) override { return SetCurrentDirectoryA(path) != FALSE; }
    uint32_t WorkingDirectory(uint32_t capacity, char* output) override {
        return GetCurrentDirectoryA(capacity, output);
    }
    bool EnsureDirectory(const char* path) override {
        return CreateDirectoryA(path, nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
    }
    bool Exists(const char* path) override {
        for (const auto& candidate : ContentLayers::Candidates(path))
            if (GetFileAttributesA(candidate.path.c_str()) != INVALID_FILE_ATTRIBUTES) return true;
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
    }
    int FindLayer(const char* path, int maxLayer) override {
        const auto candidates = ContentLayers::Candidates(path);
        if (candidates.empty())
            return maxLayer >= 0 && GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES ? 0 : -1;
        for (const auto& candidate : candidates)
            if (candidate.layer <= maxLayer && GetFileAttributesA(candidate.path.c_str()) != INVALID_FILE_ATTRIBUTES)
                return candidate.layer;
        return -1;
    }
    uint32_t ExecutablePath(char* output, uint32_t capacity) override {
        return GetModuleFileNameA(nullptr, output, capacity);
    }
    uint32_t ReadIniString(const char* section, const char* key,
        const char* fallback, char* output, uint32_t capacity, const char* path) override {
        return GetPrivateProfileStringA(section, key, fallback, output, capacity, path);
    }
    uint32_t ReadIniSection(const char* section, char* output,
        uint32_t capacity, const char* path) override {
        return GetPrivateProfileSectionA(section, output, capacity, path);
    }
    bool WriteIniString(const char* section, const char* key,
        const char* value, const char* path) override {
        return WritePrivateProfileStringA(section, key, value, path) != FALSE;
    }
};
}
IFileSystem& GetFileSystem() {
    static Win32FileSystem fileSystem;
    return fileSystem;
}
} // namespace Platform
