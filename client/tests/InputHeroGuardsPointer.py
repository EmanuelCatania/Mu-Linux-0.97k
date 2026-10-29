#!/usr/bin/env python3
"""Dos condiciones reales de entrada: flags del héroe con dirección nativa.

Extrae sólo las condiciones +765/+846. No ejecuta Player_ProcessInput ni
certifica sus ramas, copias de nombre, paquetes o UI completos.
"""
import argparse
import os
from pathlib import Path
import re
import tempfile
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run, function, read_source

SERVICES=r'''
void* DAT_07abf5d8;DWORD DAT_00559c84,MouseRButtonPush;int SelectedCharacter;
'''
MAIN=r'''
std::uint64_t Run(BYTE* memory){
 const DWORD switches[]={0,1,0x100,0x80000000u,0xffffffffu};
 const int selections[]={-1,0,-2,399,static_cast<int>(0x80000000u),0x7fffffff};
 std::uint64_t hash=14695981039346656037ull;unsigned cases=0,coverage=0;
 for(unsigned profile=0;profile<16;++profile){
  for(unsigned i=0;i<1100;++i)memory[i]=static_cast<BYTE>(profile+i*37);
  BYTE* hero=memory+16+(profile%2)*8;DAT_07abf5d8=hero;
  DAT_00559c84=switches[profile%5];MouseRButtonPush=switches[(profile/3)%5];SelectedCharacter=selections[(profile/2)%6];
  BYTE expected[1100];std::memcpy(expected,memory,1100);
  for(unsigned dead=0;dead<256;++dead)for(unsigned safe=0;safe<256;++safe){
   hero[765]=static_cast<BYTE>(dead);hero[846]=static_cast<BYTE>(safe);expected[(hero-memory)+765]=static_cast<BYTE>(dead);expected[(hero-memory)+846]=static_cast<BYTE>(safe);
   const bool stop=DeadGuard(),hover=HoverGuard();
   const bool wantedHover=DAT_00559c84!=0 && safe!=0 && SelectedCharacter!=-1 && MouseRButtonPush!=0;
   Require(stop==(dead!=0) && hover==wantedHover && DAT_07abf5d8==hero && std::memcmp(memory,expected,1100)==0);
   Require(DAT_00559c84==switches[profile%5] && MouseRButtonPush==switches[(profile/3)%5] && SelectedCharacter==selections[(profile/2)%6]);
   coverage|=stop?1u:2u;coverage|=hover?4u:8u;
   hash^=unsigned(stop)+2*unsigned(hover);hash*=1099511628211ull;++cases;
  }
 }
 // Sólo la segunda condición puede evaluarse sin héroe si su primera guarda corta.
 DAT_07abf5d8=nullptr;DAT_00559c84=0;SelectedCharacter=0;MouseRButtonPush=1;Require(!HoverGuard());
 Require(cases==1048576 && coverage==15);return hash;
}
int main(){
 auto* low=static_cast<BYTE*>(Reserve(0x26000000u));Require(low);const auto expected=Run(low);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uintptr_t(0x126000000ull)));Require(high);std::memset(low,0x7b,1100);Require(Run(high)==expected);for(unsigned i=0;i<1100;++i)Require(low[i]==0x7b);std::puts("heroe alto; flags completos, readonly y alias intactos");}
 std::printf("Input hero guards: 1048576 pares por ruta %016llx\n",static_cast<unsigned long long>(expected));
}
'''

def condition(source,offset):
    body=function(source,'Player_ProcessInput');matches=[]
    for start in re.finditer(r'\bif\s*\(',body):
        first=start.end();end=first;depth=1
        while depth:
            depth+=(body[end]=='(')-(body[end]==')');end+=1
        expression=body[first:end-1]
        if offset in expression and 'DAT_07abf5d8' in expression:matches.append(expression)
    if len(matches)!=1:raise ValueError(f'Condición inesperada: {offset}')
    return matches[0]

def harness(revision=None):
    source=read_source('src/Game/Player_InputTick.cpp',revision)
    guards='\n'.join(f'bool {name}(){{return ({condition(source,offset)});}}' for name,offset in [('DeadGuard','0x2fd'),('HoverGuard','0x34e')])
    return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+guards+MAIN

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--architecture',choices=('Win32','x64'),default='x64')
    parser.add_argument('--generator')
    parser.add_argument('--reference-ref')
    args=parser.parse_args()
    if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):parser.error('Referencia sólo en Windows x86')
    with tempfile.TemporaryDirectory(prefix='mu-input-hero-guards-') as temp:
        directory=Path(temp);current=build_and_run(directory/'current',harness(),args);print(current,end='')
        if args.reference_ref:
            reference=build_and_run(directory/'reference',harness(args.reference_ref),args)
            if current!=reference:raise RuntimeError('Diferencia con referencia x86')
            print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
