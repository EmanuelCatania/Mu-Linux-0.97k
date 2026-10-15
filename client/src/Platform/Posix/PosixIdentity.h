#pragma once
#include "Platform/IIdentity.h"
#include <cstdio>
#include <cstring>
namespace Platform {
class PosixIdentity final : public IIdentity {
    const char* primary;
    const char* alternate;
    static bool ReadFile(const char* path, char (&output)[36]) {
        FILE* file = std::fopen(path, "rb");
        if (!file) return false;
        char input[128];
        const auto count = std::fread(input, 1, sizeof(input), file);
        const bool failed = std::ferror(file) != 0;
        std::fclose(file);
        if (failed || count < 32 || count == sizeof(input)) return false;
        bool nonzero = false;
        for (unsigned i = 0; i < 32; ++i) {
            char c = input[i];
            if (c >= 'a' && c <= 'f') c -= 'a' - 'A';
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
            nonzero |= c != '0'; input[i] = c;
        }
        if (!nonzero) return false;
        for (auto i = std::size_t(32); i < count; ++i)
            if (input[i] != '\n' && input[i] != '\r' && input[i] != ' ' && input[i] != '\t') return false;
        // Representación determinista de los 128 bits, sin identificador constante de respaldo.
        unsigned position = 0;
        for (unsigned i = 0; i < 32; ++i) {
            if (i && i % 8 == 0) output[position++] = '-';
            output[position++] = input[i];
        }
        output[position] = 0;
        return true;
    }
public:
    explicit PosixIdentity(const char* first = "/etc/machine-id", const char* second = "/var/lib/dbus/machine-id")
        : primary(first), alternate(second) {}
    bool Read(char (&output)[36], const char*& error) override {
        output[0] = 0; error = nullptr;
        if (ReadFile(primary, output) || ReadFile(alternate, output)) return true;
        error = "No hay machine-id válido en /etc/machine-id ni /var/lib/dbus/machine-id";
        return false;
    }
};
}
