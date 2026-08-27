// Camera_Movement.cpp
//
// Extracted from stubs_game.cpp.  Owns the intro/login camera walk animation.
// The function comment retains its original IDA symbol/address.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
// FUN_0051E4E0 @ 0x0051E4E0 — MoveCamera; intro/login camera walk animation
// Drives camera walk paths using CameraWalk[] waypoints (6 floats each: pos+angle).
// Two modes: lerp (type 0) or linear delta (type 1).
// SceneFlag==2: random waypoint selection. Otherwise: fixed waypoint 5.
void __stdcall MoveCamera_stub(void) {
    // 0x0051E4E0 — Intro/login camera walk animation
    // CameraWalk[] = DAT_005615ec (waypoints: 6 floats per entry = pos[3] + angle[3])
    // CurrentCameraCount = DAT_005615e8, CurrentCameraNumber = DAT_083a7c3c
    // CurrentCameraWalkType = DAT_083a7c38, CameraWalkCut = DAT_083a7af4
    // CurrentCameraPosition = DAT_083a432c[3], CurrentCameraAngle = DAT_083a7ad0[3]
    // CurrentCameraWalkDelta = DAT_083a7adc[6] (pos delta[3] + angle delta[3])
    // _DAT_00552d44 = step scale, _DAT_00552a30 = lerp speed
    // SceneFlag = DAT_07e11980, CameraFOV = DAT_00561554
    // CameraAngle = float[3] at 0x083a42b8, CameraPosition = float[3] at 0x083a42d4

    // BUG-FIX 2026-04-21: usar los arrays concretos en vez de `(float*)&DAT_*`
    // los DAT_* son referencias (float&/DWORD&) y tomar `&` sobre una referencia
    // no siempre da el address que uno espera con MSVC/extern. CurrentCameraAngle
    // quedaba en (0,0,0) aunque el init block corriera → CameraPitch nunca llegaba
    // a -80° → escena se veía sin pitch ("volteada").
    float* CamWalk = CameraWalk_005615ec;        // concrete float[36]
    float* CurPos = CurrentCameraPosition;       // concrete float[3]
    float* CurAngle = CurrentCameraAngle;        // concrete float[3]
    float* CurDelta = DAT_083a7adc;              // CurrentCameraWalkDelta[6]
    DWORD walkType = DAT_083a7c38;
    int camNum = (int)DAT_083a7c3c;

    // First frame init: count==-1
    if ((int)DAT_005615e8 == -1) {
        CurAngle[0] = CamWalk[3];
        CurAngle[1] = CamWalk[4];
        CurAngle[2] = CamWalk[5];
        CurPos[0] = CamWalk[0];
        CurPos[1] = CamWalk[1];
        CurPos[2] = CamWalk[2];
        camNum = 1;
        walkType = 1;
        DAT_083a7c3c = 1;
        DAT_083a7c38 = 1;
        for (int i = 0; i < 3; i++) {
            CurDelta[i]     = (CamWalk[6 + i] - CurPos[i]) * _DAT_00552d44;
            CurDelta[i + 3] = (CamWalk[9 + i] - CurAngle[i]) * _DAT_00552d44;
        }
        {
            char m[200];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "MoveCamera INIT: CamWalk[0..5]=(%.1f,%.1f,%.1f,%.1f,%.1f,%.1f) CurAngle=(%.1f,%.1f,%.1f)",
                CamWalk[0],CamWalk[1],CamWalk[2],CamWalk[3],CamWalk[4],CamWalk[5],
                CurAngle[0],CurAngle[1],CurAngle[2]);
            DbgLogPublic(m);
        }
    }

    DAT_005615e8 = DAT_005615e8 + 1;

    // Check if we should advance to next waypoint
    bool advance = false;
    if (DAT_083a7af4 == 0) {
        if (DAT_005615e8 >= 0x28) advance = true;
    } else if (DAT_083a7af4 >= 1 && DAT_005615e8 >= 0x80) {
        advance = true;
    }

    if (advance) {
        DAT_005615e8 = 0;
        if (DAT_083a7af4 == 0) {
            DAT_083a7af4 = 1;
        } else if (DAT_005615c0 == 2) {
            // BUG-FIX: el decompile original usaba DAT_07e11980 (una variable
            // que NO existe como xref en el binario; siempre 0). La instrucción
            // real en PE @ 0x0051E5D5 es `CMP [0x005615c0], 2` → g_GameState.
            // Con la variable equivocada, la rama siempre caía al else y
            // elegía wp5 (200,-800,300, roll=-10°) → la cámara saltaba de
            // golpe a posición angulada tras ~128 frames (~3.2s).
            // SceneFlag==2 (login): random waypoint 1..4, random walk type 0..1
            unsigned int r = rand();
            unsigned int rMod = r & 0x80000003;
            if ((int)rMod < 0) rMod = (rMod - 1 | 0xfffffffc) + 1;
            camNum = rMod + 1;
            DAT_083a7c3c = camNum;
            r = rand();
            walkType = r & 0x80000001;
            if ((int)walkType < 0) walkType = (walkType - 1 | 0xfffffffe) + 1;
            DAT_083a7c38 = walkType;
        } else {
            // Other scenes: fixed waypoint 5, lerp mode
            walkType = 0;
            camNum = 5;
            DAT_083a7c3c = 5;
            DAT_083a7c38 = 0;
        }
        // Recompute deltas for new target waypoint
        float* target = CamWalk + camNum * 6;
        for (int i = 0; i < 3; i++) {
            CurDelta[i]     = (target[i] - CurPos[i]) * _DAT_00552d44;
            CurDelta[i + 3] = (target[i + 3] - CurAngle[i]) * _DAT_00552d44;
        }
    }

    // Update camera position based on walk type
    if (walkType == 0) {
        // Lerp mode: smooth approach to target
        float* target = CamWalk + camNum * 6;
        for (int i = 0; i < 3; i++) {
            CurPos[i]   = (target[i] - CurPos[i]) * _DAT_00552a30 + CurPos[i];
            CurAngle[i] = (target[i + 3] - CurAngle[i]) * _DAT_00552a30 + CurAngle[i];
        }
    } else {
        // Linear delta mode: add delta each frame (only X and Y)
        for (int i = 0; i < 2; i++) {
            CurPos[i] = CurDelta[i] + CurPos[i];
        }
    }

    // Apply to actual camera: build rotation matrix from angle, transform position
    CameraFOV = 45.0f;
    CameraAngle[0] = 0.0f;
    CameraAngle[1] = 0.0f;
    CameraAngle[2] = CurAngle[2];

    float angles[3] = { CameraAngle[0], CameraAngle[1], CameraAngle[2] };
    float matrix[3][4];
    AngleMatrix(angles, matrix);

    float outPos[3];
    VectorIRotate(CurPos, matrix, outPos);

    CameraPosition[0] = outPos[0];
    CameraPosition[1] = outPos[1];
    CameraPosition[2] = outPos[2];
    CameraAngle[0] = CurAngle[0];

    // DIAG: log camera state every ~20 frames to trace abrupt jumps
    {
        static int s_f = 0;
        if ((s_f % 20) == 0) {
            char m[240];
            _snprintf_s(m, sizeof(m), _TRUNCATE,
                "Cam#%d wpN=%d wT=%d advCnt=%d cur=(%.1f,%.1f,%.1f) ang=(%.1f,%.1f,%.1f) Tgt=wp[%d]=(%.1f,%.1f,%.1f) Delta=(%.3f,%.3f,%.3f) | CamPos=(%.1f,%.1f,%.1f) CamAng=(%.1f,%.1f,%.1f) step=%f lerp=%f",
                s_f, (int)DAT_083a7c3c, (int)DAT_083a7c38, (int)DAT_005615e8,
                CurPos[0], CurPos[1], CurPos[2],
                CurAngle[0], CurAngle[1], CurAngle[2],
                (int)DAT_083a7c3c,
                CamWalk[(int)DAT_083a7c3c*6+0], CamWalk[(int)DAT_083a7c3c*6+1], CamWalk[(int)DAT_083a7c3c*6+2],
                CurDelta[0], CurDelta[1], CurDelta[2],
                CameraPosition[0], CameraPosition[1], CameraPosition[2],
                CameraAngle[0], CameraAngle[1], CameraAngle[2],
                Ff(_DAT_00552d44), Ff(_DAT_00552a30));
            DbgLogPublic(m);
        }
        s_f++;
    }
}
