// Scene_ServerSelect_Input.cpp — FUN_0051e7e0 @ 0x0051e7e0
// Server-select UI: mouse hit-test for group buttons + individual server entries.
//
// Called each frame during server select to handle mouse clicks.
// Server list: DAT_083a45d8, stride 0x21e, 24 entries.
//   +0x00: server name (null-terminated)
//   +0x14: active flag (DAT_083a45ec offset)
//   +0x15: group flag: 0=group1, 1=group2 (DAT_083a45ed offset)
//
// Screen layout (X axis):
//   Group 1 button:  X = 0x96..0xdc  (150..220)
//   Group 2 button:  X = 0x177..0x1bd (375..445)
//   Server entries:  X = 0xe5..0x16c  (229..364), Y stacked by 0x14 per entry
//
// Mouse position: DAT_083a427c = cursor X, DAT_083a4278 = cursor Y
// Click flag:     DAT_083a4124 — set by WndProc on left click, cleared here after handling
//
// On group button click:
//   Sends 4-byte packet [0xC1][0x04][0xF4][?] to select server group.
//   Saves selected group index to DAT_00561694; resets DAT_00561698 = -1.
//   Calls FUN_00404bc0(0x19, 0, 0) (BGM_Play).
//
// On server entry click (if group already selected, ping < 100, no cooldown):
//   Saves channel = (port % 0x14 + 1) to DAT_0056169c.
//   Sends 6-byte packet [0xC1][0x06][0xF4][?][portLo][portHi].
//   Sets connect timer: DAT_083a7ac8 = GetTickCount().
//   Calls FUN_00480620 twice to copy IP/port into connection context.
//   Sets cooldown: DAT_083a7c44 = 0x32 frames.
//
// Packet buffer layout (stack, sent via send() with WOULDBLOCK fallback):
//   Group select (4 bytes):  { 0xC1, 0x04, 0xF4, subcode }
//   Server select (6 bytes): { 0xC1, 0x06, 0xF4, subcode, portLo, portHi }
//
// Related globals:
//   DAT_083a7c24 / _7c28 — UI flash/cooldown counters
//   DAT_083a7c44          — server click cooldown (decremented per frame)
//   DAT_083a4320          — reset to 0 on server select
//   DAT_083a4328          — updated with selected port

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include <winsock2.h>

// ─────────────────────────────────────────────────────────────────────────────

void FUN_0051e7e0(void)
{
    // Re-enabled: la lista estática poblada en WinMain (entry "MuServer") hace
    // que el loop tenga algo que procesar. Sin populate el Pass 1 no encontraba
    // entries y el `do...while ((int)local_830 < 0x83a7adb);` podía divergir —
    // por eso el AUTO-SKIP original. Ahora que hay ≥1 entry, el loop termina
    // correctamente en 24 iteraciones.
    // ── Pass 1: classify servers into group1 / group2; compute button Y coords ─
    int iVar8  = 0;   // group1 count
    int iVar10 = 0;   // group2 count

    // IDA: for ( i = 0; i < 13550; i += 542 )  — 25 iterations (0x34ee / 0x21e).
    // BUG-FIX: `(&DAT_083a45ec)[i]` es DWORD* (scale 4). IDA usa byte arith:
    //   `*((_BYTE*)&unk_83A45EC + i)`. Sin el cast leíamos memoria equivocada.
    for (int iVar12 = 0; iVar12 < 0x34ee; iVar12 += 0x21e) {
        if (*((unsigned char*)&DAT_083a45ec + iVar12) != '\0') {
            // Check if server name ends with '2' → group 2
            // IDA: unk_83A45D7 + i + strlen(&ServerList[i]) == 50   (50 = '2')
            const char *name = (const char*)DAT_083a45d8 + iVar12;
            int len = 0;
            while (name[len]) len++;
            if (len > 0 && name[len - 1] == '2') {
                *((unsigned char*)&DAT_083a45ed + iVar12) = 1;
                iVar10++;
            } else {
                *((unsigned char*)&DAT_083a45ed + iVar12) = 0;
                iVar8++;
            }
        }
    }

    // Y position base for each group column
    int iVar12 = iVar8 * -0x10 + 0x1bc;
    if (iVar12 > 0xdc) iVar12 = 0xdc;
    int iVar4 = ((iVar8 - iVar10) - 1) * 0x10 + iVar12;

    // BUG-FIX: guardar Y base ANTES de que Pass 2 mute iVar12/iVar4.
    // IDA: v38 = v3 (non-PVP base), v40 = v7 (PVP base) — usadas en Pass 3.
    // Antes Pass 3 leía iVar12/iVar4 post-Pass-2 → hit-area 16px debajo del render.
    const int iYNonPvpBase = iVar12;
    const int iYPvpBase    = iVar4;

    int local_834 = 0;
    const char *local_830 = &DAT_083a45ed;
    int local_820 = iVar12;
    int local_824 = iVar4;

    // ── Pass 2: hit-test group buttons ───────────────────────────────────────
    do {
        if (local_830[-1] != '\0') {
            int iVar9, iVar3, iVar6;
            if (*local_830 == '\0') {
                // Group 1 entry
                iVar6 = 0x96;
                iVar9 = iVar12;
                iVar3 = iVar12 + 0x10;
                iVar12 = iVar3;
                local_820 = iVar3;
            } else {
                // Group 2 entry
                iVar6 = 0x177;
                iVar9 = iVar4;
                iVar3 = iVar4 + 0x10;
                iVar4 = iVar3;
                local_824 = iVar3;
            }

            // Check mouse in button rect and click pending
            if (iVar6 <= (int)DAT_083a427c && (int)DAT_083a427c < iVar6 + 0x46 &&
                iVar9 <= (int)DAT_083a4278 && (int)DAT_083a4278 < iVar3 &&
                DAT_083a4124 != '\0')
            {
                FUN_00405540(&DAT_055c9bf0, "> Server group selected");
                DAT_083a4124 = '\0';

                // Build and send 4-byte group-select packet
                // IDA: buf[2]=-63(0xC1), buf[3]=4(len), v49=-12(0xF4), v50=2(subcode)
                // Ghidra malinterpretó v50 como 0xC1 (era 2). F4/02 = server list req.
                char pkt[4];
                pkt[0] = (char)0xC1;
                pkt[1] = 4;
                pkt[2] = (char)0xF4;
                pkt[3] = 2;            // subcode 2 (F4/02 = request server list)

                unsigned int uVar5 = 4;
                int iOff = 0;
                if (DAT_055ca168 != 0xffffffff) {
                    do {
                        int iVar4b = send(DAT_055ca168, pkt + iOff, (int)uVar5, 0);
                        if (iVar4b == -1) {
                            int err = WSAGetLastError();
                            if (err == WSAEWOULDBLOCK) {
                                if (DAT_055cc16c + (int)uVar5 < 0x2001) {
                                    const char *src = pkt;
                                    char *dst = (char*)DAT_055ca16c + DAT_055cc16c;
                                    for (unsigned int u = uVar5 >> 2; u; u--) {
                                        *(unsigned int *)dst = *(unsigned int *)src;
                                        src += 4; dst += 4;
                                    }
                                    for (unsigned int u = uVar5 & 3; u; u--)
                                        *dst++ = *src++;
                                    DAT_055cc16c += (int)uVar5;
                                } else {
                                    Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                                }
                            } else {
                                Net_Disconnect(((int)(uintptr_t)DAT_055ca160));
                            }
                            break;
                        }
                        if (iVar4b == 0) break;
                        if (DAT_055ce174) FUN_0043de60();
                        uVar5 -= (unsigned int)iVar4b;
                        iOff  += iVar4b;
                    } while ((int)uVar5 > 0);
                }

                DAT_00561694 = local_834;
                DAT_00561698 = -1;
                FUN_00404bc0(0x19, 0, 0);
                iVar12 = local_820;
                iVar4  = local_824;
            }
        }
        local_830 += 0x21e;
        local_834++;
    // BUG-FIX: bound absoluto 0x83a7adb era la dirección ORIGINAL del binario
    // (0x083a45ed + 0x34ee). En nuestro build DAT_083a45d8 está en otra
    // dirección asignada por el linker → el loop nunca terminaba y walking
    // memoria inválida. IDA: `while ((int)v6 < 138050267)` = mismo bug.
    // Fix: bound relativo al base del array.
    } while ((int)local_830 < (int)((const char*)&DAT_083a45ed + 0x34ee));

    // ── Pass 3: hit-test individual server entries in selected group ──────────
    if ((int)DAT_00561694 == -1) goto done;

    {
        int iVar10b = (int)DAT_00561694 * 0x21e;
        int iVar8b;

        // IDA Pass 3 uses v38 (non-PVP base) / v40 (PVP base), NOT the mutated
        // v4/v7. Mismatch de 16px hacía que ciertas zonas del hit-area no
        // coincidieran con el render.
        unsigned char numCh = *((unsigned char*)&DAT_083a45ec + iVar10b);
        if (*((unsigned char*)&DAT_083a45ed + iVar10b) == 0) {
            // Non-PVP server: cnt = posición en la lista non-PVP (antes de este)
            // Para single-entry, cnt=0. Scan strcmp omitido (solo relevante con >1 server).
            int cnt = 0;
            iVar8b = cnt * 0x10 + (int)numCh * -10 + 8 + iYNonPvpBase;
        } else {
            // PVP server
            int cnt = 0;
            iVar8b = cnt * 0x10 + (int)numCh * -10 + 8 + iYPvpBase;
        }

        int server_count = (int)numCh;   // BUG-FIX: reuse byte-correct numCh from above
        if (server_count == 0) goto done;

        // Max Y extent check
        int iVar12b = (0x18 - server_count) * 0x14;
        if (iVar12b <= iVar8b) iVar8b = iVar12b;

        DAT_00561698 = -1;
        int local_834b = 0;
        int local_830b = 0;   // byte offset into server port/flag arrays for this group

        do {
            // Check mouse in entry rect (X=0xe5..0x16c) and server not full (flag & 0x80 == 0)
            if (0xe5 <= (int)DAT_083a427c && (int)DAT_083a427c < 0x16c &&
                iVar8b <= (int)DAT_083a4278 && (int)DAT_083a4278 < iVar8b + 0x14)
            {
                // BUG-FIX byte-arith: DAT_083a4604 es *(DWORD*) y DAT_083a4606
                // *(WORD*) lvalues → `&DAT + i` / `(&DAT)[i]` escalan el offset
                // ×4/×2. Con el server en slot 23 (CS) eso leía FUERA del array.
                // Igual que en el render (Scene_Login_ServerSelect), castear a
                // char*/unsigned char* para aritmética de bytes.
                DAT_0056169c = (unsigned int)
                    *(unsigned short *)((char*)&DAT_083a4604 + iVar10b + local_830b) % 0x14 + 1;
                DAT_00561698 = local_834b;

                bool full = ((*((unsigned char*)&DAT_083a4606 + iVar10b + local_830b)) & 0x80) == 0x80;
                bool ping_ok = ((*((unsigned char*)&DAT_083a4606 + iVar10b + local_830b)) & 0x7f) < 100;

                if (!full && DAT_083a4124 != '\0') {
                    DAT_083a4124 = '\0';
                    FUN_00404bc0(0x19, 0, 0);

                    if (ping_ok && DAT_083a7c44 == 0) {
                        DAT_0056169c = (unsigned int)
                            *(unsigned short *)((char*)&DAT_083a4604 + (int)DAT_00561694 * 0x21e + local_830b)
                            % 0x14 + 1;
                        FUN_00405540(&DAT_055c9bf0, "> Server selected");

                        DAT_083a7c44 = 0x32;
                        DAT_083a7c24 = DAT_083a7c28;
                        DAT_083a7c28 = 0;
                        DAT_083a7ac8 = GetTickCount();
                        DAT_083a4320 = 0;

                        // Selected server's ServerCode (channel_id, +0x2c field).
                        unsigned short serverCode =
                            *(unsigned short *)((char*)&DAT_083a4604 + (int)DAT_00561694 * 0x21e + local_830b);
                        DAT_083a4328 = (DAT_083a4328 & 0xffff0000) | serverCode;

                        if (g_ConnectServerMode) {
                            // ── Flujo ConnectServer (fiel al binario original) ──
                            // Mandar F4/03 (server-info request) al ConnectServer.
                            // Responde F4/03 con IP:port del GameServer →
                            // Recv_Redirect (Net_Process.cpp) desconecta del CS,
                            // conecta al GameServer y arranca el login (JoinServer).
                            // IDA CServerSelWin: v43[2]=0xC1, v43[3]=6, v44=0xF4,
                            // v45=3, v46=LOWORD(ServerCode).
                            BYTE pkt6[6] = { 0xC1, 0x06, 0xF4, 0x03,
                                             (BYTE)(serverCode & 0xff),
                                             (BYTE)(serverCode >> 8) };
                            extern void CS_SendPlain(const BYTE* data, int len);
                            CS_SendPlain(pkt6, 6);
                        } else {
                            // ── Flujo directo (server.cfg 1 línea) ──────────────
                            // No hay ConnectServer: conectar directo al GameServer
                            // de server.cfg. La transición a state 1 (Connecting)
                            // arranca el progress bar + espera JoinServer.
                            extern void Net_ConnectServer(const char *server, unsigned int port);
                            Net_ConnectServer(PTR_s_connect_muonline_co_kr_005615b8,
                                         (unsigned int)DAT_005615bc);
                        }
                        DAT_083a7c14 = 1;   // state = Connecting

                        // IDA 0x0051E7E0 L315-316 (CServerSelWin): these are
                        // GlobalText[470]/[471], not the standalone buffers
                        // DAT_07d4c3ec / lpString_07d4c518 that Ghidra split.
                        FUN_00480620(DAT_083a7c64, GlobalText[470], 1);
                        FUN_00480620((const char*)&DAT_083a7c68, GlobalText[471], 1);

                    } else if (ping_ok) {
                        // Already has cooldown — flash counters
                        if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x18;
                        else                    DAT_083a7c28 = 0x18;
                    }
                }
            }

            local_834b++;
            iVar8b    += 0x14;
            local_830b += 0x1a;   // stride in port/flag arrays
        } while (local_834b < server_count);
    }

done:
    // Decrement per-frame cooldown
    if (DAT_083a7c44 > 0)
        DAT_083a7c44--;
}
