#!/usr/bin/env python3
"""Ciclo de tela real: altas, solver, dibujo, propietarios y las tres rutas de baja."""
import argparse
import re
import tempfile
from pathlib import Path
from SceneObjectPointer import build_and_run, read_source
from ObjectLayoutContract import record

SOURCES = {
    "src/Physics/Cloth_Simulation.cpp": "Vec3_Length SpringNode_Delta Cloth_SpringRange Cloth_SpringEqual ClothNode_ApplyForces VerletSystem_Flush SpringNode_ZeroFields SpringNode_SetPos SpringNode_GetPos VerletNode_AddAccel ClothNode_Integrate ClothNode_AddCorrection SpringNode_Ctor VerletNode_SetTarget VerletNode_GetPos VerletNode_ZeroFields VerletNode_CtorBase VerletNode_CtorExt ClothAnchor_Ctor ClothAnchor_SetParams ClothAnchor_Collide Cloth_CollideAnchors Cloth_PinTopRow Cloth_Wind Cloth_Integrate Spring_StoreEdge ClothAnchor_VSlot0 ClothAnchor_VSlot1",
    "src/UI/UI_LegacyWidgetSystem.cpp": "Widget_NodeInit Widget_CtorBase Widget_Release Widget_CheckState GridSpring_Create L_YGXPAXIHP6EX0_Z1_Z Cloth_VSlot0",
    "src/Net/Crypto.cpp": "VerletNode_AddToSystem Cloth_Solve Widget_Ctor SpringMesh_Create FUN_00408ff0 FUN_004090b0 FUN_004091d0",
    "src/Core/Runtime_Small.cpp": "FUN_00407970 FUN_00407de0 FUN_00407ec0 FUN_00407ea0",
    "src/Core/Runtime_Medium.cpp": "FUN_00408070",
    "src/UI/UI_LegacyWidgets.cpp": "FUN_00408680 FUN_0045aaa0_impl",
    "src/Entity/Entity_LegacyEffects.cpp": "Sound_UpdateChannel3D_Tick",
    "src/Entity/CharacterCloth.cpp": "ClothNew PrepareMeshClothVertices CharacterCloth_Tick",
    "src/Render/Render_LegacyBillboards.cpp": "DeleteCloth",
}

def function(source,name):
    source=re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S)
    found=list(re.finditer(r'^[^\n;{}]*\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',source,re.M))
    if len(found)!=1:raise ValueError(f"Definiciones de {name}: {len(found)}")
    start=found[0].start();brace=source.index('{',start);end=brace+1;depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

PREAMBLE = r'''
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
using BYTE=std::uint8_t;using WORD=std::uint16_t;using DWORD=std::uint32_t;using UINT=std::uint32_t;
#define __cdecl
#define __fastcall
#endif
using float10=long double;using vec3_t=float[3];using vec34_t=float[3][4];
void Require(bool x){if(!x){std::fputs("Contrato/canario/ciclo de tela incorrecto\n",stderr);std::abort();}}
'''

SERVICES = r'''
OBJECT* AsObject(void* p){return static_cast<OBJECT*>(p);}OBJECT* AsObject(uintptr_t p){return reinterpret_cast<OBJECT*>(p);}
CHARACTER* AsCharacter(void* p){return static_cast<CHARACTER*>(p);}CHARACTER* AsCharacter(uintptr_t p){return reinterpret_cast<CHARACTER*>(p);}
BMD* AsModel(uintptr_t p){return reinterpret_cast<BMD*>(p);} BMD* AsModel(void* p){return static_cast<BMD*>(p);}
struct Map{int world=0;int GetCurrentMap()const{return world;}}gMapManager;
struct Catalog{int EntityDrawModel(const void*,short type)const{return type;}}gContentCatalog;
struct EquipmentModelContext{explicit EquipmentModelContext(void*){}};
void ObjectPose_PrepareBones(OBJECT*,void*,float*,int){}
void ObjectPose_PrepareSimulation(OBJECT*,int){}
template<class... T>void Skeleton_Transform(T...){}
extern "C" const char* Guild_GetMarkName(int){return "guild";}
char GuildWarName[32]="rival",SoccerTeamName[2][32]={{0},{0}};
bool EnableSoccer=false;int SceneFlag=5;DWORD MoveSceneFrame;
uintptr_t DAT_05828d58,DAT_07abf5d8;BYTE DAT_05826d33;
float DAT_0584621c[15000*4*3];
float g_BoneScratch[16000],g_AttackEffectMatrix_04D[12],g_AttackEffectMatrix_04D_Alt[12],g_AttackEffectMatrix_04D_Aux[12];
DWORD DAT_00552508;
float RenderInterpolation_Alpha(){return .375f;}
bool RenderInterpolation_OwnerContinuous(const void* p){Require(p!=nullptr);return true;}
float RenderBlend(float a,float b,float t){return a+(b-a)*t;}
void AngleMatrix(float*,float(*out)[4]){std::memset(out,0,48);out[0][0]=out[1][1]=out[2][2]=1;}
void BMD_TransformPosition(void* model,float* m,float* p,float* out,int){
    BMD* b=AsModel(model);
    for(int i=0;i<3;++i)out[i]=p[0]*m[i*4]+p[1]*m[i*4+1]+p[2]*m[i*4+2]+m[i*4+3]+b->BodyOrigin[i];
}
alignas(16) static BYTE arena[8*1024*1024];
struct Allocation{BYTE* p;std::size_t size;bool live;};std::vector<Allocation> allocations;std::size_t cursor;
std::uint64_t lifetimeDigest=14695981039346656037ull;unsigned allocated,freed;
void LifetimeEvent(std::uint32_t value){for(unsigned i=0;i<4;++i){lifetimeDigest^=(value>>(i*8))&255;lifetimeDigest*=1099511628211ull;}}
void* operator_new(std::size_t n){
    cursor=(cursor+15)&~std::size_t(15);Require(cursor+n+32<sizeof(arena));
    BYTE* p=arena+cursor+16;std::memset(p-16,0xc3,n+32);std::memset(p,0xa5,n);
    LifetimeEvent(0x10000000u+static_cast<unsigned>(allocations.size()));++allocated;
    allocations.push_back({p,n,true});cursor+=n+32;return p;
}
void operator_delete(void* p){
    if(!p)return;
    auto it=std::find_if(allocations.begin(),allocations.end(),[&](const Allocation& a){return a.p==p;});
    Require(it!=allocations.end()&&it->live);it->live=false;
    LifetimeEvent(0x20000000u+static_cast<unsigned>(it-allocations.begin()));++freed;
    for(unsigned i=0;i<16;++i)Require(it->p[int(i)-16]==0xc3&&it->p[it->size+i]==0xc3);
}
void ResetArena(){for(auto& a:allocations)Require(!a.live);allocations.clear();cursor=0;}
std::uint64_t digest=14695981039346656037ull;unsigned vertices,binds;
void Hash(const void* p,std::size_t n){auto* b=static_cast<const BYTE*>(p);while(n--){digest^=*b++;digest*=1099511628211ull;}}
void GL_ResetState(){}void GL_SetBlendSrcOver(char){}void GL_BindTextureSlot(int t){++binds;Hash(&t,4);}
namespace BufferedDraw{
enum class Primitive{Quads};void Begin(Primitive){}void End(){}void Color3f(float,float,float){}
void TexCoord2f(float a,float b){Hash(&a,4);Hash(&b,4);}void Vertex3f(float a,float b,float c){++vertices;Hash(&a,4);Hash(&b,4);Hash(&c,4);}
}
'''

MAIN = r'''
static CHARACTER owner;static BMD models[512];static vec34_t bones[32];
static ClothModelMesh meshes[3];static BYTE meshVertices[4*16],triangles[2*36];
void State(ClothWidget* w){
    Require(w->Owner==&owner.Object);if(sizeof(void*)==8){Require(reinterpret_cast<uintptr_t>(w)>0xffffffffull);Require(reinterpret_cast<uintptr_t>(w->Owner)>0xffffffffull);Require(reinterpret_cast<uintptr_t>(w->Head)>0xffffffffull);Require(reinterpret_cast<uintptr_t>(w->Tail)>0xffffffffull);if(w->NodeCount)Require(reinterpret_cast<uintptr_t>(w->Nodes)>0xffffffffull);if(w->SpringCount)Require(reinterpret_cast<uintptr_t>(w->Springs)>0xffffffffull);}
    Hash(&w->Flags,4);Hash(&w->NodeCount,4);Hash(&w->SpringCount,4);Hash(&w->AnchorCount,4);
    for(int i=0;i<w->NodeCount;++i){auto& n=w->Nodes[i];Hash(n.Acceleration,36);Hash(&n.Flags,1);Hash(&n.CorrectionCount,4);Hash(n.Correction,12);}
    for(int i=0;i<w->SpringCount;++i){auto& s=w->Springs[i];Hash(&s.A,12);Hash(&s.Flags,1);}
    unsigned count=0;ClothLink* previous=w->Head;
    for(ClothLink* e=w->Head->Next;e!=w->Tail;e=e->Next){Require(e&&e->Previous==previous&&++count<16);if(sizeof(void*)==8)Require(reinterpret_cast<uintptr_t>(e)>0xffffffffull&&reinterpret_cast<uintptr_t>(e->Value)>0xffffffffull);previous=e;}
    Require(w->Tail->Previous==previous&&count==unsigned(w->AnchorCount));
}
ClothWidget* NewWidget(bool prefix=true){
    if(!prefix)return static_cast<ClothWidget*>(Widget_CtorBase(operator_new(sizeof(ClothWidget))));
    auto* raw=static_cast<std::uint32_t*>(operator_new(sizeof(ClothWidget)+4));*raw=1;
    return static_cast<ClothWidget*>(Widget_CtorBase(raw+1));
}
void OwnerReset(){
    std::memset(&owner,0,sizeof(owner));owner.Object.Type=390;owner.Object.Scale=1;owner.Object.EnableBoneMatrix=1;owner.Object.BoneMatrices=bones;owner.GuildMarkIndex=-1;
    for(auto& p:owner.BodyPart)p.Type=-1;
    for(auto& b:bones){std::memset(b,0,sizeof(b));b[0][0]=b[1][1]=b[2][2]=1;b[0][3]=2;b[1][3]=-3;b[2][3]=5;}
    DAT_05828d58=reinterpret_cast<uintptr_t>(models);DAT_07abf5d8=reinterpret_cast<uintptr_t>(&owner);
    for(auto& m:models){m.BodyScale=1;m.BodyOrigin[0]=3;m.BodyOrigin[1]=-2;m.BodyOrigin[2]=1;m.Meshs=reinterpret_cast<BYTE*>(meshes);}
    std::memset(meshes,0,sizeof(meshes));for(auto& m:meshes){m.VertexCount=4;m.TriangleCount=2;m.Vertices=meshVertices;m.Triangles=triangles;}
    for(unsigned i=0;i<4;++i){short bone=short(i%2?18:2);std::memcpy(meshVertices+i*16,&bone,2);}
    short a[4]={3,0,1,2},b[4]={3,0,2,3};std::memcpy(triangles,a,8);std::memcpy(triangles+36,b,8);
    for(unsigned i=0;i<sizeof(DAT_0584621c)/sizeof(float);++i)DAT_0584621c[i]=float(int(i%13)-6);
}
void Draw(ClothWidget* w){
    std::vector<BYTE> before(reinterpret_cast<BYTE*>(w->Nodes),reinterpret_cast<BYTE*>(w->Nodes)+sizeof(ClothNode)*w->NodeCount);
    FUN_00408ff0(w);Require(std::memcmp(before.data(),w->Nodes,before.size())==0);
}
int main(){
    unsigned cases=0;
    // Los writers x86 incrementan DWORD; Flush interpreta el mismo contador con signo.
    ClothNode edge{};float noCorrection[3]={0,0,0};edge.CorrectionCount=0x7fffffffu;
    ClothNode_AddCorrection(reinterpret_cast<uintptr_t>(&edge),noCorrection);
    Require(edge.CorrectionCount==0x80000000u);
    Require(VerletSystem_Flush(reinterpret_cast<uintptr_t>(&edge))==0&&edge.CorrectionCount==0x80000000u);
    edge.CorrectionCount=0xffffffffu;ClothNode_AddCorrection(reinterpret_cast<uintptr_t>(&edge),noCorrection);
    Require(edge.CorrectionCount==0);
    OwnerReset();owner.Object.BoneMatrices=nullptr;
    ClothWidget guard{};ClothNode guardedNode{};guard.Owner=&owner.Object;guard.Nodes=&guardedNode;guard.NodeCount=1;
    const ClothNode unchanged=guardedNode;
    Cloth_Integrate(reinterpret_cast<int*>(&guard),.005f);
    Require(Cloth_Solve(reinterpret_cast<DWORD*>(&guard))==1&&std::memcmp(&unchanged,&guardedNode,sizeof(unchanged))==0);
    Require(Widget_CheckState(nullptr,0x3ba3d70a,5)==1&&Widget_CheckState(reinterpret_cast<int*>(&guard),0x3ba3d70a,0)==1);
    ResetArena();OwnerReset();owner.Object.ClothCount=7;
    DELETE_OWNER
    Require(owner.Object.ClothCount==7);++cases;
    for(unsigned flags:{0u,4u,0x100u,0x400u,0x1100u,0x1400u,1029u,4097u})for(int anchors:{0,1,4}){
        ResetArena();OwnerReset();ClothWidget* w=NewWidget();owner.Object.Cloth=w;owner.Object.ClothCount=1;
        CREATE_GRID
        for(int i=0;i<anchors;++i)VerletNode_AddToSystem(w,float(i*3),-15,-20,30,2);
        for(int frame=0;frame<5;++frame){MoveSceneFrame=frame*11;ClothPresentation_BeginStep();int alive=Widget_CheckState(w,0x3ba3d70a,5);Hash(&alive,4);State(w);Draw(w);if(!alive)break;}
        DELETE_OWNER
        Require(!owner.Object.Cloth&&!owner.Object.ClothCount);ResetArena();++cases;
    }
    {
        ResetArena();OwnerReset();const unsigned flags=0x1400;
        auto* raw=static_cast<std::uint32_t*>(operator_new(2*sizeof(ClothWidget)+4));*raw=2;
        auto* base=reinterpret_cast<ClothWidget*>(raw+1);owner.Object.Cloth=base;owner.Object.ClothCount=2;
        for(int i=0;i<2;++i){auto* w=static_cast<ClothWidget*>(Widget_CtorBase(base+i));CREATE_GRID
            VerletNode_AddToSystem(w,0,-10,0,30,2);State(w);}
        DELETE_OWNER
        ResetArena();++cases;
    }
    for(int route=0;route<3;++route){
        ResetArena();OwnerReset();ClothWidget* w=NewWidget(route==0);CREATE_GRID
        if(route==0){owner.Object.Cloth=w;owner.Object.ClothCount=1;DELETE_OWNER}
        else if(route==1){owner.BodyPart[2].Cloth=w;DELETE_PARTS}
        else {owner.Weapon[0].Cloth=w;DELETE_SINGLE}
        Require(!owner.Object.Cloth&&!owner.BodyPart[2].Cloth&&!owner.Weapon[0].Cloth);ResetArena();++cases;
    }
    for(int meshIndex:{0,2}){
        ResetArena();OwnerReset();auto* raw=static_cast<std::uint32_t*>(operator_new(sizeof(ClothMeshWidget)+4));*raw=1;
        auto* w=static_cast<ClothWidget*>(Widget_Ctor(raw+1));owner.Object.Cloth=w;owner.Object.ClothCount=1;
        CREATE_MESH
        VerletNode_AddToSystem(w,0,-20,0,30,18);State(w);ClothPresentation_BeginStep();int alive=Widget_CheckState(w,0x3ba3d70a,5);Hash(&alive,4);State(w);Draw(w);DELETE_OWNER
        ResetArena();++cases;
    }
    for(int monster:{0,55,71,74,77,89,95,112,118,124,130,136})for(int kind:{0,3})for(int pants:{-1,706}){
        ResetArena();OwnerReset();owner.MonsterIndex=BYTE(monster);owner.Class=BYTE(kind);owner.BodyPart[1].Type=short(pants);
        for(unsigned frame=0;frame<3;++frame){MoveSceneFrame=frame;ClothPresentation_BeginStep();TICK_CHARACTER
            if(owner.Object.Cloth){auto* w=static_cast<ClothWidget*>(owner.Object.Cloth);State(w);Draw(w);}
            for(auto& p:owner.BodyPart)if(p.Cloth){auto* w=static_cast<ClothWidget*>(p.Cloth);State(w);Draw(w);}
        }
        DELETE_OWNER
        ResetArena();++cases;
    }
    // Oráculo capturado del código x86 7d4da9f5 con los mismos servicios controlados.
    Require(cases==79&&vertices==78096&&binds==492&&digest==0x0c7414cc038659adull);
    Require(allocated==1046&&freed==1046&&lifetimeDigest==0x6ae9aaf04bc62ea5ull);
    std::printf("ClothRuntime: %u ciclos reales, vertices=%u binds=%u, eventos/estado %016llx, altas/bajas=%u/%u orden=%016llx\n",cases,vertices,binds,(unsigned long long)digest,allocated,freed,(unsigned long long)lifetimeDigest);
}
'''


def harness(revision=None):
    units=[function(read_source(path,revision),name) for path,names in SOURCES.items() for name in names.split()]
    if revision:units.append(function(read_source("src/Entity/CharacterCloth.cpp",revision),"PtrAsFloatBits"))
    code="\n".join(units)
    header=read_source("src/Entity/EntityTypes.h",revision)
    types="\n#pragma pack(push,1)\n"+"\n".join(record(header,n) for n in ["OBJECT","PART_t","CHARACTER"])+"\n#pragma pack(pop)\n"
    if revision:
        types=types.replace("BYTE     _unk180[0x30];","BYTE ClothCount; BYTE pad181[3]; void* Cloth; BYTE pad188[40];")
        types=types.replace("BYTE     _unk14[4];","void* Cloth;")
    model=record(read_source("src/Model/ModelTypes.h",revision),"BMD")
    types+="\n#pragma pack(push,1)\n"+model+"\n#pragma pack(pop)\n"
    cloth=read_source("src/Physics/ClothTypes.h").replace("#pragma once","")
    declarations="\n".join(u[:u.index("{")].strip()+";" for u in units)
    globals_h=read_source("src/globals.h",revision);globals_cpp=read_source("src/globals.cpp",revision)
    constants=""
    for name in sorted(set(re.findall(r'\b_?DAT_[0-9a-f]+',code))):
        if name in SERVICES:continue
        m=re.search(rf'^[^/\n]*\b{name}\s*=\s*[^;]+;',globals_cpp,re.M)
        if not m:raise RuntimeError("Global sin servicio: "+name)
        constants+=m.group()+"\n"
    presentation=read_source("src/Entity/ClothPresentation.cpp",revision)
    presentation=presentation[presentation.index("namespace {"):]
    decl_presentation="void ClothPresentation_BeginStep();void ClothPresentation_Forget(const void*);void ClothPresentation_Before(void*);void ClothPresentation_After(void*);void ClothPresentation_Vertex(const void*,int,float*);\n"
    tables=""
    for path,names in [("src/Physics/Cloth_Simulation.cpp",["g_ClothNodeVTable","g_ClothAnchorVTable"]),("src/UI/UI_LegacyWidgetSystem.cpp",["g_ClothVTable"])]:
        text=read_source(path,revision)
        for name in names:
            tables+=re.search(r'(?:static\s+)?void\s*\*\s*'+name+r'\[[^\]]*\]\s*=\s*\{.*?\};',text,re.S).group()+"\n"
    main=MAIN.replace("Widget_CheckState(w,","Widget_CheckState(reinterpret_cast<int*>(w),")
    main=main.replace("CREATE_GRID","GridSpring_Create(w,&owner.Object,2,10.f,0.f,5,6,45.f,85.f,1276,1276,int(flags));")
    # Las rutas de liberación usan un patrón de flags fijo cuando no pertenecen al primer bucle.
    main=main.replace("for(int route=0;route<3;++route){","for(int route=0;route<3;++route){ const unsigned flags=0x1400;")
    main=main.replace("CREATE_MESH","SpringMesh_Create(w,&owner.Object,meshIndex,2,0x400,-1);")
    main=main.replace("DELETE_OWNER","DeleteCloth(&owner,&owner.Object,nullptr);").replace("DELETE_PARTS","DeleteCloth(&owner,nullptr,nullptr);").replace("DELETE_SINGLE","DeleteCloth(nullptr,nullptr,&owner.Weapon[0]);").replace("TICK_CHARACTER","CharacterCloth_Tick(&owner);")
    if revision:
        main="float OwnerBits(OBJECT* p){float f;auto v=static_cast<std::uint32_t>(reinterpret_cast<uintptr_t>(p));std::memcpy(&f,&v,4);return f;}\n"+main
        main=main.replace("GridSpring_Create(w,&owner.Object,","GridSpring_Create(w,OwnerBits(&owner.Object),").replace("SpringMesh_Create(w,&owner.Object,meshIndex,","SpringMesh_Create(w,int(reinterpret_cast<uintptr_t>(&owner.Object)),reinterpret_cast<short*>(uintptr_t(meshIndex)),")
        main=main.replace("DeleteCloth(&owner,&owner.Object,nullptr);","DeleteCloth(int(reinterpret_cast<uintptr_t>(&owner)),int(reinterpret_cast<uintptr_t>(&owner.Object)),0);").replace("DeleteCloth(&owner,nullptr,nullptr);","DeleteCloth(int(reinterpret_cast<uintptr_t>(&owner)),0,0);").replace("DeleteCloth(nullptr,nullptr,&owner.Weapon[0]);","DeleteCloth(0,0,int(reinterpret_cast<uintptr_t>(&owner.Weapon[0])));").replace("CharacterCloth_Tick(&owner);","CharacterCloth_Tick(int(reinterpret_cast<uintptr_t>(&owner)));")
    return PREAMBLE+types+cloth+SERVICES+constants+declarations+decl_presentation+tables+code+presentation+main


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--architecture",choices=("Win32","x64"),default="x64");parser.add_argument("--generator");parser.add_argument("--reference-ref")
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="mu-cloth-runtime-") as temp:
        result=build_and_run(Path(temp)/"current",harness(),args);print(result,end="")
        if args.reference_ref:
            reference=build_and_run(Path(temp)/"reference",harness(args.reference_ref),args)
            if result!=reference:raise RuntimeError(f"Diferencia con referencia: {result!r} / {reference!r}")
            print("Equivalencia con "+args.reference_ref+": OK")


if __name__=="__main__":main()
