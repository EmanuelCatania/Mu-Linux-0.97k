#include "stdafx.h"
#include "net/ServerTrust.h"
#include "app/config/ServerConfig.h"
#include "platform/IFileSystem.h"
#include <cstdio>
#include <cstring>

// La clave fijada de cada server: lectura y escritura.  La decisión y la
// verificación de firmas están en ServerTrustCore.cpp.

namespace ServerTrust {

namespace {

std::string s_Dir;

bool FromHex(const char* text, std::uint8_t* out, std::size_t size)
{
    if (!text || std::strlen(text) < size * 2) return false;
    for (std::size_t n = 0; n < size; ++n) {
        int value = 0;
        for (int d = 0; d < 2; ++d) {
            const char c = text[n * 2 + d];
            int digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else return false;
            value = value * 16 + digit;
        }
        out[n] = (std::uint8_t)value;
    }
    return true;
}

std::string PinFile() { return s_Dir + "/server.pub"; }

bool ReadPin(std::uint8_t out[KeySize])
{
    if (!s_Dir.empty()) {
        if (FILE* f = Platform::GetFileSystem().Open(PinFile().c_str(), "rb")) {
            char text[KeySize * 2 + 1] = {};
            const std::size_t got = fread(text, 1, KeySize * 2, f);
            fclose(f);
            if (got == KeySize * 2 && FromHex(text, out, KeySize)) return true;
        }
    }
    // Sin archivo propio vale la clave compilada, si el build trae una.
    return FromHex(ServerConfig::ServerPublicKey, out, KeySize);
}

void WritePin(const std::uint8_t key[KeySize])
{
    if (s_Dir.empty()) return;
    Platform::GetFileSystem().EnsureDirectory(s_Dir.c_str());
    FILE* f = Platform::GetFileSystem().Open(PinFile().c_str(), "wb");
    if (!f) return;
    static const char digits[] = "0123456789abcdef";
    char text[KeySize * 2];
    for (std::size_t n = 0; n < KeySize; ++n) {
        text[n * 2] = digits[key[n] >> 4];
        text[n * 2 + 1] = digits[key[n] & 15];
    }
    fwrite(text, 1, sizeof(text), f);
    fclose(f);
}

} // namespace

void SetServer(const std::string& cacheDir) { s_Dir = cacheDir; }

Result Check(const std::uint8_t publicKey[KeySize],
             const std::uint8_t* previousKey,
             const std::uint8_t* rotationSignature)
{
    std::uint8_t pinned[KeySize] = {};
    const bool hasPinned = ReadPin(pinned);
    const Result result = Decide(hasPinned, pinned, publicKey, previousKey, rotationSignature);
    if (result == Result::FirstUse || result == Result::Rotated) WritePin(publicKey);
    return result;
}

} // namespace ServerTrust
