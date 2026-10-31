// Núcleo congelado de fd542091 para comparar layouts sin alterar la matemática.
void AnimateWithScratch(void* this_, float* param_1, float param_2, unsigned int param_3,
                           unsigned char param_4, unsigned int* param_5,
                           float* param_6, char param_7, char param_8,
                           char* quaternionScratch, float* parentScratch) {
    int*  piModel = (int*)this_;

    short boneCount = *(short*)((char*)this_ + 0x26);
    if (boneCount <= (short)(unsigned short)param_4)
        param_4 = 0;
    if (boneCount <= (short)(unsigned short)*(unsigned char*)((char*)this_ + 0xa0))
        *(unsigned char*)((char*)this_ + 0xa0) = 0;

    *(unsigned int*)((char*)this_ + 0x78) = param_5[0];
    *(unsigned int*)((char*)this_ + 0x7c) = param_5[1];
    *(unsigned int*)((char*)this_ + 0x80) = param_5[2];
    *(float*)((char*)this_ + 0xa4) = param_2;

    long long lv = (long long)(int)param_2;
    short curFrame = (short)lv;
    *(short*)((char*)this_ + 0xa8) = curFrame;
    float fFrac = param_2 - (float)(int)curFrame;
    float fFrac2 = _DAT_0055256c - fFrac;





    float priorFrame;
    memcpy(&priorFrame, &param_3, 4);
    int v39 = (int)priorFrame;
    if (boneCount > 0) {
        int* animA = (int*)(*(int*)((char*)this_ + 0x30) + (unsigned int)param_4 * 0x10);
        if (*(short*)(animA + 2) <= v39) v39 = 0;



        int animBIdxClamp = *(unsigned char*)((char*)this_ + 0xa0);
        int* animBHdr = (int*)(*(int*)((char*)this_ + 0x30) + animBIdxClamp * 0x10);
        if (*(short*)(animBHdr + 2) <= *(short*)((char*)this_ + 0xa8))
            *(short*)((char*)this_ + 0xa8) = 0;

        for (int boneIdx = 0; boneIdx < (int)(short)*(short*)((char*)this_+0x22); boneIdx++) {
            int bonePtr = *(int*)((char*)this_ + 0x2c) + boneIdx * 0x8c;
            if (*(char*)(bonePtr + 0x22) == '\0') {
                int* animPtrA = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)param_4 * 0xc);
                int* animPtrB = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)*(unsigned char*)((char*)this_ + 0xa0) * 0xc);
                float quatA[4], quatB[4];
                if (boneIdx == *(int*)((char*)this_ + 0x54)) {




                    float posA[4], posB[4];
                    int   curFrameB = *(short*)((char*)this_ + 0xa8);
                    posA[0] = *(float*)(animPtrA[1] + v39 * 0xc)     - param_6[0] * _DAT_005528b0;
                    posA[1] = *(float*)(animPtrA[1] + v39 * 0xc + 4);
                    posA[2] = *(float*)(animPtrA[1] + v39 * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    posB[0] = *(float*)(animPtrB[1] + curFrameB * 0xc)     - param_6[0] * _DAT_005528b0;
                    posB[1] = *(float*)(animPtrB[1] + curFrameB * 0xc + 4);
                    posB[2] = *(float*)(animPtrB[1] + curFrameB * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    EulerToQuat(posA, quatA, 0, 0);
                    EulerToQuat(posB, quatB, 0, 0);
                } else {

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



                int quatBone = boneIdx;
                if (quatBone < 0 || quatBone >= 200) quatBone = 0;
                float* outQuat = (float*)(quaternionScratch + quatBone * 0x10);
                int same = Terrain_QuadEqual(quatA, quatB, 0, 0);
                if (!same)
                    QuatSlerp(quatA, quatB, fFrac, outQuat);
                else { outQuat[0]=quatA[0]; outQuat[1]=quatA[1]; outQuat[2]=quatA[2]; outQuat[3]=quatA[3]; }




                float rot33[12] = {0};
                QuatToMatrix(outQuat, rot33, 0, 0);

                float* ptransA = (float*)(*animPtrA + v39 * 0xc);
                float* ptransB = (float*)(*animPtrB + *(short*)((char*)this_ + 0xa8) * 0xc);
                float tX, tY, tZ;
                if (boneIdx == 0 &&
                    (*(char*)((unsigned int)param_4 * 0x10 + 10 + *(int*)((char*)this_+0x30)) != '\0' ||
                     *(char*)((unsigned int)*(unsigned char*)((char*)this_+0xa0)*0x10+10+*(int*)((char*)this_+0x30)) != '\0')) {





                    tX = *(float*)*animPtrB;
                    tY = *(float*)(*animPtrB + 4);
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2] + *(float*)((char*)this_ + 0x84);
                } else {
                    tX = fFrac2 * ptransA[0] + fFrac * ptransB[0];
                    tY = fFrac * ptransB[1] + fFrac2 * ptransA[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2];
                }

                short parentIdx = *(short*)(bonePtr + 0x20);
                float* parentMat;
                if (parentIdx == -1) {
                    if (param_7 == '\0') {
                        Matrix_BuildFromEuler((float*)((char*)this_ + 0x78), parentScratch);
                        if (param_8 != '\0') {







                            float sc = *(float*)((char*)this_ + 0x68);
                            float* pm = parentScratch;


                            for (int y = 0; y < 3; ++y) {
                                for (int x = 0; x < 3; ++x) {
                                    pm[y * 4 + x] *= sc;
                                }
                            }

                            pm[3]  = *(float*)((char*)this_ + 0x6c);
                            pm[7]  = *(float*)((char*)this_ + 0x70);
                            pm[11] = *(float*)((char*)this_ + 0x74);
                        }
                    }
                    parentMat = parentScratch;
                } else {
                    parentMat = param_1 + parentIdx * 12;
                }

                rot33[3]  = tX;
                rot33[7]  = tY;
                rot33[11] = tZ;
                R_ConcatTransforms(parentMat, rot33, param_1 + boneIdx * 12);
            }
        }
    }
}
