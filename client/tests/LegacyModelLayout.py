"""Vistas explícitas x86 de fixtures B1; no certifican el ABI nativo.

Los bancos nuevos de modelo/animación/cache usan declaraciones reales.
"""
import re
from SceneObjectPointer import read_source
from ObjectLayoutContract import record

header=read_source('src/Model/ModelTypes.h')
records='\n'.join(record(header,name)for name in ('ModelAnimationKeys','ModelAnimationBone','ModelAnimationAction','BMD'))
records=re.sub(r'\b(?:float|BYTE|std::uint8_t|short|ModelAnimationKeys)\s*\*\s*(\w+)\s*;',r'std::uint32_t \1;',records)
VIEW='\nusing vec3_t=float[3];\n#pragma pack(push,1)\n'+records+'\n#pragma pack(pop)\n'+r'''
static_assert(sizeof(BMD)==188 && sizeof(ModelAnimationBone)==140);
inline BMD* AsModel(void* p){return static_cast<BMD*>(p);}
inline BMD* AsModel(std::uintptr_t p){return reinterpret_cast<BMD*>(p);}
'''
