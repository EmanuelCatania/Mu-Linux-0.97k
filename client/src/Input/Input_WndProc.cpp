// Input_WndProc.cpp — mensajes de mouse, IME y texto del WndProc.
//
// IDA: WndProc (0x004149D0), segunda pasada del dispatch (mouse/IME) y WM_CHAR.

#include "stdafx.h"

// Helper de src/UI/Chat_InputTick.cpp — envía una línea de chat
// escrita en InputText[0] (DAT_07db8710 slot 0) por WM_CHAR; la llama el
// handler de Enter en WndProc cuando InputEnable=1 y el buffer no está vacío.
extern "C" void Chat_SendChatLine(const char* text);
extern "C" BYTE InputTextHide[10];

// Chat_TryAssignMacro -- "/1 texto" guarda una macro en la tecla 1.
//
// DESVIACION DOCUMENTADA.  El 0.97k NO tiene esto: sus macros
// salen unicamente de Data\\Macro.txt (OpenMacro 0x50F750) y no hay una sola
// escritura al array fuera de ese loader -- verificado con los xrefs de
// 0x07E0FFC8.  La asignacion por chat aparece recien en MU 5.2
// (ZzzInterface.cpp, CheckCommand): compara los dos primeros caracteres contra
// "/1".."/9" y "/0", y copia el texto desde el indice 3.
//
// Se porta de ahi, con dos diferencias deliberadas:
//   - 5.2 termina el slot con `MacroText[i][iTextSize-3] = NULL` donde
//     iTextSize quedo en el ULTIMO indice recorrido, asi que se come el ultimo
//     caracter del mensaje.  Aca se copia entero.
//   - la lista de comandos prohibidos (CheckMacroLimit) se compara contra los
//     literales, no contra GlobalText: los indices de esa tabla son los de 5.2
//     y no tienen por que coincidir con los del 0.97k.
//
// Devuelve true si la linea era una asignacion (y entonces no se envia).
static bool Chat_TryAssignMacro(const char* text)
{
    if (!text || text[0] != '/') return false;
    if (text[1] < '0' || text[1] > '9') return false;
    if (text[2] != ' ') return false;

    const char* body = text + 3;
    while (*body == ' ') ++body;
    if (*body == '\0') return false;   // "/1 " solo: no es asignacion

    // CheckMacroLimit: comandos que abren un dialogo con otro jugador no se
    // pueden dejar en una macro.
    static const char* const kBlocked[] = {
        "/trade", "/party", "/pt", "/guild", "/guildwar", "/battlesoccer"
    };
    for (int i = 0; i < (int)(sizeof(kBlocked) / sizeof(kBlocked[0])); ++i) {
        if (_stricmp(body, kBlocked[i]) == 0)
            return false;
    }

    const int slot = (text[1] == '0') ? 9 : (text[1] - '1');
    char* dst = (char*)MacroText + slot * 0x100;
    memset(dst, 0, 0x100);
    lstrcpynA(dst, body, 0x100);
    PlayBuffer(0x19, 0, 0);   // SOUND_CLICK01
    return true;
}

void Input_OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_MOUSEMOVE:     // 0x200
        // g_MouseX = LOWORD(lParam) * 640 / g_ScreenW  → DAT_083a427c
        // g_MouseY = HIWORD(lParam) * 480 / g_ScreenH  → DAT_083a4278
        {
            DWORD sw = gWindow.GetWidth() ? gWindow.GetWidth() : 640;
            DWORD sh = gWindow.GetHeight() ? gWindow.GetHeight() : 480;
            DAT_083a427c = ((DWORD)(short)LOWORD(lParam) * 640) / sw;
            DAT_083a4278 = ((DWORD)(short)HIWORD(lParam) * 480) / sh;
        }
        break;

    case WM_LBUTTONDOWN:   // 0x201
        DAT_083a413c = 0;                    // cancelar "dialog close" anterior
        if (DAT_083a42c4 == 0) {
            DAT_083a4124 = 1;                // g_ClickFlag = 1
        }
        DAT_083a42c4 = 1;                    // g_DblClickPending = 1
        DAT_055ca03c = DAT_083a427c;         // snapshot posición al down
        DAT_055ca040 = DAT_083a4278;
        break;

    case WM_LBUTTONUP:     // 0x202
        DAT_083a4124 = 0;                    // g_ClickFlag = 0
        // Si no hubo drag desde el down, marcar "dialog close / click confirmado"
        if (DAT_055ca03c == DAT_083a427c && DAT_055ca040 == DAT_083a4278) {
            DAT_083a413c = 1;
        } else {
            DAT_083a413c = 0;                // fue drag
        }
        DAT_083a42c4 = 0;                    // clear pending
        DAT_055ca03c = DAT_083a427c;
        DAT_055ca040 = DAT_083a4278;
        break;

    case WM_LBUTTONDBLCLK: // 0x203
        DAT_083a4299 = 1;                    // g_DblClickFlag
        break;

    case WM_RBUTTONDOWN:   // 0x204
        // 004149D0 WndProc: MouseRButtonPop = 0; if (!MouseRButton)
        // MouseRButtonPush = 1; MouseRButton = 1.  Attack (0049CBF0)
    // consume esos dos flags para arrancar y sostener el casteo de un skill.
        if (DAT_083a42ac == 0) {
            MouseRButtonPush = 1;                // MouseRButtonPush
        }
        DAT_083a42ac = 1;                    // MouseRButton
        break;

    case WM_RBUTTONUP:     // 0x205
        // El original limpia Push y suelta MouseRButton (también setea el flag
        // Pop aparte, que hoy ningún camino de gameplay compilado lee).
        MouseRButtonPush = 0;
        DAT_083a42ac = 0;
        break;

    case WM_MOUSEWHEEL:    // 0x20A
        break;

    // 0x10F = WM_IME_COMPOSITION — chat coreano DBCS
    //   DAT_07e11cec[DAT_07e11d78*4] = HIWORD(wParam) >> 8
    //   DAT_07e11ced[DAT_07e11d78*4] = LOWORD(wParam) & 0xFF
    //   toggle de IME vía IME_SetConversion @ 0x0047ED80
    case WM_IME_STARTCOMPOSITION:  // 0x10D
        DAT_055ca019 = 1;
        break;
    case WM_IME_ENDCOMPOSITION:    // 0x10E
        DAT_055ca019 = 0;
        break;

    // WM_CHAR — entrada de texto para usuario/contraseña/chat/zen/guild.
    // Portado del WndProc de IDA @ 0x004149D0 líneas 1890-2010.
    // Antes no se manejaba (→ DefWindowProcA), lo que descartaba cada tecla
    // en la pantalla de login, así que no se podía tipear usuario/contraseña.
    //
    // Globals mapping (IDA → ours):
    //   InputIndex            → DAT_07e11d78   (active slot 0..9)
    //   InputLength[i]        → ((DWORD*)DAT_07d780a8)[i]
    //   InputText[i][j]       → DAT_07db8710 + i*0x100 + j
    //   InputEnable           → DAT_00559c84
    //   InputNumber           → InputNumber
    //   InputTextMax[0]       → InputTextMax (alias floatizado del DWORD 0x559c94)
    //   byte_55CA019 (IME)    → DAT_055ca019
    //   byte_55CA038 (Enter)  → DAT_055ca038   (lo lee el disparador de login de Game_SceneUpdate)
    case WM_CHAR:          // 0x102
    {
        // Diagnóstico: confirma que WM_CHAR está llegando a la ventana y loguea
        // el estado de los flags del subsistema de input, para poder distinguir
        // "la tecla se perdió" de "la tecla se ignoró".

        // Clampea el slot a [0,9] para evitar OOB si el índice por slot es basura.
        DWORD  slot = DAT_07e11d78 & 0x0F;
        if (slot > 9) slot = 0;
        DWORD* lens = (DWORD*)DAT_07d780a8;
        int    len  = (int)lens[slot];
        char*  buf  = (char*)DAT_07db8710 + slot * 0x100;

        if (!DAT_055ca019) {
            // Limpia el buffer de composición del IME para este slot (stride 4)
            *((char*)&DAT_07e11cec + slot * 4) = 0;
        }

        if (wParam == 8) {           // Backspace
            if (len > 0) {
                lens[slot] = (DWORD)(len - 1);
                buf[len - 1] = 0;
            }
            break;
        }
        if (wParam == 9) {           // Tab — rotate active slot
            if (DAT_00559c84 && InputNumber > 1) {
                DAT_07e11d78 = (DAT_07e11d78 + 1) % InputNumber;
                PlayBuffer(0x19, 0, 0);   // PlayBuffer(25) — click sfx
            }
            break;
        }
        if (wParam == 0xD) {         // Enter — signals login/submit + chat toggle
            DAT_055ca038 = 1;

            // Baúl: guardar/sacar zen. Per IDA WndProc L2047-2051:
            //   InputIndex = 0;
            //   if (GoldInputEnable) { InputGold = atoi(InputText[0]); ... }
            // Así el diálogo de zen del baúl sabe cuánto tecleó el jugador.
            if (GoldInputEnable) {
                DAT_07e11d78 = 0;                     // InputIndex = 0
                char* goldBuf = (char*)DAT_07db8710;  // InputText[0]
                goldBuf[0xFF] = 0;
                // IDA: WndProc (0x004149D0).
                // DESVIACION (fix del DLL, Patchs.cpp 0x004EB8C7): validar diez dígitos y rango antes de convertir Zen.
                int gold = 0;
                for (const char* digit = goldBuf; *digit; ++digit) {
                    if (digit - goldBuf >= 10 || *digit < '0' || *digit > '9' ||
                        gold > (2000000000 - (*digit - '0')) / 10) {
                        gold = 2000000001; // Valor fuera de rango: el diálogo muestra el error 118 sin enviar.
                        break;
                    }
                    gold = gold * 10 + (*digit - '0');
                }
                InputGold = gold;
                // LABEL_591: limpia el slot y cierra el input (el envío lo hace
                // UI_InGameMenu case 116 leyendo DAT_055ca038 el frame siguiente).
                memset(goldBuf, 0, 0x100);
                lens[0] = 0;
                DAT_00559c84 = 0;                     // InputEnable = 0
                break;
            }

            // Toggle del chat in-game (per IDA WndProc:2011-2046).
            //   - state=5 (in-world)
            //   - input vacío + InputEnable=0  → abre el chat (InputEnable=1)
            //   - input vacío + InputEnable=1  → cierra el chat (InputEnable=0)
            //   - input con texto + InputEnable=1  → envía el chat y cierra
            //
            // IDA: WndProc reserva Enter cuando hay foco de Guild o cuando
            // ErrorMessage 126/152 está capturando un Personal Code; en esos
            // casos el input no puede abrir ni enviar chat.
            const bool guildDeleteCodeDialog =
                DAT_083a7c24 == 126 || DAT_083a7c24 == 152;
            if (SceneFlag == 5 && GuildInputEnable == 0 && !guildDeleteCodeDialog) {
                bool empty = (lens[slot] == 0);
                if (empty) {
                    if (DAT_00559c84) {
                        // Cierra el input de chat.
                        DAT_00559c84 = 0;
                    } else {
                        // Abre el input de chat — inicializa los campos de texto.
                        // Per IDA Game.cpp:4326-4327: InputTextMax[0]=42, [1]=10.
                        _InputTextMaxArr[0] = 42;               // chat msg max
                        _InputTextMaxArr[1] = 10;               // whisper target name max
                        // IDA: WndProc restablece el modo visible al abrir
                        // chat. El diálogo 126 deja este slot enmascarado
                        // para el Personal Code.
                        InputTextHide[0] = 0;
                        InputNumber = 2;                       // InputNumber = 2 (chat + whisper target)
                        // GoldInputEnable = 0 — ya está en 0 en el juego normal
                        DAT_00559c84 = 1;                       // InputEnable = 1
                        DAT_07e11d78 = 0;                       // InputIndex = 0
                    }
                    PlayBuffer(0x19, 0, 0);   // click sfx
                } else if (DAT_00559c84) {
                    // Con texto + chat abierto → envía la línea (slot 0), limpia y
                    // cierra. Refleja las líneas 2046-2120 del WndProc de IDA condensadas
                    // en un solo camino (el original de IDA separa /whisper +
                    // /pvp + comandos de GM, pero por ahora los reenviamos al
                    // loop de canales de Chat_InputTick; el envío crudo anda
                    // bien para el caso del chat normal).
                    char* line = (char*)DAT_07db8710 + slot * 0x100;
                    // "/N texto" guarda la macro N en vez de enviarse (desviacion).
                    if (Chat_TryAssignMacro(line)) {
                        memset(line, 0, 0x100);
                        lens[slot] = 0;
                        DAT_00559c84 = 0;
                        break;
                    }
                    // IDA WndProc L2265-2268: antes del SendChat (no en el
                    // susurro) revisa los gestos, salvo montado fuera de zona segura.
                    if (((const char*)&DAT_07db8810)[0] == '\0') {
                        WORD helper = *(WORD*)(DAT_07abf5d8 + 0x2b8);
                        if ((helper != 818 && helper != 819) || *(char*)(DAT_07abf5d8 + 0x34e))
                            CheckChatText(line);
                    }
                    Chat_SendChatLine(line);

                    // Limpia el slot de input y su longitud.
                    memset(line, 0, 0x100);
                    lens[slot] = 0;
                    // Cierra el chat (coincide con el comportamiento "enviar + autocerrar";
                    // la queja del usuario era que Enter-con-texto NO
                    // close).
                    DAT_00559c84 = 0;
                    PlayBuffer(0x19, 0, 0);   // click sfx
                }
            }
            break;
        }
        // Carácter imprimible — se agrega si hay algún modo de input activo.
        // El gate de InputEnable cubre login + chat; GuildInputEnable y los
        // modales 126/152 comparten el buffer, pero preservan un foco distinto
        // y no deben abrir el chat.
        // Per IDA WndProc L1953-1963 el gate es `InputEnable || GoldInputEnable || …`
        // (el diálogo de zen del baúl abre con InputEnable=0 + GoldInputEnable=1,
        // sub_4EB5D0) y con GoldInputEnable sólo se aceptan dígitos '0'..'9'.
        const bool guildDeleteCodeDialog =
            DAT_083a7c24 == 126 || DAT_083a7c24 == 152;
        if (DAT_00559c84 || GoldInputEnable || GuildInputEnable || guildDeleteCodeDialog) {
            BYTE c = (BYTE)wParam;
            if (GoldInputEnable && (c < '0' || c > '9')) break;
            // RANGO ACEPTADO — DESVIACIÓN DELIBERADA, hermana del charset de
            // CreateFontA (ver WinMain paso 15).
            // Acá había `c < 0x7F`, o sea ASCII puro: por eso no se podía
            // escribir 'ñ' (0xF1 en Windows-1252) ni vocales acentuadas.
            // El original acepta 0x20..0x7E y manda los bytes >= 0x81 por el
            // camino DBCS (lead byte + trail byte), porque su entrada es
            // coreana.  Nuestro texto es Latin-1 de un solo byte, así que ese
            // camino los destrozaría igual que HANGEUL_CHARSET destrozaba el
            // render.  Los aceptamos como carácter simple.
            // 0x7F es DEL: se sigue descartando.
            if (c >= 0x20 && c != 0x7F) {
                // InputTextMax[8] @ 0x00559c94 — leído como int desde el array
                // tipado (NO vía la referencia aliasada a float, que castea el valor).
                int maxLen = _InputTextMaxArr[slot];
                if (maxLen <= 0) maxLen = 10;  // login-default fallback
                if (len < maxLen) {
                    buf[len]   = (char)c;
                    lens[slot] = (DWORD)(len + 1);
                }
            }
        }
        break;
    }

    default:
        break;
    }
}
