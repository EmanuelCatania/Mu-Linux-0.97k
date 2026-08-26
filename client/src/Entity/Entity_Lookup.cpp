// Entity_Lookup.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Entity_FindById @ 0x0045ac80 ─────────────────────────────────────────────
// Linear scan of entity array (stride 0x394, count 400) for entity_id at +476.
// Returns slot index (0-399), or 400 if not found.  Per IDA FindCharacterIndex
// (50-byte original): also requires the active flag at +0 to be non-zero, AND
// returns 400 (sentinel) on not-found — NOT 0 (slot 0 is the local player).
// 2026-05-07: fixed to match IDA — previously returned 0 on miss + ignored the
// active flag, which caused PacketHandler_0x5c writes for unknown entities to
// land on (potentially NULL or player) slot 0.
int __cdecl Entity_FindById(int entity_id) {
    BYTE* base = (BYTE*)DAT_07abf5d0;
    if (!base) return 400;
    for (int i = 0; i < 400; i++) {
        BYTE* ent = base + i * 0x394;
        if (*(BYTE*)ent != 0 && *(short*)(ent + 476) == (short)entity_id)
            return i;
    }
    return 400;
}

