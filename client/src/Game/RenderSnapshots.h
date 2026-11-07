#pragma once
#include <vector>

struct RenderPose {
    float position[3] = {};
    float angle[3] = {};
    float headAngle[3] = {};
    float animationFrame = 0, priorAnimationFrame = 0;
    float scale = 1, alpha = 1;
    unsigned char action = 0, priorAction = 0;
};
struct RenderPosePair {
    RenderPose previous, current;
    bool continuous = false;
};

// Pose autoritativa de cámara; la proyección/rayo de picking permanece aparte.
struct RenderCameraPose {
    float position[3]{}, angle[3]{}, fov=0, farPlane=0, distance=0;
    int viewport[4]{}, mode=0;
};
struct RenderCameraPair {
    RenderCameraPose previous, current;
    bool continuous=false;
};
bool RenderSnapshots_Camera(float maxDisplacement, RenderCameraPair& out);

// Funciones matemáticas de presentación, sin escrituras en los pools.
float RenderSnapshots_Alpha(int remainder);
RenderPose RenderSnapshots_Interpolate(const RenderPosePair& pair, float alpha);

// Captura antes y después de CADA paso, también durante recuperación.
void RenderSnapshots_BeginStep();
void RenderSnapshots_EndStep();
void RenderSnapshots_NewCharacter(const void* entity);
void RenderSnapshots_NewItem(int slot);
void RenderSnapshots_NewDamagePoint(int slot);

// El caller decide el umbral de desplazamiento antes de activar cada dominio.
// No se introduce una política de teletransporte implícita para el gameplay.
bool RenderSnapshots_Character(int slot, float maxDisplacement, RenderPosePair& out);
bool RenderSnapshots_Part(int slot, int part, float maxDisplacement, RenderPosePair& out);
bool RenderSnapshots_Item(int slot, float maxDisplacement, RenderPosePair& out);
bool RenderSnapshots_DamagePoint(int slot, RenderPosePair& out);

// Objetos del mundo registrados por CreateObject, con generación de su nodo.
bool RenderSnapshots_Object(const void* node, float maxDisplacement, RenderPosePair& out);
// Objetos del mapa cuya pose cambió en el último paso (o sin continuidad).
// Los quietos ya tienen en memoria la pose que daría la interpolación.
const std::vector<const void*>& RenderSnapshots_MovingObjects();
