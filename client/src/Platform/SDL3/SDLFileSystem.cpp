#include "Platform/IFileSystem.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
namespace Platform {
namespace {
namespace fs = std::filesystem;
std::string Lower(std::string value) {
    for (char& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return value;
}
std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
}
struct Directory {
    fs::file_time_type modified{};
    bool loaded = false;
    std::unordered_map<std::string, std::string> names;
};
std::unordered_map<std::string, Directory> directories;
// Sólo se resuelven los nombres: fread/fseek y los headers OZJ/OZT no cambian.
fs::path Resolve(const char* input, bool newLeaf = false) {
    if (!input || !*input) return {};
    std::string normalized(input);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    fs::path path(normalized);
    std::error_code error;
    path = fs::absolute(path, error).lexically_normal();
    if (error) return {};
    fs::path resolved = path.root_path();
    auto relative = path.relative_path();
    for (auto it = relative.begin(); it != relative.end(); ++it) {
        const std::string component = it->string();
        auto exact = resolved / *it;
        if (fs::exists(exact, error)) { resolved = exact; continue; }
        error.clear();
        const auto stamp = fs::last_write_time(resolved, error);
        if (error) return {};
        auto& cache = directories[resolved.string()];
        if (!cache.loaded || cache.modified != stamp) {
            cache.names.clear(); cache.loaded = false;
            fs::directory_iterator entries(resolved, error);
            if (error) return {};
            const fs::directory_iterator end;
            for (; entries != end; entries.increment(error)) {
                if (error) return {};
                auto name = entries->path().filename().string();
                auto inserted = cache.names.emplace(Lower(name), name);
                // Una colisión Foo/foo exige coincidencia exacta; no elegir al azar.
                if (!inserted.second) inserted.first->second.clear();
            }
            if (error) return {};
            cache.modified = stamp; cache.loaded = true;
        }
        const auto found = cache.names.find(Lower(component));
        if (found == cache.names.end()) {
            if (newLeaf && std::next(it) == relative.end()) return exact;
            return {};
        }
        if (found->second.empty()) return {};
        resolved /= found->second;
    }
    return resolved;
}
uint32_t Copy(const std::string& value, char* output, uint32_t capacity) {
    if (!capacity || !output) return 0;
    auto size = std::min<size_t>(value.size(), capacity - 1);
    std::memcpy(output, value.data(), size); output[size] = 0;
    return static_cast<uint32_t>(size);
}
#ifndef _WIN32
struct IniLine { std::string original, section, key, value; bool heading = false; };
std::vector<IniLine> ReadIni(const char* path) {
    std::vector<IniLine> lines;
    const auto resolved = Resolve(path);
    if (resolved.empty()) return lines;
    std::ifstream stream(resolved);
    std::string line, section;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        IniLine parsed; parsed.original = line;
        const auto text = Trim(line);
        if (text.size() >= 2 && text.front() == '[' && text.back() == ']') {
            section = Lower(Trim(text.substr(1, text.size() - 2))); parsed.heading = true;
        } else if (!text.empty() && text.front() != ';' && text.front() != '#') {
            const auto equal = text.find('=');
            if (equal != std::string::npos) {
                parsed.key = Lower(Trim(text.substr(0, equal)));
                parsed.value = Trim(text.substr(equal + 1));
            }
        }
        parsed.section = section; lines.push_back(parsed);
    }
    return lines;
}
#endif
class SDLFileSystem final : public IFileSystem {
public:
    FILE* Open(const char* path, const char* mode) override {
        const auto resolved = Resolve(path, mode && (std::strchr(mode, 'w') || std::strchr(mode, 'a')));
        if (resolved.empty()) return nullptr;
        return std::fopen(resolved.string().c_str(), mode);
    }
    bool EnsureDirectory(const char* path) override {
#ifdef _WIN32
        return CreateDirectoryA(path, nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
        const auto resolved = Resolve(path, true);
        if (resolved.empty()) return false;
        std::error_code error;
        if (fs::create_directory(resolved, error)) return true;
        return !error && fs::is_directory(resolved, error);
#endif
    }
    bool Exists(const char* path) override { return !Resolve(path).empty(); }
    bool SetWorkingDirectory(const char* path) override {
        auto resolved = Resolve(path);
        if (resolved.empty()) return false;
        std::error_code error; fs::current_path(resolved, error); return !error;
    }
    uint32_t WorkingDirectory(uint32_t capacity, char* output) override {
        std::error_code error; auto path = fs::current_path(error).string();
        if (error) return 0;
        if (capacity <= path.size()) return static_cast<uint32_t>(path.size() + 1);
        return Copy(path, output, capacity);
    }
    uint32_t ExecutablePath(char* output, uint32_t capacity) override {
#ifdef _WIN32
        return GetModuleFileNameA(nullptr, output, capacity);
#else
        std::error_code error;
        auto path = fs::read_symlink("/proc/self/exe", error).string();
        if (error) return 0;
        return Copy(path, output, capacity);
#endif
    }
    uint32_t ReadIniString(const char* section, const char* key, const char* fallback,
        char* output, uint32_t capacity, const char* path) override {
#ifdef _WIN32
        return GetPrivateProfileStringA(section, key, fallback, output, capacity, path);
#else
        std::string result = fallback ? fallback : "";
        if (section && key) for (const auto& line : ReadIni(path)) {
            if (!line.heading && line.section == Lower(section) && line.key == Lower(key)) {
                result = line.value;
                if (result.size() >= 2 && ((result.front() == '"' && result.back() == '"') ||
                    (result.front() == '\'' && result.back() == '\''))) result = result.substr(1, result.size() - 2);
                break;
            }
        }
        return Copy(result, output, capacity);
#endif
    }
    uint32_t ReadIniSection(const char* section, char* output, uint32_t capacity, const char* path) override {
#ifdef _WIN32
        return GetPrivateProfileSectionA(section, output, capacity, path);
#else
        if (!output || !capacity) return 0;
        std::string result;
        for (const auto& line : ReadIni(path)) if (!line.heading && !line.key.empty() && line.section == Lower(section)) {
            result += Trim(line.original); result.push_back(0);
        }
        if (result.empty()) { output[0] = 0; if (capacity > 1) output[1] = 0; return 0; }
        const auto size = std::min<size_t>(result.size(), capacity - 1);
        std::memcpy(output, result.data(), size); output[size] = 0;
        if (result.size() >= capacity) { if (capacity > 1) output[capacity - 2] = 0; return capacity > 1 ? capacity - 2 : 0; }
        return static_cast<uint32_t>(size);
#endif
    }
    bool WriteIniString(const char* section, const char* key, const char* value, const char* path) override {
#ifdef _WIN32
        return WritePrivateProfileStringA(section, key, value, path) != FALSE;
#else
        if (!section) return false;
        auto resolved = Resolve(path, true); if (resolved.empty()) return false;
        auto lines = ReadIni(path); const auto target = Lower(section);
        bool foundSection = false, written = false; std::vector<std::string> output;
        for (const auto& line : lines) {
            if (foundSection && line.heading && line.section != target && key && value && !written) {
                output.push_back(std::string(key) + "=" + value); written = true;
            }
            if (line.section == target) {
                foundSection = true;
                if (!key) continue;
                if (!line.heading && line.key == Lower(key)) {
                    if (value && !written) output.push_back(std::string(key) + "=" + value);
                    written = true; continue;
                }
            }
            output.push_back(line.original);
        }
        if (key && value && !written) {
            if (!foundSection) output.push_back(std::string("[") + section + "]");
            output.push_back(std::string(key) + "=" + value);
        }
        std::ofstream stream(resolved, std::ios::binary | std::ios::trunc);
        if (!stream) return false;
        for (const auto& line : output) stream << line << "\r\n";
        stream.flush(); return bool(stream);
#endif
    }
};
}
IFileSystem& GetFileSystem() { static SDLFileSystem fileSystem; return fileSystem; }
}
