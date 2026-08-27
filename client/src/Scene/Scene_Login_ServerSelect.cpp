// Scene_Login_ServerSelect.cpp
// Scene_Login_ServerSelect @ 0x0051F020   (375 lines decompiled)
//
// Renders the server-selection panel shown in Login sub-state 0.
// Called directly from Scene_Login when DAT_083a7c14 == 0.
//
// ─── SERVER LIST DATA ──────────────────────────────────────────────────────────
//
//  Base array : DAT_083a45d8
//  Stride     : 0x21e bytes per server-group entry
//  Count      : 24 entries  (0x83a7ac6 - 0x83a45d8) / 0x21e
//
//  Server group entry layout (offsets from entry base):
//    +0x00  char[20]  name          — display name of the server group
//    +0x14  byte      num_channels  — how many sub-servers/channels
//    +0x15  byte      pvp_flag      — 0 = Non-PVP column, 1 = PVP column
//                                    (derived in 2nd pass: name contains '2' → PVP)
//    +0x2c  ushort    channel_id    — first channel's ID (mod 20 + 1 = display index)
//    +0x2e  byte      load          — server load byte:
//                                      0x80+ = FULL  (red tint)
//                                      0..99 = LOW   (green tint)
//                                      100+  = NORMAL (white tint)
//
//  Sub-server (channel) stride within entry: 0x1a bytes
//  So channel[i] load is at: entry+0x2e + i*0x1a
//      channel[i] id  is at: entry+0x2c + i*0x1a  (ushort)
//
//  DAT_083a45ec = &DAT_083a45d8 + 0x14  (alias: start of num_channels field)
//  DAT_083a4606 = &DAT_083a45d8 + 0x2e  (alias: start of load bytes)
//
// ─── SELECTION STATE ───────────────────────────────────────────────────────────
//
//  DAT_00561694  g_SelectedServer     — index of highlighted server group (-1 = none)
//  DAT_00561698  g_SelectedChannel    — index of highlighted channel within group
//  DAT_00559c8c                       — set to 0x100 on entry (scissor/clip enable?)
//  DAT_00559c78                       — set to 0xffffffff (clip mask full)
//  DAT_00559c80                       — set to 0 then 0x80000000
//
// ─── RENDERING LAYOUT (640×480 reference coords) ───────────────────────────────
//
//  PASS 1 — format server name strings into stack buffer (100 bytes each):
//    Uses crt_sprintf(buf, fmt) to build display strings like:
//      "%s %s"          → single channel entry (offline/maintenance)
//      "%s %d Non-PVP %s" → non-PVP channel with number and load status
//      "%s %d PVP %s"    → PVP channel with number and load status
//    Format string addresses encode load status:
//      s__s__s_00561914   = full  (server full)
//      s__s__s_0056191c   = normal
//      s__s__s_00561924   = low load
//      s__s__d__s_0056192c = PVP + channel_num + full
//      s__s__d__s_00561940 = PVP + channel_num + normal
//      s__s__d__s_00561954 = PVP + channel_num + low
//      s__s__d__s_00561968 = Non-PVP + channel_num + full
//      s__s__d__s_00561974 = Non-PVP + channel_num + normal
//      s__s__d__s_00561980 = Non-PVP + channel_num + low
//
//  PASS 2 — tag PVP servers:
//    For each entry: if server_name[X] == '2' → pvp_flag=1, else pvp_flag=0
//    Counts: iVar8 = num PVP servers, iVar10 = num Non-PVP servers
//
//  PASS 3 — draw server group list (two columns):
//    Non-PVP column  x=0x96 (150px),  row y advances +16 per entry
//    PVP    column   x=0x177 (375px), row y advances +16 per entry
//
//    Per entry:
//      If selected (iVar8 == DAT_00561694):
//        Tex 0x12 quad at (0xa0, y, 160, 16)   Non-PVP selected
//        Tex 0x12 quad at (0x16d, y, 160, 16)  PVP selected
//        glColor3f(1.0, 1.0, 1.0)              — full brightness
//      Else:
//        Tex 0x12 quad at (0x96, y, 150, 16)   Non-PVP normal
//        Tex 0x12 quad at (0x177, y, 150, 16)  PVP normal
//        glColor3f(0.8, 0.8, 0.8)              — dimmed
//      Text via UI_RenderText: centered at column_x, y+2
//        Width from GetTextExtentPointA → used to center string
//
//  PASS 4 — draw channel sub-list (right panel, only if g_SelectedServer != -1):
//    Panel x=230, w=134, h=20 per channel row
//    Channel count = server[g_SelectedServer].num_channels
//    Max visible channels = (0x18 - num_channels) * 0x14  (scroll clamped)
//
//    Per channel row:
//      Color (R,G,B):
//        load & 0x80 → (1.0 * _DAT_00552928, 1.0 * _DAT_00552504)  = RED   (full)
//        load < 100  → (1.0, 1.0, var)                              = GREEN (low)
//          special: if channel_id%20==0 or ==0xe: B = _DAT_00552928 (teal)
//        else        → (_DAT_005528b4, _DAT_005528b4, _DAT_005528b4)= WHITE (normal)
//      Selected channel: same colors but at full brightness (fVar3=1.0)
//      Non-selected: fVar3=0.8 (dimmed)
//      Tex 0x12 quad at (230, y, 134, 20)
//      Text: UI_RenderText(0x129 - textWidth/2, y+1, channelNameBuf)
//
//    Load bar (for non-full servers, below channel text):
//      Position: x = _DAT_00552d48, y = channel_y + _DAT_00552d4c
//      Filled dots  (0..load/5): Tex 0xf6 at (x+i*5, y, 3, 3)
//        Color fades: brighter at right end (iStack00000020 counts 20→0 * fVar3 * _DAT_005524f4)
//      Empty dots (load/5..20): Tex 0xf6, gray (fVar3 * _DAT_005524f4)
//
//  PASS 5 — selected server IP display:
//    Compares selected server name vs DAT_07d52c38 (currently connected server)
//    If match:
//      glColor3f(1.0, 0.3, 0.1)  — orange-red
//      UI_RenderText(0x96, y-0x2e, DAT_07d530e8)   — server IP line 1
//      UI_RenderText(0x96, y-0x1f, DAT_07d53214)   — server IP line 2
//
// ─── HELPER FUNCTIONS ──────────────────────────────────────────────────────────
//  FUN_00541c10 = __chkstk_probe (0x00541c10)
//                — MSVC CRT stack-probe stub, NOT game logic.
//                  Called automatically by the compiler because this function
//                  allocates ~48 KB of locals (24 groups × 20 channels × 100 chars).
//                  Loop: while(EAX > 0x1000) { probe page; EAX -= 0x1000; }
//                  Renamed __chkstk_probe in Ghidra.
//  crt_sprintf(buf,fmt) — sprintf-like into stack buffer
//  GL_DrawTexture(tex,x,y,w,h,u0,v0,u1,v1,f1,f2) — draw textured quad
//  UI_RenderText(x,y,str,sizeOut,center,shadow)   — draw text string
//  FUN_00406b10(a,b)     — lookup/flag check (returns 0 or non-zero → PVP type)

#include "stdafx.h"
#include "Scene.h"

extern "C" { void DbgLogPublic(const char*); }

// Scene_Login_ServerSelect @ 0x0051F020
// Renders the server-group + channel selection panel inside the login screen.
// Server list: DAT_083a45d8, stride 0x21e, 24 entries.
// Stack allocates ~48 KB of display strings (triggers __chkstk_probe).
int Scene_Login_ServerSelect(void)
{

    char        cVar1;
    byte        bVar2;
    float       fVar3, fVar18, fVar20;
    // fVar4: era "current Y of last drawn row" en Pass 3 (float-but-really-int).
    // Pass 4 la lee para posicionar la columna de canales. Inicializar a 0xdd
    // (221, el tope que asigna iStartY cuando hay poca lista) para que, si
    // DAT_00561694 != -1, Pass 4 use un Y razonable. El bug-fix en Pass 3 usa
    // iYNonPvp/iYPvp locales y sincroniza fVar4 al final.
    float       fVar4 = 0.0f;
    int         iVar5, iVar8, iVar10;
    uint        uVar6;
    char       *pcVar7, *pcVar13;
    undefined1 *puVar9, *puVar12;
    byte       *pbVar11, *pbVar14;
    LPCSTR      lpString;
    bool        bVar15;
    undefined8  uVar16;
    // BUG-FIX: era undefined4 (unsigned int). Al pasar a glColor3f(float,...)
    // hacia conversion int→float: 0x3f800000 → 1065353216.0f (se clampaba a 1.0).
    // Usar float directamente.
    float       uVar17;
    int         iStack00000004;
    float       fStack00000008, fStack0000000c, fStack00000014, fStack00000018;
    int         iStack0000001c, iStack00000020;
    LPSIZE      ptVar19;
    undefined1 *puStack00000010;
    // 24 server groups × 20 channels × 100 bytes per name string
    char        chan_buf[24 * 20 * 100];
    SIZE        text_size;

    /* __chkstk_probe(); */  // MSVC stack-frame allocator stub, not game logic

    iVar8  = 0;
    puVar9 = (undefined1*)chan_buf;
    DAT_00559c8c = 0x100;
    iStack00000004 = 0;
    puVar12 = (undefined1*)&DAT_083a45d8;

    // ── PASS 1: format channel name strings ──────────────────────────────────
    // IDA reference (0x0051F020):
    //   Single-ch : sprintf(buf, "%s %s", name, status)
    //   Multi NON-PVP channel: sprintf(buf, "%s-%d(Non-PVP) %s", name, chNum, status)
    //   Multi PVP channel:     sprintf(buf, "%s-%d %s", name, chNum, status)
    //   status = GlobalText[560/561/562]   (FULL/NORMAL/LOW)
    //
    // BUG-FIX vs Ghidra: las llamadas eran `crt_sprintf(buf, fmt)` sin args —
    // sprintf leía basura del stack y rendería "?TOO?TOO". Ahora pasamos los
    // args correctos. Hardcoded status strings (no tenemos GlobalText[] cargado).
    {
        const char* STATUS_FULL   = "FULL";
        const char* STATUS_NORMAL = "NORMAL";
        const char* STATUS_LOW    = "LOW";

        do {
            pbVar11 = puVar12 + 0x2e;   // load byte of first channel
            puStack00000010 = puVar9;
            const char* serverName = (const char*)puVar12;    // +0x00 = name (null-term)
            unsigned char numCh    = (unsigned char)puVar12[0x14];

            if (numCh == 1) {
                // Single-channel entry
                const char* status;
                if ((*pbVar11 & 0x80) == 0x80)            status = STATUS_FULL;
                else if ((*pbVar11 & 0x7f) < 100)         status = STATUS_LOW;
                else                                       status = STATUS_NORMAL;
                crt_sprintf((char*)puVar9, "%s %s", serverName, status);
                puStack00000010 = puVar9;
            } else if (numCh != 0) {
                iVar10 = 0;
                do {
                    int chNum = (int)((unsigned)*(unsigned short *)(pbVar11 - 2) % 0x14 + 1);
                    int isPvp = FUN_00406b10(iStack00000004, chNum);
                    const char* status;
                    if ((*pbVar11 & 0x80) == 0x80)        status = STATUS_FULL;
                    else if ((*pbVar11 & 0x7f) < 100)     status = STATUS_LOW;
                    else                                   status = STATUS_NORMAL;

                    char* outBuf = chan_buf + (iVar8 + iVar10) * 100;
                    if (isPvp == 0) {
                        // Non-PVP channel
                        crt_sprintf(outBuf, "%s-%d(Non-PVP) %s", serverName, chNum, status);
                    } else {
                        // PVP channel (Open-PVP i.e. sin etiqueta)
                        crt_sprintf(outBuf, "%s-%d %s", serverName, chNum, status);
                    }
                    iVar10++;
                    pbVar11 = pbVar11 + 0x1a;   // next channel (stride 0x1a)
                } while (iVar10 < (int)numCh);
            }
            puVar12 = puVar12 + 0x21e;   // next server group
            iStack00000004++;
            puVar9 = puStack00000010 + 2000;
            iVar8  = iVar8 + 0x14;
        } while ((int)puVar12 < (int)(uintptr_t)(DAT_083a45d8 + 0x34ee));
    }

    // ── PASS 2: tag PVP/non-PVP groups, count each ───────────────────────────
    iVar8  = 0;   // PVP count
    iVar10 = 0;   // non-PVP count
    pcVar7 = (char*)&DAT_083a45ec;   // pvp_flag field of first entry (+0x14 from base)
    do {
        if (*pcVar7 != '\0') {
            // strlen of server name (pcVar7 - 0x14)
            uVar6  = 0xffffffff;
            pcVar13 = pcVar7 - 0x14;
            do {
                if (uVar6 == 0) break;
                uVar6--;
                cVar1 = *pcVar13;
                pcVar13++;
            } while (cVar1 != '\0');
            // If last relevant char in name == '2' → PVP server
            if (pcVar7[~uVar6 - 0x16] == '2') {
                pcVar7[1] = '\x01';   // pvp_flag = 1
                iVar8++;
            } else {
                pcVar7[1] = '\0';
                iVar10++;
            }
        }
        pcVar7 = pcVar7 + 0x21e;
    } while ((int)pcVar7 < (int)(uintptr_t)(DAT_083a45d8 + 0x3502));

    // ── PASS 3: draw two-column server group list ─────────────────────────────
    puStack00000010 = puVar9;
    glColor3f(1.0f, 1.0f, 1.0f);

    // Compute base Y for non-PVP column
    // BUG-FIX: Ghidra tipó los slots de Y como float, pero el asm original los
    // manipulaba como int32 (bit-pattern 0xdd=221). (int)3.08286e-43f→0, rompiendo
    // el cálculo. Reescrito con ints limpios: Y_top = min(0x1bc - 16*N, 0xdd).
    int iYNonPvp;   // current non-PVP row Y (bumped +16 per entry)
    int iYPvp;      // current PVP row Y
    {
        int iStartY = iVar10 * -0x10 + 0x1bc;   // 444 - 16*non_pvp_count
        // IDA: v10 = 220 default; if (444-16*v8 <= 220) v10 = 444-16*v8.
        // Equivale a min(220, 444-16*v8). Cap a 0xdc=220, NO 0xdd=221.
        if (iStartY > 0xdc)                      // cap top at Y=220
            iStartY = 0xdc;
        iYNonPvp = iStartY;
        iYPvp    = ((iVar10 - iVar8) + -1) * 0x10 + iStartY;
        // Bridge: Pass 4/5 leen fStack00000008 como "PVP base Y" y fVar4 como
        // "non-PVP base Y" para posicionar la columna de canales.
        // BUG-FIX: fVar4 se quedaba en 0, haciendo que el panel de canales
        // non-PVP renderizara arriba de pantalla (Y negativo). IDA usa v10
        // (non-PVP base Y) en `v21 = 16 * v23 - 10 * v20 + v10 + 8`.
        fStack00000008 = (float)iYPvp;
        fVar4          = (float)iStartY;   // non-PVP base Y, leído por Pass 4
    }

    DAT_00559c78 = 0xffffffff;
    iVar8 = 0;
    DAT_00559c80 = 0;
    lpString = (LPCSTR)&DAT_083a45d8;

    do {
        if (lpString[0x14] != '\0') {
            int iDrawY;   // Y where this row's quad + text draw
            if (lpString[0x15] == '\0') {
                // Non-PVP entry
                iVar10 = 0x96;
                iDrawY = iYNonPvp;
                iYNonPvp += 0x10;
                iStack00000004 = 0x96;
                if (iVar8 != DAT_00561694) goto LAB_0051f314;
                iVar10 = 0xa0;
                uVar17 = 1.0f;         // selected: full brightness
                iStack00000004 = 0xa0;
            } else {
                // PVP entry
                iVar10 = 0x177;
                iDrawY = iYPvp;
                iYPvp += 0x10;
                iStack00000004 = 0x177;
                if (iVar8 == DAT_00561694) {
                    iVar10 = 0x16d;
                    uVar17 = 1.0f;         // selected
                    iStack00000004 = 0x16d;
                } else {
LAB_0051f314:
                    uVar17 = 0.8f;         // not selected: dimmed
                }
            }
            glColor3f(uVar17, uVar17, uVar17);
            GL_DrawTexture(0x12, (float)iStack00000004, (float)iDrawY,
                         70.0f, 16.0f, 0.0f, 0.0f, 0.5234375f, 0.9375f, '\x01', '\x01');
            ptVar19 = &text_size;
            iVar5 = lstrlenA(lpString);
            GetTextExtentPointA(DAT_055c9fec, lpString, iVar5, ptVar19);
            UI_RenderText((iVar10 - ((uint)(text_size.cx * 0x280) / DAT_0056156c >> 1)) + 0x23,
                         iDrawY + 2, lpString, (LPSIZE)0x0, '\0', 0);
        }
        lpString = lpString + 0x21e;
        iVar8++;
    } while ((int)lpString < (int)(uintptr_t)(DAT_083a45d8 + 0x34ee));

    // ── PASS 4: draw channel sub-list for selected server ─────────────────────
    iVar8 = DAT_00561694;
    if (DAT_00561694 != -1) {
        iVar8 = DAT_00561694 * 0x21e;
        if ((&DAT_083a45ed)[iVar8] == '\0') {
            // Non-PVP server: find Y position by counting preceding non-PVP entries
            iStack00000004 = 0;
            pcVar7 = (char*)&DAT_083a45ec;
            do {
                pbVar14 = (byte*)&DAT_083a45d8 + iVar8;
                pbVar11 = (byte *)(pcVar7 - 0x14);
                do {
                    bVar2 = *pbVar11;
                    bVar15 = bVar2 < *pbVar14;
                    if (bVar2 != *pbVar14) { iVar10 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f4b0; }
                    if (bVar2 == 0) break;
                    bVar2 = pbVar11[1]; bVar15 = bVar2 < pbVar14[1];
                    if (bVar2 != pbVar14[1]) { iVar10 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f4b0; }
                    pbVar11 += 2; pbVar14 += 2;
                } while (bVar2 != 0);
                iVar10 = 0;
LAB_0051f4b0:
                iVar5 = iStack00000004;
                if (iVar10 == 0) goto LAB_0051f4d9;
                if ((*pcVar7 != '\0') && (pcVar7[1] == '\0'))
                    iStack00000004++;
                pcVar7 = pcVar7 + 0x21e;
            } while ((int)pcVar7 < (int)(uintptr_t)(DAT_083a45d8 + 0x3502));
            iVar5 = 0;
LAB_0051f4d9:
            // BUG-FIX: DAT_083a45ec es `*(DWORD*)(...)` lvalue → &DAT es DWORD*,
            // así que (&DAT)[iVar8] avanza iVar8*4 bytes. Disasm @ 0x0051f44e/4db
            // muestra `MOV AL, byte ptr [ECX + 0x83a45ec]` con ECX=iVar8 (byte
            // offset). Castear base a char* para byte arith.
            uVar6  = (uint)*((unsigned char*)&DAT_083a45ec + iVar8);
            iVar10 = iVar5 * 0x10 + uVar6 * -10 + 8 + (int)fVar4;
        } else {
            // PVP server: find Y position by counting preceding PVP entries
            iStack00000004 = 0;
            pcVar7 = (char*)&DAT_083a45ec;
            do {
                pbVar11 = (byte *)(pcVar7 - 0x14);
                pbVar14 = (byte*)&DAT_083a45d8 + iVar8;
                do {
                    bVar2 = *pbVar11; bVar15 = bVar2 < *pbVar14;
                    if (bVar2 != *pbVar14) { iVar10 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f428; }
                    if (bVar2 == 0) break;
                    bVar2 = pbVar11[1]; bVar15 = bVar2 < pbVar14[1];
                    if (bVar2 != pbVar14[1]) { iVar10 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f428; }
                    pbVar11 += 2; pbVar14 += 2;
                } while (bVar2 != 0);
                iVar10 = 0;
LAB_0051f428:
                iVar5 = iStack00000004;
                if (iVar10 == 0) goto LAB_0051f44c;
                if ((*pcVar7 != '\0') && (pcVar7[1] == '\x01'))
                    iStack00000004++;
                pcVar7 = pcVar7 + 0x21e;
            } while ((int)pcVar7 < (int)(uintptr_t)(DAT_083a45d8 + 0x3502));
            iVar5 = 0;
LAB_0051f44c:
            uVar6  = (uint)*((unsigned char*)&DAT_083a45ec + iVar8);  // BUG-FIX (ver arriba)
            iVar10 = iVar5 * 0x10 + uVar6 * -10 + 8 + (int)fStack00000008;
        }

        // Clamp scroll
        iVar5 = (0x18 - uVar6) * 0x14;
        if (iVar5 <= iVar10) iVar10 = iVar5;

        // Render each channel row
        iVar5 = 0;
        if (uVar6 != 0) {
            fStack00000018 = 0.0f;
            iStack00000004 = iVar10;
            do {
                fVar4 = fStack00000018;
                fVar3 = 1.0f;
                if (iVar5 != DAT_00561698)
                    fVar3 = 0.8f;   // not selected → dim

                // Color by load
                // BUG-FIX: DAT_083a4606 es `*(WORD*)(...)` y DAT_083a4604 es
                // `*(DWORD*)(...)` lvalues. Indexar `(&DAT)[idx]` o `&DAT+idx`
                // multiplica el offset por 2/4. Disasm @ 0x0051f538/540:
                //   MOV AX,word ptr [ECX+EBP*1+0x83a4604]   ; channel_id
                //   MOV CL,byte ptr [ECX+EBP*1+0x83a4606]   ; load byte
                // ECX=server*0x21e, EBP=chan*0x1a (ambos byte offsets).
                if ((*((unsigned char*)&DAT_083a4606 + (int)fStack00000018 + iVar8) & 0x80) == 0x80) {
                    // FULL: red-ish
                    fVar18 = fVar3 * _DAT_00552928;
                    fVar20 = fVar3 * _DAT_00552504;
                } else if ((*((unsigned char*)&DAT_083a4606 + (int)fStack00000018 + iVar8) & 0x7f) < 100) {
                    // LOW: greenish; special teal for channel_id % 20 == 0 or 14
                    fVar18 = fVar3;
                    if (((uint)*(ushort *)((char*)&DAT_083a4604 + (int)fStack00000018 + iVar8) % 0x14 == 0) ||
                        (fVar20 = fVar3,
                         (uint)*(ushort *)((char*)&DAT_083a4604 + (int)fStack00000018 + iVar8) % 0x14 == 0xe))
                    {
                        fVar20 = fVar3 * _DAT_00552928;
                    }
                } else {
                    // NORMAL: white
                    fStack00000008 = fVar3 * _DAT_005528b4;
                    fVar18 = fStack00000008; fVar20 = fStack00000008;
                }
                glColor3f(fVar3, fVar18, fVar20);

                // Channel background quad
                fVar18 = (float)iStack00000004;
                GL_DrawTexture(0x12, 230.0f, fVar18, 134.0f, 20.0f,
                             0.0f, 0.0f, 0.5234375f, 0.9375f, '\x01', '\x01');

                // Channel name text
                ptVar19 = &text_size;
                iVar8 = lstrlenA(chan_buf + (iVar5 + DAT_00561694 * 0x14) * 100);
                GetTextExtentPointA(DAT_055c9fec,
                                    chan_buf + (iVar5 + DAT_00561694 * 0x14) * 100,
                                    iVar8, ptVar19);
                UI_RenderText(0x129 - ((uint)(text_size.cx * 0x280) / DAT_0056156c >> 1),
                             iStack00000004 + 1,
                             chan_buf + (iVar5 + DAT_00561694 * 0x14) * 100,
                             (LPSIZE)0x0, '\0', 0);

                // Load bar (only for non-full servers)
                // BUG-FIX (idem): WORD lvalue → necesita byte arith (char* cast)
                if ((*((unsigned char*)&DAT_083a4606 + DAT_00561694 * 0x21e + (int)fVar4) & 0x80) != 0x80) {
                    uVar6 = *((unsigned char*)&DAT_083a4606 + DAT_00561694 * 0x21e + (int)fVar4) & 0x7f;
                    if (100 < uVar6) uVar6 = 100;
                    uVar6 = uVar6 / 5;   // 0..20 dots
                    fStack00000008 = 0.0f;
                    if (uVar6 != 0) {
                        // Filled dots (gradient: darker on left, brighter on right)
                        fVar20 = fVar18 + _DAT_00552d4c;
                        iStack0000001c = 0;
                        iStack00000020 = 0x14;
                        do {
                            iVar8 = (int)fStack00000008;
                            fStack0000000c = (float)iStack00000020 * fVar3 * _DAT_005524f4;
                            glColor3f((float)(int)fStack00000008 * fVar3 * _DAT_005524f4,
                                      fStack0000000c, fStack0000000c);
                            GL_DrawTexture(0xf6, (float)iStack0000001c + _DAT_00552d48,
                                         fVar20, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
                            fStack00000008 = (float)(iVar8 + 1);
                            iStack00000020--;
                            iStack0000001c += 5;
                            fVar4 = fStack00000018;
                        } while ((int)fStack00000008 < (int)uVar6);
                    }
                    // Empty dots (gray)
                    fStack00000008 = fVar3 * _DAT_005524f4;
                    glColor3f(fStack00000008, fStack00000008, fStack00000008);
                    if (uVar6 < 0x14) {
                        fVar18 = fVar18 + _DAT_00552d4c;
                        fVar3  = (float)(uVar6 * 5);
                        fStack0000000c = fVar18;
                        do {
                            GL_DrawTexture(0xf6, (float)(int)fVar3 + _DAT_00552d48,
                                         fVar18, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
                            fVar3 = (float)((int)fVar3 + 5);
                        } while ((int)fVar3 < 100);
                    }
                }

                iVar5++;
                iVar8 = DAT_00561694 * 0x21e;
                iStack00000004 += 0x14;
                fStack00000018 = (float)((int)fVar4 + 0x1a);
            } while (iVar5 < (int)(uint)*((unsigned char*)&DAT_083a45ec + iVar8));  // BUG-FIX: byte arith
        }

        // ── PASS 5: selected server IP display ───────────────────────────────
        DAT_00559c78 = 0xffffffff;
        DAT_00559c80 = 0x80000000;
        // BUG-FIX: literales 0x3fxxxxxx eran int → float value-cast. Usar literales float.
        glColor3f(1.0f, 0.2f, 0.1f);   // orange-red (IP text)

        fVar4   = (float)(int)puStack00000010;
        pbVar14 = (byte*)&DAT_07d52c38;
        pbVar11 = (byte*)&DAT_083a45d8 + DAT_00561694 * 0x21e;
        do {
            bVar2 = *pbVar11; bVar15 = bVar2 < *pbVar14;
            if (bVar2 != *pbVar14) { iVar8 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f8b4; }
            if (bVar2 == 0) break;
            bVar2 = pbVar11[1]; bVar15 = bVar2 < pbVar14[1];
            if (bVar2 != pbVar14[1]) { iVar8 = (1 - (uint)bVar15) - (uint)(bVar15 != 0); goto LAB_0051f8b4; }
            pbVar11 += 2; pbVar14 += 2;
        } while (bVar2 != 0);
        iVar8 = 0;
LAB_0051f8b4:
        if (iVar8 == 0) {
            // Currently connected to this server → show IP
            UI_RenderText(0x96, (int)puStack00000010 - 0x2e, (char*)&DAT_07d530e8, (LPSIZE)0x0, '\0', 0);
            UI_RenderText(0x96, (int)fVar4 - 0x1f, (char*)&DAT_07d53214, (LPSIZE)0x0, '\0', 0);
            iVar8  = 0;
        }
    }
    return iVar8;
}
