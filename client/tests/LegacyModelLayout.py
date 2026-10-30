"""Vista BMD de los bancos históricos x86; no certifica el ABI nativo.

Los consumidores de contexto nativo se prueban con el BMD real en
ModelContextPointer y ClothRuntime. Esta vista conserva los oráculos previos.
"""
VIEW=r'''
struct BMD {
 BYTE opaque0[104];float BodyScale;float BodyOrigin[3];BYTE opaque120[12];
 float BodyHeight;BYTE opaque136[24];BYTE CurrentAction;BYTE opaque161[27];
};
static_assert(sizeof(BMD)==188);
inline BMD* AsModel(void* p){return static_cast<BMD*>(p);}
inline BMD* AsModel(std::uintptr_t p){return reinterpret_cast<BMD*>(p);}
'''
