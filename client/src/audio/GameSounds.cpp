// IDA: OpenSounds @ 0x0050F030 — Sound_LoadGameSamples
//
// Precarga los sonidos globales del juego. La lista (id, archivo, canales,
// 3D) está en la sección [sound] de Content/audio/sound/sounds.toml; al
// lado de cada línea queda el nombre coreano del binario.
//
// Los archivos del pack están renombrados al inglés (pWalk(Soil).wav en
// lugar de p걷기(땅).wav), igual que los .mp3. La tabla se reconstruyó
// parseando OpenSounds del `.text` del binario original
// (MD5 eb95ac0785e40a7ad60c9ddb5d8bef34).
//
// NOTA: el orden 82..85 -> sKnightSkill1..4 es la correspondencia secuencial
// entre los 4 ataques de guerrero del binario (내려찍기 / 찌르기 / 올려치기 /
// 돌려치기) y los 4 archivos renombrados.  Lo respalda que el Power Slash
// (case 0x38 de Skills_PacketHandler) usa el id 85 = 돌려치기 = golpe giratorio.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "audio/SoundTable.h"

// IDA: OpenSounds
void __cdecl Sound_LoadGameSamples(void)
{
    SoundTable_LoadStartup();
}
