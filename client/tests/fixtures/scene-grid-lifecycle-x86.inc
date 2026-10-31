// Referencia x86 de e817dbd3: reserva/enlazado, bajas y descarga; sin inicialización visual.
alignas(16) char g_ObjectBucketGrid[4096];
#define DAT_083a0218 (*(DWORD*)(g_ObjectBucketGrid))
#define DAT_083a021c (*(DWORD*)(g_ObjectBucketGrid+4))
void* __cdecl CreateObject(int param_1, float* param_2, float* param_3, float param_4) {


    int igx = (int)(param_2[0] * _DAT_00552d20);
    int igy = (int)(param_2[1] * _DAT_00552d20);
    if (igx < 0 || igy < 0 || igx > 15 || igy > 15) return nullptr;
    BYTE  bVar4 = (BYTE)((char)igx * 0x10 + (char)igy);
    UINT  uVar2 = (UINT)bVar4;

    UINT* puVar3 = (UINT*)operator_new(0x1bc);
    if (!puVar3) return nullptr;
    SceneObjects::Register(puVar3);
    UINT* p = puVar3;
    for (int i = 0x6f; i != 0; i--) *p++ = 0;


    UINT** headArr = (UINT**)&DAT_083a021c;
    UINT** tailArr = (UINT**)((char*)&DAT_083a021c + 4);
    if (headArr[uVar2 * 4] == nullptr) {
        puVar3[0x6d] = 0; puVar3[0x6e] = 0;
        headArr[uVar2 * 4] = puVar3;
    } else {
        UINT* prevTail = tailArr[uVar2 * 4];
        *(UINT**)((char*)prevTail + 0x1b8) = puVar3;
        puVar3[0x6d] = (UINT)prevTail;
        puVar3[0x6e] = 0;
    }
    tailArr[uVar2 * 4] = puVar3;

    return puVar3; }
void __cdecl Entity_GridUnlink(void *param_1_v, void* cell) {
    char *param_1 = (char*)param_1_v;
    char *param_2 = static_cast<char*>(cell);
    if (!param_1) return;
    SceneObjects::Unregister(param_1);
    int iVar1 = *(int*)(param_1 + 0x1b8);
    int iVar2 = *(int*)(param_1 + 0x1b4);
    if (iVar1 != 0) {
        if (iVar2 != 0) {
            *(int*)(iVar2 + 0x1b8) = iVar1;
            *(int*)(iVar1 + 0x1b4) = *(int*)(param_1 + 0x1b4);
            operator_delete(param_1);
            return;
        }
        *(int*)(iVar1 + 0x1b4) = 0;
        *(int*)(param_2 + 4) = iVar1;
        operator_delete(param_1);
        return;
    }
    if (iVar2 != 0) {
        *(int*)(iVar2 + 0x1b8) = 0;
        *(int*)(param_2 + 8) = iVar2;
        operator_delete(param_1);
        return;
    }
    *(int*)(param_2 + 4) = 0;
    *(int*)(param_2 + 8) = 0;
    operator_delete(param_1);
}
void __cdecl DeleteObjects(void) {

    for (int i = 0; i < 0x7580; i += 0xbc)
        BMD__Release(i + DAT_05828d58);




    char* puVar5 = (char*)&DAT_083a0218;
    char* gridEnd = (char*)g_ObjectBucketGrid + 0xFFF;
    do {
        for (int cell = 0; cell < 0x10; cell++) {
            char* head = (char*)*(DWORD*)(puVar5 + 8);
            while (head != nullptr) {
                char* next = (char*)*(DWORD*)(head + 0x1b4);
                Entity_GridUnlink(head, puVar5);
                head = next;
            }
            *(DWORD*)(puVar5 + 4) = 0;
            *(DWORD*)(puVar5 + 8) = 0;
            puVar5 += 0x10;
        }
        if (gridEnd < puVar5) {

            for (int ti = 0x23; ti < 0x68; ti++) UnloadImage(ti);















            memset(DAT_083a2370, 0, sizeof(DAT_083a2370));
            return;
        }
    } while (true);
}
#undef DAT_083a0218
#undef DAT_083a021c
