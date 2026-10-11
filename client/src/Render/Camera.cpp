#include "stdafx.h"
#include "Item/ContentCatalog.h"
#pragma warning(disable: 4554 4244 4700)
#include "Render/Camera.h"

// External math helpers are declared in functions.h (included via stdafx.h)

// ── FUN_004F9050 @ 0x004F9050 — Camera_SetupFrustum ─────────────────────────
// Computes view frustum from camera position + FOV, then derives tile-space
// bounding boxes for the terrain chunk-cull loop, and 5 frustum half-planes.
//
// param_1: screen width ratio = Screen_GetWidth() * DAT_0055283c
// param_2: camera world position [x, y, z]
void __cdecl Camera_SetupFrustum(float param_1, float *param_2)
{
    // Min/max world-space trackers for frustum corners
    float local_7c = 25600.0f;   // min X (init to big value)
    float local_84 = 25600.0f;   // min Y
    float local_80 = 0.0f;       // max X
    float local_88 = 0.0f;       // max Y

    // Vec3_Transform output buffer.
    // VectorIRotate (Vector_InverseRotate) escribe un float[3] contiguo (en el
    // binario original: ebp-0x78/-0x74/-0x70). Tiene que ser un array real: con
    // locals sueltas MSVC no garantiza el layout y el frustum colapsa. Mismo
    // patrón que CreateTerrainNormal.
    float out3[3];

    // 5 frustum corners × 3 floats (contiguous, passed by pointer to Vector_InverseRotate)
    // Corner 0 = apex (at camera, origin in view space)
    // Corners 1–4 = near-plane corners: TL, TR, BR, BL
    float corners[15];

    float local_30[12];  // view rotation matrix from GL

    // Step 1 — FOV half-extents ───────────────────────────────────────────────
    // near_w = tan(FOV * PI/180) * (near_clip * viewscale) * screen_ratio + tile_scale
    // near_h = near_w * aspect_ratio
    long double fov_tan = tanl((long double)Ff(DAT_00561554) * (long double)_DAT_00552cc4);
    long double near_w  = fov_tan * (long double)(Ff(DAT_00561550) * _DAT_005526e8) *
                          (long double)param_1 + (long double)_DAT_005524f0;
    long double near_h  = near_w * (long double)_DAT_00552adc;
    // DESVIACION (in-game): la pirámide del binario es más angosta que la
    // vista real: usa tan(FOV/2) como semiancho sin el aspecto 4:3 y el alto
    // en 0.75 de eso.  Con el FOV de 35 alcanzaba; con 50 cortaba objetos y
    // terreno en los bordes.  Acá se arma con la proyección de verdad
    // (gluPerspective: semialto tan(FOV/2), semiancho * W/H de la ventana,
    // que en pantallas anchas no es 4:3).
    if (SceneFlag == 5) {
        const long double depth = (long double)(Ff(DAT_00561550) * _DAT_005526e8);
        near_h = fov_tan * depth + (long double)_DAT_005524f0;
        const long double aspect = (gWindow.GetHeight() > 0)
            ? (long double)gWindow.GetWidth() / (long double)gWindow.GetHeight() : (4.0L / 3.0L);
        near_w = fov_tan * depth * (long double)param_1 * aspect + (long double)_DAT_005524f0;
    }
    float near_dist     = -(Ff(DAT_00561550) * _DAT_005526e8);  // negative near depth

    // Step 2 — 5 view-space corners ───────────────────────────────────────────
    corners[0]  = 0.0f;           // apex (origin)
    corners[1]  = 0.0f;
    corners[2]  = 0.0f;
    corners[3]  = (float)-near_w; // corner 1: top-left
    corners[4]  = (float) near_h;
    corners[5]  = near_dist;
    corners[6]  = (float) near_w; // corner 2: top-right
    corners[7]  = (float) near_h;
    corners[8]  = near_dist;
    corners[9]  = (float) near_w; // corner 3: bottom-right
    corners[10] = (float)-near_h;
    corners[11] = near_dist;
    corners[12] = (float)-near_w; // corner 4: bottom-left
    corners[13] = (float)-near_h;
    corners[14] = near_dist;

    // Step 3 — Rotate to world space + track min/max ──────────────────────────
    GL_GetModelViewMatrix((unsigned int *)local_30);  // get current GL modelview rotation

    for (int i = 0; i < 5; i++)
    {
        Vector_InverseRotate(&corners[i * 3], local_30, out3);

        // World corner = camera_pos + rotated_view_corner
        float wx = _DAT_083a42d4 + out3[0];
        float wy = _DAT_083a42d8 + out3[1];
        float wz = _DAT_083a42dc + out3[2];

        // Store to global frustum corner array
        switch (i) {
        case 0: FrustrumVertex=wx; DAT_07eab1b4=wy; DAT_07eab1b8=wz; break;
        case 1: DAT_07eab1bc=wx; DAT_07eab1c0=wy; DAT_07eab1c4=wz; break;
        case 2: DAT_07eab1c8=wx; DAT_07eab1cc=wy; DAT_07eab1d0=wz; break;
        case 3: DAT_07eab1d4=wx; DAT_07eab1d8=wy; DAT_07eab1dc=wz; break;
        case 4: DAT_07eab1e0=wx; DAT_07eab1e4=wy; DAT_07eab1e8=wz; break;
        }

        if (wx < local_7c) local_7c = wx;
        if (wy < local_84) local_84 = wy;
        if (local_80 < wx) local_80 = wx;
        if (local_88 < wy) local_88 = wy;
    }

    // Step 4 — World coords → tile indices with 4-tile alignment margin ───────
    // Faithful to Ghidra: floor4(tile_index) ± 4, clamped.
    // The compiler used fchs + __ftol; equivalent: (int)(v/scale)/4*4 ± 4.
    auto floor4tile_min = [](float world, float scale) -> int {
        int t = (int)(world / scale);
        return ((t + (t >> 31 & 3)) >> 2) * 4 - 4;
    };
    auto floor4tile_max = [](float world, float scale) -> int {
        int t = (int)(world / scale);
        return ((t + (t >> 31 & 3)) >> 2) * 4 + 4;
    };

    // Near tile bounds — clamped [0, 0xFB = 251]
    int iVar2 = floor4tile_min(local_7c, _DAT_005524f0);
    FrustrumBoundMinX_1 = iVar2;
    int iVar3 = floor4tile_min(local_84, _DAT_005524f0);
    FrustrumBoundMinY_1 = iVar3;
    int iVar1 = floor4tile_max(local_80, _DAT_005524f0);
    FrustrumBoundMaxX_1 = iVar1;
    FrustrumBoundMaxY_1 = floor4tile_max(local_88, _DAT_005524f0);

    if (iVar2 < 0)              { iVar2 = 0; FrustrumBoundMinX_1 = 0; }
    if (iVar3 < 0)              { iVar3 = 0; FrustrumBoundMinY_1 = 0; }
    if (iVar1 < 0)              { iVar1 = 0; FrustrumBoundMaxX_1 = 0; }
    if ((int)FrustrumBoundMaxY_1 < 0)  FrustrumBoundMaxY_1 = 0;
    if (0xfb < iVar2)           FrustrumBoundMinX_1 = 0xfb;
    if (0xfb < iVar3)           FrustrumBoundMinY_1 = 0xfb;
    if (0xfb < iVar1)           FrustrumBoundMaxX_1 = 0xfb;
    if (0xfb < (int)FrustrumBoundMaxY_1) FrustrumBoundMaxY_1 = 0xfb;

    // Extended (far) tile bounds — clamped [0, 0xEF = 239]
    iVar2 = floor4tile_min(local_7c, _DAT_005524f0);
    FrustrumBoundMinX_2 = iVar2;
    iVar3 = floor4tile_min(local_84, _DAT_005524f0);
    FrustrumBoundMinY_2 = iVar3;
    iVar1 = floor4tile_max(local_80, _DAT_005524f0);
    FrustrumBoundMaxX_2 = iVar1;
    FrustrumBoundMaxY_2 = floor4tile_max(local_88, _DAT_005524f0);

    if (iVar2 < 0)              { iVar2 = 0; FrustrumBoundMinX_2 = 0; }
    if (iVar3 < 0)              { iVar3 = 0; FrustrumBoundMinY_2 = 0; }
    if (iVar1 < 0)              { iVar1 = 0; FrustrumBoundMaxX_2 = 0; }
    if ((int)FrustrumBoundMaxY_2 < 0)  FrustrumBoundMaxY_2 = 0;
    if (0xef < iVar2)           FrustrumBoundMinX_2 = 0xef;
    if (0xef < iVar3)           FrustrumBoundMinY_2 = 0xef;
    if (0xef < iVar1)           FrustrumBoundMaxX_2 = 0xef;
    if (0xef < (int)FrustrumBoundMaxY_2) FrustrumBoundMaxY_2 = 0xef;

    // Step 5 — 5 frustum plane normals ────────────────────────────────────────
    // Planes from: apex + 2 adjacent near corners; near plane from 3 near corners.
    float plane0[3], plane1[3], plane2[3], plane3[3], plane4[3];
    float c0[3] = { FrustrumVertex, DAT_07eab1b4, DAT_07eab1b8 };  // apex
    float c1[3] = { DAT_07eab1bc, DAT_07eab1c0, DAT_07eab1c4 };  // top-left
    float c2[3] = { DAT_07eab1c8, DAT_07eab1cc, DAT_07eab1d0 };  // top-right
    float c3[3] = { DAT_07eab1d4, DAT_07eab1d8, DAT_07eab1dc };  // bot-right
    float c4[3] = { DAT_07eab1e0, DAT_07eab1e4, DAT_07eab1e8 };  // bot-left

    // IDA Camera_SetupFrustum L169-173 — los 4 planos laterales salen del ápice
    // y el quinto es la BASE de la pirámide (el que corta por distancia):
    //     FaceNormalize(V[0], V[1], V[2], normal[0]);
    //     FaceNormalize(V[0], V[2], V[3], normal[1]);
    //     FaceNormalize(V[0], V[3], V[4], normal[2]);
    //     FaceNormalize(V[0], V[4], V[1], normal[3]);
    //     FaceNormalize(V[3], V[2], V[1], normal[4]);   <-- base, NO el ápice
    // El D del quinto plano se referencia a V[1]. Afecta a TODO lo que pasa por
    // sub_4F9590.
    Triangle_ComputeNormal(c0, c1, c2, plane0);
    Triangle_ComputeNormal(c0, c2, c3, plane1);
    Triangle_ComputeNormal(c0, c3, c4, plane2);
    Triangle_ComputeNormal(c0, c4, c1, plane3);
    Triangle_ComputeNormal(c3, c2, c1, plane4);

    // Store plane normals to globals (read by Frustum_IsVisible)
    FrustrumFaceNormal = plane0[0]; DAT_0838b7c8 = plane0[1]; DAT_0838b7cc = plane0[2];
    DAT_0838b7d0 = plane1[0]; DAT_0838b7d4 = plane1[1]; DAT_0838b7d8 = plane1[2];
    DAT_0838b7dc = plane2[0]; DAT_0838b7e0 = plane2[1]; DAT_0838b7e4 = plane2[2];
    DAT_0838b7e8 = plane3[0]; DAT_0838b7ec = plane3[1]; DAT_0838b7f0 = plane3[2];
    DAT_0838b7f4 = plane4[0]; DAT_0838b7f8 = plane4[1]; DAT_0838b7fc = plane4[2];

    // Step 6 — Plane D values: D = -(normal · reference_point) ───────────────
    // Los planos 0-3 se referencian al ápice (V[0] = c0); el 4 a V[1] = c1
    // (IDA L186-188: `FrustrumFaceD[4] = -(dot(normal[4], FrustrumVertex[1]))`).
    Ff(FrustrumFaceD) = -(plane0[0]*c0[0] + plane0[1]*c0[1] + plane0[2]*c0[2]);
    Ff(DAT_07eeb204) = -(plane1[0]*c0[0] + plane1[1]*c0[1] + plane1[2]*c0[2]);
    Ff(DAT_07eeb208) = -(plane2[0]*c0[0] + plane2[1]*c0[1] + plane2[2]*c0[2]);
    Ff(DAT_07eeb20c) = -(plane3[0]*c0[0] + plane3[1]*c0[1] + plane3[2]*c0[2]);
    Ff(DAT_07eeb210) = -(plane4[0]*c1[0] + plane4[1]*c1[1] + plane4[2]*c1[2]);

    // Step 7 — Apply camera view matrix
    CreateFrustrum2D(param_2);
}

// ── FUN_005112F0 @ 0x005112F0 — Camera_MouseRay ─────────────────────────────
// Converts screen-space mouse position to world-space ray direction.
// out_ray[0..2] = world-space ray direction vector
void __cdecl Camera_MouseRay(int mouseX, int mouseY, float *out_ray)
{
    float view_x = ((float)gWindow.GetWidth() * (float)mouseX / 0x280 - Ff(ViewportCenterX)) *
                   Ff(DAT_083a42a4) * Ff(DAT_00561550);
    float view_y = -((float)gWindow.GetHeight() * (float)mouseY / 0x1e0 - Ff(ViewportCenterY)) *
                   Ff(DAT_083a42a8) * Ff(DAT_00561550);
    float view_z = -Ff(DAT_00561550);

    // Camera forward (negated local-space)
    float cam_fwd_neg[3] = {
        -Ff(DAT_083a414c),
        -Ff(DAT_083a415c),
        -Ff(DAT_083a416c)
    };
    float view_dir[3] = { view_x, view_y, view_z };

    // Transform to world space using stored view matrix
    Vector_InverseRotate(cam_fwd_neg, (float*)&CameraMatrix, (float*)&CameraRayOriginX);
    Vector_InverseRotate(view_dir,    (float*)&CameraMatrix, out_ray);

    out_ray[0] += Ff(CameraRayOriginX);
    out_ray[1] += Ff(CameraRayOriginY);
    out_ray[2] += Ff(CameraRayOriginZ);
}

// ── Compatibility helper; no standalone IDA function ─────────────────────────
// CÓDIGO MUERTO: esta función NO se llama. Es una decompilación errónea
// que asume corners en DAT_07eab1bc..1e8 (que ya están en world coords post
// Camera_SetupFrustum). La verdadera FUN_004F8EB0 (CreateFrustrum2D) vive en
// src/Net/SecondPassword.cpp — usa 4 corners hardcoded escalados por
// GetScreenWidth(), rotados Z=45°, trasladados por cam_pos. Se mantiene por la
// declaración en Camera.h pero no debe llamarse.
void __cdecl Camera_SetMatrix(float *cam_pos)
{
    float angles[3] = { 0.0f, 0.0f, 45.0f };  // fixed roll=45°
    float rot_mat[12];

    Matrix_BuildFromEuler(angles, rot_mat);  // build rotation from euler

    // Transform the 4 near corners (skipping apex at index 0)
    float corners_world[4][3];
    float near_corners[4][3] = {
        { DAT_07eab1bc, DAT_07eab1c0, DAT_07eab1c4 },  // TL
        { DAT_07eab1c8, DAT_07eab1cc, DAT_07eab1d0 },  // TR
        { DAT_07eab1d4, DAT_07eab1d8, DAT_07eab1dc },  // BR
        { DAT_07eab1e0, DAT_07eab1e4, DAT_07eab1e8 },  // BL
    };

    for (int i = 0; i < 4; i++)
    {
        float out[3];
        Vector_Rotate(near_corners[i], rot_mat, out);
        corners_world[i][0] = out[0] + cam_pos[0];
        corners_world[i][1] = out[1] + cam_pos[1];
        corners_world[i][2] = out[2] + cam_pos[2];
    }

    // TestFrustrum2D (Frustum_IsVisible) hace test point-in-quad usando los
    // 4 vertices en FrustrumX[0..3] (X) y FrustrumY[0..3] (Y): hay que escribir
    // los 4 (loop de 4 iteraciones, stride 1 float, como el original).
    float* outX = (float*)&FrustrumX;
    float* outY = (float*)&FrustrumY;
    for (int i = 0; i < 4; i++) {
        outX[i] = corners_world[i][0] / _DAT_005524f0;  // tile-space X
        outY[i] = corners_world[i][1] / _DAT_005524f0;  // tile-space Y
    }
}


// FUN_004F9590 @ 0x004F9590 — Frustum_TestSphere
// Tests if a world-space point is inside the view frustum.
// param_1: xyz position (float[3])
// param_2: radius (frustum half-width extension)
// Returns a short: low byte 1 if inside all planes, high byte flags if outside.
// Recorre los 5 planos del frustum (normales desde FrustrumFaceNormal /
// DAT_0838b7c8.., distancias desde FrustrumFaceD). El original camina las
// normales con un puntero acotado por el literal `< 0x838b804`; acá cada
// componente es un global SEPARADO (el linker no garantiza contigüidad), así
// que se leen por nombre, desenrollado 5×. Los escribe Camera_SetupFrustum.
int __cdecl Frustum_TestSphere(float *param_1, float param_2)
{
    float fVar1 = -param_2;
    byte  bVar3 = 0;

    #define PLANE_TEST(nx, ny, nz, d_global)                                   \
    do {                                                                       \
        float fVar2 = (nx) * param_1[0] + (ny) * param_1[1] + (nz) * param_1[2]\
                    + *(float*)&(d_global);                                    \
        bool ltf = fVar2 < fVar1;                                              \
        bool eqf = fVar2 == fVar1;                                             \
        bVar3 |= (byte)(ltf ? 1 : 0)                                           \
              |  (byte)((ltf || eqf) ? 0x40 : 0);                              \
        if (ltf) return (int)bVar3 << 8;                                       \
    } while (0)

    PLANE_TEST(FrustrumFaceNormal, DAT_0838b7c8, DAT_0838b7cc, FrustrumFaceD);
    PLANE_TEST(DAT_0838b7d0, DAT_0838b7d4, DAT_0838b7d8, DAT_07eeb204);
    PLANE_TEST(DAT_0838b7dc, DAT_0838b7e0, DAT_0838b7e4, DAT_07eeb208);
    PLANE_TEST(DAT_0838b7e8, DAT_0838b7ec, DAT_0838b7f0, DAT_07eeb20c);
    PLANE_TEST(DAT_0838b7f4, DAT_0838b7f8, DAT_0838b7fc, DAT_07eeb210);

    #undef PLANE_TEST

    return ((int)bVar3 << 8) | 1;  // CONCAT11(bVar3, 1)
}


// FUN_004FA5C0 @ 0x004FA5C0 — Camera_SetViewport
// Stores viewport parameters into globals.
// param_1: x offset,  param_2: y offset
// param_3: width,     param_4: height (int → converted to float at _DAT_0055a7bc)
// Globals: DAT_0055a7b0/b4/b8/bc
// IDA: SetActionObject (0x004FA5C0)
void __cdecl SetActionObject(int param_1,int param_2,int param_3,int param_4)
{
  DAT_0055a7b4 = param_1;
  _DAT_0055a7bc = (float)param_4;
  DAT_0055a7b0 = param_2;
  DAT_0055a7b8 = param_3;
  return;
}


// FUN_005113F0 @ 0x005113F0 — Camera_ProjectWorldToScreen (World_ToScreen)
// Projects a 3D world-space point to 2D screen pixel coordinates (640×480 logical).
// param_1: world xyz (float[3])
// param_2: output screen X (int*)
// param_3: output screen Y (int*)
void __cdecl Camera_ProjectWorldToScreen(float *param_1,int *param_2,int *param_3)
{
  // Port directo del IDA Projection (sub_5113F0). Usa la matriz de cámara
  // GUARDADA en CameraMatrix[12] (poblada por GetOpenGLMatrix tras BeginOpengl)
  // en lugar de gluProject sobre el GL state actual. Esto permite proyectar
  // posiciones del mundo a coords de pantalla EVEN AFTER haber cambiado a
  // 2D ortho (necesario para los name labels del char-select que se renderan
  // tras el `glPopMatrix` y `BeginBitmap`).
  float TPos[3];
  Vector_Transform(param_1, (float*)&CameraMatrix, TPos);

  // Perspective divide (eye-space → NDC → window pixels)
  if (TPos[2] == 0.0f) { *param_2 = -1000; *param_3 = -1000; return; }
  float invZ = 1.0f / TPos[2];
  int sx = (int)ViewportCenterX - (int)(TPos[0] * invZ / _DAT_083a42a4);
  int sy = (int)ViewportCenterY + (int)(TPos[1] * invZ / _DAT_083a42a8);

  // Scale from real-window pixels to logical 640×480
  // IDA: `*sx = 640 * *sx / (int)WindowWidth;` — aritmetica CON SIGNO (el cast
  // a (int) del divisor esta justamente para eso).
  int ww = (int)gWindow.GetWidth();
  int wh = (int)gWindow.GetHeight();
  if (ww == 0) ww = 640;
  if (wh == 0) wh = 480;
  int outX = 640 * sx / ww;
  int outY = 480 * sy / wh;



  *param_2 = outX;
  *param_3 = outY;
}


// FUN_005119B0 @ 0x005119B0 — GL_BeginViewport (GL_SetupView)
// Configures the full 3D projection + modelview matrix for a sub-viewport.
// param_1/2/3/4: logical tile coordinates mapped from the 640x480 grid.
// Sets perspective, camera rotation (yaw/pitch/roll), translation, enables
// depth test, blending, fog; saves modelview matrix to CameraMatrix.
void __cdecl GL_BeginViewport(int param_1,int param_2,int param_3,int param_4)
{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;

  uVar1 = gWindow.GetWidth() * param_1;
  uVar2 = gWindow.GetHeight() * param_2;
  uVar3 = (uint)(gWindow.GetWidth() * param_3) / 0x280;
  uVar4 = (uint)(gWindow.GetHeight() * param_4) / 0x1e0;
  glMatrixMode(0x1701);
  glPushMatrix();
  glLoadIdentity();
  GL_SetViewport(uVar1 / 0x280,uVar2 / 0x1e0,uVar3,uVar4);
  // DAT_00561550 es DWORD (bit-pattern float): reinterpretar con Ff() antes de
  // multiplicar. `DAT_00561550 * float` haría int→float (0x461c4000=10000.0f
  // pasaría a 1.17e9) y daría un far plane astronómico.
  //
  // FOV y Near pasan crudos como `int` — GL_SetPerspective (GL_LegacyState.cpp)
  // los recibe como `int fov, int near_clip` y hace `Ff()` internamente.
  // Pasarles Ff() aquí causaría DOBLE reinterpretación:
  // 55.0f→int 55→Ff(55)=7.7e-44 → FOV≈0 → pantalla negra.
  // Solo el far necesita Ff() en el caller porque lo multiplicamos por _DAT_00552d34
  // (1.4f) ANTES de pasarlo — la multiplicación es en float-space aquí.
  GL_SetPerspective(DAT_00561554,(float)uVar3 / (float)uVar4,DAT_0056154c,Ff(DAT_00561550) * _DAT_00552d34);
  glMatrixMode(0x1700);
  glPushMatrix();
  glLoadIdentity();
  // Los ángulos DAT_083a42b8/bc/c0 son DWORD (bit-pattern de float): en el asm
  // original FLD los lee como float. Pasarlos directo a glRotatef haría int→float
  // (pitch=-40.0f, 0xc2200000, saldría 3.26e9°): se reinterpretan con Ff(). El
  // eje va como 1.0f literal, no como el bit pattern 0x3f800000.
  glRotatef(Ff(DAT_083a42bc), 0.0f, 1.0f, 0.0f);
  if (CameraTopViewEnabled == '\0') {
    glRotatef(Ff(DAT_083a42b8), 1.0f, 0.0f, 0.0f);
  }
  glRotatef(Ff(DAT_083a42c0), 0.0f, 0.0f, 1.0f);
  glTranslatef(-_DAT_083a42d4,-_DAT_083a42d8,-_DAT_083a42dc);
  glDisable(0xbc0);   // GL_ALPHA_TEST
  glEnable(0xde1);    // GL_TEXTURE_2D
  glEnable(0xb71);    // GL_DEPTH_TEST
  glEnable(0xb44);    // GL_CULL_FACE (matches original binary)
  glDepthMask(1);
  DAT_083a411d = 0;
  DAT_083a4125 = 1;
  DAT_083a411e = 1;
  DAT_083a411c = 1;
  DAT_083a42e8 = 1;
  glDepthFunc(0x203);
  // 0.25f literal: 0x3e800000 es el bit pattern de 0.25f y glAlphaFunc espera
  // GLclampf. Como int se convertiría a 1048576000.0f → clamp a 1.0 → el test
  // "alpha > 1.0" siempre falla y todo el UI con alpha-test queda invisible.
  glAlphaFunc(GL_GREATER, 0.25f);
  if (DAT_083a42ea != '\0' && gContentCatalog.HasMapFog()) {
    // DESVIACION (DLL MapFog glEnableFog, ahora catálogo 0.97.20): niebla
    // lineal 2000..2700 con el color del mapa.
    float fog[4];
    gContentCatalog.GetFogColor((int)gMapManager.GetCurrentMap(), fog);
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 2000.0f);
    glFogf(GL_FOG_END, 2700.0f);
    glFogfv(GL_FOG_COLOR, fog);
    GL_GetModelViewMatrix((unsigned int *)&CameraMatrix);
    return;
  }
  if (DAT_083a42ea != '\0') {
    glEnable(0xb60);
    glFogi(0xb65,0x801);
    glFogf(0xb62,DAT_00561558);
    glFogfv(0xb66,(const GLfloat*)&FogColor);
    GL_GetModelViewMatrix((unsigned int *)&CameraMatrix);
    return;
  }
  glDisable(0xb60);
  GL_GetModelViewMatrix((unsigned int *)&CameraMatrix);
  return;
}
