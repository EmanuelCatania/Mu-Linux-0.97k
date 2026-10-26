#pragma once

// Muestras de las dos rutas aleatorias de materiales, externas a OBJECT/BMD.
struct MaterialVisualSample {
    int model=-1;
    int attribute[2]={};
    float u=0, v=0;
    float alpha=1;
    unsigned char action=0;
};
void MaterialVisuals_BeginStep();
void MaterialVisuals_Prepare(MaterialVisualSample &sample, int model, bool attributes, bool draw);
void MaterialVisuals_PrepareObject(const void *owner, int model, bool attributes, bool draw);
void MaterialVisuals_PrepareWeaponObject(int entity, int model);
void MaterialVisuals_TickObjectDraw(int entity);
int ItemMaterials_ResolvePreviewModel(int itemType, int level, int *adjustedLevel=nullptr);
void MaterialVisuals_PrepareUiItem(int itemType, int level=0);
void MaterialVisuals_TickUi();
void MaterialVisuals_TickWingPreviews();
void MaterialVisuals_BeginRender();
const MaterialVisualSample *MaterialVisuals_UiSample(int model);
const MaterialVisualSample *MaterialVisuals_ObjectSample(const void *owner, int model);
const MaterialVisualSample *MaterialVisuals_Current();
const MaterialVisualSample &MaterialVisuals_DrawSample(const void *owner, int model);
class MaterialVisualScope {
    const MaterialVisualSample *previous;
public:
    explicit MaterialVisualScope(const MaterialVisualSample *sample);
    ~MaterialVisualScope();
};
// El creador original conserva su RNG; esta entrada sólo consume la muestra.
struct OBJECT;
int ItemObjectAttributeForRender(OBJECT* entity);
void ItemMaterials_PrepareEquipmentUi();
void ItemMaterials_PrepareQuestUi();
