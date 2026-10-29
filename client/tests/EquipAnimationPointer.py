#!/usr/bin/env python3
"""Reconstrucción real de slots: tipos/bytes/clase y personaje nativo."""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
CHARACTER* active;
unsigned stopCalls,scaleCalls;
BYTE scaleSnapshot[192];
const BYTE* beforeStop;
void SetPlayerStop(CHARACTER* character){Require(character==active && std::memcmp(reinterpret_cast<BYTE*>(character)+480,beforeStop+480,192)==0);++stopCalls;}
void SetCharacterScale(CHARACTER* character){Require(character==active);++scaleCalls;std::memcpy(scaleSnapshot,reinterpret_cast<BYTE*>(character)+480,192);}
'''
MAIN=r'''
void Update(CHARACTER* character){Character_UpdateEquipSlotAnimations(ARG);}
std::uint64_t Run(BYTE* memory){
    std::uint64_t digest=14695981039346656037ull;
    const int bases[]={912,919,926,933,940};const unsigned checked[]={1,3,4,5,6};
    for(unsigned trial=0;trial<65536;++trial){
        std::memset(memory,0xa5,948);active=reinterpret_cast<CHARACTER*>(memory+16);
        active->Class=static_cast<BYTE>(trial);active->Object.CurrentAction=static_cast<BYTE>(trial/256);
        active->Object.Type=trial%17?390:static_cast<short>((trial/17)%3==0?0:(trial/17)%3==1?391:-1);
        const unsigned mask=(trial*7+(trial/256)*13)%32;const int cls=active->Class&7,second=active->Class>>3;
        for(unsigned part=0;part<6;++part){active->BodyPart[part].Type=-1;active->BodyPart[part].Level=static_cast<BYTE>(trial+part);active->BodyPart[part].Option1=static_cast<BYTE>(trial*7+part);}
        active->Weapon[0].Type=-1;
        for(unsigned gate=0;gate<5;++gate)if(mask&(1u<<gate)){if(checked[gate]==6)active->Weapon[0].Type=static_cast<short>(cls+bases[gate]);else active->BodyPart[checked[gate]].Type=static_cast<short>(cls+bases[gate]);}
        alignas(CHARACTER) BYTE expected[948];std::memcpy(expected,memory,948);beforeStop=expected+16;
        const BYTE action=active->Object.CurrentAction;const bool expectedStop=active->Object.Type==390 && !(action>=133 && action<=140) && (action<34 || action>91);
        stopCalls=scaleCalls=0;Update(active);
        // Las condiciones leen slots que aún no fueron escritos; el destino es1..5.
        if(reinterpret_cast<CHARACTER*>(expected+16)->Object.Type==390)for(unsigned gate=0;gate<5;++gate)if(mask&(1u<<gate)){
            BYTE* target=expected+16+480+(gate+1)*24;const short type=static_cast<short>(cls+4*second+bases[gate]);std::memcpy(target,&type,2);target[2]=target[3]=0;
        }
        const bool player=reinterpret_cast<CHARACTER*>(expected+16)->Object.Type==390;
        Require(stopCalls==static_cast<unsigned>(expectedStop) && scaleCalls==static_cast<unsigned>(player) && std::memcmp(memory,expected,948)==0);
        if(player)Require(std::memcmp(scaleSnapshot,expected+16+480,192)==0);
        for(BYTE value:expected){digest^=value;digest*=1099511628211ull;}digest^=stopCalls;digest*=1099511628211ull;
    }return digest;
}
int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
    if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));Require(high && reinterpret_cast<std::uintptr_t>(high)>0xffffffffull);std::memset(low,0x7b,948);Require(Run(high)==expected);for(unsigned i=0;i<948;++i)Require(low[i]==0x7b);std::puts("personaje alto: slots/stop/bytes iguales; alias bajo intacto");}
    std::printf("Equip animation: 65536 casos, slots/clases/canarios %016llx\n",static_cast<unsigned long long>(expected));
}
'''


def harness(revision=None):
    header=read_source('src/Entity/EntityTypes.h',revision)
    part=re.search(r'struct PART_t\s*\{.*?\};',header,re.S).group();character=header[header.index('struct CHARACTER {'):header.index('    PART_t   Wing;')]
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#ifndef _WIN32\nusing WORD=std::uint16_t;\n#endif\nstruct OBJECT{BYTE opaque0[2];short Type;BYTE opaque4[257];BYTE CurrentAction;BYTE remainder[182];};\n'
    prefix+=part+character+'BYTE remainder[244];};\nstatic_assert(sizeof(OBJECT)==444);static_assert(offsetof(OBJECT,CurrentAction)==261);static_assert(sizeof(CHARACTER)==916);static_assert(offsetof(CHARACTER,BodyPart)==480);static_assert(offsetof(CHARACTER,Weapon)==624);\n'
    prefix+='\n'.join(re.findall(r'^inline (?:const )?CHARACTER\* AsCharacter[^\n]+',header,re.M))+SERVICES
    code=function(read_source('src/Scene/Scene_CharSelect_Nav.cpp',revision),'Character_UpdateEquipSlotAnimations')
    arg='character' if 'CHARACTER* param_1' in code else 'static_cast<int>(reinterpret_cast<uintptr_t>(character))'
    return prefix+code+MAIN.replace('ARG',arg)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator');parser.add_argument('--reference-ref');args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-equip-animation-')as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')


if __name__=='__main__':main()
