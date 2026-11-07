#!/usr/bin/env python3
"""Reserva global real: header completo, crecimiento y carga BMD sobre slots del pool."""
import argparse,os,subprocess,tempfile
from pathlib import Path
from SceneObjectPointer import ROOT,read_source

CONTRACTS=r'''
#include "Model/ModelTypes.h"
#include <cstdio>
static_assert(sizeof(BMD)==(sizeof(void*)==4?188:212));
static_assert(offsetof(BMD,Bones)==(sizeof(void*)==4?44:48));
static_assert(offsetof(BMD,LightMaps)==(sizeof(void*)==4?64:84));
static_assert(offsetof(BMD,BodyLight)==(sizeof(void*)==4?72:96));
static_assert(offsetof(BMD,Sounds)==(sizeof(void*)==4?170:194));
int main(){std::printf("ModelReservation contrato: BMD=%zu ptr=%zu OK\n",sizeof(BMD),sizeof(void*));}
'''

def build(directory,args,source=None,contracts=False,resource_source=None):
    directory.mkdir()
    if source is not None:(directory/'main.cpp').write_text(source,encoding='utf-8')
    units=['main.cpp']if source is not None else [str(ROOT/'tests/ResourceTables.cpp')]
    if not contracts:units.append(str(resource_source or ROOT/'src/Render/ResourceTables.cpp'))
    cmake='cmake_minimum_required(VERSION 3.25)\nproject(ModelReservation LANGUAGES CXX)\nadd_executable(reservation '+' '.join('"'+u.replace('\\','/')+'"' for u in units)+')\n'
    cmake+='target_include_directories(reservation PRIVATE "'+(ROOT/'src').as_posix()+'")\ntarget_compile_features(reservation PRIVATE cxx_std_17)\nif(MSVC)\ntarget_compile_options(reservation PRIVATE /EHsc /fp:precise /utf-8)\nelse()\ntarget_compile_options(reservation PRIVATE -fno-strict-aliasing)\nendif()\n'
    (directory/'CMakeLists.txt').write_text(cmake,encoding='utf-8')
    command=['cmake','-S',str(directory),'-B',str(directory/'build')]
    if args.generator:command+=['-G',args.generator]
    if os.name=='nt':command+=['-A',args.architecture,'-T','v143']
    subprocess.run(command,check=True);subprocess.run(['cmake','--build',str(directory/'build'),'--config','Release'],check=True)
    binary=directory/'build/Release/reservation.exe'if os.name=='nt'else directory/'build/reservation'
    return subprocess.check_output([str(binary)],cwd=directory,text=True)

def loader():
    import BMDLoaderNative as test
    from ModelReleaseLifecycle import PREFIX
    original=test.prefix
    test.prefix=lambda:PREFIX+'\n#include "Model/ModelTypes.h"\n#include "Physics/ClothTypes.h"\n#include "Render/ResourceTables.h"\n'
    try:code=test.harness()
    finally:test.prefix=original
    before='static GuardModel storage;'
    after='''static unsigned selectedModelSlot;
#define storage (*reinterpret_cast<GuardModel*>(static_cast<BYTE*>(ResourceTables::ModelBase())+selectedModelSlot*sizeof(BMD)-16))
'''
    assert before in code;code=code.replace(before,after)
    code=code.replace('int main(){Reader reader;','int main(){ResourceTables::EnsureModels(32768);if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(ResourceTables::ModelBase())>0xffffffffull);Reader reader;')
    code=code.replace('blocks.clear();ids.clear();','const unsigned slots[]={1,18,19,20,21,22,963,1500,3000,32766};selectedModelSlot=slots[trial%10];blocks.clear();ids.clear();')
    code=code.replace('auto expected=storage;','auto expected=storage;auto* poolBase=static_cast<BYTE*>(ResourceTables::ModelBase());const std::size_t poolSize=ResourceTables::ModelCount()*sizeof(BMD),first=selectedModelSlot*sizeof(BMD)-16,last=first+sizeof(GuardModel);std::vector<BYTE> poolBefore(poolBase,poolBase+poolSize);')
    needle='Hash(m->Name,32);'
    code=code.replace(needle,'Require(std::memcmp(poolBase,poolBefore.data(),first)==0&&std::memcmp(poolBase+last,poolBefore.data()+last,poolSize-last)==0);'+needle)
    return code

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--contracts-only',action='store_true');p.add_argument('--reference-ref');a=p.parse_args()
    if a.reference_ref and(os.name!='nt'or a.architecture!='Win32'):p.error('Referencia sólo Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-model-reservation-')as temp:
        root=Path(temp);print(build(root/'contract',a,CONTRACTS,contracts=True),end='')
        if a.contracts_only:return
        growth=build(root/'growth',a);print(growth,end='')
        from BMDFileOracle import prepare
        prepare(ROOT,root);code=loader();loaded=build(root/'loader',a,code);print(loaded,end='')
        if a.reference_ref:
            resource=root/'reference-resource.cpp';resource.write_text(read_source('src/Render/ResourceTables.cpp',a.reference_ref),encoding='utf-8')
            if build(root/'reference-growth',a,resource_source=resource)!=growth or build(root/'reference-loader',a,code,resource_source=resource)!=loaded:raise RuntimeError('Diferencia de reserva frente a x86')
            print('Equivalencia del gestor con '+a.reference_ref+': OK')
        print('ModelReservation: carga/postcarga/baja sobre pool real, vecinos intactos OK')
if __name__=='__main__':main()
