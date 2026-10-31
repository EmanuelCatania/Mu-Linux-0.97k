#!/usr/bin/env python3
"""Contrato de BMD/colisión según IDA y rayos reales x86; hotfix sin migrar layouts."""
import argparse,os,re,subprocess,tempfile
from pathlib import Path
from MassTeleportTarget import build_and_run
ROOT=Path(__file__).resolve().parents[1]
def read(path,revision=None):
 return subprocess.check_output(['git','show',revision+':'+path],cwd=ROOT).decode('utf-8') if revision else (ROOT/path).read_text(encoding='utf-8')
def extract(source,name):
 source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S);m=re.search(r'(?:unsigned int|bool|int|void)\s+(?:__cdecl\s+)?'+name+r'\([^;{]*\)\s*\{',source);assert m,name;end=m.end();depth=1
 while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
 return source[m.start():end]+'\n'
PREAMBLE=r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
using BYTE=unsigned char;using DWORD=std::uint32_t;
#define __cdecl
#endif
using uintptr_t=std::uintptr_t;
void Require(bool value){if(!value){std::fprintf(stderr,"Contrato de colisión incorrecto\n");std::exit(1);}}
void* Reserve(uintptr_t address){
#ifdef _WIN32
 return VirtualAlloc(reinterpret_cast<void*>(address),65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
#else
 void* p=mmap(reinterpret_cast<void*>(address),65536,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);return p==MAP_FAILED?nullptr:p;
#endif
}
float pickingStart[3],pickingEnd[3];float& CameraRayOriginX=pickingStart[0];float& DAT_083a4110=pickingEnd[0];
float vertices[3*15000*3];float& DAT_0584621c=vertices[0];float DAT_083a4120,DAT_083a4130,DAT_083a4134,DAT_083a4138;
double Math_Fabs(double x){return std::fabs(x);}double Math_Fmin(double a,double b){return std::fmin(a,b);}double Math_Fmax(double a,double b){return std::fmax(a,b);}
std::vector<int> calls;float* rayStart;float* rayEnd;bool realGeometry,collisionFlag;int hitFace;
'''
DISPATCH=r'''
unsigned int CollisionDetectLineToFace(float* start,float* end,int polygon,float* a,float* b,float* c,float* d,float* normal,char collision){
 Require(start==rayStart&&end==rayEnd);Require(reinterpret_cast<uintptr_t>(a)>=reinterpret_cast<uintptr_t>(vertices)&&reinterpret_cast<uintptr_t>(a)<reinterpret_cast<uintptr_t>(vertices+3*15000*3));
 int vertex=int((a-vertices)/3),mesh=vertex/15000,triangle=(vertex%15000)/4;Require(triangle<2);int base=(mesh*15000+triangle*4)*3;
 Require(polygon==(triangle?4:3)&&a==vertices+base&&b==vertices+base+3&&c==vertices+base+(triangle?9:6)&&d==vertices+base+(triangle?6:9)&&normal[0]==0&&normal[1]==0&&normal[2]==1&&collision==char(collisionFlag));calls.push_back(mesh*10+triangle);
#if INTPTR_MAX == INT32_MAX
 if(realGeometry)return RealFace(start,end,polygon,a,b,c,d,normal,collision);
#endif
 return mesh*2+triangle==hitFace;
}
'''
MAIN=r'''
template<class T>void Put(BYTE* p,int off,T value){std::memcpy(p+off,&value,sizeof(value));}
int main(){auto* model=static_cast<BYTE*>(Reserve(0x26000000));auto* meshes=static_cast<BYTE*>(Reserve(0x28000000));auto* triangles=static_cast<BYTE*>(Reserve(0x2a000000));Require(model&&meshes&&triangles);std::memset(model,0x6d,188);std::memset(meshes,0x6d,120);std::memset(triangles,0x6d,216);Put(model,40,DWORD(reinterpret_cast<uintptr_t>(meshes)));
 const float positions[4][3]={{0,0,0},{10,0,0},{0,10,0},{10,10,0}};for(int m=0;m<3;++m){Put(meshes,m*40+28,DWORD(reinterpret_cast<uintptr_t>(triangles+m*72)));for(int t=0;t<2;++t){BYTE* tri=triangles+m*72+t*36;tri[0]=BYTE(t?4:3);short order[4]={0,1,short(t?3:2),short(t?2:3)};for(int k=0;k<4;++k){Put(tri,2+k*2,short(t*4+order[k]));std::memcpy(vertices+(m*15000+t*4+k)*3,positions[k],12);}}}
 unsigned cases=0;std::uint64_t hash=14695981039346656037ull;float start[3]={2,2,10},end[3]={4,5,-10};rayStart=start;rayEnd=end;realGeometry=false;
 for(short count:{short(-1),short(0),short(1),short(3)})for(int pattern=0;pattern<27;++pattern)for(int skipM:{-1,0,1,2})for(int skipT:{-1,0,1})for(int hit=-1;hit<6;++hit)for(bool flag:{false,true}){
  Put(model,36,count);int n=pattern;int counts[3];for(int m=0;m<3;++m){counts[m]=n%3;n/=3;Put(meshes,m*40+10,short(counts[m]));}hitFace=hit;collisionFlag=flag;calls.clear();std::vector<int> expected;bool expectedHit=false;
  for(int m=0;m<count&&!expectedHit;++m)for(int t=0;t<counts[m];++t){if(m==skipM&&t==skipT)continue;expected.push_back(m*10+t);if(m*2+t==hit){expectedHit=true;break;}}
  BYTE modelBefore[188],meshBefore[120],triBefore[216];std::memcpy(modelBefore,model,188);std::memcpy(meshBefore,meshes,120);std::memcpy(triBefore,triangles,216);
  bool result=BMD__CollisionDetectLineToMesh(DWORD(reinterpret_cast<uintptr_t>(model)),start,end,flag,skipM,skipT);Require(result==expectedHit&&calls==expected&&std::memcmp(modelBefore,model,188)==0&&std::memcmp(meshBefore,meshes,120)==0&&std::memcmp(triBefore,triangles,216)==0);for(int c:calls){hash^=unsigned(c);hash*=1099511628211ull;}hash^=unsigned(result);hash*=1099511628211ull;++cases;
 }
 Require(cases==18144);unsigned rays=0;
#if INTPTR_MAX == INT32_MAX
 realGeometry=true;Put(model,36,short(1));Put(meshes,10,short(2));
 for(int triangle=0;triangle<2;++triangle)for(int scenario=0;scenario<4;++scenario)for(bool flag:{false,true}){
  start[0]=end[0]=scenario==1?20.f:triangle?8.f:2.f;start[1]=end[1]=triangle?8.f:2.f;start[2]=scenario==2?-10.f:10.f;end[2]=-start[2];float limit=scenario==3?.25f:1.f;DAT_083a4120=limit;DAT_083a4130=DAT_083a4134=DAT_083a4138=999;collisionFlag=flag;calls.clear();
  bool result=BMD__CollisionDetectLineToMesh(DWORD(reinterpret_cast<uintptr_t>(model)),start,end,flag,0,1-triangle);bool wanted=scenario==0;Require(result==wanted);if(wanted&&flag)Require(DAT_083a4120==.5f&&DAT_083a4130==start[0]&&DAT_083a4134==start[1]&&DAT_083a4138==0);else Require(DAT_083a4120==limit&&DAT_083a4130==999&&DAT_083a4134==999&&DAT_083a4138==999);++rays;
 }
#endif

#if INTPTR_MAX == INT32_MAX
 Put(model,36,short(1));Put(meshes,10,short(1));pickingStart[0]=pickingEnd[0]=2;pickingStart[1]=pickingEnd[1]=2;pickingStart[2]=10;pickingEnd[2]=-10;rayStart=pickingStart;rayEnd=pickingEnd;realGeometry=true;collisionFlag=true;DAT_083a4120=1;DAT_083a4130=DAT_083a4134=DAT_083a4138=999;calls.clear();Require(PickingCall(model)&&DAT_083a4120==.5f&&DAT_083a4130==2&&DAT_083a4134==2&&DAT_083a4138==0&&calls.size()==1&&calls[0]==0);++rays;
#endif
 std::printf("BMD collision:18144 contratos,%u rayos reales %016llx\n",rays,(unsigned long long)hash);}
'''
def harness(revision=None):
 math=extract(read('src/Math/Math_LegacyTransforms.cpp',revision),'FaceNormalize');point=extract(read('src/Scene/Scene_CharSelect_Nav.cpp',revision),'Collision_PointInPolygon');face=extract(read('src/Terrain/Terrain_RayCollision.cpp',revision),'CollisionDetectLineToFace').replace('CollisionDetectLineToFace(','RealFace(',1)
 caller=read('src/Scene/Scene_ObjectInteraction.cpp',revision);start=caller.index('BMD__CollisionDetectLineToMesh(');end=caller.index('))',start)+1;expression=caller[start:end]
 expression=re.sub(r'(BMD__CollisionDetectLineToMesh\(\s*)model,',r'\1AsModel(model),',expression)
 wrapper='\n#if INTPTR_MAX == INT32_MAX\nbool PickingCall(void* model){return '+expression+';}\n#endif\n'
 from AnimationBonesPointer import layouts
 from ObjectLayoutContract import record
 body=extract(read('src/Render/BMD_Collision.cpp',revision),'BMD__CollisionDetectLineToMesh')
 code=re.sub(r'\b188\b','sizeof(BMD)',MAIN);code=re.sub(r'\b120\b','(3*sizeof(ClothModelMesh))',code).replace('m*40','m*sizeof(ClothModelMesh)')
 code=code.replace('Put(model,40,DWORD(reinterpret_cast<uintptr_t>(meshes)));','reinterpret_cast<BMD*>(model)->Meshs=meshes;')
 code=code.replace('Put(meshes,m*sizeof(ClothModelMesh)+28,DWORD(reinterpret_cast<uintptr_t>(triangles+m*72)));','reinterpret_cast<ClothModelMesh*>(meshes)[m].Triangles=triangles+m*72;')
 if 'BMD* pThis' in body:code=code.replace('DWORD(reinterpret_cast<uintptr_t>(model))','reinterpret_cast<BMD*>(model)')
 code=code.replace('#if INTPTR_MAX == INT32_MAX','').replace('#endif','')
 old='int main(){auto* model=static_cast<BYTE*>(Reserve(0x26000000));auto* meshes=static_cast<BYTE*>(Reserve(0x28000000));auto* triangles=static_cast<BYTE*>(Reserve(0x2a000000));'
 assert old in code
 code=code.replace(old,'std::uint64_t Run(BYTE* model,BYTE* meshes,BYTE* triangles,float* world){vertices=world;')
 code=code.replace('std::printf("BMD collision:18144 contratos,%u rayos reales %016llx\\n",rays,(unsigned long long)hash);','Require(rays==17);return hash;')
 main=r'''int main(){auto* low=static_cast<BYTE*>(Reserve(0x26000000));Require(low);auto expected=Run(low,low+1024,low+2048,reinterpret_cast<float*>(low+65536));if(sizeof(void*)==8){auto* high=static_cast<BYTE*>(Reserve(0x126000000ull));Require(high);for(unsigned mask=1;mask<16;++mask)Require(Run((mask&1?high:low),(mask&2?high:low)+1024,(mask&4?high:low)+2048,reinterpret_cast<float*>((mask&8?high:low)+65536))==expected);}std::printf("BMD collision:18144 contratos,17 rayos reales por ruta %016llx\n",(unsigned long long)expected);}'''
 pre=PREAMBLE.replace('65536','1048576').replace('float vertices[3*15000*3];float& DAT_0584621c=vertices[0];','float* vertices;\n#define DAT_0584621c (*vertices)\n')
 wrapper=wrapper.replace('#if INTPTR_MAX == INT32_MAX','').replace('#endif','')
 dispatch=DISPATCH.replace('#if INTPTR_MAX == INT32_MAX','').replace('#endif','')
 return pre+layouts()+'\n#pragma pack(push,1)\n'+record(read('src/Physics/ClothTypes.h'),'ClothModelMesh')+'\n#pragma pack(pop)\n'+math+point+face+dispatch+body+wrapper+code+main

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='Win32' if os.name=='nt' else 'x64');p.add_argument('--generator');p.add_argument('--reject-reference');args=p.parse_args()
 with tempfile.TemporaryDirectory(prefix='mu-bmd-collision-') as temp:
  d=Path(temp);build_and_run(d/'current',harness(),args)
  if args.reject_reference:
   if os.name!='nt' or args.architecture!='Win32':p.error('Referencia anterior sólo en Win32')
   try:build_and_run(d/'reference',harness(args.reject_reference),args)
   except subprocess.CalledProcessError as error:
    if error.returncode!=1 or 'mass_teleport_test.exe' not in str(error.cmd):raise
    print('Regresión reproducida en '+args.reject_reference+' por aserción de contrato')
   else:raise RuntimeError('La referencia anterior no reprodujo la regresión')
if __name__=='__main__':main()
