// Referencia de 34b885e5: offsets x86 del contexto y de la transformación.
void PrepareContext(OBJECT* entity, void* model)
{
    BYTE* modelBase = static_cast<BYTE*>(model);
    *(DWORD*)(modelBase + 0x68) = *(DWORD *)&AsObject(entity)->Scale;
    *(BYTE *)(modelBase + 0xa0) = AsObject(entity)->CurrentAction;
    *(DWORD*)(modelBase + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
    *(DWORD*)(modelBase + 0x70) = *(DWORD *)&AsObject(entity)->Position[1];
    *(DWORD*)(modelBase + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];

    short cls = AsObject(entity)->Type;
    if (cls == 0x14b) {
        *(DWORD*)(modelBase + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)(modelBase + 0x70) = AsObject(entity)->Position[1] + _DAT_0055290c;
        *(DWORD*)(modelBase + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    } else if (cls == 0x14a) {
        *(DWORD*)(modelBase + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)(modelBase + 0x70) = AsObject(entity)->Position[1] + _DAT_00552908;
        *(DWORD*)(modelBase + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    }
}
void __cdecl BMD_TransformPosition(void *this_,float *param_1,float *param_2,float *param_3,char param_4)
{
  if (param_4 != '\0') {
    float local_buf[3];
    Vector_Transform(param_2,param_1, local_buf);
    const BYTE* modelBytes = static_cast<const BYTE*>(this_);
    float scale = *reinterpret_cast<const float*>(modelBytes + 0x68);

    {
      static DWORD s_lastTP = 0;
      DWORD now = Platform::GetTimer().Ticks();
      if (now - s_lastTP > 1000) {
        s_lastTP = now;
      }
    }
    param_3[0] = scale * local_buf[0] + *reinterpret_cast<const float*>(modelBytes + 0x6c);
    param_3[1] = scale * local_buf[1] + *reinterpret_cast<const float*>(modelBytes + 0x70);
    param_3[2] = scale * local_buf[2] + *reinterpret_cast<const float*>(modelBytes + 0x74);
    return;
  }
  Vector_Transform(param_2,param_1,param_3);
  return;
}

void __fastcall BMD__TransformPosition(void *This, float (*BoneMatrix)[4], float *Pos, float *WorldPos, bool Translate) {

    float temp[3];
    float *dst = Translate ? temp : WorldPos;
    dst[0] = Pos[0] * BoneMatrix[0][0] + Pos[1] * BoneMatrix[0][1] + Pos[2] * BoneMatrix[0][2] + BoneMatrix[0][3];
    dst[1] = Pos[0] * BoneMatrix[1][0] + Pos[1] * BoneMatrix[1][1] + Pos[2] * BoneMatrix[1][2] + BoneMatrix[1][3];
    dst[2] = Pos[0] * BoneMatrix[2][0] + Pos[1] * BoneMatrix[2][1] + Pos[2] * BoneMatrix[2][2] + BoneMatrix[2][3];

    if (Translate) {

        const BYTE* modelBytes = static_cast<const BYTE*>(This);
        float scale = *reinterpret_cast<const float*>(modelBytes + 0x68);
        const float *origin = reinterpret_cast<const float*>(modelBytes + 0x6c);
        WorldPos[0] = scale * temp[0] + origin[0];
        WorldPos[1] = scale * temp[1] + origin[1];
        WorldPos[2] = scale * temp[2] + origin[2];
    }
}

// Entrada de rotación congelada antes de migrar el acceso al modelo.
void __cdecl BMD__RotationPosition(void *model, float *bone_mat, float *pos_in, float *pos_out)
{

  Vector_Rotate(pos_in, bone_mat, pos_out);

  if (model) {
    float sc = *(float*)((char*)model + 0x68);
    pos_out[0] *= sc;
    pos_out[1] *= sc;
    pos_out[2] *= sc;
  }


  float* dst = (float*)&DAT_06989c9c;
  for (int i = 0; i < 12; ++i) dst[i] = bone_mat[i];
}
