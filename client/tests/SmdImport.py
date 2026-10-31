#!/usr/bin/env python3
"""Importador SMD productivo: filesystem real, conversión, bounds y animación."""
import argparse,json,re,tempfile
from pathlib import Path
from GatePointer import PREAMBLE
from SceneObjectPointer import ROOT,read_source,function,build_and_run

SERVICES=r'''
uintptr_t DAT_05828d58;FILE* SMDFile;
namespace ResourceTables {std::size_t ModelCount(){return 4;}void EnsureModels(std::size_t n){Require(n<=4);}}
namespace Platform {struct FS{FILE* Open(const char* p,const char* m){return std::fopen(p,m);}};FS& GetFileSystem(){static FS fs;return fs;}}
'''
MAIN=r'''
#include <sstream>
#include <iomanip>
bool Near(float a,float b){return std::fabs(a-b)<0.00001f;}
void Free(BMD& model){Smd::Forget(&model);auto* bones=reinterpret_cast<Smd::Bone*>(model.Bones);auto* meshes=reinterpret_cast<Smd::Mesh*>(model.Meshs);auto* actions=reinterpret_cast<Smd::Action*>(model.Actions);
 for(int b=0;b<model.NumBones;++b){for(int a=0;a<model.NumActions;++a){auto& k=bones[b].Actions[a];std::free(k.Positions);std::free(k.Angles);std::free(k.Quaternions);}std::free(bones[b].Actions);}
 for(int i=0;i<model.NumMeshs;++i){std::free(meshes[i].Vertices);std::free(meshes[i].Normals);std::free(meshes[i].UVs);std::free(meshes[i].Faces);std::free(meshes[i].Script);}for(int a=0;a<model.NumActions;++a)std::free(actions[a].Positions);
 std::free(model.Bones);std::free(model.Meshs);std::free(model.Actions);std::free(model.Textures);std::free(model.IndexTexture);model={};}
bool Text(const std::string& text,int kind=0,bool flip=false){auto* f=std::tmpfile();Require(f);Require(std::fwrite(text.data(),1,text.size(),f)==text.size());std::rewind(f);bool r=Smd::Read(f,kind,flip);std::fclose(f);return r;}
std::string Basic(const std::string& pose="0 0 0 0 0 0 0\n"){return "version 1\nnodes\n0 \"Root\" -1\nend\nskeleton\ntime 0\n"+pose+"end\ntriangles\nmat.tga\n0 1 0 0 1 0 0 0 0\n0 0 1 0 1 0 0 1 0\n0 0 0 1 1 0 0 0 1\nend\n";}
void Hierarchies(){
 using V=std::array<double,3>;using M=std::array<V,3>;
 auto rotation=[](V a){double x=a[0],y=a[1],z=a[2];double sx=std::sin(x),cx=std::cos(x),sy=std::sin(y),cy=std::cos(y),sz=std::sin(z),cz=std::cos(z);return M{{{{cy*cz,sx*sy*cz-cx*sz,cx*sy*cz+sx*sz}},{{cy*sz,sx*sy*sz+cx*cz,cx*sy*sz-sx*cz}},{{-sy,sx*cy,cx*cy}}}};};
 auto transform=[](M m,V v){V out{};for(int i=0;i<3;++i)for(int j=0;j<3;++j)out[i]+=m[i][j]*v[j];return out;};
 for(int trial=0;trial<96;++trial){V a{{(trial%7-3)*.31,(trial%11-5)*.17,(trial%13-6)*.23}},b{{(trial%5-2)*.41,(trial%3-1)*.29,(trial%17-8)*.11}};auto root=rotation(a),child=rotation(b);M world{};for(int y=0;y<3;++y)for(int x=0;x<3;++x)for(int k=0;k<3;++k)world[y][x]+=root[y][k]*child[k][x];V origin=transform(root,V{{4,5,6}});for(int i=0;i<3;++i)origin[i]+=i+1;V n=transform(world,V{{.5,1,1.5}});
  std::ostringstream text;text<<std::fixed<<std::setprecision(9)<<"version 1\nnodes\n0 \"Root\" -1\n1 \"Child\" 0\nend\nskeleton\ntime 0\n0 1 2 3 ";for(auto v:a)text<<v<<' ';text<<"\n1 4 5 6 ";for(auto v:b)text<<v<<' ';text<<"\nend\ntriangles\nmat.tga\n";
  const V local[]={{1,0,0},{0,1,0},{0,0,1}};for(auto v:local){auto p=transform(world,v);text<<"1 ";for(int i=0;i<3;++i)text<<p[i]+origin[i]<<' ';for(auto q:n)text<<q<<' ';text<<"0 0\n";}text<<"end\n";
  Require(Text(text.str()) && Smd::Fixup());auto& mesh=Smd::Current().Meshes[0];Require(mesh.Vertices.size()==3 && mesh.Normals.size()==1);for(int j=0;j<3;++j)for(int i=0;i<3;++i)Require(std::fabs(mesh.Vertices[j].Position[i]-local[j][i])<.00002);for(int i=0;i<3;++i)Require(std::fabs(mesh.Normals[0].Direction[i]-(i+1)/std::sqrt(14.0))<.00002);
 }
}
void Limits(){
 auto good=Basic();unsigned truncated=0;for(std::size_t n=0;n<good.rfind("end")+3;++n){Require(!Text(good.substr(0,n)));++truncated;}
 Require(!Text("// comentario sin salto final"));Require(!Text(std::string("version 1\nnodes\n0 \"")+std::string(32,'a')+"\" -1\nend\n"));
 std::ostringstream nodes;nodes<<"version 1\nnodes\n";for(int i=0;i<201;++i)nodes<<i<<" \"bone\" -1\n";nodes<<"end\n";Require(!Text(nodes.str()));
 const std::string head="version 1\nnodes\n0 \"Root\" -1\nend\nskeleton\ntime 0\n0 0 0 0 0 0 0\nend\ntriangles\n";
 const std::string corners="0 1 0 0 1 0 0 0 0\n0 0 1 0 1 0 0 0 0\n0 0 0 1 1 0 0 0 0\n";
 std::string materials=head;for(int i=0;i<101;++i)materials+="\"mat"+std::to_string(i)+".tga\"\n"+corners;Require(Text(materials+"end\n") && !Smd::Fixup());
 std::string triangles=head;for(int i=0;i<15001;++i)triangles+="mat.tga\n"+corners;Require(!Text(triangles+"end\n"));
 std::string vertices=head;for(int tri=0;tri<3334;++tri){vertices+="mat.tga\n";for(int j=0;j<3;++j)vertices+="0 "+std::to_string(tri*3+j)+" 0 0 1 0 0 0 0\n";}Require(Text(vertices+"end\n") && !Smd::Fixup());
 Require(Text(good));std::string frames="version 1\nskeleton\n";for(int i=0;i<101;++i)frames+="time "+std::to_string(i)+"\n0 0 0 0 0 0 0\n";Require(!Text(frames+"end\n",1));
 Require(Text(good));Require(!Text("version 1\nskeleton\ntime 0\n0 0 0 0 0 0 0\ntime 0\n0 0 0 0 0 0 0\nend\n",1));
 Require(Text("version 1\nnodes\nend\nskeleton\nend\ntriangles\nend\n"));BMD empty{};Require(Smd::BuildModel(&empty,0) && empty.NumBones==0 && empty.NumMeshs==0);Free(empty);
 std::printf("SMD: %u truncamientos y límites nodes/frames/materiales/triángulos/vértices, modelo vacío OK\n",truncated);
}
void CheckModel(BMD& m,bool flip){Require(m.NumBones==3 && m.NumMeshs==2 && m.NumActions==0 && m.NumLightMaps==0 && m.BoneHead==-1 && m.StreamMesh==-1);
 auto* meshes=reinterpret_cast<Smd::Mesh*>(m.Meshs);auto* bones=reinterpret_cast<Smd::Bone*>(m.Bones);Require(bones[0].Parent==-1 && bones[1].Parent==0 && std::strcmp(bones[1].Name,"Child bone")==0 && bones[2].Dummy==1);
 Require(bones[0].BoundsEnabled==1 && bones[1].BoundsEnabled==1 && bones[2].BoundsEnabled==0);
 const float maximum[]={1,1,1},minimum[]={0,0,0};for(int c=0;c<8;++c)for(int axis=0;axis<3;++axis)Require(bones[0].Bounds[c][axis]==((c&(1<<(2-axis)))?minimum[axis]:maximum[axis]));
 auto& a=meshes[0];Require(a.VertexCount==3 && a.NormalCount==1 && a.UVCount==3 && a.FaceCount==2 && a.Texture==0 && a.Script && a.Script[0]==1 && a.Script[1]==1 && a.Script[2]==0 && a.Script[3]==0);
 Require(std::strcmp(reinterpret_cast<char*>(m.Textures),"body_RH.tga")==0 && std::strcmp(reinterpret_cast<char*>(m.Textures)+32,"detail.tga")==0);
 const float want[3][3]={{1,0,0},{0,1,0},{0,0,1}};
 for(int i=0;i<3;++i){int j=flip?2-i:i;Require(a.Vertices[i].Bone==(j?1:0));for(int k=0;k<3;++k)Require(Near(a.Vertices[i].Position[k],want[j][k]));Require(a.Faces[0].Vertices[i]==i && a.Faces[1].Vertices[i]==i && a.Faces[0].Normals[i]==0);}
 Require(a.Faces[0].Count==3 && Near(a.Normals[0].Direction[2],1) && Near(a.UVs[0].V,flip?0:1));Require(meshes[1].NormalCount==1 && meshes[1].Normals[0].Direction[0]==0 && meshes[1].Normals[0].Direction[1]==0 && meshes[1].Normals[0].Direction[2]==0);
}
void Run(BYTE* table,char* path){
 DAT_05828d58=reinterpret_cast<uintptr_t>(table+16);auto* models=reinterpret_cast<BMD*>(table+16);
 for(unsigned flip=0;flip<2;++flip){std::memset(table,0x5a,4*sizeof(BMD)+32);models[1]={};std::vector<BYTE> before(table,table+4*sizeof(BMD)+32);std::strcpy(path,MODEL_PATH);auto* sentinel=std::tmpfile();Require(sentinel);SMDFile=sentinel;std::fputs("tablas",sentinel);std::rewind(sentinel);
  OpenSMDModel(1,path,2,static_cast<char>(flip));Require(SMDFile==sentinel && std::ftell(sentinel)==0);CheckModel(models[1],flip!=0);
  std::strcpy(path,ANIM_PATH);OpenSMDAnimation(1,path,1);auto& m=models[1];Require(m.NumActions==1);auto* a=reinterpret_cast<Smd::Action*>(m.Actions);auto* b=reinterpret_cast<Smd::Bone*>(m.Bones);Require(a[0].Frames==2 && a[0].Lock==1);float speed;std::memcpy(&speed,a[0].Flags+4,4);Require(speed==0.3f);
  const float delta[]={2,4,6,0,0,0};for(int i=0;i<6;++i)Require(Near(a[0].Positions[i],delta[i]));Require(Near(b[0].Actions[0].Quaternions[2],0) && Near(b[0].Actions[0].Quaternions[3],1) && Near(b[0].Actions[0].Quaternions[6],0.70710678f) && Near(b[0].Actions[0].Quaternions[7],0.70710678f));
  std::strcpy(path,"missing-smd-file.smd");OpenSMDAnimation(1,path,0);Require(m.NumActions==1);
  std::strcpy(path,ANIM_PATH);OpenSMDAnimation(1,path,0);Require(m.NumActions==2 && a[1].Lock==0);OpenSMDAnimation(1,path,1);Require(m.NumActions==2);
  Require(SMDFile==sentinel && std::fgetc(sentinel)=='t');std::fclose(sentinel);SMDFile=nullptr;
  std::memcpy(before.data()+16+sizeof(BMD),&m,sizeof(BMD));Require(std::memcmp(table,before.data(),before.size())==0);Free(m);
 }
}
int main(){auto* table=static_cast<BYTE*>(Reserve(0x26000000u));auto* names=static_cast<char*>(Reserve(0x24000000u));Require(table&&names);Run(table,names);
 if(sizeof(void*)>4){auto* high=static_cast<BYTE*>(Reserve(std::uint64_t(0x126000000ull)));auto* highNames=static_cast<char*>(Reserve(std::uint64_t(0x124000000ull)));Require(high&&highNames);for(unsigned mask=1;mask<4;++mask){std::memset(table,0x7b,65536);std::memset(names,0x6b,65536);Run(mask&1?high:table,mask&2?highNames:names);if(mask&1)for(int i=0;i<65536;++i)Require(table[i]==0x7b);if(mask&2)for(int i=0;i<65536;++i)Require(names[i]==0x6b);}}
 unsigned rejected=0;for(const std::string& text:{std::string(),std::string("version 2\n"),std::string("version 1\nnodes\n0 \"sin cierre"),std::string("version 1\nnodes\n0 \"Root\" 0\nend\n"),Basic().substr(0,90)}){Require(!Text(text));++rejected;}
 auto good=Basic();Require(Text(good) && Smd::Fixup());Require(!Text("version 1\nskeleton\ntime 2\n0 0 0 0 0 0 0\nend\n",1));++rejected;
 // Rotación no trivial: el punto global (1,0,0) queda aproximadamente en (0,-1,0).
 Require(Text(Basic("0 0 0 0 0 0 1.5707963\n")) && Smd::Fixup());const auto& mesh=Smd::Current().Meshes[0];Require(Near(mesh.Vertices[0].Position[0],0) && Near(mesh.Vertices[0].Position[1],-1) && Near(mesh.Normals[0].Direction[1],-1));
 std::puts("SMD: geometría, flip, materiales, deduplicación, animación, capacidad, archivo ausente y aislamiento OK");std::printf("SMD: %u rechazos y rotación analítica OK\n",rejected);
 Hierarchies();std::puts("SMD: 96 jerarquías con rotaciones XYZ contra oráculo analítico OK");
 Limits();
}
'''

def harness():
    header=read_source('src/Model/ModelTypes.h')
    bmd=re.search(r'struct BMD\s*\{.*?\n\};',header,re.S).group()
    prefix=PREAMBLE.split('char DAT_00559bb4',1)[0]+'\n#include <cmath>\nusing vec3_t=float[3];\n#pragma pack(push,1)\n'+bmd+'\n#pragma pack(pop)\nBMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);}\n'
    source=read_source('src/Render/SMD_Legacy.cpp')
    for h in ('stdafx.h','globals.h','functions.h','Render/ResourceTables.h'):source=source.replace('#include "'+h+'"','')
    source=source.replace('"Model/SmdImport.h"',json.dumps((ROOT/'src/Model/SmdImport.h').as_posix()))
    runtime=function(read_source('src/Core/Runtime_Medium.cpp'),'OpenSMDFile')
    wrapper='extern "C" bool __cdecl OpenSMDFile(const char* p,int t,char f){return OpenSMDFile(const_cast<char*>(p),t,f!=0);}\n'
    entry=read_source('src/Render/BMD_LegacyLoading.cpp')
    main=MAIN.replace('MODEL_PATH',json.dumps((ROOT/'tests/fixtures/smd/reference.smd').as_posix())).replace('ANIM_PATH',json.dumps((ROOT/'tests/fixtures/smd/walk.smd').as_posix()))
    return prefix+SERVICES+source+runtime+wrapper+function(entry,'OpenSMDModel')+function(entry,'OpenSMDAnimation')+main

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--architecture',choices=('Win32','x64'),default='x64');p.add_argument('--generator');a=p.parse_args()
    with tempfile.TemporaryDirectory(prefix='mu-smd-import-') as t:print(build_and_run(Path(t)/'current',harness(),a),end='')
if __name__=='__main__':main()
