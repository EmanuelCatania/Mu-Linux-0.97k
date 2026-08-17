#include "stdafx.h"
#pragma warning(disable: 4554 4244 4700)
#include "Render/Camera.h"

// External math helpers are declared in functions.h (included via stdafx.h)

// ── Camera_SetupFrustum @ 0x004F9050 ─────────────────────────────────────────
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
    // BUG-FIX 2026-06-29: antes eran 3 locals SEPARADAS (`float local_78,
    // local_74, local_70;`).  VectorIRotate (FUN_004fa110) escribe un float[3]
    // desde &local_78 asumiendo contigüidad (en el binario original están en
    // ebp-0x78/-0x74/-0x70, contiguas).  MSVC no garantiza ese layout con vars
    // sueltas → out[1]/out[2] caían en stack equivocado y wy/wz leían basura
    // constante → TODOS los corners del frustum con el mismo Y (frustum colapsado
    // a una línea → bound no contenía al héroe → terreno negro).  Mismo bug que
    // CreateTerrainNormal.  Fix: array contiguo real.
    float out3[3];

    // 5 frustum corners × 3 floats (contiguous, passed by pointer to FUN_004fa110)
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
    FUN_005111d0((unsigned int *)local_30);  // get current GL modelview rotation

    for (int i = 0; i < 5; i++)
    {
        FUN_004fa110(&corners[i * 3], local_30, out3);

        // World corner = camera_pos + rotated_view_corner
        float wx = _DAT_083a42d4 + out3[0];
        float wy = _DAT_083a42d8 + out3[1];
        float wz = _DAT_083a42dc + out3[2];

        // Store to global frustum corner array
        switch (i) {
        case 0: DAT_07eab1b0=wx; DAT_07eab1b4=wy; DAT_07eab1b8=wz; break;
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
    DAT_0839bc90 = iVar2;
    int iVar3 = floor4tile_min(local_84, _DAT_005524f0);
    DAT_0839bc94 = iVar3;
    int iVar1 = floor4tile_max(local_80, _DAT_005524f0);
    DAT_0055a774 = iVar1;
    DAT_0055a778 = floor4tile_max(local_88, _DAT_005524f0);

    if (iVar2 < 0)              { iVar2 = 0; DAT_0839bc90 = 0; }
    if (iVar3 < 0)              { iVar3 = 0; DAT_0839bc94 = 0; }
    if (iVar1 < 0)              { iVar1 = 0; DAT_0055a774 = 0; }
    if ((int)DAT_0055a778 < 0)  DAT_0055a778 = 0;
    if (0xfb < iVar2)           DAT_0839bc90 = 0xfb;
    if (0xfb < iVar3)           DAT_0839bc94 = 0xfb;
    if (0xfb < iVar1)           DAT_0055a774 = 0xfb;
    if (0xfb < (int)DAT_0055a778) DAT_0055a778 = 0xfb;

    // Extended (far) tile bounds — clamped [0, 0xEF = 239]
    iVar2 = floor4tile_min(local_7c, _DAT_005524f0);
    DAT_0839bc98 = iVar2;
    iVar3 = floor4tile_min(local_84, _DAT_005524f0);
    DAT_0839bc9c = iVar3;
    iVar1 = floor4tile_max(local_80, _DAT_005524f0);
    DAT_0055a77c = iVar1;
    DAT_0055a780 = floor4tile_max(local_88, _DAT_005524f0);

    if (iVar2 < 0)              { iVar2 = 0; DAT_0839bc98 = 0; }
    if (iVar3 < 0)              { iVar3 = 0; DAT_0839bc9c = 0; }
    if (iVar1 < 0)              { iVar1 = 0; DAT_0055a77c = 0; }
    if ((int)DAT_0055a780 < 0)  DAT_0055a780 = 0;
    if (0xef < iVar2)           DAT_0839bc98 = 0xef;
    if (0xef < iVar3)           DAT_0839bc9c = 0xef;
    if (0xef < iVar1)           DAT_0055a77c = 0xef;
    if (0xef < (int)DAT_0055a780) DAT_0055a780 = 0xef;

    // Step 5 — 5 frustum plane normals ────────────────────────────────────────
    // Planes from: apex + 2 adjacent near corners; near plane from 3 near corners.
    float plane0[3], plane1[3], plane2[3], plane3[3], plane4[3];
    float c0[3] = { DAT_07eab1b0, DAT_07eab1b4, DAT_07eab1b8 };  // apex
    float c1[3] = { DAT_07eab1bc, DAT_07eab1c0, DAT_07eab1c4 };  // top-left
    float c2[3] = { DAT_07eab1c8, DAT_07eab1cc, DAT_07eab1d0 };  // top-right
    float c3[3] = { DAT_07eab1d4, DAT_07eab1d8, DAT_07eab1dc };  // bot-right
    float c4[3] = { DAT_07eab1e0, DAT_07eab1e4, DAT_07eab1e8 };  // bot-left

    FUN_004fa4d0(c0, c1, c2, plane0);  // right plane:  apex, TL, TR
    FUN_004fa4d0(c0, c2, c3, plane1);  // bottom plane: apex, TR, BR
    FUN_004fa4d0(c0, c3, c4, plane2);  // top plane:    apex, BR, BL
    FUN_004fa4d0(c0, c4, c1, plane3);  // left plane:   apex, BL, TL
    FUN_004fa4d0(c2, c1, c0, plane4);  // near plane:   TR, TL, apex (inward normal)

    // Store plane normals to globals (read by Frustum_IsVisible)
    DAT_0838b7c4 = plane0[0]; DAT_0838b7c8 = plane0[1]; DAT_0838b7cc = plane0[2];
    DAT_0838b7d0 = plane1[0]; DAT_0838b7d4 = plane1[1]; DAT_0838b7d8 = plane1[2];
    DAT_0838b7dc = plane2[0]; DAT_0838b7e0 = plane2[1]; DAT_0838b7e4 = plane2[2];
    DAT_0838b7e8 = plane3[0]; DAT_0838b7ec = plane3[1]; DAT_0838b7f0 = plane3[2];
    DAT_0838b7f4 = plane4[0]; DAT_0838b7f8 = plane4[1]; DAT_0838b7fc = plane4[2];

    // Step 6 — Plane D values: D = -(normal · reference_point) ───────────────
    // Planes 0–3 use the apex (c0); plane 4 (near) uses c2 (top-right)
    Ff(DAT_07eeb200) = -(plane0[0]*c0[0] + plane0[1]*c0[1] + plane0[2]*c0[2]);
    Ff(DAT_07eeb204) = -(plane1[0]*c0[0] + plane1[1]*c0[1] + plane1[2]*c0[2]);
    Ff(DAT_07eeb208) = -(plane2[0]*c0[0] + plane2[1]*c0[1] + plane2[2]*c0[2]);
    Ff(DAT_07eeb20c) = -(plane3[0]*c0[0] + plane3[1]*c0[1] + plane3[2]*c0[2]);
    Ff(DAT_07eeb210) = -(plane4[0]*c2[0] + plane4[1]*c2[1] + plane4[2]*c2[2]);

    // Step 7 — Apply camera view matrix
    FUN_004f8eb0(param_2);
}

// ── Camera_MouseRay @ 0x005112F0 ──────────────────────────────────────────────
// Converts screen-space mouse position to world-space ray direction.
// out_ray[0..2] = world-space ray direction vector
void __cdecl Camera_MouseRay(int mouseX, int mouseY, float *out_ray)
{
    float view_x = (Ff(DAT_0056156c) * (float)mouseX / 0x280 - Ff(DAT_083a429c)) *
                   Ff(DAT_083a42a4) * Ff(DAT_00561550);
    float view_y = -(Ff(DAT_00561570) * (float)mouseY / 0x1e0 - Ff(DAT_083a42a0)) *
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
    FUN_004fa110(cam_fwd_neg, (float*)&DAT_083a4140, (float*)&DAT_083a4284);
    FUN_004fa110(view_dir,    (float*)&DAT_083a4140, out_ray);

    out_ray[0] += Ff(DAT_083a4284);
    out_ray[1] += Ff(DAT_083a4288);
    out_ray[2] += Ff(DAT_083a428c);
}

// ── Camera_SetMatrix @ 0x004F8EB0 ────────────────────────────────────────────
// DEAD CODE 2026-05-04: esta función NO se llama. Es una decompilación errónea
// que asume corners en DAT_07eab1bc..1e8 (que ya están en world coords post
// Camera_SetupFrustum). La verdadera FUN_004f8eb0 (CreateFrustrum2D) vive en
// stubs.cpp:9127 — usa 4 corners hardcoded escalados por GetScreenWidth(),
// rotados Z=45°, trasladados por cam_pos. Mantenida por compatibilidad
// histórica del header pero no debe llamarse.
void __cdecl Camera_SetMatrix(float *cam_pos)
{
    float angles[3] = { 0.0f, 0.0f, 45.0f };  // fixed roll=45°
    float rot_mat[12];

    FUN_004f9db0(angles, rot_mat);  // build rotation from euler

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
        FUN_004fa0b0(near_corners[i], rot_mat, out);
        corners_world[i][0] = out[0] + cam_pos[0];
        corners_world[i][1] = out[1] + cam_pos[1];
        corners_world[i][2] = out[2] + cam_pos[2];
    }

    // BUG-FIX 2026-05-01: el código previo SOLO escribía corner[0]. Pero
    // FUN_004f8ff0 (Frustum_IsVisible) hace test point-in-quad usando los
    // 4 vertices en DAT_07eeb228[0..3] (X) y DAT_07eeb218[0..3] (Y).
    // Con 3/4 vertices en (0,0), el quad degenerado rechazaba TODOS los
    // chunks → mapa renderizaba vacío de objetos pese a que se spawn 2142.
    //
    // Per ghidra_backup line 6127-6128: loop 4 iterations, j stride 4 bytes
    // (= 1 float), escribiendo 4 vertices contiguos en cada array.
    float* outX = (float*)&DAT_07eeb228;
    float* outY = (float*)&DAT_07eeb218;
    for (int i = 0; i < 4; i++) {
        outX[i] = corners_world[i][0] / _DAT_005524f0;  // tile-space X
        outY[i] = corners_world[i][1] / _DAT_005524f0;  // tile-space Y
    }
}


// FUN_004f9590 — Frustum_TestPoint
// Tests if a world-space point is inside the view frustum.
// param_1: xyz position (float[3])
// param_2: radius (frustum half-width extension)
// Returns a short: low byte 1 if inside all planes, high byte flags if outside.
// Iterates 6 frustum planes stored at DAT_0838b7c8 (normal[3] stride=3) +
// corresponding plane-distances at DAT_07eeb200.
// 2026-05-03: AUTO-SKIP removed. The original Ghidra walked five plane normals
// at &DAT_0838b7c8 (= plane[0].Y) bound by literal `< 0x838b804`. In our build
// each plane component is a SEPARATE global (DAT_0838b7c4..7fc, 15 floats) —
// the linker may not place them contiguously, so the pointer walk would read
// random memory between plane components. Camera_SetupFrustum (Camera.cpp:148)
// writes all 5 planes; here we read them by name. Unrolled 5×.
int __cdecl FUN_004f9590(float *param_1, float param_2)
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

    PLANE_TEST(DAT_0838b7c4, DAT_0838b7c8, DAT_0838b7cc, DAT_07eeb200);
    PLANE_TEST(DAT_0838b7d0, DAT_0838b7d4, DAT_0838b7d8, DAT_07eeb204);
    PLANE_TEST(DAT_0838b7dc, DAT_0838b7e0, DAT_0838b7e4, DAT_07eeb208);
    PLANE_TEST(DAT_0838b7e8, DAT_0838b7ec, DAT_0838b7f0, DAT_07eeb20c);
    PLANE_TEST(DAT_0838b7f4, DAT_0838b7f8, DAT_0838b7fc, DAT_07eeb210);

    #undef PLANE_TEST

    return ((int)bVar3 << 8) | 1;  // CONCAT11(bVar3, 1)
}


// FUN_004fa5c0 — Camera_SetViewport
// Stores viewport parameters into globals.
// param_1: x offset,  param_2: y offset
// param_3: width,     param_4: height (int → converted to float at _DAT_0055a7bc)
// Globals: DAT_0055a7b0/b4/b8/bc
void __cdecl FUN_004fa5c0(int param_1,int param_2,int param_3,int param_4)
{
  DAT_0055a7b4 = param_1;
  _DAT_0055a7bc = (float)param_4;
  DAT_0055a7b0 = param_2;
  DAT_0055a7b8 = param_3;
  return;
}


// FUN_005113f0 — World_ToScreen
// Projects a 3D world-space point to 2D screen pixel coordinates (640×480 logical).
// param_1: world xyz (float[3])
// param_2: output screen X (int*)
// param_3: output screen Y (int*)
//
// ── BUG-FIX 2026-04-26 ────────────────────────────────────────────────────────
// El decompile original tenía:
//   FUN_004fa170(param_1, mat, local_c);
//   lVar1 = __ftol();   *param_2 = DAT_083a429c - lVar1;
//   lVar1 = __ftol();   *param_3 = lVar1 + DAT_083a42a0;
// Ghidra perdió la aritmética FPU entre la transformación y __ftol — el código
// original calculaba perspective divide (x_view*scale/z_view) antes de truncar.
// Como `__ftol` aquí está stubbed a `GetTickCount()` (stdafx.h), el resultado
// era basura y rompía: name labels en char-select, hit-test del mouse, todo lo
// que dependa de proyección mundo→pantalla.
// Solución: usar gluProject con el GL state actual. Más robusto que recrear
// la perspective math; respeta cualquier viewport/projection set por
// FUN_005119b0.
extern "C" void DbgLogPublic(const char*);
void __cdecl FUN_005113f0(float *param_1,int *param_2,int *param_3)
{
  // Port directo del IDA Projection (sub_5113F0). Usa la matriz de cámara
  // GUARDADA en DAT_083a4140[12] (poblada por GetOpenGLMatrix tras BeginOpengl)
  // en lugar de gluProject sobre el GL state actual. Esto permite proyectar
  // posiciones del mundo a coords de pantalla EVEN AFTER haber cambiado a
  // 2D ortho (necesario para los name labels del char-select que se renderan
  // tras el `glPopMatrix` y `BeginBitmap`).
  float TPos[3];
  FUN_004fa170(param_1, (float*)&DAT_083a4140, TPos);

  // ── DIAG: log first few projections per second to verify matrix/globals ─
  static DWORD s_lastProj = 0;
  static int   s_projCount = 0;
  DWORD now = GetTickCount();
  if (now - s_lastProj > 1000) {
      s_lastProj = now;
      s_projCount = 0;
  }
  if (s_projCount < 3) {
      s_projCount++;
      float* M = (float*)&DAT_083a4140;
      char b[400];
      _snprintf_s(b, sizeof(b), _TRUNCATE,
          "PROJ in=(%.1f,%.1f,%.1f) TPos=(%.2f,%.2f,%.2f) "
          "M[0..2]=(%.3f,%.3f,%.3f,%.3f / %.3f,%.3f,%.3f,%.3f / %.3f,%.3f,%.3f,%.3f) "
          "ScrCtr=(%d,%d) Persp=(%.6f,%.6f) Win=(%d,%d)",
          param_1[0], param_1[1], param_1[2],
          TPos[0], TPos[1], TPos[2],
          M[0],M[1],M[2],M[3], M[4],M[5],M[6],M[7], M[8],M[9],M[10],M[11],
          (int)DAT_083a429c, (int)DAT_083a42a0,
          _DAT_083a42a4, _DAT_083a42a8,
          (int)DAT_0056156c, (int)DAT_00561570);
      DbgLogPublic(b);
  }

  // Perspective divide (eye-space → NDC → window pixels)
  if (TPos[2] == 0.0f) { *param_2 = -1000; *param_3 = -1000; return; }
  float invZ = 1.0f / TPos[2];
  int sx = (int)DAT_083a429c - (int)(TPos[0] * invZ / _DAT_083a42a4);
  int sy = (int)DAT_083a42a0 + (int)(TPos[1] * invZ / _DAT_083a42a8);

  // Scale from real-window pixels to logical 640×480
  unsigned int ww = (unsigned int)DAT_0056156c;
  unsigned int wh = (unsigned int)DAT_00561570;
  if (ww == 0) ww = 640;
  if (wh == 0) wh = 480;
  int outX = (int)(640u * (unsigned int)sx / ww);
  int outY = (int)(480u * (unsigned int)sy / wh);

  // ── DIAG part 2: log intermediate sx/sy and final out, gated by same throttle
  static DWORD s_lastProj2 = 0;
  static int   s_projCount2 = 0;
  DWORD now2 = GetTickCount();
  if (now2 - s_lastProj2 > 1000) { s_lastProj2 = now2; s_projCount2 = 0; }
  if (s_projCount2 < 3) {
      s_projCount2++;
      char b[200];
      _snprintf_s(b, sizeof(b), _TRUNCATE,
          "PROJ2 invZ=%.6f sx=%d sy=%d ww=%u wh=%u → out=(%d,%d)",
          invZ, sx, sy, ww, wh, outX, outY);
      DbgLogPublic(b);
  }

  *param_2 = outX;
  *param_3 = outY;
}


// FUN_005119b0 — GL_SetupView
// Configures the full 3D projection + modelview matrix for a sub-viewport.
// param_1/2/3/4: logical tile coordinates mapped from the 640x480 grid.
// Sets perspective, camera rotation (yaw/pitch/roll), translation, enables
// depth test, blending, fog; saves modelview matrix to DAT_083a4140.
extern "C" { void DbgLogPublic(const char* msg); }
void __cdecl FUN_005119b0(int param_1,int param_2,int param_3,int param_4)
{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;

  uVar1 = DAT_0056156c * param_1;
  uVar2 = DAT_00561570 * param_2;
  uVar3 = (uint)(DAT_0056156c * param_3) / 0x280;
  uVar4 = (uint)(DAT_00561570 * param_4) / 0x1e0;
  glMatrixMode(0x1701);
  glPushMatrix();
  glLoadIdentity();
  FUN_00511910(uVar1 / 0x280,uVar2 / 0x1e0,uVar3,uVar4);
  // BUG-FIX: DAT_00561550 es DWORD (bit-pattern float). En original asm el FLD
  // lee como float. En C++ `DAT_00561550 * float` hace int→float (convierte el
  // bit-pattern 0x461c4000=10000.0f a 1.17e9), dando far plane astronómico.
  // Reinterpretar con Ff() antes de multiplicar.
  //
  // NOTA (2026-04-21): FOV y Near pasan crudos como `int` — FUN_00511220 los
  // recibe como `int fov, int near_clip` y hace `Ff()` internamente (ver
  // stubs.cpp:2386-2388). Pasarles Ff() aquí causaría DOBLE reinterpretación:
  // 55.0f→int 55→Ff(55)=7.7e-44 → FOV≈0 → pantalla negra.
  // Solo el far necesita Ff() en el caller porque lo multiplicamos por _DAT_00552d34
  // (1.4f) ANTES de pasarlo — la multiplicación es en float-space aquí.
  FUN_00511220(DAT_00561554,(float)uVar3 / (float)uVar4,DAT_0056154c,Ff(DAT_00561550) * _DAT_00552d34);
  glMatrixMode(0x1700);
  glPushMatrix();
  glLoadIdentity();
  // BUG-FIX: 0x3f800000 es el bit pattern de 1.0f pero glRotatef espera GLfloat.
  // La conversion int→float daba 1065353216.0f (inofensivo porque glRotatef
  // normaliza el eje, pero ilegible). Usar 1.0f literal.
  // BUG-FIX 2: los ángulos DAT_083a42b8/bc/c0 son DWORD (bit-pattern de float).
  // En original asm FLD los lee como float. En C++, pasar DWORD→GLfloat hace
  // int→float: con pitch=-40.0f (bitpattern 0xc2200000), el valor pasado era
  // 3.26e9° (equivalente a ruido aleatorio tras glu). Reinterpretar con Ff().
  glRotatef(Ff(DAT_083a42bc), 0.0f, 1.0f, 0.0f);
  if (DAT_083a42e9 == '\0') {
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
  // BUG-FIX CRITICO: 0x3e800000 es bit pattern de 0.25f, glAlphaFunc espera
  // GLclampf. Como int se convierte a 1048576000.0f → clamp a 1.0 → test
  // "alpha > 1.0" siempre falla → TODO el UI con alpha-test activo era
  // invisible (login screen quedaba sin server list, botones, texto).
  glAlphaFunc(GL_GREATER, 0.25f);
  if (DAT_083a42ea != '\0') {
    glEnable(0xb60);
    glFogi(0xb65,0x801);
    glFogf(0xb62,DAT_00561558);
    glFogfv(0xb66,(const GLfloat*)&DAT_0056155c);
    FUN_005111d0((unsigned int *)&DAT_083a4140);
    return;
  }
  glDisable(0xb60);
  FUN_005111d0((unsigned int *)&DAT_083a4140);
  return;
}
