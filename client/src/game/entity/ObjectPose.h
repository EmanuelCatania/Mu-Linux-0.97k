#pragma once

struct OBJECT;

// Prepara la pose esquelética en el destino elegido por el caller. No dibuja,
// no avanza frames ni emite efectos. Conserva las escrituras de contexto BMD
// y scratch de BMD_Animation; el caller SIM debe aislarlos del render.
// translate es el mismo flag de Calc_RenderObject: 0 incluye escala/origen
// en las matrices; 1 deja la traslación para Skeleton_Transform.
void ObjectPose_PrepareBones(OBJECT* entity, void* model, float* bones, char translate);

// Preparación SIM sin dibujo. El caller conserva/restaura el contexto BMD y
// sus temporales; los destinos persistentes son huesos y OBB de la entidad.
void ObjectPose_PrepareSimulation(OBJECT* entity, char translate);
void ObjectPose_PrepareBounds(OBJECT* entity, char translate);

void ObjectPose_TickMeshEvents(OBJECT* entity, void *model, float *bones, char translate);

// Lote efímero de poses principales de personajes visibles; sólo presentación.
void ObjectPose_BeginBatch();
void ObjectPose_EndBatch();

// Restituye sólo resultados cuyos parámetros, fuentes y huesos siguen iguales.
bool ObjectPose_RestoreGeometry(void* model,const float* bones,float rotationScale,char translate,
    const float* direction,const float* coefficients,float* positions,float* normals,float* light);
