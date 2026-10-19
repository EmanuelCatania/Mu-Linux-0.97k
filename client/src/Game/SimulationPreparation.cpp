#include "stdafx.h"
#include "Game/SimulationPreparation.h"
#include "Entity/CharacterCloth.h"
#include "Entity/CharacterEffects.h"
#include "Entity/ObjectPose.h"
#include "Item/ContentCatalog.h"
#include "Render/Terrain_Render.h"
#include "Render/Camera.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include "Render/EquipmentVisuals.h"
#include "Render/WorldEffects.h"

void DivineSkirt_Tick(int entity, int modelType, int part, void *model);
extern "C" void HeroEquipWatchdog(int entity);

namespace {
struct CameraPose {
    float position[3], angles[3], fov, farPlane, distance;
};
struct PickingCamera : CameraPose {
    bool valid = false;
    int scene, world;
    DWORD matrix[12], origin[3], centerX, centerY;
    DWORD width, height, x, y, projectionBits[2];
    float target[3], projection[2], centerYFlip;
};
PickingCamera camera;
CameraPose renderCamera;

void CapturePose(CameraPose &pose) {
    memcpy(pose.position, CameraPosition, sizeof(pose.position));
    memcpy(pose.angles, CameraAngle, sizeof(pose.angles));
    pose.fov = CameraFOV; pose.farPlane = CameraViewFar; pose.distance = CameraDistance;
}
void ApplyPose(const CameraPose &pose) {
    memcpy(CameraPosition, pose.position, sizeof(pose.position));
    memcpy(CameraAngle, pose.angles, sizeof(pose.angles));
    CameraFOV = pose.fov; CameraViewFar = pose.farPlane; CameraDistance = pose.distance;
}

void CaptureCamera() {
    camera.valid = true;
    camera.scene = SceneFlag; camera.world = gMapManager.GetCurrentMap();
    memcpy(camera.matrix, CameraMatrix, sizeof(camera.matrix));
    memcpy(camera.origin, CameraRayOriginX_arr, sizeof(camera.origin));
    memcpy(camera.target, &DAT_083a4110, sizeof(camera.target));
    CapturePose(camera);
    camera.centerX = ViewportCenterX; camera.centerY = ViewportCenterY;
    camera.width = OpenglWindowWidth; camera.height = OpenglWindowHeight;
    camera.x = OpenglWindowX; camera.y = OpenglWindowY;
    camera.projection[0] = _DAT_083a42a4; camera.projection[1] = _DAT_083a42a8;
    camera.projectionBits[0] = DAT_083a42a4; camera.projectionBits[1] = DAT_083a42a8;
    camera.centerYFlip = ScreenCenterYFlip;
}

void PrepareCamera() {
    if (SceneFlag == 5) {
        // DESVIACION: cámara y suavizado avanzan una vez por paso de 40 ms.
        MoveMainCamera();
        float origin[3] = {};
        if (DAT_07abf5d8) memcpy(origin, AsObject((int)DAT_07abf5d8)->Position, sizeof(origin));
        const int width = GetScreenWidth();
        GL_BeginViewport(0, 0, width, 48);
        Camera_SetupFrustum((float)width * _DAT_0055283c, origin);
        GL_EndOpenGL();
        const bool full = CameraTopViewEnabled || gUserSettings.GetAntilag(ANTILAG_INTERFACE);
        GL_BeginViewport(0, 0, width, full ? 480 : 432);
    } else if (SceneFlag == 4) {
        GL_BeginViewport(0, 0, 640, 480);
    } else {
        GL_BeginViewport(0, 80, 640, 320);
    }
    // Se reutiliza la preparación GL/CPU existente, sin comandos de dibujo.
    // Los pushes se balancean aquí; no queda una pasada abierta entre ticks.
    Camera_BuildMouseRay(DAT_083a427c, DAT_083a4278, (float *)&DAT_083a4110);
    CaptureCamera();
    GL_EndOpenGL();
}

void PrepareItems() {
    if (SceneFlag != 5 || CameraTopViewEnabled) return;
    BYTE *items = (BYTE *)DAT_07e12840;
    for (int slot = 0; slot < 1000; ++slot) {
        BYTE *base = items + slot * 0x204;
        if (!base[72]) continue;
        OBJECT &o = *(OBJECT *)(base + 72);
        const char visible = (char)Frustum_TestSphere(o.Position, 400.0f);
        base[424] = visible;
        if (!visible) continue;
        int type = o.Type;
        if (type >= 624 && type < 784) type = 390;
        else if (type == 860) {
            const int variant = (*(int *)(base + 8) >> 3) & 15;
            if (variant == 0) type = 947;
            else if (variant == 2) type = 948;
        }
        if (type < 0 || type >= 1200) continue;
        float point[3] = { o.Position[0], o.Position[1], o.Position[2] + _DAT_0055284c };
        int x, y;
        Camera_ProjectWorldToScreen(point, &x, &y);
        *(short *)(base + 164) = (short)x;
        *(short *)(base + 166) = (short)y;
    }
}

void PrepareCharacter(CHARACTER &c) {
    OBJECT &o = c.Object;
    void *model = (void *)(DAT_05828d58 + o.Type * 0xbc);
    if (*(short *)((char *)model + 0x26) == 0) return;
    // DESVIACION: la sincronización del equipo puede cambiar piezas/acción y
    // liberar tela. Debe preceder a su pose SIM y no ejecutarse desde el draw.
    if (SceneFlag == 5 && (void *)&c == (void *)DAT_07abf5d8) {
        HeroEquipWatchdog((int)&c);
        model = (void *)(DAT_05828d58 + o.Type * 0xbc);
    }
    char context[0xbc];
    memcpy(context, model, sizeof(context));
    if ((void *)&c == (void *)DAT_07abf5d8 && SceneFlag == 5) o.Visible = 1;
    ObjectPose_PrepareSimulation((int)&c, 1);
    CharacterCloth_Tick((int)&c);
    const int slot = (int)(((uintptr_t)&c - (uintptr_t)DAT_07abf5d0) / sizeof(CHARACTER));
    const bool hero = (void *)&c == (void *)DAT_07abf5d8 && SceneFlag == 5;
    const int highlighted = !hero && (slot == SelectedCharacter || slot == SelectedNpc);
    CharacterEffectScratch effects;
    Equipment_BeginCharacter((int)&c);
    Equipment_SetStage(CharacterEffects_EarlyStage);
    CharacterEffects_TickEarly((int)&c, effects);
    Equipment_EndCharacter();
    Equipment_Tick((int)&c, highlighted);
    Equipment_BeginCharacter((int)&c);
    Equipment_SetStage(CharacterEffects_LateStage);
    CharacterEffects_TickLate((int)&c, effects);
    Equipment_EndCharacter();
    memcpy(model, context, sizeof(context));
    const bool subtypeNpc = o.Type == 390 && o.SubType >= 206 && o.SubType <= 208;
    if (!subtypeNpc && !c.Change) {
        for (int part = 0; part < 6; ++part) {
            PART_t &piece = c.BodyPart[part];
            if (piece.Type == -1) continue;
            const int type = gContentCatalog.EntityDrawModel(&c, piece.Type);
            if (type != 707 || o.Alpha <= _DAT_005524f8) continue;
            void *pants = (void *)(DAT_05828d58 + type * 0xbc);
            memcpy(context, pants, sizeof(context));
            *(float *)((char *)pants + 0x68) = o.Scale;
            memcpy((char *)pants + 0x6c, o.Position, sizeof(o.Position));
            DivineSkirt_Tick((int)&c, type, (int)&piece, pants);
            memcpy(pants, context, sizeof(context));
        }
    }
}

bool SceneReady() {
    if (SceneFlag == 2) return DAT_083a7c49 != 0;
    if (SceneFlag == 4) return CharSelectSceneInitialized != 0 && DAT_05826cb0 > 50;
    return SceneFlag == 5 && DAT_083a7c10 != 0 && DAT_07e11d1c <= 30;
}
}

void Simulation_PrepareStep()
{
    Equipment_BeginStep();
    WorldEffects_BeginStep();
    if (!SceneReady() || !DAT_07abf5d0 || !DAT_05828d58) return;
    PrepareCamera();
    // BMD_Animation usa estos temporales compartidos; la publicación SIM sólo
    // conserva sus matrices de salida, sin filtrar contexto a otros callers.
    char quaternions[sizeof(DAT_05826e18)];
    char bonesScratch[sizeof(g_BoneScratch)];
    float root[12];
    memcpy(quaternions, DAT_05826e18, sizeof(quaternions));
    memcpy(bonesScratch, g_BoneScratch, sizeof(bonesScratch));
    memcpy(root, &DAT_06989c9c, sizeof(root));
    const float lod = _DAT_005597c8;
    Terrain_PrepareSimulation();
    for (int slot = 0; slot < 400; ++slot) {
        CHARACTER &c = ((CHARACTER *)DAT_07abf5d0)[slot];
        const OBJECT &o = c.Object;
        if (!o.Live || (!o.Visible && !((void *)&c == (void *)DAT_07abf5d8 && SceneFlag == 5))) continue;
        PrepareCharacter(c);
    }
    // El panel de creación dibuja otra CHARACTER fuera del pool de 400.
    if (SceneFlag == 4 && DAT_083a7c14 > 26 && DAT_083a7c14 < 29 && DAT_005616a4 >= 0) {
        CHARACTER &preview = *(CHARACTER *)DAT_07abf050;
        preview.Object.Angle[0] = 0.0f;
        preview.Object.Angle[1] = 5.0f;
        preview.Object.Scale = 1.0f;
        PrepareCharacter(preview);
    }
    memcpy(DAT_05826e18, quaternions, sizeof(quaternions));
    memcpy(g_BoneScratch, bonesScratch, sizeof(bonesScratch));
    memcpy(&DAT_06989c9c, root, sizeof(root));
    _DAT_005597c8 = lod;
    PrepareItems();
    // DESVIACION: selección una vez tras cada paso, también al recuperar
    // demoras. El input siguiente consume estos targets con la latencia previa.
    if (SceneFlag == 5 || (SceneFlag == 4 && DAT_005616b0 == -1))
        Mouse_UpdateHoverTargets();
}

void Simulation_RestorePickingCamera()
{
    if (!camera.valid || camera.scene != SceneFlag || camera.world != gMapManager.GetCurrentMap()) return;
    memcpy(CameraMatrix, camera.matrix, sizeof(camera.matrix));
    memcpy(CameraRayOriginX_arr, camera.origin, sizeof(camera.origin));
    memcpy(&DAT_083a4110, camera.target, sizeof(camera.target));
    // No restaurar la pose aquí: Camera3D puede haber recibido un giro nuevo.
    // El rayo conserva su muestra SIM y el giro se aplicará en el próximo tick.
    ViewportCenterX = camera.centerX; ViewportCenterY = camera.centerY;
    OpenglWindowWidth = camera.width; OpenglWindowHeight = camera.height;
    OpenglWindowX = camera.x; OpenglWindowY = camera.y;
    _DAT_083a42a4 = camera.projection[0]; _DAT_083a42a8 = camera.projection[1];
    DAT_083a42a4 = camera.projectionBits[0]; DAT_083a42a8 = camera.projectionBits[1];
    ScreenCenterYFlip = camera.centerYFlip;
}

void Simulation_BeginRender()
{
    // Conservar también cambios de Camera3D recibidos entre pasos: un draw sin
    // tick no debe consumirlos ni dejar la pose del preview como estado lógico.
    CapturePose(renderCamera);
    Simulation_RestorePickingCamera();
    if (camera.valid && camera.scene == SceneFlag && camera.world == gMapManager.GetCurrentMap())
        ApplyPose(camera);
}

void Simulation_EndRender()
{
    ApplyPose(renderCamera);
    Simulation_RestorePickingCamera();
}
