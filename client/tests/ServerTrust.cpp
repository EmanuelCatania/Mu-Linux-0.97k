// Fase 5, C3: firma del manifiesto del catálogo y confianza en la clave del
// server.  La prueba hace de server con Monocypher directo (mismos mensajes que
// arma GameServer/ContentCatalog.cpp y ServerIdentity.cpp) y verifica con el
// código del cliente (net/ServerTrustCore.cpp).

#include "net/ServerTrust.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include "net/crypto/monocypher.h"
}

namespace {

int s_Failures = 0;

void Require(bool ok, const char* what)
{
    if (!ok) {
        std::printf("FALLA: %s\n", what);
        ++s_Failures;
    }
}

struct KeyPair {
    std::uint8_t secret[64];
    std::uint8_t pub[32];
};

KeyPair MakeKey(std::uint8_t fill)
{
    KeyPair k;
    std::uint8_t seed[32];
    std::memset(seed, fill, sizeof(seed));
    crypto_eddsa_key_pair(k.secret, k.pub, seed);
    return k;
}

// Mensaje del manifiesto: contexto + count + (subh + hash[8]) por sección.
std::vector<std::uint8_t> Manifest(const std::vector<std::pair<std::uint8_t, std::uint64_t>>& sections)
{
    const char context[] = "MuCatalog1";
    std::vector<std::uint8_t> m(context, context + sizeof(context) - 1);
    m.push_back((std::uint8_t)sections.size());
    for (const auto& s : sections) {
        m.push_back(s.first);
        for (int b = 0; b < 8; ++b) m.push_back((std::uint8_t)(s.second >> (8 * b)));
    }
    return m;
}

void Rotation(std::uint8_t out[64], const KeyPair& oldKey, const std::uint8_t newPub[32])
{
    const char context[] = "MuIdentityRotation1";
    std::uint8_t message[sizeof(context) - 1 + 32];
    std::memcpy(message, context, sizeof(context) - 1);
    std::memcpy(message + sizeof(context) - 1, newPub, 32);
    crypto_eddsa_sign(out, oldKey.secret, message, sizeof(message));
}

} // namespace

int main()
{
    using ServerTrust::Result;
    const KeyPair server = MakeKey(0x11);
    const KeyPair next = MakeKey(0x22);
    const KeyPair other = MakeKey(0x33);

    // ── Firma del manifiesto ────────────────────────────────────────────────
    const std::vector<std::pair<std::uint8_t, std::uint64_t>> sections = {
        { 0xE7, 0x0102030405060708ull }, { 0xE8, 0x1112131415161718ull }, { 0xF3, 0x2122232425262728ull } };
    std::vector<std::uint8_t> manifest = Manifest(sections);
    std::uint8_t signature[64];
    crypto_eddsa_sign(signature, server.secret, manifest.data(), manifest.size());

    Require(ServerTrust::Verify(signature, server.pub, manifest.data(), manifest.size()), "la firma del server vale");
    Require(!ServerTrust::Verify(signature, other.pub, manifest.data(), manifest.size()), "con otra clave no vale");

    std::vector<std::uint8_t> tampered = manifest;
    tampered[tampered.size() - 1] ^= 1;   // un bit del hash de la última sección
    Require(!ServerTrust::Verify(signature, server.pub, tampered.data(), tampered.size()), "un hash cambiado invalida la firma");

    std::vector<std::uint8_t> shorter = Manifest({ sections[0], sections[1] });
    Require(!ServerTrust::Verify(signature, server.pub, shorter.data(), shorter.size()), "quitar una sección invalida la firma");

    std::uint8_t badSignature[64];
    std::memcpy(badSignature, signature, sizeof(badSignature));
    badSignature[10] ^= 0x80;
    Require(!ServerTrust::Verify(badSignature, server.pub, manifest.data(), manifest.size()), "una firma alterada no vale");

    // ── Confianza en la clave ───────────────────────────────────────────────
    Require(ServerTrust::Decide(false, server.pub, server.pub, nullptr, nullptr) == Result::FirstUse, "sin clave fijada: primer contacto");
    Require(ServerTrust::Decide(true, server.pub, server.pub, nullptr, nullptr) == Result::Trusted, "la clave fijada es de confianza");
    Require(ServerTrust::Decide(true, server.pub, other.pub, nullptr, nullptr) == Result::Mismatch, "otra clave sin rotación se rechaza");

    std::uint8_t rotation[64];
    Rotation(rotation, server, next.pub);
    Require(ServerTrust::Decide(true, server.pub, next.pub, server.pub, rotation) == Result::Rotated, "rotación firmada por la clave fijada");
    Require(ServerTrust::Decide(true, next.pub, next.pub, server.pub, rotation) == Result::Trusted, "tras rotar, la nueva es la fijada");

    // Rotación firmada por una clave que no es la fijada.
    std::uint8_t forged[64];
    Rotation(forged, other, next.pub);
    Require(ServerTrust::Decide(true, server.pub, next.pub, other.pub, forged) == Result::Mismatch, "rotación de otra clave se rechaza");
    Require(ServerTrust::Decide(true, server.pub, next.pub, server.pub, forged) == Result::Mismatch, "rotación con firma ajena se rechaza");

    // Rotación válida pero para otra clave nueva.
    Require(ServerTrust::Decide(true, server.pub, other.pub, server.pub, rotation) == Result::Mismatch, "la rotación no sirve para otra clave");

    if (s_Failures == 0) std::printf("ServerTrust: OK\n");
    return s_Failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
