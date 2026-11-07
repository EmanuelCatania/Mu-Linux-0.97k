// GameGuard_LegacyHealth.cpp — arranque de nProtect (desactivado) y GameGuard_HealthCheck.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD GoldenArcherOpenType;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl operator_delete(void* ptr);
extern void ClearActionObject(void);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// FUN_0053d430 @ 0x0053D430 (325 bytes) — PreInitNPGameMon(gameName)
//
// Arranca nProtect GameGuard. Singleton: sale si lpParameter ya esta seteado.
// Aloca el contexto de 0x34C bytes, instala su propio TopLevelExceptionFilter,
// llama FUN_0053d890 (el init grande, 4130 bytes) y restaura el filtro anterior.
//
// FUN_0053d890 lanza DOS procesos de nProtect y es donde se veia el splash:
//   1. GameGuard.des  — CreateProcess + WaitForSingleObject(INFINITE), espera exit 1877
//   2. GameMon.des    — CreateProcess(CREATE_SUSPENDED) + ResumeThread
// El banner lo dibujan ESOS procesos; el cliente no tiene codigo de splash.
//
// Desactivada a proposito: los .des son binarios propietarios de nProtect que no
// estan (ni pueden estar) en el repo, y sin ellos la cadena real aborta el arranque.
void __cdecl FUN_0053d430(unsigned char *gameName) {
    (void)gameName;
}

// GameGuard_HealthCheck @ 0x0053EA90 (44 lines) — GameGuard per-tick health check
// Checks GG process status, heartbeat event, returns error codes.
// In our build, GameGuard is disabled — return 0x755 (OK/running).
int __cdecl GameGuard_HealthCheck(void *param) {
    (void)param;
    return 0x755; // GG status OK
}
