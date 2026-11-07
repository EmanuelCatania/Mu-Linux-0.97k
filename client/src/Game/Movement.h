#pragma once
// Movement.h — Waypoint system, angle lerp, pathfinding, packet handlers 0x10/0x11.
float Entity_GetDirCode(float x1, float y1, float x2, float y2);
float Angle_ShortestLerp(float current, float target, float step);
float Angle_Delta(float a, float b, char abs_flag);
void  Entity_UpdateFacing(float* pos, float* entity_angle, float* target, float step);
void  Entity_ApplyRotation(float* out, float* quat, float* vec);
