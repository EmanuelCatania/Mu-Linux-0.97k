#pragma once
// Camera3D.h — cámara orbital del juego.
//
// DESVIACION (DLL Camera3D.cpp + Controller.cpp): el 0.97k tiene la cámara
// fija.  F10 activa la cámara 3D y F11 la vuelve a la posición inicial; con
// ella activa, arrastrar con el botón del medio la gira (horizontal) y la
// inclina (vertical), y la rueda cambia el FOV.  MoveMainCamera toma de acá
// el FOV, la inclinación, la altura y el alcance.

class CCamera3D {
public:
    bool IsEnabled() const { return m_Enabled; }
    void Toggle();
    void Restore();

    void BeginDrag(int x, int y);
    void EndDrag() { m_Dragging = false; }
    void Drag(int x, int y);
    // true si la rueda se usó para el zoom (y no tiene que llegar al chat).
    bool Wheel(int delta);

    float GetFov() const { return m_Fov; }
    float GetPitch() const { return m_Pitch; }
    float GetHeight() const { return m_Height; }
    // Alcance del plano lejano; crece al alejar la cámara del suelo.
    float GetViewFar() const;

private:
    bool  m_Enabled  = false;
    bool  m_Dragging = false;
    int   m_CursorX  = 0;
    int   m_CursorY  = 0;
    float m_Fov      = 50.0f;   // DLL: SetFloat(0x00524CC1, 50.0f)
    float m_Pitch    = 48.5f;   // binario: 0x00552D54
    float m_Height   = 150.0f;  // binario: 0x0055297C
};

extern CCamera3D gCamera3D;
