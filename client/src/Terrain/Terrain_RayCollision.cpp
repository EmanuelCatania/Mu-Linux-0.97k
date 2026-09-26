// Terrain_RayCollision.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Terrain tile outline / ray-triangle intersection stubs ───────────────────
// RenderTerrainFace (RenderTerrainFace) — PORTADO 1:1 en src/Terrain/Terrain_RenderFace.cpp
// (antes era un no-op stub de 4 args; la firma real es 5 args con lodf).
// CollisionDetectLineToFace @ 0x00512D40 — CollisionDetectLineToFace (a.k.a. Ray_TriIntersect)
// BUG-FIX 2026-04-26 (audit #7): activated full IDA port.  Old stub returned 0
// always, so terrain triangle picking *never* registered a hit — click-to-move
// would only land on whatever fallback path remained.  The dormant gated port
// (formerly behind IDA_PORT_00512D40) is now the live implementation.
//   xf → DAT_083a4130  (cursor billboard screen X / picked world X)
//   yf → DAT_083a4134  (cursor billboard screen Y / picked world Y)
//   DAT_083a4120 is the running closest-hit ray parameter (float, not DWORD).
unsigned int __cdecl CollisionDetectLineToFace(float *Position, float *Target, int Polygon,
    float *v1, float *v2, float *v3, float *v4, float *Normal, char Collision)
{
    float v24 = Target[0] - Position[0];
    float v25 = Target[1] - Position[1];
    float v26 = Target[2] - Position[2];
    double v12 = v26 * Normal[2] + v25 * Normal[1] + v24 * Normal[0];
    if ( v12 >= 0.0 )
        return 0;
    float v20 = (float)(-((Normal[1] * Position[1]
                          + Normal[0] * Position[0]
                          + Normal[2] * Position[2]
                          - (v1[2] * Normal[2]
                           + v1[1] * Normal[1]
                           + v1[0] * Normal[0]))
                         / v12));
    if ( v20 < 0.0f || (double)v20 > (double)DAT_083a4120 )
        return 0;
    float Targeta   = v20 * v24 + Position[0];
    float Positiona = v20 * v25 + Position[1];
    float Normala   = v20 * v26 + Position[2];

    float v17 = (float)Math_Fabs(v26);
    float v15 = (float)Math_Fabs(v25);
    float v14 = (float)Math_Fabs(v24);
    float v16 = (float)Math_Fmin(v14, v15);
    float v21 = (float)Math_Fmin(v16, v17);

    if ( (float)Math_Fabs(v24) == v21 ) {
        if ( !((float)Math_Fmin(Position[1], Target[1]) <= Positiona
            && (float)Math_Fmax(Position[1], Target[1]) >= Positiona
            && (float)Math_Fmin(Position[2], Target[2]) <= Normala
            && (float)Math_Fmax(Position[2], Target[2]) >= Normala) )
            return 0;
    } else if ( (float)Math_Fabs(v25) == v21 ) {
        if ( (float)Math_Fmin(Position[2], Target[2]) > Normala
          || (float)Math_Fmax(Position[2], Target[2]) < Normala
          || (float)Math_Fmin(Position[0], Target[0]) > Targeta
          || (float)Math_Fmax(Position[0], Target[0]) < Targeta )
            return 0;
    } else if ( (float)Math_Fmin(Position[0], Target[0]) > Targeta
             || (float)Math_Fmax(Position[0], Target[0]) < Targeta
             || (float)Math_Fmin(Position[1], Target[1]) > Positiona
             || (float)Math_Fmax(Position[1], Target[1]) < Positiona ) {
        return 0;
    }

    float v19;
    int   v18;
    if ( Normal[0] <= -0.5f || Normal[0] >= 0.5f ) {
        v19 = Normal[0];
        v18 = 1;
    } else if ( Normal[1] <= -0.5f || Normal[1] >= 0.5f ) {
        v19 = Normal[1];
        v18 = 2;
    } else {
        v19 = Normal[2];
        v18 = 4;
    }

    if ( !Collision_PointInPolygon(Targeta, Positiona, Normala, Polygon,
                       (int)v1, (int)v2, (int)v3, (int)v4, v18, v19) )
        return 0;

    if ( Collision ) {
        DAT_083a4120 = v20;
        DAT_083a4130 = Targeta;     // xf
        DAT_083a4134 = Positiona;   // yf
        DAT_083a4138 = Normala;
    }
    return 1;
}


// ════════════════════════════════════════════════════════════════════
// IDA HEX-RAYS PORTS reference block (307 IDA-only gated functions, ~29k lines)
// moved to src/stubs_IDA_ports.cpp (B3 refactor 2026-05-07).
// All functions there are gated by IDA_PORT_xxxxxxxx macros; none are
// active in the default build.
// ════════════════════════════════════════════════════════════════════
