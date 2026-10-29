#!/usr/bin/env python3
"""Envío real del camino: paquete, estado y dos punteros nativos; sin sockets."""
import argparse,os,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import build_and_run,function,read_source
SERVICES=r'''
using std::uintptr_t;
#pragma pack(push,1)
struct OBJECT{BYTE bytes[444];};
struct CHARACTER{BYTE beforeMovement[0x2ec];BYTE Movement;BYTE beforeTarget[0x306-0x2ed];BYTE TargetX,TargetY;BYTE beforePath[0x356-0x308];BYTE PathNum;BYTE tail[916-0x357];};
#pragma pack(pop)
static_assert(sizeof(CHARACTER)==916&&offsetof(CHARACTER,PathNum)==0x356&&offsetof(CHARACTER,Movement)==0x2ec&&offsetof(CHARACTER,TargetX)==0x306);
CHARACTER* AsCharacter(uintptr_t p){return reinterpret_cast<CHARACTER*>(p);}CHARACTER* AsCharacter(void* p){return static_cast<CHARACTER*>(p);}
int DAT_00559bec,DAT_083a7c24,closed;std::vector<BYTE> sent;
void SendMove_CloseWindows97k(){++closed;}
struct CNetwork{static BYTE XorKey[32];void SendRaw(const char* bytes,int length){Require(sent.empty());sent.assign(bytes,bytes+length);}}gNetwork;
BYTE CNetwork::XorKey[32];
'''
MAIN=r'''
std::uint64_t Run(BYTE* actorMemory,BYTE* objectMemory){
 BYTE* actor=actorMemory+16;BYTE* object=objectMemory+16;std::uint64_t hash=14695981039346656037ull;unsigned cases=0;
 for(unsigned count=0;count<256;++count)for(unsigned profile=0;profile<9;++profile)for(int blocked:{0,32})for(int session:{0,113}){
  for(int i=0;i<960;++i)actorMemory[i]=BYTE(i*31+profile);for(int i=0;i<480;++i)objectMemory[i]=BYTE(i*13+7);
  actor[0x356]=BYTE(count);actor[0x306]=40;actor[0x307]=40;object[0x78]=BYTE(0x81|blocked);
  const int directions[9][2]={{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{0,0}};
  for(int i=0;i<15;++i){actor[0x357+i]=BYTE(40+(i+1)*directions[profile][0]);actor[0x366+i]=BYTE(40+(i+1)*directions[profile][1]);}
  BYTE before[960],objectBefore[480];std::memcpy(before,actorMemory,960);std::memcpy(objectBefore,objectMemory,480);
  DAT_00559bec=987;DAT_083a7c24=session;closed=0;sent.clear();Invoke(actor,object);
  int expectedDelay=blocked?987:(count<3?0:(count==3?5:int(count*3+4)));Require(DAT_00559bec==expectedDelay);
  if(blocked||count==0){Require(sent.empty()&&closed==0);}else{
   unsigned n=count>14?14:count;Require(sent.size()==5+((n>>1)+1)&&closed==1);std::vector<BYTE> packet=sent;
   for(int i=int(packet.size())-1;i>=3;--i)packet[i]^=packet[i-1]^CNetwork::XorKey[i&31];
   std::vector<BYTE> wanted(packet.size(),0);wanted[0]=0xc1;wanted[1]=BYTE(wanted.size());wanted[2]=0x10;wanted[3]=actor[0x357];wanted[4]=actor[0x366];
   const BYTE direction[9]={0,1,2,3,4,5,6,7,0};wanted[5]=BYTE((direction[profile]<<4)|(n-1));for(unsigned i=1;i<n;++i){unsigned index=5+((i+1)>>1);wanted[index]|=BYTE(direction[profile]<<((i&1)?4:0));}
   Require(packet==wanted);if(session!=113)before[16+0x2ec]=1;
  }
  Require(std::memcmp(actorMemory,before,960)==0&&std::memcmp(objectMemory,objectBefore,480)==0);
  for(BYTE b:sent){hash^=b;hash*=1099511628211ull;}hash^=unsigned(DAT_00559bec);hash*=1099511628211ull;hash^=closed;hash*=1099511628211ull;hash^=actor[0x2ec];hash*=1099511628211ull;++cases;
 }Require(cases==9216);return hash;
}
int main(){for(int i=0;i<32;++i)CNetwork::XorKey[i]=BYTE(i*53+19);auto* low=static_cast<BYTE*>(Reserve(0x26000000u));auto* object=static_cast<BYTE*>(Reserve(0x28000000u));Require(low&&object);auto expected=Run(low,object);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(uintptr_t(0x126000000ull)));auto* highObject=static_cast<BYTE*>(Reserve(uintptr_t(0x128000000ull)));auto* sign=static_cast<BYTE*>(Reserve(0x8c000000u));auto* signObject=static_cast<BYTE*>(Reserve(0x8e000000u));Require(high&&highObject&&sign&&signObject);
 std::memset(low,0xa7,960);std::memset(object,0x20,480);BYTE alias[960],aliasObject[480];std::memcpy(alias,low,960);std::memcpy(aliasObject,object,480);Require(Run(high,highObject)==expected&&std::memcmp(alias,low,960)==0&&std::memcmp(aliasObject,object,480)==0);Require(Run(sign,signObject)==expected);
 }std::printf("Send move:9216 casos/ruta %016llx\n",(unsigned long long)expected);}
'''
def harness(revision=None):
 body=function(read_source('src/Combat/Combat.cpp',revision),'Combat_SendMovePathPacket')
 call='Combat_SendMovePathPacket(reinterpret_cast<CHARACTER*>(actor),reinterpret_cast<OBJECT*>(object))' if not revision else 'Combat_SendMovePathPacket(static_cast<int>(reinterpret_cast<uintptr_t>(actor)),static_cast<int>(reinterpret_cast<uintptr_t>(object)))'
 return PREAMBLE.split('char DAT_00559bb4',1)[0]+SERVICES+body+'\nvoid Invoke(BYTE* actor,BYTE* object){'+call+';}\n'+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');p.add_argument('--reference-ref');args=p.parse_args()
 if args.reference_ref and (os.name!='nt' or args.architecture!='Win32'):p.error('Referencia sólo en Windows x86')
 with tempfile.TemporaryDirectory(prefix='mu-send-move-') as temp:
  d=Path(temp);current=build_and_run(d/'current',harness(),args);print(current,end='')
  if args.reference_ref:
   reference=build_and_run(d/'reference',harness(args.reference_ref),args)
   if current!=reference:raise RuntimeError('Diferencia con referencia x86')
   print(f'Equivalencia con {args.reference_ref}: OK')
if __name__=='__main__':main()
