// Parte pura de ServerTrust (Fase 5, C3): verificación de firmas y decisión de
// confianza.  Sin stdafx.h ni plataforma, para compilarla tal cual en las pruebas
// (tests/ServerTrust.cpp).

#include "net/ServerTrust.h"
#include <cstring>

extern "C" {
#include "net/crypto/monocypher.h"
}

namespace ServerTrust {

namespace {
// Igual que SERVER_IDENTITY_ROTATION_CONTEXT del server.
const char kRotationContext[] = "MuIdentityRotation1";
}

bool Verify(const std::uint8_t signature[SignatureSize],
            const std::uint8_t publicKey[KeySize],
            const std::uint8_t* message, std::size_t size)
{
    return crypto_eddsa_check(signature, publicKey, message, size) == 0;
}

Result Decide(bool hasPinned, const std::uint8_t pinned[KeySize],
              const std::uint8_t publicKey[KeySize],
              const std::uint8_t* previousKey,
              const std::uint8_t* rotationSignature)
{
    if (!hasPinned) return Result::FirstUse;
    if (crypto_verify32(pinned, publicKey) == 0) return Result::Trusted;
    // Otra clave: sólo vale si la fijada es la "anterior" y firmó a la nueva.
    if (!previousKey || !rotationSignature) return Result::Mismatch;
    if (crypto_verify32(pinned, previousKey) != 0) return Result::Mismatch;
    std::uint8_t message[sizeof(kRotationContext) - 1 + KeySize];
    std::memcpy(message, kRotationContext, sizeof(kRotationContext) - 1);
    std::memcpy(message + sizeof(kRotationContext) - 1, publicKey, KeySize);
    return Verify(rotationSignature, previousKey, message, sizeof(message)) ? Result::Rotated : Result::Mismatch;
}

} // namespace ServerTrust
