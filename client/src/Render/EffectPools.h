#pragma once

// Tamaños de los pools de efectos del cliente.
//
// DESVIACION (Fase 5, paso 1c): el binario usa 200 efectos (0x07B11670),
// 500 joints (0x07B27150), 3000 partículas (0x07ABF5F0) y 1002 sprites
// (0x07C85890). Con el pool lleno el efecto nuevo directamente no se crea, y con
// muchos monstruos en pantalla (bench de 100+ Dark Phoenix) se dejaban de ver
// los ataques del jugador. Se agrandan los cuatro; los recorridos usan estas
// constantes, así que no puede volver a quedar un tamaño escrito a mano.
constexpr int kEffectPoolSize   = 1000;   // IDA: 200
constexpr int kJointPoolSize    = 2000;   // IDA: 500
constexpr int kParticlePoolSize = 6000;   // IDA: 3000
constexpr int kSpritePoolSize   = 3000;   // IDA: 1002
