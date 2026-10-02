# Historial de comentarios: `src/Config/`

Comentarios de desarrollo movidos desde `src/Config/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en el tag `0.97.00`.

## `src/Config/Config.h`

### globals de Config_Load (g_ScreenW / g_ScreenH) (línea 27)

```cpp
// g_ScreenW / g_ScreenH REMOVIDAS (2026-08-26). Eran variables propias de
// Config_Load.cpp que duplicaban WindowWidth / WindowHeight (DAT_0056156c/70,
// declaradas en globals.h) sin ninguna sincronizacion: el render usaba las
// segundas y nada copiaba un par al otro. Usar WindowWidth / WindowHeight.
```

### g_MusicOn (línea 33)

```cpp
// (definido en globals.cpp). No declarar una variable propia aca: hasta 2026-08-17
// habia dos memorias distintas — esta se escribia y la de Music.cpp se leia — y
// por eso PlayMp3 salia siempre por el early-return.
```

### overrides de server.cfg (línea 41)

```cpp
// -- Overrides de server.cfg (DESVIACION DOCUMENTADA, 2026-09-24) ------------
// El 0.97k solo lee estas opciones del registro (la clave Config de Webzen/Mu),
// que es donde las deja el launcher oficial.  Sin launcher no hay forma de
// mandar el cliente preconfigurado, asi que `server.cfg` acepta las mismas
// tres claves y, cuando estan, ganan sobre el registro.
```

### modo ventana (línea 52)

```cpp
// -- Modo ventana (DESVIACION DELIBERADA, 2026-09-27) ------------------------
// El 0.97k solo corre a pantalla completa: WinMain busca un modo de video con
// dmBitsPerPel == 16 y StartWindow crea la ventana WS_POPUP en (0,0).  En
// Windows 10/11 no existe ningun modo de 16 bits, asi que ese
// ChangeDisplaySettings NO ENCUENTRA NADA y el cliente queda como un popup sin
// bordes del tamano configurado, pegado a la esquina, sobre el escritorio.
//
// Se porta el modo ventana del DLL de inyeccion (Source/Client/Main/Window.cpp,
// CWindow::StartWindow + ChangeDisplaySettingsFunction), que ademas elige el
// modo de video por la mayor profundidad de color disponible en vez de exigir
// 16 bits.
```

## `src/Config/Config_Load.cpp`

### globals (g_ScreenW / g_ScreenH) (línea 53)

```cpp
// g_ScreenW / g_ScreenH ELIMINADAS (2026-08-26): eran variables propias que
// duplicaban WindowWidth / WindowHeight (DAT_0056156c/70) sin sincronizarse con
// ellas. Ver la nota en el switch de resolucion, mas abajo. Config_Load ahora
// escribe los globals reales, igual que el binario.
```

### Config_Load, paso 2/3 (version) (línea 117)

```cpp
    // de ahi viene el nombre "0.97k".  El port calculaba versionWords y lo
    // descartaba, y leia el ini directo sobre m_ExeVersion.
```

### Config_Load, valor "ID" del registro (línea 147)

```cpp
        // ¡OJO! En el port anterior se escribía a m_ExeVersion pisando la versión de config.ini.
```

### Config_Load, paso 5 (resolucion) (línea 179)

```cpp
    // --- 5. Resolution -> screen dimensions ---
    //
    // 2026-08-26: esto escribia `g_ScreenW`/`g_ScreenH`, que eran DOS VARIABLES
    // PROPIAS de este archivo (el comentario decia "DAT_0056156c/70" y no lo
    // eran). El render entero — ventana, viewport, ortho, proyeccion, mouse —
    // lee `WindowWidth`/`WindowHeight` (DAT_0056156c/70), con 169 y 50
    // lecturas respectivamente, y nada copiaba un par al otro: `g_ScreenH` no
    // tenia ni un solo lector y `g_ScreenW` solo alimentaba la escala de abajo.
    // O sea el valor del registro se calculaba y se tiraba, y cambiar
    // `Resolution` no tenia ningun efecto.
    //
    // IDA escribe WindowWidth/WindowHeight aca mismo (0041E0A0 L121-138), asi
    // que esto no es una unificacion inventada: es restaurar el original.
```

### Config_Load, paso 6 (escalas) (línea 208)

```cpp
    // El port solo tenia la X, y calculada desde `g_ScreenW` (la variable
    // muerta). La Y no se calculaba en ningun lado: quedaba en su valor de
    // inicializacion, 1.0f, con 18 lectores dividiendo por ella. A 640x480 no
    // se notaba porque la escala real ES 1.0; a cualquier otra resolucion el
    // texto se mezclaba con el layout 640x480 sin corregir y el error crecia
    // en proporcion.
    //
```

### Config_ReadServerAddr, claves de identidad (línea 356)

```cpp
// Además acepta líneas `clave=valor` con la identidad del server, que es de
// donde sale la clave de encriptación de MuEmu (2026-08-26):
```

### Config_ReadServerAddr, WindowMode / Borderless (línea 470)

```cpp
                    // DESVIACION DELIBERADA (2026-09-27): modo ventana, portado
```

### Config_ReadServerAddr, MusicOnOff / SoundOnOff / Resolution (línea 490)

```cpp
                    // DESVIACION DOCUMENTADA (2026-09-24): el 0.97k lee estas
                    // tres del registro y nada mas.  Sin launcher que las deje
                    // escritas no hay forma de distribuir el cliente ya
                    // configurado, asi que se aceptan tambien aca y Config_Load
                    // las aplica DESPUES del registro (ver 4b).
```

### Config_ReadServerAddr, derivacion de la clave de MuEmu (línea 569)

```cpp
    // Sin esas líneas quedan los valores por defecto (CustomerName="MuLinux"),
    // que es el comportamiento que tenía el cliente antes de esto.
```

## `src/Config/Config_TokenParser.cpp`

### cabecera del archivo (línea 2)

```cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.

// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.
```
