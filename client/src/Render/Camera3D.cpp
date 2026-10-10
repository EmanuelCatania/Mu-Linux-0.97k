#include "stdafx.h"
#include "Render/Camera3D.h"
#include "globals.h"
#include "functions.h"

CCamera3D gCamera3D;

namespace {
    const float kDefaultFov    = 50.0f;
    const float kDefaultPitch  = 48.5f;
    const float kDefaultHeight = 150.0f;
    const float kDefaultYaw    = -45.0f;   // Game_CharSelectTick
}

void CCamera3D::Toggle()
{
    if (SceneFlag != 5) return;
    m_Enabled = !m_Enabled;
    m_Dragging = false;
    UI_AddNotice((char*)(m_Enabled ? "Camara 3D activada" : "Camara 3D desactivada"), 1);
}

void CCamera3D::Restore()
{
    if (!m_Enabled || SceneFlag != 5) return;
    m_Fov = kDefaultFov;
    m_Pitch = kDefaultPitch;
    m_Height = kDefaultHeight;
    CameraAngle[2] = kDefaultYaw;
    UI_AddNotice((char*)"Camara 3D restaurada", 1);
}

void CCamera3D::BeginDrag(int x, int y)
{
    if (!m_Enabled || SceneFlag != 5) return;
    m_Dragging = true;
    m_CursorX = x;
    m_CursorY = y;
}

// Pasos y topes del DLL: 6 grados de giro por evento, 2.42 de inclinación y
// 44 de altura, inclinación entre 22.5 y 90.
void CCamera3D::Drag(int x, int y)
{
    if (!m_Enabled || !m_Dragging || SceneFlag != 5) return;
    if (x > m_CursorX) CameraAngle[2] += 6.0f;
    if (x < m_CursorX) CameraAngle[2] -= 6.0f;
    if (CameraAngle[2] > 315.0f || CameraAngle[2] < -405.0f) CameraAngle[2] = kDefaultYaw;
    if (y > m_CursorY && m_Pitch > 22.5f) { m_Pitch -= 2.42f; m_Height -= 44.0f; }
    if (y < m_CursorY && m_Pitch < 90.0f) { m_Pitch += 2.42f; m_Height += 44.0f; }
    m_CursorX = x;
    m_CursorY = y;
}

// Rueda hacia atrás aleja (más FOV), hacia adelante acerca; entre el 50 % y
// el 200 % del valor inicial, de a 2 grados.
bool CCamera3D::Wheel(int delta)
{
    if (!m_Enabled || m_Dragging || SceneFlag != 5 || delta == 0) return false;
    if (delta < 0 && m_Fov <= kDefaultFov * 2.0f) m_Fov += 2.0f;
    if (delta > 0 && m_Fov >= kDefaultFov * 0.5f) m_Fov -= 2.0f;
    return true;
}

float CCamera3D::GetViewFar() const
{
    const float lift = m_Height - kDefaultHeight;
    return 3000.0f + (lift < 0.0f ? -lift : lift) * 3.0f;
}
