#pragma once

// Muestra compartida de presentación: equipo y sprites del cuerpo.
// IDA: equipo de RenderCharacter (0x00456770) y RenderLinkObject (0x00455430).
void Equipment_BeginStep();
void Equipment_Tick(int entity, int highlighted);
void Equipment_BeginCharacter(int entity);
void Equipment_EndCharacter();
void Equipment_SetStage(int stage);
void Equipment_Invalidate(const void *entity);
void Equipment_Draw(int entity, int stage);
bool Equipment_IsDrawing();
void Equipment_AddSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode);
void Equipment_WingSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode);
void Equipment_AddMesh(int model, const void *object, const float *light,
    float alpha, unsigned int level, unsigned char option, unsigned int flags, char translate);
void Equipment_EmitWing790(void *model, int object, int owner);

// Los modelos son contexto compartido; el tick de un link no lo publica.
class EquipmentModelContext {
    void *model;
    char saved[0xbc];
public:
    explicit EquipmentModelContext(void *value);
    ~EquipmentModelContext();
};
