#pragma once

struct RenderCameraPose;
bool Simulation_ReadCameraPose(RenderCameraPose& out);

// Estado derivado autoritativo publicado al finalizar cada paso de 40 ms.
void Simulation_PrepareStep();

// Recupera matriz/proyección/rayo SIM antes del input; conserva giros nuevos.
void Simulation_RestorePickingCamera();

// Acota las poses temporales de los previews al dibujo.
void Simulation_BeginRender();
void Simulation_EndRender();
