// Frontera x86 congelada para el caller legacy; banco nativo separado.
void* __cdecl RenderPartObjectBodyColor(void *model, uintptr_t entity, int etype, float scale,
                            int flags, float alpha, int rgba)
{
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else if (etype == 0x144) {
        if ((flags & 0x100) == 0) {
            color[0] = 0.2f; color[1] = 0.2f; color[2] = 0.8f;
        } else {
            flags -= 0x100;
            color[0] = 1.0f; color[1] = 0.1f; color[2] = 0.1f;
        }
        BMD__RenderBody(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), -1, (uint)rgba);
        return nullptr;
    } else {
        PartObjectColor(etype, scale, alpha, color, (char)((flags >> 8) & 1));
    }
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = 2;
    } else if (etype == 0x235) {
        BMD__RenderBody(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), 1, (uint)rgba);
        return nullptr;
    } else if (etype != 0x1af && etype != 0x1fa && etype != 0x260) {
        BMD__RenderBody(model, (uint)flags, scale,
                     *(int *)(entity+100), *(float*)(entity+0x68),
                     *(float*)(entity+0x6c), *(float*)(entity+0x70), -1, (uint)rgba);
        return nullptr;
    }
    int fVar1 = *(int*)(entity + 0x58);
    BMD__RenderBody(model, (uint)flags, scale,
                 *(int *)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    if (etype == 0x1d7) {
        *(int *)(entity + 0x58) = -1;
    }
    return nullptr;
}
void* __cdecl Entity_SetModelColorAlt(void *model, uintptr_t entity, int etype, float scale,
                             int flags, float alpha, int rgba)
{
    float *color = (float *)((char*)model + 0x48);
    if ((flags & 0x10) == 0x10) {
        color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f;
    } else {
        Weapon_SetColorAlt(etype, scale, alpha, color);
    }
    int fVar1 = (etype == 0x235) ? 1 : -1;   
    BMD__RenderBody(model, (uint)flags, scale,
                 *(int *)(entity+100), *(float*)(entity+0x68),
                 *(float*)(entity+0x6c), *(float*)(entity+0x70), fVar1, (uint)rgba);
    return nullptr;
}