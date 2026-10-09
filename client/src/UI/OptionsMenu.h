#pragma once
// OptionsMenu.h — ventana de opciones del menú Escape (ErrorMessage 150).
//
// IDA: el 0.97k dibuja en RenderErrorMessage (0x0051AF50) y atiende en
// UI_InGameMenu (0x00514310) una ventana de 4 botones: volver, ataque
// automático, sonido de susurros y cerrar.
// DESVIACION (DLL OptionsMenu.cpp): se reemplaza por el menú expandido del DLL,
// con las mismas cajas (bitmap 240) y textos de Text.bmd (GlobalText 385-388 y
// 919-936).  Los cambios se escriben en Config.ini al momento.

class COptionsMenu {
public:
    // Llamar al principio de RenderErrorMessage: sin cartel, vuelve a la lista.
    void OnErrorMessageFrame();
    void Render();
    // true si el click cayó en el menú; si no, el caller lo descarta.
    bool UpdateMouse();

private:
    enum Page { PAGE_MAIN, PAGE_GENERAL, PAGE_COUNT };

    void RenderMain();
    bool UpdateMain();
    void RenderGeneral();
    bool UpdateGeneral();

    void RenderBox(float x, float y, float width, float height, bool title = false) const;
    void RenderLabel(float x, float y, float width, const char* text) const;
    void RenderToggle(float y, const char* label, bool value) const;
    void RenderLevelBar(float y, const char* label, bool enabled, int level) const;
    void RenderLanguage(float y) const;
    void RenderMusicControls(float y) const;

    bool Clicked(int x, int y, int width, int height) const;
    void ConsumeClick() const;
    int  LevelBarHit(int y) const;
    void ChangeLanguage(int language);

    Page m_Page = PAGE_MAIN;
};

extern COptionsMenu gOptionsMenu;
