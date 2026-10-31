#!/usr/bin/env python3
"""Animación no trivial contra núcleo x86 congelado, con tablas nativas altas."""
import argparse,tempfile
from pathlib import Path
from AnimationBonesPointer import harness as bone_harness,MAIN as BONE_MAIN
from SceneObjectPointer import read_source,build_and_run
MAIN=r'''
int main(){
 auto* raw=static_cast<BYTE*>(Reserve(0x23000000u));Require(raw);
 static BMD native;static ModelAnimationBone bones[6];static ModelAnimationAction actions[3];static ModelAnimationKeys keys[6][3];
 static float positions[6][3][5][3],eulers[6][3][5][3],quats[6][3][5][4];
 if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(&native)>0xffffffffull&&reinterpret_cast<uintptr_t>(bones)>0xffffffffull&&reinterpret_cast<uintptr_t>(actions)>0xffffffffull&&reinterpret_cast<uintptr_t>(keys)>0xffffffffull&&reinterpret_cast<uintptr_t>(positions)>0xffffffffull&&reinterpret_cast<uintptr_t>(eulers)>0xffffffffull&&reinterpret_cast<uintptr_t>(quats)>0xffffffffull);
 auto Pointer32=[&](BYTE* p,const void* v){auto address=reinterpret_cast<uintptr_t>(v);Require(address<=0xffffffffull);std::uint32_t a=std::uint32_t(address);std::memcpy(p,&a,4);};
 std::uint64_t hash=14695981039346656037ull;
 for(unsigned trial=0;trial<4096;++trial){
  std::memset(raw,0xa5,65536);std::memset(&native,0xa5,sizeof(native));std::memset(bones,0x5a,sizeof(bones));std::memset(actions,0x3c,sizeof(actions));
  auto* old=raw+16;auto* oldBones=raw+512;auto* oldActions=raw+2048;auto* oldKeys=raw+4096;
  native.Bones=reinterpret_cast<BYTE*>(bones);native.Actions=reinterpret_cast<BYTE*>(actions);native.NumBones=short(1+trial%6);native.NumActions=3;native.CurrentAction=BYTE(trial%5);native.BoneHead=int(trial%8)-1;native.BodyScale=.25f*float(1+trial%7);native.BodyHeight=float(trial%5);native.BodyOrigin[0]=-3;native.BodyOrigin[1]=float(trial%13);native.BodyOrigin[2]=7;
  Pointer32(old+44,oldBones);Pointer32(old+48,oldActions);Store(old,34,native.NumBones);Store(old,38,native.NumActions);old[160]=native.CurrentAction;Store(old,84,native.BoneHead);Store(old,104,native.BodyScale);std::memcpy(old+108,native.BodyOrigin,12);Store(old,132,native.BodyHeight);
  for(unsigned a=0;a<3;++a){actions[a].FrameCount=short(3+a);actions[a].LockPositions=BYTE((trial>>(a+1))&1);Store(oldActions,a*16+8,actions[a].FrameCount);oldActions[a*16+10]=actions[a].LockPositions;}
  for(unsigned b=0;b<6;++b){bones[b].Parent=short(b==0||((trial+b)%4==0)?-1:b-1);bones[b].Dummy=BYTE((trial+b)%11==0);bones[b].Keys=keys[b];Store(oldBones,b*140+32,bones[b].Parent);oldBones[b*140+34]=bones[b].Dummy;Pointer32(oldBones+b*140+36,oldKeys+b*36);
   for(unsigned a=0;a<3;++a){auto* p=raw+8192+(b*3+a)*64;auto* e=raw+20000+(b*3+a)*64;auto* q=raw+40000+(b*3+a)*80;
    keys[b][a]={&positions[b][a][0][0],&eulers[b][a][0][0],&quats[b][a][0][0]};Pointer32(oldKeys+b*36+a*12,p);Pointer32(oldKeys+b*36+a*12+4,e);Pointer32(oldKeys+b*36+a*12+8,q);
    for(unsigned f=0;f<5;++f){for(unsigned axis=0;axis<3;++axis){positions[b][a][f][axis]=float(int((trial+b*5+a*3+f*7+axis)%37)-18)*.125f;eulers[b][a][f][axis]=float(int((trial+b*9+a*11+f*13+axis*17)%61)-30)*.03125f;}EulerToQuat(eulers[b][a][f],quats[b][a][f],0,0);}
    std::memcpy(p,positions[b][a],60);std::memcpy(e,eulers[b][a],60);std::memcpy(q,quats[b][a],80);
   }
  }
  float nativeOutput[80],oldOutput[80],nativeParent[16],oldParent[16];char nativeScratch[112],oldScratch[112];
  for(unsigned i=0;i<80;++i)nativeOutput[i]=oldOutput[i]=float(i%13)*.25f;
  for(unsigned i=0;i<16;++i)nativeParent[i]=oldParent[i]=float(int(i)-7)*.125f;
  std::memset(nativeScratch,0x6b,sizeof(nativeScratch));std::memcpy(oldScratch,nativeScratch,sizeof(nativeScratch));
  float angles[3]={float(trial%31),-float(trial%17),float(trial%23)};float head[3]={.125f*float(trial%3),-.25f,0};
  float frame=float(trial%23)*.25f,prior=float(trial%19)*.25f;unsigned priorBits;std::memcpy(&priorBits,&prior,4);BYTE priorAction=BYTE((trial/5)%5);char special=char(trial%2),translate=char((trial/2)%2);
  BMD expected=native;BYTE oldBefore[65536];std::memcpy(oldBefore,raw,65536);
  AnimateWithScratch(&native,nativeOutput,frame,priorBits,priorAction,reinterpret_cast<unsigned*>(angles),head,special,translate,nativeScratch,nativeParent);
  Frozen::AnimateWithScratch(old,oldOutput,frame,priorBits,priorAction,reinterpret_cast<unsigned*>(angles),head,special,translate,oldScratch,oldParent);
  std::memcpy(expected.BodyAngle,old+120,12);std::memcpy(&expected.CurrentAnimation,old+164,4);std::memcpy(&expected.CurrentAnimationFrame,old+168,2);expected.CurrentAction=old[160];
  Require(std::memcmp(&native,&expected,sizeof(native))==0&&std::memcmp(nativeOutput,oldOutput,sizeof(nativeOutput))==0&&std::memcmp(nativeParent,oldParent,sizeof(nativeParent))==0&&std::memcmp(nativeScratch,oldScratch,sizeof(nativeScratch))==0);
  // El original sólo puede modificar esos campos del contexto, nunca sus tablas.
  std::memcpy(oldBefore+16+120,old+120,12);std::memcpy(oldBefore+16+164,old+164,6);oldBefore[16+160]=old[160];Require(std::memcmp(raw,oldBefore,65536)==0);
  for(auto value:reinterpret_cast<const BYTE(&)[sizeof(nativeOutput)]>(nativeOutput)){hash^=value;hash*=1099511628211ull;}
 }
 std::printf("Animation differential: 4096 poses no triviales, núcleo congelado/canarios %016llx ptr=%zu\n",(unsigned long long)hash,sizeof(void*));
}
'''
def harness():
 source=bone_harness().split("void Invoke(",1)[0]
 return source+'\nnamespace Frozen{\n'+read_source('tests/fixtures/animation-x86.inc')+'\n}\n'+MAIN

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
 with tempfile.TemporaryDirectory(prefix='mu-animation-diff-')as t:print(build_and_run(Path(t)/'current',harness(),a),end='')
if __name__=='__main__':main()
