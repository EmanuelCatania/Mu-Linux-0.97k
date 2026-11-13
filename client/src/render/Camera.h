#pragma once
// Camera.h — Camera_SetupFrustum, MouseRay, SetMatrix.
void __cdecl Camera_SetupFrustum(float fov_w, float *cam_pos);
void __cdecl Camera_MouseRay(int mouseX, int mouseY, float *out_ray);
void __cdecl Camera_SetMatrix(float *cam_pos);
