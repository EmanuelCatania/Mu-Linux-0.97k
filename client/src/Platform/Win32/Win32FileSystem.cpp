#include "stdafx.h"
#include "Platform/IFileSystem.h"

namespace Platform {
namespace {
class Win32FileSystem final : public IFileSystem {
public:
    FILE* Open(const char* path, const char* mode) override { return fopen(path, mode); }
    bool SetWorkingDirectory(const char* path) override { return SetCurrentDirectoryA(path) != FALSE; }
    uint32_t WorkingDirectory(uint32_t capacity, char* output) override {
        return GetCurrentDirectoryA(capacity, output);
    }
    bool Exists(const char* path) override {
        return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
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
