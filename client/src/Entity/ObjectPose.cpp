#include "stdafx.h"
#include "Entity/ObjectPose.h"
#include "Entity/ObjectBounds.h"
#include "functions.h"
#include "globals.h"

// IDA: preparación de pose de Calc_RenderObject (0x004FAA70).
// El destino explícito permite preparar huesos SIM sin utilizar el buffer
// de la entidad que el render puede sobrescribir con una pose interpolada.
void ObjectPose_PrepareBones(int entity, void* model, float* bones, char translate)
{
    *(DWORD*)((int)model + 0x68) = *(DWORD *)&AsObject(entity)->Scale;
    *(BYTE *)((int)model + 0xa0) = AsObject(entity)->CurrentAction;
    *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
    *(DWORD*)((int)model + 0x70) = *(DWORD *)&AsObject(entity)->Position[1];
    *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    // Desplazamiento Y especial para la elfa (0x14B) y su variante (0x14A).
    short cls = AsObject(entity)->Type;
    if (cls == 0x14b) {
        *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)((int)model + 0x70) = AsObject(entity)->Position[1] + _DAT_0055290c;
        *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    } else if (cls == 0x14a) {
        *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)((int)model + 0x70) = AsObject(entity)->Position[1] + _DAT_00552908;
        *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    }
    DWORD priorFrame;
    memcpy(&priorFrame, &AsObject(entity)->PriorAnimationFrame, sizeof(priorFrame));
    BMD_Animation(model, (int)bones, AsObject(entity)->AnimationFrame, priorFrame,
                  AsObject(entity)->PriorAction, (unsigned int*)(entity + 0x1c),
                  (float*)(entity + 0x28), '\0', translate == '\0');
}

// La caja normal no depende de mallas ni huesos. Sólo el editor 2 necesita
// transformar la geometría para obtener sus extremos; LOD 0 no dibuja.
void ObjectPose_PrepareBounds(int entity, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (o.Alpha < _DAT_005524f8) return;
    if (DAT_07e11d30 == 2) {
        Calc_RenderObject(entity, translate, 0);
        return;
    }
    float origin[3] = { o.Position[0], o.Position[1], o.Position[2] };
    if (o.Type == 0x14b) origin[1] += _DAT_0055290c;
    else if (o.Type == 0x14a) origin[1] += _DAT_00552908;
    ObjectBounds_Build(o.BoundingBoxMin, o.BoundingBoxMax, origin, o.OBB);
}

void ObjectPose_PrepareSimulation(int entity, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (o.Alpha < _DAT_005524f8) return;
    if (DAT_07e11d30 == 2) {
        Calc_RenderObject(entity, translate, 0);
        return;
    }
    void *model = (void *)(DAT_05828d58 + o.Type * 0xbc);
    *(float *)((char *)model + 0x84) = 0.0f;
    float *bones = o.EnableBoneMatrix ? (float *)o.BoneMatrices : (float *)&DAT_06970a9c;
    ObjectPose_PrepareBones(entity, model, bones, translate);
    ObjectPose_PrepareBounds(entity, translate);
}

// IDA: BMD_Animation (0x00440060)
// Computes animated bone transforms for the current frame.
// param_1 = bone transform array (param_1[bone * 0x30] = 3x4 matrix per bone)
// param_2 = anim speed (float), param_3 = RGBA color, param_4 = current anim index,
// param_5 = RGBA ptr (root-bone position hint), param_6 = position offset float[3],
// param_7/8 = flags for root-bone translation.
// Output: bone quaternion+position stored in DAT_05826E18 (bone*0x10 stride).
//         Final bone matrices written to param_1 (3x4 per bone).
void __cdecl BMD_Animation(void* this_, int param_1, float param_2, unsigned int param_3,
                           unsigned char param_4, unsigned int* param_5,
                           float* param_6, char param_7, char param_8) {
    int*  piModel = (int*)this_;
    // clamp anim indices to bone count
    short boneCount = *(short*)((char*)this_ + 0x26);
    if (boneCount <= (short)(unsigned short)param_4)
        param_4 = 0;
    if (boneCount <= (short)(unsigned short)*(unsigned char*)((char*)this_ + 0xa0))
        *(unsigned char*)((char*)this_ + 0xa0) = 0;
    // store current color/frame in model header
    *(unsigned int*)((char*)this_ + 0x78) = param_5[0];
    *(unsigned int*)((char*)this_ + 0x7c) = param_5[1];
    *(unsigned int*)((char*)this_ + 0x80) = param_5[2];
    *(float*)((char*)this_ + 0xa4) = param_2;
    // compute fractional frame and frame indices
    long long lv = (long long)(int)param_2;
    short curFrame = (short)lv;
    *(short*)((char*)this_ + 0xa8) = curFrame;
    float fFrac = param_2 - (float)(int)curFrame;
    float fFrac2 = _DAT_0055256c - fFrac;
    // param_3 es realmente `float PriorFrame` (IDA firma), no un entero. La firma
    // C nuestra lo declara `unsigned int` porque el caller en Calc_RenderObject lo
    // carga con *(DWORD*)(entity+0x10c) y los bits del float caben en un DWORD.
    // Hay que reinterpretar las bits → float y truncar a int
    // (== IDA: v12 = (std::int64_t)PriorFrame; v39 = v12).
    float priorFrame;
    memcpy(&priorFrame, &param_3, 4);
    int v39 = (int)priorFrame;
    if (boneCount > 0) {
        int* animA = (int*)(*(int*)((char*)this_ + 0x30) + (unsigned int)param_4 * 0x10);
        if (*(short*)(animA + 2) <= v39) v39 = 0;
        // IDA también clamp this[+0xA8] contra anim[this+0xA0]'s length:
        //   if ( v11 >= *(std::int16_t *)(16 * *(std::uint8_t*)(This+160) + v13 + 8) )
        //       *(_WORD *)(This + 168) = 0;
        int animBIdxClamp = *(unsigned char*)((char*)this_ + 0xa0);
        int* animBHdr = (int*)(*(int*)((char*)this_ + 0x30) + animBIdxClamp * 0x10);
        if (*(short*)(animBHdr + 2) <= *(short*)((char*)this_ + 0xa8))
            *(short*)((char*)this_ + 0xa8) = 0;
        // per-bone loop: interpolate quaternions and build transforms
        for (int boneIdx = 0; boneIdx < (int)(short)*(short*)((char*)this_+0x22); boneIdx++) {
            int bonePtr = *(int*)((char*)this_ + 0x2c) + boneIdx * 0x8c;
            if (*(char*)(bonePtr + 0x22) == '\0') {
                int* animPtrA = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)param_4 * 0xc);
                int* animPtrB = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)*(unsigned char*)((char*)this_ + 0xa0) * 0xc);
                float quatA[4], quatB[4];
                if (boneIdx == *(int*)((char*)this_ + 0x54)) {
                    // root bone: leer Euler angles (3 floats, stride 0xc) y
                    // convertir a quat via EulerToQuat (EulerToQuat), con los 3 componentes:
                    // EulerToQuat lee también Y (línea 25: v5 = a1[1]*0.5).
                    // IDA: v43[1] = *(_DWORD *)(v21 + 4); (Y sin ajuste HeadAngle)
                    float posA[4], posB[4];
                    int   curFrameB = *(short*)((char*)this_ + 0xa8);
                    posA[0] = *(float*)(animPtrA[1] + v39 * 0xc)     - param_6[0] * _DAT_005528b0;
                    posA[1] = *(float*)(animPtrA[1] + v39 * 0xc + 4);
                    posA[2] = *(float*)(animPtrA[1] + v39 * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    posB[0] = *(float*)(animPtrB[1] + curFrameB * 0xc)     - param_6[0] * _DAT_005528b0;
                    posB[1] = *(float*)(animPtrB[1] + curFrameB * 0xc + 4);
                    posB[2] = *(float*)(animPtrB[1] + curFrameB * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    EulerToQuat((int)&posA[0], (int)&quatA[0], 0, 0);
                    EulerToQuat((int)&posB[0], (int)&quatB[0], 0, 0);
                } else {
                    // regular bone: read quaternion from anim table
                    int frameOff = v39 * 0x10;
                    quatA[0] = *(float*)(animPtrA[2] + frameOff);
                    quatA[1] = *(float*)(animPtrA[2] + frameOff + 4);
                    quatA[2] = *(float*)(animPtrA[2] + frameOff + 8);
                    quatA[3] = *(float*)(animPtrA[2] + frameOff + 0xc);
                    int frameOffB = *(short*)((char*)this_ + 0xa8) * 0x10;
                    quatB[0] = *(float*)(animPtrB[2] + frameOffB);
                    quatB[1] = *(float*)(animPtrB[2] + frameOffB + 4);
                    quatB[2] = *(float*)(animPtrB[2] + frameOffB + 8);
                    quatB[3] = *(float*)(animPtrB[2] + frameOffB + 0xc);
                }
                // slerp between quatA and quatB
                // IDA: `(char *)&unk_5826E18 + 16 * boneIdx` — paso de 16 BYTES.
                // Con `&DAT_05826e18` (DWORD*) el paso salía de 64 bytes.
                int quatBone = boneIdx;
                if (quatBone < 0 || quatBone >= 200) quatBone = 0;   // guard: la tabla es de 200 huesos
                float* outQuat = (float*)(DAT_05826e18 + quatBone * 0x10);
                int same = Terrain_QuadEqual((int)&quatA, (int)&quatB, 0, 0);
                if (!same)
                    QuatSlerp((int)&quatA, (int)&quatB, *(int*)&fFrac, (int)outQuat); // pass float bits, not truncated int
                else { outQuat[0]=quatA[0]; outQuat[1]=quatA[1]; outQuat[2]=quatA[2]; outQuat[3]=quatA[3]; }
                // quaternion to 3x3 rotation matrix, embedded in a 3x4.
                // QuatToMatrix solo escribe posiciones [0,1,2,4,5,6,8,9,10] (9 floats de
                // 3x3). Las posiciones 3,7,11 (columna de translación) las lee
                // R_ConcatTransforms: zero-init + inyectar tX/tY/tZ abajo.
                float rot33[12] = {0};
                QuatToMatrix((int)outQuat, (int)rot33, 0, 0);
                // get translation from frame keys (3 floats × priorFrameInt)
                float* ptransA = (float*)(*animPtrA + v39 * 0xc);
                float* ptransB = (float*)(*animPtrB + *(short*)((char*)this_ + 0xa8) * 0xc);
                float tX, tY, tZ;
                if (boneIdx == 0 &&
                    (*(char*)((unsigned int)param_4 * 0x10 + 10 + *(int*)((char*)this_+0x30)) != '\0' ||
                     *(char*)((unsigned int)*(unsigned char*)((char*)this_+0xa0)*0x10+10+*(int*)((char*)this_+0x30)) != '\0')) {
                    // root-bone override: IDA toma frame 0 de animB (NO animA)
                    //   v52 = **(float **)v17;              // v17 = animB
                    //   v53 = *(float *)(*(_DWORD *)v17+4);
                    // Antes usábamos *animPtrA → offset origen equivocado en anim
                    // de transición entre acciones (animA≠animB).
                    tX = *(float*)*animPtrB;
                    tY = *(float*)(*animPtrB + 4);
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2] + *(float*)((char*)this_ + 0x84);
                } else {
                    tX = fFrac2 * ptransA[0] + fFrac * ptransB[0];
                    tY = fFrac * ptransB[1] + fFrac2 * ptransA[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2];
                }
                // apply parent transform
                short parentIdx = *(short*)(bonePtr + 0x20);
                float* parentMat;
                if (parentIdx == -1) {
                    if (param_7 == '\0') {
                        Matrix_BuildFromEuler((float*)((char*)this_ + 0x78), &DAT_06989c9c);
                        if (param_8 != '\0') {
                            // Se escalan los 9 elementos rotacionales (3x3 completa), no sólo la
                            // diagonal (cross-ref con 5.2 ZzzBMD.cpp:153-159; el decompile de IDA se lee
                            // mal ahí):
                            //   for (y=0; y<3; y++)
                            //     for (x=0; x<3; x++)
                            //       ParentMatrix[y][x] *= BodyScale;
                            // Translate flag escribe BodyOrigin en columna [3].
                            float sc = *(float*)((char*)this_ + 0x68);
                            float* pm = &DAT_06989c9c_matrix[0];
                            // Scale full 3x3 rotation block (rows 0..2, cols 0..2).
                            // Layout: pm[y*4 + x] for row y, col x.
                            for (int y = 0; y < 3; ++y) {
                                for (int x = 0; x < 3; ++x) {
                                    pm[y * 4 + x] *= sc;
                                }
                            }
                            // Translation column [y][3] = BodyOrigin[y].
                            pm[3]  = *(float*)((char*)this_ + 0x6c);  // BodyOrigin[0]
                            pm[7]  = *(float*)((char*)this_ + 0x70);  // BodyOrigin[1]
                            pm[11] = *(float*)((char*)this_ + 0x74);  // BodyOrigin[2]
                        }
                    }
                    parentMat = &DAT_06989c9c;
                } else {
                    parentMat = (float*)(parentIdx * 0x30 + param_1);
                }
                // Inject bone translation into rot33's 3x4 translation column.
                rot33[3]  = tX;
                rot33[7]  = tY;
                rot33[11] = tZ;
                R_ConcatTransforms(parentMat, rot33, (float*)(boneIdx * 0x30 + param_1));
            }
        }
    }
}

