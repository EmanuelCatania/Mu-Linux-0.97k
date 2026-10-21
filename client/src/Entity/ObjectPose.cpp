#include "stdafx.h"
#include "Entity/ObjectPose.h"
#include "Entity/ObjectBounds.h"
#include "functions.h"
#include "globals.h"
#include "Render/TransformWorkers.h"
#include "Render/TransformMath.h"
#include "Render/ResourceTables.h"
#include <array>
#include <vector>
#include <unordered_map>

// IDA: preparación de pose de Calc_RenderObject (0x004FAA70).
// El destino explícito permite preparar huesos SIM sin utilizar el buffer
// de la entidad que el render puede sobrescribir con una pose interpolada.
static void PrepareContext(int entity, void* model)
{
    *(DWORD*)((int)model + 0x68) = *(DWORD *)&AsObject(entity)->Scale;
    *(BYTE *)((int)model + 0xa0) = AsObject(entity)->CurrentAction;
    *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
    *(DWORD*)((int)model + 0x70) = *(DWORD *)&AsObject(entity)->Position[1];
    *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    // Desplazamiento Y especial para la elfa (0x14B) y su variante (0x14A).
    short cls = AsObject(entity)->Type;
    if (cls == 0x14b) {
        *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)((int)model + 0x70) = AsObject(entity)->Position[1] + _DAT_0055290c;
        *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    } else if (cls == 0x14a) {
        *(DWORD*)((int)model + 0x6c) = *(DWORD *)&AsObject(entity)->Position[0];
        *(float *)((int)model + 0x70) = AsObject(entity)->Position[1] + _DAT_00552908;
        *(DWORD*)((int)model + 0x74) = *(DWORD *)&AsObject(entity)->Position[2];
    }
}
static bool ReuseBones(int entity, void* model, float* bones, char translate);
void ObjectPose_PrepareBones(int entity, void* model, float* bones, char translate)
{
    PrepareContext(entity,model);
    if (ReuseBones(entity,model,bones,translate)) return;
    DWORD priorFrame;
    memcpy(&priorFrame, &AsObject(entity)->PriorAnimationFrame, sizeof(priorFrame));
    BMD_Animation(model, (int)bones, AsObject(entity)->AnimationFrame, priorFrame,
                  AsObject(entity)->PriorAction, (unsigned int*)(entity + 0x1c),
                  (float*)(entity + 0x28), '\0', translate == '\0');
}

// La caja normal no depende de mallas ni huesos. Sólo el editor 2 necesita
// transformar la geometría para obtener sus extremos; LOD 0 no dibuja.
void ObjectPose_PrepareBounds(int entity, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (o.Alpha < _DAT_005524f8) return;
    if (DAT_07e11d30 == 2) {
        Calc_RenderObject(entity, translate, 0);
        return;
    }
    float origin[3] = { o.Position[0], o.Position[1], o.Position[2] };
    if (o.Type == 0x14b) origin[1] += _DAT_0055290c;
    else if (o.Type == 0x14a) origin[1] += _DAT_00552908;
    ObjectBounds_Build(o.BoundingBoxMin, o.BoundingBoxMax, origin, o.OBB);
}

void ObjectPose_PrepareSimulation(int entity, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (o.Alpha < _DAT_005524f8) return;
    if (DAT_07e11d30 == 2) {
        Calc_RenderObject(entity, translate, 0);
        return;
    }
    void *model = (void *)(DAT_05828d58 + o.Type * 0xbc);
    *(float *)((char *)model + 0x84) = 0.0f;
    float *bones = o.EnableBoneMatrix ? (float *)o.BoneMatrices : (float *)&DAT_06970a9c;
    ObjectPose_PrepareBones(entity, model, bones, translate);
    ObjectPose_PrepareBounds(entity, translate);
}

// IDA: Draw_RenderObject (0x004FAE00), estatuas de Blood Castle.
// DESVIACION: fragmentos, audio y desactivación se producen en SIM, aun sin draw.
void ObjectPose_TickMeshEvents(int entity, void *model, float *bones, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (!o.Live || o.Alpha < _DAT_005524f8) return;
    const bool dead = o.CurrentAction == 6;
    const bool materializing = o.Type == 330 && (int)MoveSceneFrame - o.InitialSceneFrame < 25;
    if (!((o.Type == 331 && dead) || (o.Type == 330 && (dead || materializing)))) return;
    float bounds[15];
    Skeleton_Transform(model, (int)bones, o.BoundingBoxMin, o.BoundingBoxMax, bounds, translate);
    if (dead) o.Live = 0;
    PlayBuffer(106, 0, 0);
    BMD__RenderMeshEffect(model, 0, o.Type == 330 ? 260 : 262);
}

// IDA: BMD_Animation (0x00440060)
// Computes animated bone transforms for the current frame.
// param_1 = bone transform array (param_1[bone * 0x30] = 3x4 matrix per bone)
// param_2 = anim speed (float), param_3 = RGBA color, param_4 = current anim index,
// param_5 = RGBA ptr (root-bone position hint), param_6 = position offset float[3],
// param_7/8 = flags for root-bone translation.
// Output: bone quaternion+position stored in DAT_05826E18 (bone*0x10 stride).
//         Final bone matrices written to param_1 (3x4 per bone).
// DESVIACION: scratch explícito para calcular poses independientes en un lote.
// La entrada histórica de BMD conserva sus temporales y escrituras originales.
static void AnimateWithScratch(void* this_, int param_1, float param_2, unsigned int param_3,
                           unsigned char param_4, unsigned int* param_5,
                           float* param_6, char param_7, char param_8,
                           char* quaternionScratch, float* parentScratch) {
    int*  piModel = (int*)this_;
    // clamp anim indices to bone count
    short boneCount = *(short*)((char*)this_ + 0x26);
    if (boneCount <= (short)(unsigned short)param_4)
        param_4 = 0;
    if (boneCount <= (short)(unsigned short)*(unsigned char*)((char*)this_ + 0xa0))
        *(unsigned char*)((char*)this_ + 0xa0) = 0;
    // store current color/frame in model header
    *(unsigned int*)((char*)this_ + 0x78) = param_5[0];
    *(unsigned int*)((char*)this_ + 0x7c) = param_5[1];
    *(unsigned int*)((char*)this_ + 0x80) = param_5[2];
    *(float*)((char*)this_ + 0xa4) = param_2;
    // compute fractional frame and frame indices
    long long lv = (long long)(int)param_2;
    short curFrame = (short)lv;
    *(short*)((char*)this_ + 0xa8) = curFrame;
    float fFrac = param_2 - (float)(int)curFrame;
    float fFrac2 = _DAT_0055256c - fFrac;
    // param_3 es realmente `float PriorFrame` (IDA firma), no un entero. La firma
    // C nuestra lo declara `unsigned int` porque el caller en Calc_RenderObject lo
    // carga con *(DWORD*)(entity+0x10c) y los bits del float caben en un DWORD.
    // Hay que reinterpretar las bits → float y truncar a int
    // (== IDA: v12 = (std::int64_t)PriorFrame; v39 = v12).
    float priorFrame;
    memcpy(&priorFrame, &param_3, 4);
    int v39 = (int)priorFrame;
    if (boneCount > 0) {
        int* animA = (int*)(*(int*)((char*)this_ + 0x30) + (unsigned int)param_4 * 0x10);
        if (*(short*)(animA + 2) <= v39) v39 = 0;
        // IDA también clamp this[+0xA8] contra anim[this+0xA0]'s length:
        //   if ( v11 >= *(std::int16_t *)(16 * *(std::uint8_t*)(This+160) + v13 + 8) )
        //       *(_WORD *)(This + 168) = 0;
        int animBIdxClamp = *(unsigned char*)((char*)this_ + 0xa0);
        int* animBHdr = (int*)(*(int*)((char*)this_ + 0x30) + animBIdxClamp * 0x10);
        if (*(short*)(animBHdr + 2) <= *(short*)((char*)this_ + 0xa8))
            *(short*)((char*)this_ + 0xa8) = 0;
        // per-bone loop: interpolate quaternions and build transforms
        for (int boneIdx = 0; boneIdx < (int)(short)*(short*)((char*)this_+0x22); boneIdx++) {
            int bonePtr = *(int*)((char*)this_ + 0x2c) + boneIdx * 0x8c;
            if (*(char*)(bonePtr + 0x22) == '\0') {
                int* animPtrA = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)param_4 * 0xc);
                int* animPtrB = (int*)(*(int*)(bonePtr + 0x24) + (unsigned int)*(unsigned char*)((char*)this_ + 0xa0) * 0xc);
                float quatA[4], quatB[4];
                if (boneIdx == *(int*)((char*)this_ + 0x54)) {
                    // root bone: leer Euler angles (3 floats, stride 0xc) y
                    // convertir a quat via EulerToQuat (EulerToQuat), con los 3 componentes:
                    // EulerToQuat lee también Y (línea 25: v5 = a1[1]*0.5).
                    // IDA: v43[1] = *(_DWORD *)(v21 + 4); (Y sin ajuste HeadAngle)
                    float posA[4], posB[4];
                    int   curFrameB = *(short*)((char*)this_ + 0xa8);
                    posA[0] = *(float*)(animPtrA[1] + v39 * 0xc)     - param_6[0] * _DAT_005528b0;
                    posA[1] = *(float*)(animPtrA[1] + v39 * 0xc + 4);
                    posA[2] = *(float*)(animPtrA[1] + v39 * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    posB[0] = *(float*)(animPtrB[1] + curFrameB * 0xc)     - param_6[0] * _DAT_005528b0;
                    posB[1] = *(float*)(animPtrB[1] + curFrameB * 0xc + 4);
                    posB[2] = *(float*)(animPtrB[1] + curFrameB * 0xc + 8) - param_6[1] * _DAT_005528b0;
                    EulerToQuat((int)&posA[0], (int)&quatA[0], 0, 0);
                    EulerToQuat((int)&posB[0], (int)&quatB[0], 0, 0);
                } else {
                    // regular bone: read quaternion from anim table
                    int frameOff = v39 * 0x10;
                    quatA[0] = *(float*)(animPtrA[2] + frameOff);
                    quatA[1] = *(float*)(animPtrA[2] + frameOff + 4);
                    quatA[2] = *(float*)(animPtrA[2] + frameOff + 8);
                    quatA[3] = *(float*)(animPtrA[2] + frameOff + 0xc);
                    int frameOffB = *(short*)((char*)this_ + 0xa8) * 0x10;
                    quatB[0] = *(float*)(animPtrB[2] + frameOffB);
                    quatB[1] = *(float*)(animPtrB[2] + frameOffB + 4);
                    quatB[2] = *(float*)(animPtrB[2] + frameOffB + 8);
                    quatB[3] = *(float*)(animPtrB[2] + frameOffB + 0xc);
                }
                // slerp between quatA and quatB
                // IDA: `(char *)&unk_5826E18 + 16 * boneIdx` — paso de 16 BYTES.
                // Con `&DAT_05826e18` (DWORD*) el paso salía de 64 bytes.
                int quatBone = boneIdx;
                if (quatBone < 0 || quatBone >= 200) quatBone = 0;   // guard: la tabla es de 200 huesos
                float* outQuat = (float*)(quaternionScratch + quatBone * 0x10);
                int same = Terrain_QuadEqual((int)&quatA, (int)&quatB, 0, 0);
                if (!same)
                    QuatSlerp((int)&quatA, (int)&quatB, *(int*)&fFrac, (int)outQuat); // pass float bits, not truncated int
                else { outQuat[0]=quatA[0]; outQuat[1]=quatA[1]; outQuat[2]=quatA[2]; outQuat[3]=quatA[3]; }
                // quaternion to 3x3 rotation matrix, embedded in a 3x4.
                // QuatToMatrix solo escribe posiciones [0,1,2,4,5,6,8,9,10] (9 floats de
                // 3x3). Las posiciones 3,7,11 (columna de translación) las lee
                // R_ConcatTransforms: zero-init + inyectar tX/tY/tZ abajo.
                float rot33[12] = {0};
                QuatToMatrix((int)outQuat, (int)rot33, 0, 0);
                // get translation from frame keys (3 floats × priorFrameInt)
                float* ptransA = (float*)(*animPtrA + v39 * 0xc);
                float* ptransB = (float*)(*animPtrB + *(short*)((char*)this_ + 0xa8) * 0xc);
                float tX, tY, tZ;
                if (boneIdx == 0 &&
                    (*(char*)((unsigned int)param_4 * 0x10 + 10 + *(int*)((char*)this_+0x30)) != '\0' ||
                     *(char*)((unsigned int)*(unsigned char*)((char*)this_+0xa0)*0x10+10+*(int*)((char*)this_+0x30)) != '\0')) {
                    // root-bone override: IDA toma frame 0 de animB (NO animA)
                    //   v52 = **(float **)v17;              // v17 = animB
                    //   v53 = *(float *)(*(_DWORD *)v17+4);
                    // Antes usábamos *animPtrA → offset origen equivocado en anim
                    // de transición entre acciones (animA≠animB).
                    tX = *(float*)*animPtrB;
                    tY = *(float*)(*animPtrB + 4);
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2] + *(float*)((char*)this_ + 0x84);
                } else {
                    tX = fFrac2 * ptransA[0] + fFrac * ptransB[0];
                    tY = fFrac * ptransB[1] + fFrac2 * ptransA[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2];
                }
                // apply parent transform
                short parentIdx = *(short*)(bonePtr + 0x20);
                float* parentMat;
                if (parentIdx == -1) {
                    if (param_7 == '\0') {
                        Matrix_BuildFromEuler((float*)((char*)this_ + 0x78), parentScratch);
                        if (param_8 != '\0') {
                            // Se escalan los 9 elementos rotacionales (3x3 completa), no sólo la
                            // diagonal (cross-ref con 5.2 ZzzBMD.cpp:153-159; el decompile de IDA se lee
                            // mal ahí):
                            //   for (y=0; y<3; y++)
                            //     for (x=0; x<3; x++)
                            //       ParentMatrix[y][x] *= BodyScale;
                            // Translate flag escribe BodyOrigin en columna [3].
                            float sc = *(float*)((char*)this_ + 0x68);
                            float* pm = parentScratch;
                            // Scale full 3x3 rotation block (rows 0..2, cols 0..2).
                            // Layout: pm[y*4 + x] for row y, col x.
                            for (int y = 0; y < 3; ++y) {
                                for (int x = 0; x < 3; ++x) {
                                    pm[y * 4 + x] *= sc;
                                }
                            }
                            // Translation column [y][3] = BodyOrigin[y].
                            pm[3]  = *(float*)((char*)this_ + 0x6c);  // BodyOrigin[0]
                            pm[7]  = *(float*)((char*)this_ + 0x70);  // BodyOrigin[1]
                            pm[11] = *(float*)((char*)this_ + 0x74);  // BodyOrigin[2]
                        }
                    }
                    parentMat = parentScratch;
                } else {
                    parentMat = (float*)(parentIdx * 0x30 + param_1);
                }
                // Inject bone translation into rot33's 3x4 translation column.
                rot33[3]  = tX;
                rot33[7]  = tY;
                rot33[11] = tZ;
                R_ConcatTransforms(parentMat, rot33, (float*)(boneIdx * 0x30 + param_1));
            }
        }
    }
}


void __cdecl BMD_Animation(void* model, int bones, float frame, unsigned int priorFrame,
    unsigned char priorAction, unsigned int* angles, float* head, char special, char translate)
{
    AnimateWithScratch(model,bones,frame,priorFrame,priorAction,angles,head,special,translate,
        DAT_05826e18,&DAT_06989c9c);
}

namespace {
struct FrozenModel {
    std::vector<unsigned char> boneHeaders,actionHeaders;
    std::vector<std::vector<unsigned char>> vertices,normals;
    std::array<bool,200> usedBones{};
    std::size_t epoch=0;
    bool geometry=false;
};
struct PreparedMesh {
    const unsigned char* vertices=nullptr;
    const unsigned char* normals=nullptr;
    const std::vector<unsigned char>* rawVertices=nullptr;
    const std::vector<unsigned char>* rawNormals=nullptr;
    std::vector<float> positions,rotatedNormals,light;
};
struct PreparedBones {
    alignas(float) std::array<unsigned char,0xbc> model, inputModel;
    alignas(float) std::array<unsigned char,0x1bc> object;
    std::array<float,200*12> bones;
    std::array<char,200*16> quaternions;
    std::array<float,12> parent;
    const void* originalModel=nullptr;
    FrozenModel* frozen=nullptr;
    std::vector<PreparedMesh> meshes;
    std::vector<TransformMath::Mesh> transforms;
    std::array<bool,200> usedBones{};
    std::array<float,3> direction{},coefficients{};
    int boneCount=0;
    bool geometry=false;
    bool valid=false, writesParent=false;
};
// Cada slot conserva capacidad y resultados propios; nunca se escriben los
// huesos ni los modelos vivos durante la preparación paralela.
std::vector<PreparedBones> prepared;
std::unordered_map<const void*,FrozenModel> frozenModels;
std::size_t batchEpoch=0;
std::vector<PreparedBones*> jobs;
PreparedBones* geometryPose=nullptr;
bool batchActive=false;
std::size_t batchCount=0;

void PrepareRange(void*,std::size_t begin,std::size_t end) {
    for(std::size_t slot=begin;slot<end;++slot) {
        auto& p=*jobs[slot];
        auto* o=AsObject(p.object.data());
        DWORD priorFrame; memcpy(&priorFrame,&o->PriorAnimationFrame,4);
        AnimateWithScratch(p.model.data(),(int)p.bones.data(),o->AnimationFrame,priorFrame,
            o->PriorAction,(unsigned int*)o->Angle,o->HeadAngle,0,0,
            p.quaternions.data(),p.parent.data());
        if(p.geometry) {
            auto* model=p.model.data();
            p.direction={0,0,0};
            if(model[0x44]) {
                float input[3]={0,-1.5f,0},rotation[12];
                if(DAT_005597c4) { input[0]=1.3f;input[1]=0;input[2]=2; }
                Matrix_BuildFromEuler((float*)(model+0x8c),rotation);
                Vector_InverseRotate(input,rotation,p.direction.data());
            }
            TransformMath::Batch batch{p.transforms.data(),p.transforms.size(),p.bones.data(),
                1.f,*(float*)(model+0x68),{*(float*)(model+0x6c),*(float*)(model+0x70),*(float*)(model+0x74)},
                {p.direction[0],p.direction[1],p.direction[2]},
                {p.coefficients[0],p.coefficients[1],p.coefficients[2]},1.f==_DAT_0055256c,true,model[0x44]!=0};
            std::size_t count=0;
            for(const auto& mesh:p.transforms)count+=mesh.vertexCount+mesh.normalCount;
            TransformMath::Run(&batch,0,count);
        }
    }
}
}

// IDA: RenderCharactersClient 0x0045AB00 / BMD_Animation 0x00440060.
// DESVIACION: una sola barrera para las poses principales visibles; el render
// consume copias validadas y conserva todas sus escrituras en el orden original.
void ObjectPose_BeginBatch() {
    batchActive=false; geometryPose=nullptr; batchCount=0; jobs.clear();
    if(SceneFlag!=5 || DAT_07e11d30==2 || !DAT_07abf5d0) return;
    int visible=0;
    for(int slot=0;slot<400 && visible<4;++slot) {
        const auto* o=AsObject(DAT_07abf5d0+slot*0x394);
        if(o->Live && (o->Visible || (const void*)o==(const void*)DAT_07abf5d8) && o->Alpha>=_DAT_005524f8) ++visible;
    }
    if(visible<4) return;
    if(prepared.empty()) prepared.resize(400);
    ++batchEpoch;
    for(std::size_t slot=0;slot<prepared.size();++slot) {
        auto& p=prepared[slot]; p.valid=false; p.geometry=false;
        auto* o=AsObject(DAT_07abf5d0+slot*0x394);
        if(!o->Live || (!o->Visible && (void*)o!=(void*)DAT_07abf5d8) || o->Alpha<_DAT_005524f8) continue;
        if(o->Type<0 || (std::size_t)o->Type>=ResourceTables::ModelCount()) continue;
        const auto* model=(const unsigned char*)(uintptr_t)(DAT_05828d58+o->Type*0xbc);
        const int count=*(const short*)(model+0x22);
        if(count<=0 || count>200 || *(const short*)(model+0x26)<=0 ||
           *(const DWORD*)(model+0x2c)<0x10000u || *(const DWORD*)(model+0x30)<0x10000u) continue;
        const auto* rawBones=(const unsigned char*)(uintptr_t)*(const DWORD*)(model+0x2c);
        bool safe=true; p.writesParent=false;
        for(int i=0;i<count;++i) {
            if(rawBones[i*0x8c+0x22]) continue;
            const int parent=*(const short*)(rawBones+i*0x8c+0x20);
            // Un padre no calculado depende del destino anterior: dejarlo en serie.
            if(parent< -1 || parent>=i || (parent>=0 && rawBones[parent*0x8c+0x22])) safe=false;
            if(parent== -1) p.writesParent=true;
        }
        if(!safe) continue;
        auto& frozen=frozenModels[model];
        const bool fresh=frozen.epoch!=batchEpoch;
        frozen.epoch=batchEpoch; p.frozen=&frozen;
        if(fresh) {
            frozen.boneHeaders.assign(rawBones,rawBones+count*0x8c);
            const auto* headers=(const unsigned char*)(uintptr_t)*(const DWORD*)(model+0x30);
            frozen.actionHeaders.assign(headers,headers+*(const short*)(model+0x26)*16);
        }
        memcpy(p.model.data(),model,p.model.size());
        memcpy(p.object.data(),o,p.object.size());
        *(float*)(p.model.data()+0x84)=0.f; // Calc_RenderObject, antes de animar.
        PrepareContext((int)p.object.data(),p.model.data());
        p.inputModel=p.model;
        p.originalModel=model; p.boneCount=count; p.valid=true;
        // Congelar fuentes: tela u otra pasada puede modificarlas antes del draw.
        // La validación de consumo detecta ese caso y usa la ruta original.
        const int meshCount=*(const short*)(model+0x24);
        const auto meshBase=(uintptr_t)*(const DWORD*)(model+0x28);
        p.geometry=meshCount>0 && meshCount<=200 && meshBase>=0x10000 && (fresh || frozen.geometry);
        p.transforms.clear(); p.usedBones=fresh?std::array<bool,200>{}:frozen.usedBones;
        if(p.geometry) {
            p.meshes.resize(meshCount);
            if(fresh) { frozen.vertices.resize(meshCount);frozen.normals.resize(meshCount); }
        }
        for(int m=0;p.geometry && m<meshCount;++m) {
            const auto* source=(const unsigned char*)meshBase+m*0x28;
            const int vc=*(const short*)(source+4),nc=*(const short*)(source+6);
            auto& mesh=p.meshes[m];
            mesh.vertices=(const unsigned char*)(uintptr_t)*(const DWORD*)(source+0x10);
            mesh.normals=(const unsigned char*)(uintptr_t)*(const DWORD*)(source+0x14);
            if(vc<0 || vc>15000 || nc<0 || nc>15000 ||
               (uintptr_t)mesh.vertices<0x10000 || (uintptr_t)mesh.normals<0x10000) { p.geometry=false;break; }
            if(fresh) {
                frozen.vertices[m].assign(mesh.vertices,mesh.vertices+vc*16);
                frozen.normals[m].assign(mesh.normals,mesh.normals+nc*20);
            }
            mesh.rawVertices=&frozen.vertices[m];mesh.rawNormals=&frozen.normals[m];
            mesh.positions.resize(vc*3);mesh.rotatedNormals.resize(nc*3);mesh.light.resize(nc);
            if(fresh) for(int stream=0;stream<2;++stream) {
                const auto* raw=stream?mesh.rawNormals->data():mesh.rawVertices->data();
                const int amount=stream?nc:vc,stride=stream?20:16;
                for(int i=0;i<amount;++i) {
                    const int bone=*(const short*)(raw+i*stride);
                    if(bone<0 || bone>=count || rawBones[bone*0x8c+0x22]) { p.geometry=false;break; }
                    p.usedBones[bone]=true;
                }
            }
            p.transforms.push_back({mesh.rawVertices->data(),mesh.rawNormals->data(),mesh.positions.data(),
                mesh.rotatedNormals.data(),mesh.light.data(),(std::size_t)vc,(std::size_t)nc});
        }
        if(fresh) { frozen.geometry=p.geometry;frozen.usedBones=p.usedBones; }
        if(o->Type!=0x145 && o->Type!=0xe1) p.model[0x44]=o->LightEnable;
        p.coefficients={_DAT_00552530,_DAT_005528b4,_DAT_005526e4};
        jobs.push_back(&p);
        ++batchCount;
    }
    if(batchCount<4) return;
    static TransformWorkers::Team team;
    if(!team.Workers()) return;
    team.Run(jobs.size(),nullptr,PrepareRange);
    batchActive=true;
}
void ObjectPose_EndBatch() { batchActive=false; geometryPose=nullptr; }

static bool ReuseBones(int entity,void* model,float* bones,char translate) {
    geometryPose=nullptr;
    if(!batchActive || translate!=1) return false;
    const auto base=(uintptr_t)DAT_07abf5d0, address=(uintptr_t)entity;
    if(address<base || address-base>=400*0x394 || (address-base)%0x394) return false;
    auto& p=prepared[(address-base)/0x394];
    if(!p.valid || p.originalModel!=model) return false;
    const auto* current=(const unsigned char*)model;
    // Sólo entradas leídas por BMD_Animation; BodyLight/material no intervienen.
    const int modelRanges[][2]={{0x22,2},{0x26,2},{0x2c,8},{0x54,4},{0x68,16},{0x84,4},{0xa0,1}};
    for(const auto& range:modelRanges)
        if(memcmp(current+range[0],p.inputModel.data()+range[0],range[1])) return false;
    if(memcmp((const void*)(uintptr_t)*(const DWORD*)(current+0x2c),p.frozen->boneHeaders.data(),p.frozen->boneHeaders.size()) ||
       memcmp((const void*)(uintptr_t)*(const DWORD*)(current+0x30),p.frozen->actionHeaders.data(),p.frozen->actionHeaders.size())) return false;
    const OBJECT& o=*AsObject(entity); const OBJECT& old=*AsObject(p.object.data());
    if(memcmp(&o.AnimationFrame,&old.AnimationFrame,4) ||
       memcmp(&o.PriorAnimationFrame,&old.PriorAnimationFrame,4) || o.PriorAction!=old.PriorAction ||
       memcmp(o.Angle,old.Angle,12) || memcmp(o.HeadAngle,old.HeadAngle,12)) return false;
    // Replay de los stores, no de los bytes intactos: los scratch omitidos
    // por huesos dummy deben conservar lo que dejó la entidad anterior.
    auto* output=(unsigned char*)model;
    memcpy(output+0x78,p.model.data()+0x78,12);
    memcpy(output+0xa4,p.model.data()+0xa4,6);
    output[0xa0]=p.model[0xa0];
    const auto* rawBones=(const unsigned char*)(uintptr_t)*(const DWORD*)(current+0x2c);
    for(int i=0;i<p.boneCount;++i) if(!rawBones[i*0x8c+0x22]) {
        memcpy(bones+i*12,p.bones.data()+i*12,48);
        memcpy(DAT_05826e18+i*16,p.quaternions.data()+i*16,16);
    }
    if(p.writesParent) memcpy(&DAT_06989c9c,p.parent.data(),48);
    geometryPose=&p;
    return true;
}

bool ObjectPose_RestoreGeometry(void* model,const float* bones,float rotationScale,char translate,
    const float* direction,const float* coefficients,float* positions,float* normals,float* light) {
    if(!batchActive || !geometryPose || !geometryPose->geometry || translate!=1 || rotationScale!=1.f) return false;
    const auto& p=*geometryPose;
    if(p.originalModel!=model) return false;
    const auto* current=(const unsigned char*)model;
    if(memcmp(current+0x68,p.model.data()+0x68,16) || current[0x44]!=p.model[0x44] ||
       memcmp(direction,p.direction.data(),12) || memcmp(coefficients,p.coefficients.data(),12) ||
       memcmp(current+0x28,p.model.data()+0x28,4) || *(const short*)(current+0x24)!=(int)p.meshes.size()) return false;
    for(int i=0;i<p.boneCount;++i)
        if(p.usedBones[i] && memcmp(bones+i*12,p.bones.data()+i*12,48)) return false;
    const auto* sources=(const unsigned char*)(uintptr_t)*(const DWORD*)(current+0x28);
    // Validar todo antes del primer store: un miss no deja resultados parciales.
    for(std::size_t m=0;m<p.meshes.size();++m) {
        const auto& mesh=p.meshes[m];const auto* source=sources+m*0x28;
        if(*(const short*)(source+4)!=(int)mesh.positions.size()/3 ||
           *(const short*)(source+6)!=(int)mesh.rotatedNormals.size()/3 ||
           *(const DWORD*)(source+0x10)!=(uintptr_t)mesh.vertices ||
           *(const DWORD*)(source+0x14)!=(uintptr_t)mesh.normals ||
           (!mesh.rawVertices->empty() && memcmp(mesh.vertices,mesh.rawVertices->data(),mesh.rawVertices->size())) ||
           (!mesh.rawNormals->empty() && memcmp(mesh.normals,mesh.rawNormals->data(),mesh.rawNormals->size()))) return false;
    }
    for(std::size_t m=0;m<p.meshes.size();++m) {
        const auto& mesh=p.meshes[m];
        if(!mesh.positions.empty())memcpy(positions+m*45000,mesh.positions.data(),mesh.positions.size()*sizeof(float));
        if(!mesh.rotatedNormals.empty())memcpy(normals+m*45000,mesh.rotatedNormals.data(),mesh.rotatedNormals.size()*sizeof(float));
        if(current[0x44] && !mesh.light.empty())memcpy(light+m*15000,mesh.light.data(),mesh.light.size()*sizeof(float));
    }
    return true;
}
