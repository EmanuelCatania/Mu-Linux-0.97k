// Render_LegacyTransforms.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// FUN_004f9e90 @ 0x004F9E90 — EulerToMatrix(angles[3], out_mat[12])
// Converts Euler angles (in game units × π/180) to 3×4 rotation matrix.
// Row-major: out[0..2]=X-row, out[4..6]=Y-row, out[8..10]=Z-row; out[3,7,11]=0.
void __cdecl FUN_004f9e90(float *param_1, float *param_2) {
    float sz = sinf(param_1[2] * _DAT_00552ce8);
    float cz = cosf(param_1[2] * _DAT_00552ce8);
    float sy = sinf(param_1[1] * _DAT_00552ce8);
    float cy = cosf(param_1[1] * _DAT_00552ce8);
    float sx = sinf(param_1[0] * _DAT_00552ce8);
    float cx = cosf(param_1[0] * _DAT_00552ce8);
    param_2[3]  = 0.0f;
    param_2[7]  = 0.0f;
    param_2[11] = 0.0f;
    param_2[0]  = cy * cz;
    param_2[1]  = cy * sz;
    param_2[2]  = -sy;
    param_2[4]  = sx * sy * cz - cx * sz;
    param_2[5]  = cx * cz    + sx * sy * sz;
    param_2[6]  = sx * cy;
    param_2[8]  = sx * sz    + cx * sy * cz;
    param_2[9]  = cx * sy * sz - sx * cz;
    param_2[10] = cx * cy;
}

// FUN_004f9f70 @ 0x004F9F70 — Bone_CombineMatrices(parent[12], rot[12], out[12])
// 3×4 matrix multiply: out = parent × rot.
void __cdecl FUN_004f9f70(float *p, float *r, float *o) {
    o[0]  = p[2]*r[8]  + p[1]*r[4]  + p[0]*r[0];
    o[1]  = p[2]*r[9]  + p[0]*r[1]  + p[1]*r[5];
    o[2]  = p[2]*r[10] + p[0]*r[2]  + p[1]*r[6];
    o[3]  = p[2]*r[11] + p[0]*r[3]  + p[1]*r[7]  + p[3];
    o[4]  = p[4]*r[0]  + p[5]*r[4]  + p[6]*r[8];
    o[5]  = p[4]*r[1]  + p[5]*r[5]  + p[6]*r[9];
    o[6]  = p[4]*r[2]  + p[5]*r[6]  + p[6]*r[10];
    o[7]  = p[4]*r[3]  + p[5]*r[7]  + p[6]*r[11] + p[7];
    o[8]  = r[0]*p[8]  + p[9]*r[4]  + p[10]*r[8];
    o[9]  = r[1]*p[8]  + p[9]*r[5]  + p[10]*r[9];
    o[10] = r[2]*p[8]  + p[9]*r[6]  + p[10]*r[10];
    o[11] = r[3]*p[8]  + p[9]*r[7]  + p[10]*r[11] + p[11];
}

// FUN_004fa1d0 @ 0x004FA1D0 — EulerToQuat(angles[3], out_quat[4])
// Converts Euler XYZ (game angle units) to quaternion (x,y,z,w).
// Declared as (int,int,int,int) in functions.h; callers pass float* cast to int.
void __cdecl FUN_004fa1d0(int ia, int ib, int ic, int id) {
    float *param_1 = (float*)ia;
    float *param_2 = (float*)ib;
    (void)ic; (void)id;
    float k   = _DAT_00552ce0;
    float sz  = sinf(param_1[2] * k), cz = cosf(param_1[2] * k);
    float sy  = sinf(param_1[1] * k), cy = cosf(param_1[1] * k);
    float sx  = sinf(param_1[0] * k), cx = cosf(param_1[0] * k);
    float sxcy = sx * cy, cxsy = cx * sy;
    param_2[0] = (float)((double)sxcy * cz - (double)cxsy * sz);
    param_2[1] = (float)((double)sxcy * sz + (double)cxsy * cz);
    float sxsy = sx * sy;
    param_2[2] = (float)(cx * cy * sz - sxsy * cz);
    param_2[3] = (float)(cx * cy * cz + sxsy * sz);
}

// FUN_004fa270 @ 0x004FA270 — QuatToMatrix(quat[4], out_mat[12])
// Converts unit quaternion to 3×3 rotation matrix (stored in [0,1,2,4,5,6,8,9,10]).
void __cdecl FUN_004fa270(int ia, int ib, int ic, int id) {
    float *q = (float*)ia;
    float *m = (float*)ib;
    (void)ic; (void)id;
    float x2 = q[1]*q[1], z2 = q[2]*q[2];
    m[0]  = _DAT_00552cf0 - (x2+x2) - (z2+z2);
    float t = q[0]*q[1] + q[2]*q[3]; m[4]  = t+t;
    m[8]  = (q[2]*q[0]+q[2]*q[0]) - (q[3]*q[1]+q[3]*q[1]);
    m[1]  = (q[0]*q[1]+q[0]*q[1]) - (q[2]*q[3]+q[2]*q[3]);
    float y2 = q[0]*q[0];           z2 = q[2]*q[2];
    m[5]  = _DAT_00552cf0 - (y2+y2) - (z2+z2);
    t = q[0]*q[3] + q[2]*q[1];      m[9]  = t+t;
    t = q[3]*q[1] + q[2]*q[0];      m[2]  = t+t;
    m[6]  = (q[2]*q[1]+q[2]*q[1]) - (q[0]*q[3]+q[0]*q[3]);
    y2 = q[0]*q[0]; x2 = q[1]*q[1];
    m[10] = _DAT_00552cf0 - (y2+y2) - (x2+x2);
}

// FUN_004fa350 @ 0x004FA350 — QuatSlerp(q1[4], q2[4], t, out[4])
// Spherical linear interpolation between two quaternions.
void __cdecl FUN_004fa350(int ia, int ib, int ic, int id) {
    float *q1  = (float*)ia;
    float *q2  = (float*)ib;
    float  t   = *(float*)&ic;
    float *out = (float*)id;
    // ensure shortest path
    float dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
    if (dot < 0.0f) { for(int i=0;i<4;i++) q2[i]=-q2[i]; dot=-dot; }
    float scale0, scale1;
    if (dot < 1.0f - _DAT_00552d00) {
        float angle = acosf(dot);
        float sinA  = sinf(angle);
        scale0 = sinf((1.0f - t) * angle) / sinA;
        scale1 = sinf(t * angle) / sinA;
    } else {
        scale0 = 1.0f - t;
        scale1 = t;
    }
    for (int i=0;i<4;i++) out[i] = scale0*q1[i] + scale1*q2[i];
}


// FUN_004ff580 @ 0x004FF580 — Entity_InitRenderState(entity)
// Scans render-state pool at DAT_083a2370 (stride 0xc, 128 slots).
// Finds first free slot (byte[0]==0), marks it active and stores entity ptr.
// FUN_004ff580 (IDA-activated, was Ghidra stub)
void *__cdecl FUN_004ff580(void *a1)
{
  DWORD *result; // eax

  result = (DWORD *)&DAT_083a2370;
  while ( *(BYTE *)result )
  {
    result += 3;
    if ( (int)result >= (int)&DAT_083a2cd0 )
    {
      return result;
    }
  }
  *(BYTE *)result = 1;
  result[2] = (DWORD)a1;
  return result;
}

// FUN_005129f0 @ 0x005129F0 — fabs(float) → double; was lying stub returning v unchanged.
long double   __cdecl FUN_005129f0(float v)                                  { return (long double)(v >= 0.0f ? v : -v); }
// FUN_0043e570 @ 0x0043E570 — Vector_AddRotated(pos, angle_ptr, offset_ptr)
// Builds rotation matrix from angle_ptr, rotates offset_ptr through it,
// then adds the result to pos[0..2].
void __cdecl FUN_0043e570(float *param_1, float *param_2, float *param_3) {
    float out[3], mat[12];
    FUN_004f9db0(param_2, mat);
    FUN_004fa0b0(param_3, mat, out);
    param_1[0] += out[0];
    param_1[1] += out[1];
    param_1[2] += out[2];
}
