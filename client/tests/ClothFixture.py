"""Coordenadas de los oráculos históricos, trasladadas al contrato nativo de tela.

Los números de referencia siguen siendo los offsets de IDA x86. No se cambia
su lógica matemática ni se ejecuta una copia del consumidor de producción.
"""
import re
from SceneObjectPointer import read_source

PREFIX = r'''
constexpr unsigned N(unsigned offset){return offset+(offset>=4?sizeof(void*)-4:0);}
constexpr unsigned W(unsigned offset){unsigned n=offset;for(unsigned p:{0u,4u,52u,60u,76u,80u})if(p<offset)n+=sizeof(void*)-4;return n;}
'''


def prefix():
    return "\n#include <initializer_list>\n"+read_source("src/Physics/ClothTypes.h").replace("#pragma once","")+PREFIX


def helper(main):
    main=re.sub(r'\b(a|b|node)\+(4|16|28|40|44|48|60)\b',r'\1+N(\2)',main)
    main=main.replace('node+field+','node+N(field)+').replace('a[40]','a[N(40)]').replace('node[40]','node[N(40)]')
    main=main.replace('beforeA+16+field','beforeA+16+N(field)').replace('beforeA+16+44','beforeA+16+N(44)').replace('beforeA[16+40]','beforeA[16+N(40)]')
    main=main.replace('beforeA+60','beforeA+16+N(44)').replace('beforeA+64','beforeA+16+N(48)')
    main=main.replace('(a)+60','(a)+N(60)')
    main=main.replace('first,beforeA,44','first,beforeA,16+N(28)').replace('first+56,beforeA+56,40','first+16+N(40),beforeA+16+N(40),96-16-N(40)')
    main=main.replace('first,beforeA,60','first,beforeA,16+N(44)').replace('second,beforeB,60','second,beforeB,16+N(44)')
    main=main.replace('first,beforeA,20','first,beforeA,16+N(4)').replace('first+32,beforeA+32,64','first+16+N(16),beforeA+16+N(16),96-16-N(16)')
    main=main.replace('i>=76','i>=16+sizeof(ClothNode)')
    return main


def wind(observer,main):
    observer=observer.replace('calls*60','calls*sizeof(ClothNode)')
    main=main.replace('PutInt(widget+52,static_cast<int>(reinterpret_cast<uintptr_t>(nodes)));','ClothSystem(widget).Nodes=reinterpret_cast<ClothNode*>(nodes);')
    main=re.sub(r'\b84\b','sizeof(ClothWidget)',main)
    main=re.sub(r'widget\+(20|40|48)\b',r'widget+W(\1)',main)
    main=main.replace('5792','(96*sizeof(ClothNode)+32)').replace('node*60','node*sizeof(ClothNode)').replace('/60','/sizeof(ClothNode)').replace('%60','%sizeof(ClothNode)')
    main=re.sub(r'node\*sizeof\(ClothNode\)\+(4|16|40)\b',r'node*sizeof(ClothNode)+N(\1)',main)
    main=main.replace('offset>=4 && offset<16','offset>=N(4) && offset<N(16)')
    return observer,main


def vertices(services,main):
    services=services.replace('nodes+60*visited','nodes+sizeof(ClothNode)*visited')
    main=main.replace('Put(object+52,static_cast<int>(reinterpret_cast<std::uintptr_t>(nodes)));','ClothSystem(object).Nodes=reinterpret_cast<ClothNode*>(nodes);')
    main=re.sub(r'object\+(12|16|20|40|44|48)\b',r'object+W(\1)',main)
    main=main.replace('nodes+60*i+28','nodes+sizeof(ClothNode)*i+N(28)')
    return services,main


def pin(main):
    main=main.replace('PutInt(widget+4,Address(entity));','ClothSystem(widget).Owner=reinterpret_cast<OBJECT*>(entity);').replace('PutInt(widget+52,Address(nodes));','ClothSystem(widget).Nodes=reinterpret_cast<ClothNode*>(nodes);')
    main=re.sub(r'\b84\b','sizeof(ClothWidget)',main)
    main=re.sub(r'widget\+(20|24|28|32|40|64)\b',r'widget+W(\1)',main)
    main=main.replace('i/60','i/sizeof(ClothNode)').replace('i%60','i%sizeof(ClothNode)').replace('node*60+28','node*sizeof(ClothNode)+N(28)')
    main=main.replace('offset>=28 && offset<40','offset>=N(28) && offset<N(40)').replace('offset==40','offset==N(40)')
    return main


def reset(main):
    return main.replace('i=4;i<72','i=sizeof(void*);i<W(72)').replace('expected+start+offset','expected+start+W(offset)')
