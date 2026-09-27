#pragma once

// Volcado de crash (.dmp) — DESVIACION DELIBERADA respecto del binario.
//
// El 0.97k original NO genera minidumps: buscando en `Cliente armado/main.exe`
// no aparecen ni `MiniDumpWriteDump`, ni `dbghelp`, ni `imagehlp`.  Lo unico que
// tiene es `MuError.log`, un log de texto que escribe `CErrorReport` (y que en
// este port esta stubbeado, ver Core/Runtime_Small.cpp).
//
// Se agrega por pedido, copiando el enfoque de
// `Source/MuServer/ConnectServer/MiniDump.cpp`, para que un crash del cliente
// deje un .dmp analizable igual que los del server.
//
// Dos diferencias con la version del server, las dos a proposito:
//
//  - dbghelp.dll se carga con LoadLibrary/GetProcAddress en vez de enlazar
//    dbghelp.lib.  Asi el cliente sigue arrancando en una maquina donde la DLL
//    no este o sea vieja: simplemente no se escribe el dump.
//
//  - No instala su propio SetUnhandledExceptionFilter.  El cliente YA tiene uno
//    (`DbgUnhandledException` en WinMain.cpp) que vuelca el stack y los
//    registros a debug.log; poner un segundo filtro reemplazaria al primero y
//    se perderia esa informacion.  Esto es una funcion que aquel llama.

struct _EXCEPTION_POINTERS;

class CMiniDump
{
public:

    // Escribe el .dmp en el directorio de trabajo.  Devuelve true si lo logro y
    // deja la ruta en outPath.  Es seguro llamarla desde dentro de un filtro de
    // excepciones: no aloca ni usa la CRT mas alla de wsprintf.
    static bool Write(_EXCEPTION_POINTERS* info, char* outPath, unsigned int outPathSize);
};
