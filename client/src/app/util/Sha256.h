#pragma once
// SHA-256 (FIPS 180-4), sin dependencias.  Hay una copia idéntica en
// server/Source/MuServer/Common/Sha256.h: el cliente y el server tienen que
// calcular el mismo hash para la caché del catálogo.

#include <cstddef>
#include <cstdint>
#include <cstring>

class CSha256
{
public:
    CSha256() { Reset(); }

    void Reset()
    {
        static const std::uint32_t init[8] = {
            0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
            0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u };
        std::memcpy(m_State, init, sizeof(m_State));
        m_Length = 0;
        m_Used = 0;
    }

    void Update(const void* data, std::size_t size)
    {
        const std::uint8_t* p = static_cast<const std::uint8_t*>(data);
        m_Length += size;
        while (size > 0) {
            std::size_t take = 64 - m_Used;
            if (take > size) take = size;
            std::memcpy(m_Block + m_Used, p, take);
            m_Used += take;
            p += take;
            size -= take;
            if (m_Used == 64) {
                Transform(m_Block);
                m_Used = 0;
            }
        }
    }

    void Final(std::uint8_t out[32])
    {
        const std::uint64_t bits = m_Length * 8;
        const std::uint8_t pad = 0x80;
        Update(&pad, 1);
        const std::uint8_t zero = 0;
        while (m_Used != 56) Update(&zero, 1);
        std::uint8_t length[8];
        for (int i = 0; i < 8; ++i) length[i] = static_cast<std::uint8_t>(bits >> (56 - 8 * i));
        Update(length, 8);
        for (int i = 0; i < 8; ++i) {
            out[4 * i + 0] = static_cast<std::uint8_t>(m_State[i] >> 24);
            out[4 * i + 1] = static_cast<std::uint8_t>(m_State[i] >> 16);
            out[4 * i + 2] = static_cast<std::uint8_t>(m_State[i] >> 8);
            out[4 * i + 3] = static_cast<std::uint8_t>(m_State[i]);
        }
        Reset();
    }

private:
    static std::uint32_t Rotr(std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

    void Transform(const std::uint8_t* block)
    {
        static const std::uint32_t k[64] = {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u };
        std::uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (std::uint32_t)block[4 * i] << 24 | (std::uint32_t)block[4 * i + 1] << 16 |
                   (std::uint32_t)block[4 * i + 2] << 8 | (std::uint32_t)block[4 * i + 3];
        for (int i = 16; i < 64; ++i) {
            const std::uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const std::uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        std::uint32_t a = m_State[0], b = m_State[1], c = m_State[2], d = m_State[3];
        std::uint32_t e = m_State[4], f = m_State[5], g = m_State[6], h = m_State[7];
        for (int i = 0; i < 64; ++i) {
            const std::uint32_t s1 = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
            const std::uint32_t ch = (e & f) ^ (~e & g);
            const std::uint32_t t1 = h + s1 + ch + k[i] + w[i];
            const std::uint32_t s0 = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t t2 = s0 + maj;
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        m_State[0] += a; m_State[1] += b; m_State[2] += c; m_State[3] += d;
        m_State[4] += e; m_State[5] += f; m_State[6] += g; m_State[7] += h;
    }

    std::uint32_t m_State[8];
    std::uint8_t m_Block[64];
    std::uint64_t m_Length;
    std::size_t m_Used;
};

// Hash de una sección del catálogo: SHA-256 de (tamaño de registro en 2 bytes
// little-endian + registros), truncado a 8 bytes.  El tamaño entra en el hash
// para que el mismo contenido con otro formato de registro no coincida.
inline void CatalogSectionHash(std::uint16_t recordSize, const void* records, std::size_t bytes, std::uint8_t out[8])
{
    CSha256 sha;
    const std::uint8_t size[2] = { static_cast<std::uint8_t>(recordSize), static_cast<std::uint8_t>(recordSize >> 8) };
    sha.Update(size, 2);
    if (bytes > 0) sha.Update(records, bytes);
    std::uint8_t full[32];
    sha.Final(full);
    std::memcpy(out, full, 8);
}
