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
