#include "stdafx.h"
#include "Entity/ObjectPose.h"
#include "Entity/ObjectBounds.h"
#include "functions.h"
#include "globals.h"
#include "Render/TransformWorkers.h"
#include "Render/TransformMath.h"
#include "Render/ResourceTables.h"
#include "Physics/ClothTypes.h"
#include <array>
#include <vector>
#include <unordered_map>

// IDA: preparación de pose de Calc_RenderObject (0x004FAA70).
// El destino explícito permite preparar huesos SIM sin utilizar el buffer
// de la entidad que el render puede sobrescribir con una pose interpolada.
static void PrepareContext(OBJECT* entity, void* model)
{
    BMD& context = *AsModel(model);
    // Copiar los bits, incluidas cargas NaN y ceros con signo, como los DWORD originales.
    memcpy(&context.BodyScale, &entity->Scale, sizeof(context.BodyScale));
    context.CurrentAction = entity->CurrentAction;
    memcpy(context.BodyOrigin, entity->Position, sizeof(context.BodyOrigin));
    // Desplazamiento Y especial para la elfa (0x14B) y su variante (0x14A).
    short cls = AsObject(entity)->Type;
    if (cls == 0x14b) {
        context.BodyOrigin[1] = entity->Position[1] + _DAT_0055290c;
    } else if (cls == 0x14a) {
        context.BodyOrigin[1] = entity->Position[1] + _DAT_00552908;
    }
}
static bool ReuseBones(OBJECT* entity, void* model, float* bones, char translate);
void ObjectPose_PrepareBones(OBJECT* entity, void* model, float* bones, char translate)
{
    PrepareContext(entity,model);
    if (ReuseBones(entity,model,bones,translate)) return;
    DWORD priorFrame;
    memcpy(&priorFrame, &AsObject(entity)->PriorAnimationFrame, sizeof(priorFrame));
    BMD_Animation(model, bones, AsObject(entity)->AnimationFrame, priorFrame,
                  AsObject(entity)->PriorAction, reinterpret_cast<unsigned int*>(entity->Angle),
                  entity->HeadAngle, '\0', translate == '\0');
}

// La caja normal no depende de mallas ni huesos. Sólo el editor 2 necesita
// transformar la geometría para obtener sus extremos; LOD 0 no dibuja.
void ObjectPose_PrepareBounds(OBJECT* entity, char translate)
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

void ObjectPose_PrepareSimulation(OBJECT* entity, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (o.Alpha < _DAT_005524f8) return;
    if (DAT_07e11d30 == 2) {
        Calc_RenderObject(entity, translate, 0);
        return;
    }
    BMD* model = AsModel(DAT_05828d58) + o.Type;
    model->BodyHeight = 0.0f;
    float *bones = o.EnableBoneMatrix ? (float *)o.BoneMatrices : (float *)&DAT_06970a9c;
    ObjectPose_PrepareBones(entity, model, bones, translate);
    ObjectPose_PrepareBounds(entity, translate);
}

// IDA: Draw_RenderObject (0x004FAE00), estatuas de Blood Castle.
// DESVIACION: fragmentos, audio y desactivación se producen en SIM, aun sin draw.
void ObjectPose_TickMeshEvents(OBJECT* entity, void *model, float *bones, char translate)
{
    OBJECT &o = *AsObject(entity);
    if (!o.Live || o.Alpha < _DAT_005524f8) return;
    const bool dead = o.CurrentAction == 6;
    const bool materializing = o.Type == 330 && (int)MoveSceneFrame - o.InitialSceneFrame < 25;
    if (!((o.Type == 331 && dead) || (o.Type == 330 && (dead || materializing)))) return;
    float bounds[15];
    Skeleton_Transform(model, bones, o.BoundingBoxMin, o.BoundingBoxMax, bounds, translate);
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
static void AnimateWithScratch(void* this_, float* param_1, float param_2, unsigned int param_3,
                           unsigned char param_4, unsigned int* param_5,
                           float* param_6, char param_7, char param_8,
                           char* quaternionScratch, float* parentScratch) {
    BMD& model = *AsModel(this_);
    // clamp anim indices to bone count
    short boneCount = model.NumActions;
    if (boneCount <= (short)(unsigned short)param_4)
        param_4 = 0;
    if (boneCount <= (short)(unsigned short)model.CurrentAction)
        model.CurrentAction = 0;
    // store current color/frame in model header
    memcpy(&model.BodyAngle[0], param_5 + 0, sizeof(float));
    memcpy(&model.BodyAngle[1], param_5 + 1, sizeof(float));
    memcpy(&model.BodyAngle[2], param_5 + 2, sizeof(float));
    model.CurrentAnimation = param_2;
    // compute fractional frame and frame indices
    long long lv = (long long)(int)param_2;
    short curFrame = (short)lv;
    model.CurrentAnimationFrame = curFrame;
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
        const ModelAnimationAction* actions = reinterpret_cast<const ModelAnimationAction*>(model.Actions);
        const ModelAnimationAction* animA = actions + param_4;
        if (animA->FrameCount <= v39) v39 = 0;
        // IDA también clamp this[+0xA8] contra anim[this+0xA0]'s length:
        //   if ( v11 >= *(std::int16_t *)(16 * *(std::uint8_t*)(This+160) + v13 + 8) )
        //       *(_WORD *)(This + 168) = 0;
        int animBIdxClamp = model.CurrentAction;
        const ModelAnimationAction* animBHdr = actions + animBIdxClamp;
        if (animBHdr->FrameCount <= model.CurrentAnimationFrame)
            model.CurrentAnimationFrame = 0;
        // per-bone loop: interpolate quaternions and build transforms
        for (int boneIdx = 0; boneIdx < (int)(short)model.NumBones; boneIdx++) {
            const ModelAnimationBone* bonePtr = reinterpret_cast<const ModelAnimationBone*>(model.Bones) + boneIdx;
            if (bonePtr->Dummy == '\0') {
                const ModelAnimationKeys* animPtrA = bonePtr->Keys + param_4;
                const ModelAnimationKeys* animPtrB = bonePtr->Keys + model.CurrentAction;
                float quatA[4], quatB[4];
                if (boneIdx == model.BoneHead) {
                    // root bone: leer Euler angles (3 floats, stride 0xc) y
                    // convertir a quat via EulerToQuat (EulerToQuat), con los 3 componentes:
                    // EulerToQuat lee también Y (línea 25: v5 = a1[1]*0.5).
                    // IDA: v43[1] = *(_DWORD *)(v21 + 4); (Y sin ajuste HeadAngle)
                    float posA[4], posB[4];
                    int   curFrameB = model.CurrentAnimationFrame;
                    posA[0] = animPtrA->EulerAngles[v39 * 3 + 0]     - param_6[0] * _DAT_005528b0;
                    posA[1] = animPtrA->EulerAngles[v39 * 3 + 1];
                    posA[2] = animPtrA->EulerAngles[v39 * 3 + 2] - param_6[1] * _DAT_005528b0;
                    posB[0] = animPtrB->EulerAngles[curFrameB * 3 + 0]     - param_6[0] * _DAT_005528b0;
                    posB[1] = animPtrB->EulerAngles[curFrameB * 3 + 1];
                    posB[2] = animPtrB->EulerAngles[curFrameB * 3 + 2] - param_6[1] * _DAT_005528b0;
                    EulerToQuat(posA, quatA, 0, 0);
                    EulerToQuat(posB, quatB, 0, 0);
                } else {
                    // regular bone: read quaternion from anim table
                    int frameOff = v39 * 0x10;
                    quatA[0] = animPtrA->Quaternions[frameOff / 4 + 0];
                    quatA[1] = animPtrA->Quaternions[frameOff / 4 + 1];
                    quatA[2] = animPtrA->Quaternions[frameOff / 4 + 2];
                    quatA[3] = animPtrA->Quaternions[frameOff / 4 + 3];
                    int frameOffB = model.CurrentAnimationFrame * 0x10;
                    quatB[0] = animPtrB->Quaternions[frameOffB / 4 + 0];
                    quatB[1] = animPtrB->Quaternions[frameOffB / 4 + 1];
                    quatB[2] = animPtrB->Quaternions[frameOffB / 4 + 2];
                    quatB[3] = animPtrB->Quaternions[frameOffB / 4 + 3];
                }
                // slerp between quatA and quatB
                // IDA: `(char *)&unk_5826E18 + 16 * boneIdx` — paso de 16 BYTES.
                // Con `&DAT_05826e18` (DWORD*) el paso salía de 64 bytes.
                int quatBone = boneIdx;
                if (quatBone < 0 || quatBone >= 200) quatBone = 0;   // guard: la tabla es de 200 huesos
                float* outQuat = (float*)(quaternionScratch + quatBone * 0x10);
                int same = Terrain_QuadEqual(quatA, quatB, 0, 0);
                if (!same)
                    QuatSlerp(quatA, quatB, fFrac, outQuat);
                else { outQuat[0]=quatA[0]; outQuat[1]=quatA[1]; outQuat[2]=quatA[2]; outQuat[3]=quatA[3]; }
                // quaternion to 3x3 rotation matrix, embedded in a 3x4.
                // QuatToMatrix solo escribe posiciones [0,1,2,4,5,6,8,9,10] (9 floats de
                // 3x3). Las posiciones 3,7,11 (columna de translación) las lee
                // R_ConcatTransforms: zero-init + inyectar tX/tY/tZ abajo.
                float rot33[12] = {0};
                QuatToMatrix(outQuat, rot33, 0, 0);
                // get translation from frame keys (3 floats × priorFrameInt)
                float* ptransA = animPtrA->Positions + v39 * 3;
                float* ptransB = animPtrB->Positions + model.CurrentAnimationFrame * 3;
                float tX, tY, tZ;
                if (boneIdx == 0 &&
                    (actions[param_4].LockPositions != '\0' ||
                     actions[model.CurrentAction].LockPositions != '\0')) {
                    // root-bone override: IDA toma frame 0 de animB (NO animA)
                    //   v52 = **(float **)v17;              // v17 = animB
                    //   v53 = *(float *)(*(_DWORD *)v17+4);
                    // Antes usábamos *animPtrA → offset origen equivocado en anim
                    // de transición entre acciones (animA≠animB).
                    tX = animPtrB->Positions[0];
                    tY = animPtrB->Positions[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2] + model.BodyHeight;
                } else {
                    tX = fFrac2 * ptransA[0] + fFrac * ptransB[0];
                    tY = fFrac * ptransB[1] + fFrac2 * ptransA[1];
                    tZ = fFrac * ptransB[2] + fFrac2 * ptransA[2];
                }
                // apply parent transform
                short parentIdx = bonePtr->Parent;
                float* parentMat;
                if (parentIdx == -1) {
                    if (param_7 == '\0') {
                        Matrix_BuildFromEuler(model.BodyAngle, parentScratch);
                        if (param_8 != '\0') {
                            // Se escalan los 9 elementos rotacionales (3x3 completa), no sólo la
                            // diagonal (cross-ref con 5.2 ZzzBMD.cpp:153-159; el decompile de IDA se lee
                            // mal ahí):
                            //   for (y=0; y<3; y++)
                            //     for (x=0; x<3; x++)
                            //       ParentMatrix[y][x] *= BodyScale;
                            // Translate flag escribe BodyOrigin en columna [3].
                            float sc = model.BodyScale;
                            float* pm = parentScratch;
                            // Scale full 3x3 rotation block (rows 0..2, cols 0..2).
                            // Layout: pm[y*4 + x] for row y, col x.
                            for (int y = 0; y < 3; ++y) {
                                for (int x = 0; x < 3; ++x) {
                                    pm[y * 4 + x] *= sc;
                                }
                            }
                            // Translation column [y][3] = BodyOrigin[y].
                            pm[3]  = model.BodyOrigin[0];  // BodyOrigin[0]
                            pm[7]  = model.BodyOrigin[1];  // BodyOrigin[1]
                            pm[11] = model.BodyOrigin[2];  // BodyOrigin[2]
                        }
                    }
                    parentMat = parentScratch;
                } else {
                    parentMat = param_1 + parentIdx * 12;
                }
                // Inject bone translation into rot33's 3x4 translation column.
                rot33[3]  = tX;
                rot33[7]  = tY;
                rot33[11] = tZ;
                R_ConcatTransforms(parentMat, rot33, param_1 + boneIdx * 12);
            }
        }
    }
}


void __cdecl BMD_Animation(void* model, float* bones, float frame, unsigned int priorFrame,
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
    alignas(float) std::array<unsigned char,sizeof(BMD)> model, inputModel;
    alignas(float) std::array<unsigned char,sizeof(OBJECT)> object;
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
        AnimateWithScratch(p.model.data(),p.bones.data(),o->AnimationFrame,priorFrame,
            o->PriorAction,(unsigned int*)o->Angle,o->HeadAngle,0,0,
            p.quaternions.data(),p.parent.data());
        if(p.geometry) {
            BMD* model=AsModel(p.model.data());
            p.direction={0,0,0};
            if(model->LightEnable) {
                float input[3]={0,-1.5f,0},rotation[12];
                if(DAT_005597c4) { input[0]=1.3f;input[1]=0;input[2]=2; }
                Matrix_BuildFromEuler(model->ShadowAngle,rotation);
                Vector_InverseRotate(input,rotation,p.direction.data());
            }
            TransformMath::Batch batch{p.transforms.data(),p.transforms.size(),p.bones.data(),
                1.f,model->BodyScale,{model->BodyOrigin[0],model->BodyOrigin[1],model->BodyOrigin[2]},
                {p.direction[0],p.direction[1],p.direction[2]},
                {p.coefficients[0],p.coefficients[1],p.coefficients[2]},1.f==_DAT_0055256c,true,model->LightEnable!=0};
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
    // Medición de lotes pequeños: con 4 poses la coordinación era más cara.
    // Contar trabajo antes de copiar fuentes y reservar el equipo para lotes útiles.
    int visible=0;
    std::size_t vertexWork=0,boneWork=0;
    for(int slot=0;slot<400;++slot) {
        const auto* o=AsObject(DAT_07abf5d0+slot*sizeof(CHARACTER));
        if(!o->Live || (!o->Visible && (const void*)o!=(const void*)DAT_07abf5d8) ||
           o->Alpha<_DAT_005524f8 || o->Type<0 || (std::size_t)o->Type>=ResourceTables::ModelCount()) continue;
        const BMD* model=AsModel(DAT_05828d58)+o->Type;
        if(model->NumActions<=0) continue;
        ++visible;
        const int bones=model->NumBones,count=model->NumMeshs;
        if(bones>0 && bones<=200) boneWork+=bones;
        const auto base=(uintptr_t)model->Meshs;
        if(base>=0x10000 && count>0 && count<=200) for(int m=0;m<count;++m) {
            const auto* mesh=reinterpret_cast<const ClothModelMesh*>(base)+m;
            const int vc=mesh->VertexCount,nc=mesh->NormalCount;
            if(vc>=0 && vc<=15000 && nc>=0 && nc<=15000) vertexWork+=vc+nc;
        }
    }
    if(visible<4 || (vertexWork<16384 && boneWork<1024)) return;
    std::size_t actualVertices=0,actualBones=0;
    if(prepared.empty()) prepared.resize(400);
    ++batchEpoch;
    for(std::size_t slot=0;slot<prepared.size();++slot) {
        auto& p=prepared[slot]; p.valid=false; p.geometry=false;
        auto* o=AsObject(DAT_07abf5d0+slot*sizeof(CHARACTER));
        if(!o->Live || (!o->Visible && (void*)o!=(void*)DAT_07abf5d8) || o->Alpha<_DAT_005524f8) continue;
        if(o->Type<0 || (std::size_t)o->Type>=ResourceTables::ModelCount()) continue;
        const BMD* model=AsModel(DAT_05828d58)+o->Type;
        const int count=model->NumBones;
        if(count<=0 || count>200 || model->NumActions<=0 ||
           (uintptr_t)model->Bones<0x10000u || (uintptr_t)model->Actions<0x10000u) continue;
        const auto* rawBones=reinterpret_cast<const ModelAnimationBone*>(model->Bones);
        bool safe=true; p.writesParent=false;
        for(int i=0;i<count;++i) {
            if(rawBones[i].Dummy) continue;
            const int parent=rawBones[i].Parent;
            // Un padre no calculado depende del destino anterior: dejarlo en serie.
            if(parent< -1 || parent>=i || (parent>=0 && rawBones[parent].Dummy)) safe=false;
            if(parent== -1) p.writesParent=true;
        }
        if(!safe) continue;
        auto& frozen=frozenModels[model];
        const bool fresh=frozen.epoch!=batchEpoch;
        frozen.epoch=batchEpoch; p.frozen=&frozen;
        if(fresh) {
            const auto* boneBytes=reinterpret_cast<const unsigned char*>(rawBones);
            frozen.boneHeaders.assign(boneBytes,boneBytes+count*sizeof(ModelAnimationBone));
            const auto* headers=reinterpret_cast<const unsigned char*>(model->Actions);
            frozen.actionHeaders.assign(headers,headers+model->NumActions*sizeof(ModelAnimationAction));
        }
        memcpy(p.model.data(),model,p.model.size());
        memcpy(p.object.data(),o,p.object.size());
        AsModel(p.model.data())->BodyHeight=0.f; // Calc_RenderObject, antes de animar.
        PrepareContext(AsObject(p.object.data()),p.model.data());
        p.inputModel=p.model;
        p.originalModel=model; p.boneCount=count; p.valid=true;
        // Congelar fuentes: tela u otra pasada puede modificarlas antes del draw.
        // La validación de consumo detecta ese caso y usa la ruta original.
        const int meshCount=model->NumMeshs;
        const auto meshBase=(uintptr_t)model->Meshs;
        p.geometry=meshCount>0 && meshCount<=200 && meshBase>=0x10000 && (fresh || frozen.geometry);
        p.transforms.clear(); p.usedBones=fresh?std::array<bool,200>{}:frozen.usedBones;
        if(p.geometry) {
            p.meshes.resize(meshCount);
            if(fresh) { frozen.vertices.resize(meshCount);frozen.normals.resize(meshCount); }
        }
        for(int m=0;p.geometry && m<meshCount;++m) {
            const auto* source=reinterpret_cast<const ClothModelMesh*>(meshBase)+m;
            const int vc=source->VertexCount,nc=source->NormalCount;
            auto& mesh=p.meshes[m];
            mesh.vertices=source->Vertices;
            mesh.normals=static_cast<const unsigned char*>(source->Normals);
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
                    if(bone<0 || bone>=count || rawBones[bone].Dummy) { p.geometry=false;break; }
                    p.usedBones[bone]=true;
                }
            }
            p.transforms.push_back({mesh.rawVertices->data(),mesh.rawNormals->data(),mesh.positions.data(),
                mesh.rotatedNormals.data(),mesh.light.data(),(std::size_t)vc,(std::size_t)nc});
        }
        if(fresh) { frozen.geometry=p.geometry;frozen.usedBones=p.usedBones; }
        if(o->Type!=0x145 && o->Type!=0xe1) AsModel(p.model.data())->LightEnable=o->LightEnable;
        p.coefficients={_DAT_00552530,_DAT_005528b4,_DAT_005526e4};
        jobs.push_back(&p);
        actualBones+=p.boneCount;
        if(p.geometry) for(const auto& mesh:p.transforms) actualVertices+=mesh.vertexCount+mesh.normalCount;
        ++batchCount;
    }
    if(batchCount<4 || (actualVertices<16384 && actualBones<1024)) return;
    auto& team=TransformWorkers::SharedTeam();
    if(!team.Workers()) return;
    team.Run(jobs.size(),nullptr,PrepareRange);
    batchActive=true;
}
void ObjectPose_EndBatch() { batchActive=false; geometryPose=nullptr; }

static bool ReuseBones(OBJECT* entity,void* model,float* bones,char translate) {
    geometryPose=nullptr;
    if(!batchActive || translate!=1) return false;
    const auto base=(uintptr_t)DAT_07abf5d0, address=(uintptr_t)entity;
    if(address<base || address-base>=400*sizeof(CHARACTER) || (address-base)%sizeof(CHARACTER)) return false;
    auto& p=prepared[(address-base)/sizeof(CHARACTER)];
    if(!p.valid || p.originalModel!=model) return false;
    const BMD* current=AsModel(model);
    // Sólo entradas leídas por BMD_Animation; BodyLight/material no intervienen.
    const std::size_t modelRanges[][2]={{offsetof(BMD,NumBones),2},{offsetof(BMD,NumActions),2},
        {offsetof(BMD,Bones),sizeof(current->Bones)+sizeof(current->Actions)},
        {offsetof(BMD,BoneHead),4},{offsetof(BMD,BodyScale),16},
        {offsetof(BMD,BodyHeight),4},{offsetof(BMD,CurrentAction),1}};
    for(const auto& range:modelRanges)
        if(memcmp(reinterpret_cast<const unsigned char*>(current)+range[0],p.inputModel.data()+range[0],range[1])) return false;
    if(memcmp(reinterpret_cast<const void*>(current->Bones),p.frozen->boneHeaders.data(),p.frozen->boneHeaders.size()) ||
       memcmp(reinterpret_cast<const void*>(current->Actions),p.frozen->actionHeaders.data(),p.frozen->actionHeaders.size())) return false;
    const OBJECT& o=*AsObject(entity); const OBJECT& old=*AsObject(p.object.data());
    if(memcmp(&o.AnimationFrame,&old.AnimationFrame,4) ||
       memcmp(&o.PriorAnimationFrame,&old.PriorAnimationFrame,4) || o.PriorAction!=old.PriorAction ||
       memcmp(o.Angle,old.Angle,12) || memcmp(o.HeadAngle,old.HeadAngle,12)) return false;
    // Replay de los stores, no de los bytes intactos: los scratch omitidos
    // por huesos dummy deben conservar lo que dejó la entidad anterior.
    BMD* output=AsModel(model);
    const BMD* cached=AsModel(p.model.data());
    memcpy(output->BodyAngle,cached->BodyAngle,12);
    memcpy(&output->CurrentAnimation,&cached->CurrentAnimation,6);
    output->CurrentAction=cached->CurrentAction;
    const auto* rawBones=reinterpret_cast<const ModelAnimationBone*>(current->Bones);
    for(int i=0;i<p.boneCount;++i) if(!rawBones[i].Dummy) {
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
    const BMD* current=AsModel(model);
    if(memcmp(&current->BodyScale,&AsModel(p.model.data())->BodyScale,16) || current->LightEnable!=AsModel(p.model.data())->LightEnable ||
       memcmp(direction,p.direction.data(),12) || memcmp(coefficients,p.coefficients.data(),12) ||
       memcmp(&current->Meshs,&AsModel(p.model.data())->Meshs,sizeof(current->Meshs)) || current->NumMeshs!=(int)p.meshes.size()) return false;
    for(int i=0;i<p.boneCount;++i)
        if(p.usedBones[i] && memcmp(bones+i*12,p.bones.data()+i*12,48)) return false;
    const auto* sources=reinterpret_cast<const ClothModelMesh*>(current->Meshs);
    // Validar todo antes del primer store: un miss no deja resultados parciales.
    for(std::size_t m=0;m<p.meshes.size();++m) {
        const auto& mesh=p.meshes[m];const auto* source=sources+m;
        if(source->VertexCount!=(int)mesh.positions.size()/3 ||
           source->NormalCount!=(int)mesh.rotatedNormals.size()/3 ||
           source->Vertices!=mesh.vertices ||
           source->Normals!=mesh.normals ||
           (!mesh.rawVertices->empty() && memcmp(mesh.vertices,mesh.rawVertices->data(),mesh.rawVertices->size())) ||
           (!mesh.rawNormals->empty() && memcmp(mesh.normals,mesh.rawNormals->data(),mesh.rawNormals->size()))) return false;
    }
    for(std::size_t m=0;m<p.meshes.size();++m) {
        const auto& mesh=p.meshes[m];
        if(!mesh.positions.empty())memcpy(positions+m*45000,mesh.positions.data(),mesh.positions.size()*sizeof(float));
        if(!mesh.rotatedNormals.empty())memcpy(normals+m*45000,mesh.rotatedNormals.data(),mesh.rotatedNormals.size()*sizeof(float));
        if(current->LightEnable && !mesh.light.empty())memcpy(light+m*15000,mesh.light.data(),mesh.light.size()*sizeof(float));
    }
    return true;
}
