// Recv_World.cpp — paquetes del server: mundo: clima y tiles del terreno.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

// 0x46
void NetRecv_46(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Terrain tile update (Terrain_TileUpdate en Party/Party.cpp).
    // Sub-type at pkt[3]: 0x00 = rect update, 0x01 = single tile.
    NetLog("NET:  → 0x46 TerrainTileUpdate size=%d", Size);
    extern void Terrain_TileUpdate(BYTE* pkt);
    Terrain_TileUpdate((BYTE*)Msg);
}

// 0x0F
void NetRecv_0F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Port FIEL desde IDA ProtocolCore:554
    //   case 0xF:
    //     Weather = ReceiveBuffer[3];
    //     if (Weather >> 4) {
    //       if (Weather >> 4 == 1) RainTarget = 6 * (Weather & 0xF);
    //     } else RainTarget = 0;
    //
    // Server controls weather state per map. High nibble = weather
    // type (0=clear, 1=rain). Low nibble = intensity (0..15).
    if (Size >= 4) {
        BYTE w = Msg[3];
        BYTE wType = (w >> 4) & 0x0F;
        BYTE wIntensity = w & 0x0F;
        NetLog("NET:  → 0x0F Weather type=%d intensity=%d", wType, wIntensity);
        if (wType == 1) {
            // Lluvia — setea el global RainTarget si está definido
            // Per IDA: RainTarget = 6 * intensity (= 0..90)
            // Nuestro build: buscar un global similar. Por ahora no
            // tenemos RainTarget específicamente, pero Weather.cpp
            // usa el área DAT_07eaa178 para el estado del clima.
            // Por ahora logueamos y salteamos; el clima va a andar vía MoveWeather.
        }
    }
}
