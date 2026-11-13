#pragma once

// Confianza en la identidad del server (Fase 5, C3).
//
// Cada server tiene una clave de firma propia (server: GameServer/ServerIdentity).
// El cliente la fija ("pin") de una de dos formas:
//   - compilada en ServerConfig::ServerPublicKey (hex de 64 dígitos), o
//   - la del primer contacto, guardada en User/Cache/<server>/server.pub.
// Si más adelante el server presenta otra clave, sólo se acepta cuando trae una
// rotación: la clave anterior (la fijada) firmando a la nueva.  Sin eso la
// conexión no es de confianza: puede ser otro server haciéndose pasar por éste.

#include <cstddef>
#include <cstdint>
#include <string>

namespace ServerTrust {

constexpr std::size_t KeySize = 32;
constexpr std::size_t SignatureSize = 64;

enum class Result {
    Trusted,    // es la clave fijada
    FirstUse,   // no había ninguna: queda fijada ésta
    Rotated,    // el server la cambió y la anterior firmó el cambio: queda fijada la nueva
    Mismatch    // otra clave, sin rotación válida
};

// Carpeta donde se guarda la clave de este server (la de su caché); vacía = no
// se guarda nada y cada conexión cuenta como primer contacto.
void SetServer(const std::string& cacheDir);

// previousKey/rotationSignature: nullptr si el server no mandó rotación.
Result Check(const std::uint8_t publicKey[KeySize],
             const std::uint8_t* previousKey,
             const std::uint8_t* rotationSignature);

// true si `signature` es la firma de `message` con `publicKey`.
bool Verify(const std::uint8_t signature[SignatureSize],
            const std::uint8_t publicKey[KeySize],
            const std::uint8_t* message, std::size_t size);

// Pura, para las pruebas: decide con la clave fijada en la mano (hasPinned =
// false si no hay ninguna).
Result Decide(bool hasPinned, const std::uint8_t pinned[KeySize],
              const std::uint8_t publicKey[KeySize],
              const std::uint8_t* previousKey,
              const std::uint8_t* rotationSignature);

} // namespace ServerTrust
