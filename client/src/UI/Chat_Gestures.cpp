#include <cstdint>
// Chat_Gestures.cpp — gestos del personaje disparados por el texto del chat.
//
// IDA: sub_497C70 (0x00497C70) revisa la linea que se envia contra grupos de
// palabras de GlobalText[270..347] (mas unos emoticones fijos de .rdata) y,
// con la primera coincidencia, anima al heroe con SetActionClass y le avisa
// al server con PMSG_ACTION_RECV [C1][05][18][dir][tipo].
// Lo llaman WndProc (Enter en el chat) y Chat_InputTick, solo si el heroe no
// va montado en Uniria/Dinorant o esta en zona segura.
//
// Nota de fidelidad: el binario manda el paquete dos veces por gesto — una
// dentro de SetActionClass (solo si la accion actual es de reposo, 1..12) y
// otra incondicional desde sub_497C70. Se replica tal cual.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <initializer_list>
#include "Model/Model_CustomAnim.h"
#include "Sound/EmoteSound.h"
#include <cstdio>
#include <cstdlib>

// [C1][05][18][dir][tipo]; dir = octante del facing del heroe.
void __cdecl SendRequestAction(BYTE actionType)
{
    BYTE* hero = (BYTE*)(uintptr_t)Hero;
    if (!hero) return;
    BYTE pkt[5] = {
        0xC1, 0x05, 0x18,
        (BYTE)((std::int64_t)((*(float*)(hero + 36) + 22.5f) * 0.022222223f + 1.0f) & 7),
        actionType
    };
    gNetwork.SendC1(pkt, sizeof(pkt));
}

// IDA: SetActionClass (0x00497870). La elfa usa la animacion siguiente salvo
// en las acciones 123..128, que son comunes a todas las clases.
void __cdecl SetActionClass(CHARACTER* character, OBJECT* object, int action, int actionType)
{
    const BYTE* c = reinterpret_cast<const BYTE*>(character);
    const BYTE* o = reinterpret_cast<const BYTE*>(object);
    BYTE cur = *(BYTE*)&((OBJECT*)(uintptr_t)(o))->CurrentAction;
    if (cur == 0 || cur > 12) return;
    int act = action;
    // Los gestos 68..73 (5.2) no tienen version femenina: no se corren.
    if ((*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->Class & 7) == 2 &&
        (action < 123 || action > 128) && (action < 68 || action > 73))
        act = action + 1;
    SetAction(object, act);
    SendRequestAction((BYTE)actionType);
}

// Emoticones de .rdata 0x00559E2C..0x00559E74. En el binario los coreanos
// están en EUC-KR; el texto del cliente es UTF-8, así que acá van en UTF-8.
static char s_Semicolon[] = ";";                   // 0x559E2C
static char s_Kr_TT1[]    = "\xE3\x85\xA0.\xE3\x85\xA0";   // 0x559E30
static char s_Kr_TT2[]    = "\xE3\x85\x9C.\xE3\x85\x9C";   // 0x559E38
static char s_TT[]        = "T_T";                 // 0x559E40
static char s_Kr_Dot1[]   = "\xE3\x85\xA1.\xE3\x85\xA1";   // 0x559E44
static char s_Kr_Dot2[]   = "\xE3\x85\xA1.,\xE3\x85\xA1";  // 0x559E4C
static char s_Kr_Dot3[]   = "\xE3\x85\xA1,.\xE3\x85\xA1";  // 0x559E54
static char s_DashDot[]   = "-.-";                 // 0x559E5C
static char s_DashUnd[]   = "-_-";                 // 0x559E60
static char s_Caret[]     = "^^";                  // 0x559E64
static char s_CaretDot[]  = "^.^";                 // 0x559E68
static char s_CaretUnd[]  = "^_^";                 // 0x559E6C
static char s_Kr_Slash[]  = "/\xE3\x85\xA1";           // 0x559E70
static char s_Kr_Caret[]  = "\xE3\x85\xA1^";           // 0x559E74

static bool AnyGlobal(char* text, int first, int last)
{
    for (int i = first; i <= last; ++i)
        if (FindTextA(text, GlobalText[i], false)) return true;
    return false;
}

static bool AnyOf(char* text, std::initializer_list<char*> words)
{
    for (char* w : words)
        if (FindTextA(text, w, false)) return true;
    return false;
}

static void Gesture(int action, BYTE actionType)
{
    SetActionClass(AsCharacter(Hero), AsObject(Hero), action, actionType);
    SendRequestAction(actionType);
}

// Codigo de red de las acciones propias: kCustomActionCode + n. El 0x18 lo
// retransmite el server sin validar, y NetRecv_18 lo traduce de vuelta.
constexpr int kCustomActionCode = 137;   // 137..255: alcanza para 119 emotes (byte de accion)

// DESVIACION: "/fortnite" (al azar) y "/fortnite N" (1..cantidad) reproducen una
// accion propia de CustomAnim.bmd y avisan al resto de los jugadores.
bool __cdecl Chat_TryFortniteCommand(const char* text)
{
    if (!text || strncmp(text, "/fortnite", 9) != 0 || (text[9] != '\0' && text[9] != ' ')) return false;
    char msg[96];
    const int count = CustomAnim_Count();
    if (!Hero || count <= 0) { UIChatLogWindow_AddText("", "fortnite: no hay emotes cargados", 1); return true; }
    const char* arg = text + 9;
    while (*arg == ' ') ++arg;
    int n;
    // Sin numero: al azar. El 0 (el baile clasico) sale 1 de cada 10 y el resto
    // se reparte entre los demas.
    if (*arg == '\0') n = (count <= 1 || rand() % 10 == 0) ? 0 : 1 + rand() % (count - 1);
    else if (*arg >= '0' && *arg <= '9') n = atoi(arg);
    else { snprintf(msg, sizeof(msg), "fortnite: /fortnite o /fortnite N (0 a %d)", count - 1); UIChatLogWindow_AddText("", msg, 1); return true; }
    if (n < 0 || n >= count) { snprintf(msg, sizeof(msg), "fortnite: N fuera de rango (0 a %d)", count - 1); UIChatLogWindow_AddText("", msg, 1); return true; }
    // Montado en Uniria/Dinorant no se puede, salvo en zona segura (ahi la montura no se dibuja).
    const WORD helper = *(WORD*)&AsCharacter(Hero)->Helper.Type;
    if ((helper == 818 || helper == 819) && !AsCharacter(Hero)->SafeZone) {
        UIChatLogWindow_AddText("", "fortnite: no se puede usar montado", 1);
        return true;
    }
    // Igual que SetActionClass: solo desde reposo (acciones 1..12).
    const BYTE cur = AsCharacter(Hero)->Object.CurrentAction;
    if (cur == 0 || cur > 12) return true;
    const int action = CustomAnim_Acquire(n);
    if (action < 0) return true;
    SetAction(AsObject(Hero), action);
    EmoteSound_Start((unsigned char*)(uintptr_t)Hero, n);
    SendRequestAction((BYTE)(kCustomActionCode + n));
    snprintf(msg, sizeof(msg), "fortnite %d: %s", n, CustomAnim_Name(n));
    UIChatLogWindow_AddText("", msg, 1);
    return true;
}

// IDA: sub_497C70 (0x00497C70)
void __cdecl CheckChatText(char* text)
{
    if (!text || !Hero) return;

    if (AnyGlobal(text, 270, 277)) { Gesture(93, 111); return; }
    if (AnyGlobal(text, 278, 280)) { Gesture(95, 112); return; }
    if (AnyGlobal(text, 281, 286)) { Gesture(97, 113); return; }
    if (AnyGlobal(text, 287, 291)) { Gesture(103, 114); return; }
    if (AnyGlobal(text, 292, 295)) { Gesture(101, 115); return; }
    if (AnyGlobal(text, 296, 302)) { Gesture(105, 116); return; }
    if (AnyOf(text, { s_Semicolon }) || AnyGlobal(text, 303, 305)) {
        Gesture(109, 119); return;
    }
    if (AnyOf(text, { s_Kr_TT1, s_Kr_TT2, s_TT }) || AnyGlobal(text, 306, 309)) {
        Gesture(107, 117); return;
    }
    if (AnyOf(text, { s_Kr_Dot1, s_Kr_Dot2, s_Kr_Dot3, s_DashDot, s_DashUnd }) ||
        AnyGlobal(text, 310, 311)) {
        Gesture(111, 120); return;
    }
    if (AnyOf(text, { s_Caret, s_CaretDot, s_CaretUnd }) || AnyGlobal(text, 312, 316)) {
        Gesture(115, 122); return;
    }
    if (AnyGlobal(text, 317, 321)) { Gesture(99, 118); return; }
    if (AnyGlobal(text, 322, 325)) { Gesture(113, 121); return; }
    if (AnyGlobal(text, 326, 329)) { Gesture(117, 123); return; }
    if (AnyGlobal(text, 330, 334)) { Gesture(119, 124); return; }
    if (AnyGlobal(text, 335, 338)) { Gesture(121, 125); return; }

    if (AnyGlobal(text, 339, 341)) { Gesture(123, 126); return; }
    if (AnyGlobal(text, 342, 343) || AnyOf(text, { s_Kr_Slash, s_Kr_Caret })) {
        Gesture(124, 127); return;
    }
    if (AnyGlobal(text, 344, 347)) { Gesture(128, 128); return; }

    // Gestos de 5.2 (huecos 68..73 de player.bmd). En 5.2 se disparan con
    // GlobalText[783] / [291] / [317] / [348]; en este Text_*.bmd esas entradas
    // estan vacias o son otro texto, asi que van tambien las palabras en ingles
    // (5.2 hace lo mismo con "hustle"). Coinciden dentro de la frase, distinguiendo mayusculas (la primera en mayuscula,
    // como el resto de los gestos: asi no se disparan al escribir texto comun), con o sin
    // barra, igual que el resto de los gestos. Codigos de red 132..136.
    static char w_hustle[] = "Hustle", w_dance[] = "Dance", w_comeon[] = "Come on",
                w_great[] = "Great", w_look[] = "Look around";
    if (AnyOf(text, { w_hustle }))                        { Gesture(68, 132); return; }
    if (AnyGlobal(text, 291, 291) || AnyOf(text, { w_comeon })) { Gesture(69, 133); return; }
    if (AnyGlobal(text, 348, 348) || AnyOf(text, { w_look })) { Gesture(70, 134); return; }
    if (AnyGlobal(text, 317, 317) || AnyOf(text, { w_great })) { Gesture(71, 135); return; }
    if (AnyOf(text, { w_dance }))                         { Gesture(73, 136); return; }
}
