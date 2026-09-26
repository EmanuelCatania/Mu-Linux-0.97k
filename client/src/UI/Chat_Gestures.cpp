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

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// [C1][05][18][dir][tipo]; dir = octante del facing del heroe.
void __cdecl SendRequestAction(BYTE actionType)
{
    BYTE* hero = (BYTE*)(uintptr_t)Hero;
    if (!hero) return;
    BYTE pkt[5] = {
        0xC1, 0x05, 0x18,
        (BYTE)((__int64)((*(float*)(hero + 36) + 22.5f) * 0.022222223f + 1.0f) & 7),
        actionType
    };
    Net_SendC1Packet(pkt, sizeof(pkt));
}

// IDA: SetActionClass (0x00497870). La elfa usa la animacion siguiente salvo
// en las acciones 123..128, que son comunes a todas las clases.
void __cdecl SetActionClass(int c, int o, int action, int actionType)
{
    BYTE cur = *(BYTE*)(o + 261);
    if (cur == 0 || cur > 12) return;
    int act = action;
    if ((*(BYTE*)(c + 444) & 7) == 2 && (action < 123 || action > 128))
        act = action + 1;
    SetAction(o, act);
    SendRequestAction((BYTE)actionType);
}

// Emoticones de .rdata 0x00559E2C..0x00559E74 (los bytes >= 0x80 son EUC-KR).
static char s_Semicolon[] = ";";                   // 0x559E2C
static char s_Kr_TT1[]    = "\xA4\xD0.\xA4\xD0";   // 0x559E30
static char s_Kr_TT2[]    = "\xA4\xCC.\xA4\xCC";   // 0x559E38
static char s_TT[]        = "T_T";                 // 0x559E40
static char s_Kr_Dot1[]   = "\xA4\xD1.\xA4\xD1";   // 0x559E44
static char s_Kr_Dot2[]   = "\xA4\xD1.,\xA4\xD1";  // 0x559E4C
static char s_Kr_Dot3[]   = "\xA4\xD1,.\xA4\xD1";  // 0x559E54
static char s_DashDot[]   = "-.-";                 // 0x559E5C
static char s_DashUnd[]   = "-_-";                 // 0x559E60
static char s_Caret[]     = "^^";                  // 0x559E64
static char s_CaretDot[]  = "^.^";                 // 0x559E68
static char s_CaretUnd[]  = "^_^";                 // 0x559E6C
static char s_Kr_Slash[]  = "/\xA4\xD1";           // 0x559E70
static char s_Kr_Caret[]  = "\xA4\xD1^";           // 0x559E74

static bool AnyGlobal(char* text, int first, int last)
{
    for (int i = first; i <= last; ++i)
        if (FindText(text, GlobalText[i], false)) return true;
    return false;
}

static bool AnyOf(char* text, std::initializer_list<char*> words)
{
    for (char* w : words)
        if (FindText(text, w, false)) return true;
    return false;
}

static void Gesture(int action, BYTE actionType)
{
    SetActionClass((int)Hero, (int)Hero, action, actionType);
    SendRequestAction(actionType);
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
    if (AnyGlobal(text, 344, 347)) Gesture(128, 128);
}
