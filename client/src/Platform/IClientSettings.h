#pragma once
#include <cstdint>
namespace Platform {
struct InitialClientSettings {
    char username[11]{};
    std::uint32_t sound = 1, music = 0, resolution = 0, textOut = 0;
};
class IClientSettings {
public:
    virtual ~IClientSettings() = default;
    virtual void ReadDefaults(InitialClientSettings& settings, const char* iniPath) = 0;
    virtual bool ExecutableVersion(unsigned short words[4]) = 0;
};
IClientSettings& GetClientSettings();
}
