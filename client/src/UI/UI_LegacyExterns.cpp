// UI_LegacyExterns.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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


// FUN_004c4650 and FUN_004c8d70 — implemented in src/UI/RenderItemInfo.cpp

// FUN_004c9730 @ 0x004C9730 — UI_CommandPanel_BuildEntry(chardata, slot)
// Real logic: calls FUN_0047e4f0 (GetMagicSkillDamage) and GetSkillInformation for the
// skill in CharacterAttribute->Skill[param_2+4], then sprintf's skill name, damage, mana cost,
// distance, and class-specific descriptions into the Items[999] text buffer (stride 100 bytes).
// Class 0 (Dark Wizard): skill 0x10 gets 3 extra stat lines (MaxMana, Energy, Dexterity).
// Class 2 (Fairy Elf): skills 0x1A/0x1B/0x1C get special description lines.
// Class 1 (Dark Knight): skill 0x2F gets extra combo line.
// Finally calls FUN_004c2420 (CharMenu_RenderTextList) with unaff_retaddr as Y position.
// STUB: unaff_retaddr carries screen Y position from caller — cannot resolve without
// call-site disassembly. Also uses CharacterAttribute (undeclared typed struct).
// FUN_004c9730 @ 0x004C9730 — Skill_RenderTooltip(float a1, int a2, int hoveredSkillIdx)
// Ported from IDA `sub_4C9730` decompile (1844 bytes).
//
// Builds a tooltip text-list for the hovered skill (TextList[0..n]) describing
// name / damage / distance / mana / skillMana, then calls FUN_004c2420
// (CharMenu_RenderTextList) to draw it at (a1, a2).
//
// Skipped (per anti-tamper policy):
//   - Hash-table ref-count dance on CharacterMachine (start + end)
//   - XOR encryption pass over the CharacterMachine buffer (end of function)
//
// Ported game logic:
//   1. Read SkillTable[a3].Skill = *(BYTE*)(CharacterAttribute + a3 + 87)
//   2. CHARACTER_MACHINE::GetMagicSkillDamage → piMinDamage / piMaxDamage
//   3. GetSkillInformation → szName, piMana, piDistance, piSkillMana
//   4. Class-specific damage formulas (DW skill 16 / Elf 0x1A/0x1B/0x1C / DK 47)
//   5. Distance / Mana / SkillMana lines
//   6. FUN_004c2420 with computed Y / count
extern "C++" {
extern char    GlobalText[GLOBALTEXT_ROWS][300];   // ver globals.h
extern char    lpString_07e90798[];
extern int     DAT_07e91708[30];
extern int     DAT_07ea7b10[30];
extern HDC     m_hFontDC;
extern float   _DAT_055c9b74;
}

static void SkillTooltip_RenderLines(int sx, int sy, char lines[][100], int count)
{
    if (count <= 0 || !m_hFontDC) return;

    TEXTMETRICA tm = {};
    GetTextMetricsA(m_hFontDC, &tm);
    const int lineH = tm.tmHeight + tm.tmExternalLeading;
    const int padX = 4;
    const int padY = 3;

    int maxWidth = 0;
    for (int i = 0; i < count; ++i) {
        SIZE sz = {};
        GetTextExtentPointA(m_hFontDC, lines[i], (int)strlen(lines[i]), &sz);
        if (sz.cx > maxWidth) maxWidth = sz.cx;
    }

    int boxW = maxWidth + padX * 2;
    int boxH = count * lineH + padY * 2;
    int drawX = sx - boxW / 2;
    int drawY = sy - boxH - 8;
    if (drawY < 0) drawY = sy + 8;
    if (drawX < 0) drawX = 0;
    if (drawX + boxW > (int)WindowWidth) drawX = (int)WindowWidth - boxW;
    if (drawX < 0) drawX = 0;

    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    GL_DrawRect((float)(drawX - 1), (float)(drawY - 1), (float)(boxW + 2), 1.0f);
    GL_DrawRect((float)(drawX - 1), (float)(drawY + boxH), (float)(boxW + 2), 1.0f);
    GL_DrawRect((float)(drawX - 1), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    GL_DrawRect((float)(drawX + boxW), (float)(drawY - 1), 1.0f, (float)(boxH + 2));
    glColor4f(0.0f, 0.0f, 0.0f, 0.82f);
    GL_DrawRect((float)drawX, (float)drawY, (float)boxW, (float)boxH);

    int lineY = drawY + padY;
    for (int i = 0; i < count; ++i) {
        DAT_00559c78 = (i == 0) ? 0xFFFFFF00 : 0xFFFFFFFF;
        m_dwBackColor = 0;
        FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, drawX + padX, lineY, lines[i], 0);
        lineY += lineH;
    }
}

static void FUN_004c9730_old(float a1, int a2, int a3)
{
    // 2026-05-05: SIMPLIFIED safe version. La versión completa hacía 10+
    // sprintf_s con GlobalText[N] format strings. Si cualquier slot de
    // GlobalText estaba sin cargar/corrupted (e.g. tenía "%s" donde el código
    // pasa un int), sprintf interpretaba el int como char* → AV crash on
    // hover. Esta versión solo muestra el nombre del skill (sin damage,
    // mana, distance lines) hasta que GlobalText loader esté validado.
    //
    // Bounds check: a3 (slot index hovered) debe ser 0..63 para evitar OOB
    // read en CharacterAttribute[87 + a3].
    if (a3 < 0 || a3 >= 60 || !CharacterAttribute) return;

    unsigned char skillType = *(unsigned char*)((char*)CharacterAttribute + a3 + 87);
    if (skillType == 0 || skillType >= 64) return;  // empty slot or OOB type

    char szName[256];
    szName[0] = 0;
    int piMana = 0, piDistance = 0, piSkillMana = 0;
    GetSkillInformation((int)skillType, 1, szName, &piMana, &piDistance, &piSkillMana);
    if (szName[0] == 0) return;  // no name → don't render

    char lines[5][100] = {};
    int count = 0;
    sprintf_s(lines[count++], 100, "%s", szName);
    if (piDistance > 0) sprintf_s(lines[count++], 100, "Distance: %d", piDistance);
    sprintf_s(lines[count++], 100, "Mana: %d", piMana);
    if (piSkillMana > 0) sprintf_s(lines[count++], 100, "Skill Mana: %d", piSkillMana);
    SkillTooltip_RenderLines((int)a1, a2, lines, count);
    return;

    // Original full impl preserved below for reference but disabled.
#if 0
    int  piMinDamage = 0, piMaxDamage = 0;
    int  piMana = 0, piDistance = 0, piSkillMana = 0;
    (void)piMinDamage; (void)piMaxDamage; (void)piMana; (void)piDistance; (void)piSkillMana;

    // Get min/max damage range — FUN_0047e4f0 (GetMagicSkillDamage, ~700 bytes, IDA-only
    // and gated behind IDA_PORT_0047E4F0). Without it we leave piMin/piMaxDamage at 0;
    // the damage line will show "0~0" until the helper is unconditionally ported.

    // Skill name + mana / distance / mana costs
    GetSkillInformation(skillType, 1, szName, &piMana, &piDistance, &piSkillMana);

    // ── Build TextList lines ────────────────────────────────────────────────
    char* TextList0 = lpString_07e90798;
    auto TextListN = [&](int i) -> char* { return lpString_07e90798 + i * 100; };
    int* TextListColor = DAT_07e91708;
    int* TextBold = DAT_07ea7b10;

    sprintf_s(TextList0, 100, "\n");
    sprintf_s(TextListN(1), 100, "%s", szName);
    TextListColor[1] = 1;
    TextBold[1] = 1;
    sprintf_s(TextListN(2), 100, "\n");

    int v10 = 3;
    int heroClassFlag = *(unsigned char*)(Hero + 444) & 7;

    // Class 0 (Dark Wizard / magic class): damage line
    if (heroClassFlag == 0) {
        if (skillType == 16) {
            // Twister-style: derived from Strength (CA+34), Energy (CA+26), Agility (CA+22)
            int agi = *(unsigned short*)((char*)CharacterAttribute + 22);
            int ene = *(unsigned short*)((char*)CharacterAttribute + 26);
            int str = *(unsigned short*)((char*)CharacterAttribute + 34);
            int dmg = (int)((double)agi * 0.02 + (double)ene * 0.0049999999 + 10.0);
            int strBoost = (int)((double)str * 0.02);
            int dur = (int)((double)ene * 0.025 + 60.0);
            sprintf_s(TextListN(3), 100, GlobalText[578], (unsigned int)dmg);
            TextListColor[3] = 0; TextBold[3] = 0;
            sprintf_s(TextListN(4), 100, GlobalText[880], strBoost);
            TextListColor[4] = 0; TextBold[4] = 0;
            sprintf_s(TextListN(5), 100, GlobalText[881], (unsigned int)dur);
            TextListColor[5] = 0; TextBold[5] = 0;
            v10 = 6;
        } else {
            sprintf_s(TextListN(3), 100, GlobalText[170], piMinDamage, piMaxDamage);
            TextListColor[3] = 0; TextBold[3] = 0;
            v10 = 4;
        }
    }

    // Class 2 (Elf): buff strength preview
    if (heroClassFlag == 2) {
        int ene = *(unsigned short*)((char*)CharacterAttribute + 26);
        bool emitted = true;
        switch (skillType) {
        case 0x1A:
            sprintf_s(TextListN(v10), 100, GlobalText[171], ene / 5 + 5);
            break;
        case 0x1B:
            sprintf_s(TextListN(v10), 100, GlobalText[172], (ene >> 3) + 2);
            break;
        case 0x1C:
            sprintf_s(TextListN(v10), 100, GlobalText[173], ene / 7 + 3);
            break;
        default:
            emitted = false;
            break;
        }
        if (emitted) {
            TextListColor[v10] = 0;
            TextBold[v10] = 0;
            v10++;
        }
    }

    // Distance line
    if (piDistance) {
        sprintf_s(TextListN(v10), 100, GlobalText[174], piDistance);
        TextListColor[v10] = 0;
        TextBold[v10] = 0;
        v10++;
    }

    // Mana line
    sprintf_s(TextListN(v10), 100, GlobalText[175], piMana);
    TextListColor[v10] = 0;
    TextBold[v10] = 0;
    int v15 = v10 + 1;

    // SkillMana (if > 0)
    if (piSkillMana > 0) {
        sprintf_s(TextListN(v15), 100, GlobalText[360], piSkillMana);
        TextListColor[v15] = 0;
        TextBold[v15] = 0;
        v15++;
    }

    // Class 1 (DK) + skill 47 → "cannot use" message
    if (heroClassFlag == 1 && skillType == 47) {
        sprintf_s(TextListN(v15), 100, "%s", GlobalText[96]);
        TextListColor[v15] = 5;
        TextBold[v15] = 0;
        v15++;
    }

    sprintf_s(TextListN(v15), 100, "\n");
    int v16 = v15 + 1;

    // Anti-tamper XOR encryption pass on CharacterMachine — skipped (anti-tamper).

    // Compute Y position from text height
    SIZE sz; sz.cx = 0; sz.cy = 0;
    GetTextExtentPointA(m_hFontDC, TextList0, 1, &sz);
    int v31 = 3 * sz.cy / 2 + sz.cy * (v16 - 3);
    float yPos = (float)v31 / _DAT_055c9b74;

    // Render. Our FUN_004c2420 has 6-int signature (mode, startIdx, count, x, layout, border).
    // Best-effort mapping of IDA's 7-arg float-mixed call:
    //   mode=2 (boxed), startIdx=0, count=v16, x=a2-yPos, layout=0, border=1
    FUN_004c2420(2, 0, v16, a2 - (int)yPos, 0, 1);
    (void)a1;  // a1 (float Y) not used by our simplified render path
#endif
}

void __cdecl FUN_004c9730(float a1, int a2, int a3)
{
    int skillTipX = *(int*)&a1;
    if (a3 < 0 || a3 >= 60 || !CharacterAttribute || !Hero) return;

    unsigned char skillType = *(unsigned char*)((char*)CharacterAttribute + a3 + 87);
    if (skillType == 0 || skillType >= 64) return;

    unsigned char* skillBaseTbl = DAT_07d29d20 ? (unsigned char*)(uintptr_t)DAT_07d29d20 : nullptr;
    unsigned char* skillStatTbl = DAT_07cf1ff8 ? (unsigned char*)(uintptr_t)DAT_07cf1ff8 : skillBaseTbl;
    if (!skillBaseTbl && !skillStatTbl) return;

    char szName[256] = {};
    int piMinDamage = 0, piMaxDamage = 0;
    int piMana = 0, piDistance = 0, piSkillMana = 0;

    unsigned char* skillBaseRec = skillBaseTbl ? skillBaseTbl + (int)skillType * 0x28 : nullptr;
    unsigned char* skillStatRec = skillStatTbl ? skillStatTbl + (int)skillType * 0x28 : skillBaseRec;
    const char* skillName = "";
    if (skillBaseRec && ((const char*)skillBaseRec)[0] != '\0')
        skillName = (const char*)skillBaseRec;
    else if (skillStatRec && ((const char*)skillStatRec)[0] != '\0')
        skillName = (const char*)skillStatRec;
    if (skillName && skillName[0]) {
        strncpy_s(szName, sizeof(szName), skillName, 31);
        szName[31] = '\0';
    }
    GetSkillInformation((int)skillType, 1, szName[0] ? NULL : szName, &piMana, &piDistance, &piSkillMana);

    if (skillStatRec) {
        int rangeVal = (int)skillStatRec[0x27];
        if (rangeVal > 0 && rangeVal < 50) {
            piDistance = rangeVal;
        } else if (piDistance < 0 || piDistance > 50) {
            piDistance = 0;
        }
    }

    if (piMana < 0 || piMana > 5000) piMana = 0;
    if (piSkillMana < 0 || piSkillMana > 5000) piSkillMana = 0;

    if (CharacterMachine && skillStatRec) {
        __try {
            int baseDamage = (int)*(WORD*)(skillStatRec + 0x24);
            if (baseDamage > 0 && baseDamage < 5000) {
                piMinDamage = baseDamage + *(unsigned short*)((char*)CharacterMachine + 70);
                piMaxDamage = baseDamage + (baseDamage >> 1) + *(unsigned short*)((char*)CharacterMachine + 72);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            piMinDamage = 0;
            piMaxDamage = 0;
        }
    }

    // ── Build TextList lines ────────────────────────────────────────────────
    // Estructura del binario (0x004C97xx, y el volcado de IDA que quedo en
    // FUN_004c9730_old bajo `#if 0`):
    //   slot 0 = "\n"                       (separador de media altura)
    //   slot 1 = nombre de la skill         color 1 (azul claro), NEGRITA
    //   slot 2 = "\n"
    //   slot 3+ = dano / rango / mana / AG  color 0 (blanco)
    //   opcional "no puede usarla"          color 5 (blanco con franja)
    //   ultimo  = "\n"
    //
    // 2026-08-18: antes esto pintaba su PROPIA caja (cuarta reimplementacion
    // inventada del tooltip, con colores ARGB y textos en ingles hardcodeados).
    // Ahora usa lpString_07e90798 + FUN_004c2420, que es lo que hace el binario
    // — misma rutina que el tooltip de item y el menu de personaje.
    auto  TextListN     = [](int i) -> char* { return lpString_07e90798 + i * 100; };
    auto  GlobalTextOr  = [](int idx, const char* fallback) -> const char* {
        if (idx >= 0 && idx < GLOBALTEXT_ROWS && GlobalText[idx][0])
            return GlobalText[idx];
        return fallback;
    };

    int idx = 0;
    for (int i = 0; i < 30; ++i) {
        lpString_07e90798[i * 100] = 0;
    }

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    _snprintf_s(TextListN(idx), 100, _TRUNCATE, "%s", szName);
    DAT_07e91708[idx] = 1;
    DAT_07ea7b10[idx] = 1;
    idx++;

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    if (piMinDamage > 0 || piMaxDamage > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(170, "Wizardry Dmg:%d~%d"), piMinDamage, piMaxDamage);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    if (piDistance > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(174, "Range: %d"), piDistance);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(175, "Mana: %d"), piMana);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }
    if (piSkillMana > 0) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE,
                    GlobalTextOr(360, "AG: %d"), piSkillMana);
        DAT_07e91708[idx] = 0;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }

    // DK (clase 1) con la skill 47: linea de "no puede usarla", color 5 — el
    // unico color con franja de fondo (m_dwBackColor = 0xff0000a0).
    if (Hero && ((*(BYTE*)((char*)Hero + 0x1BC) & 7) == 1) && skillType == 47) {
        _snprintf_s(TextListN(idx), 100, _TRUNCATE, "%s", GlobalTextOr(96, ""));
        DAT_07e91708[idx] = 5;
        DAT_07ea7b10[idx] = 0;
        idx++;
    }

    crt_sprintf(TextListN(idx), "\n");
    DAT_07e91708[idx] = 0;
    DAT_07ea7b10[idx] = 0;
    idx++;

    // Posicion Y — port literal de 0x004C9DF7..0x004C9E36:
    //   v31 = (count - 3) * cy + (3 * cy) / 2
    //   y   = a2 - (int)(v31 / g_fScreenRate_y)
    //   DrawItemInfoBox(x, y, count, 0, 2, 1)
    {
        SIZE sz;
        sz.cx = 0;
        sz.cy = 0;
        GetTextExtentPointA(m_hFontDC, lpString_07e90798, 1, &sz);
        int v31 = (idx - 3) * sz.cy + (3 * sz.cy) / 2;
        int yBox = a2 - (int)((float)v31 / _DAT_055c9b74);
        FUN_004c2420(skillTipX, yBox, idx, 0, 2, 1);
    }
}

extern "C" int Text_MeasureOrthoWidth(const char* text);   // definido más abajo

// FUN_0047F7A0 @ 0x0047F7A0 (IDA)
// UI_DrawText — Text_Draw(x, y, text, maxw, iSort, extra)
// Draws text via Font vtable dispatch (FUN_0040f610) if non-empty or maxw!=0.
// Return type is void per functions.h declaration.
//
// 2026-05-04: BUG-FIX — el flag `param_5` (iSort) determinaba alineación:
//   1 = left-align (default)
//   2 = center within [x, x+maxw]
// Antes era `(void)param_5` → todo render quedaba left-aligned. El menú C
// (RenderText con centered=1 sobre maxw=70-150) renderizaba "mago", "[Soul
// Master]", "Fuerza:1000" etc. pegados a la izquierda en lugar de centrados.
// IDA original delega al `CUIRenderText::RenderText` que maneja iSort
// internamente; lo replicamos inline acá.
// IDA: FUN_0047F7A0
void __cdecl UI_DrawText(int param_1, int param_2, char *param_3, int param_4, int param_5, int param_6) {
    (void)param_6;
    if (param_3 == nullptr) return;
    // 2026-05-08: bug-fix — múltiples crashes en ucrtbase.dll's strlen/lstrlenA
    // venían de pasar punteros pequeños (< 0x100000) tipo 0x2A00 (= type*64
    // con DAT_07d78068=0). Validar el puntero antes de cualquier strlen.
    // Range: heap user-space [0x100000..0x80000000). rdata strings in our
    // exe live in [0x004XXXXX..0x00500000) — also valid.
    {
        uintptr_t p = (uintptr_t)param_3;
        if (p < 0x100000 || p >= 0x80000000) return;
    }
    __try {
        if (strlen(param_3) == 0 && param_4 == 0) return;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return;   // strlen crashed — bogus param_3
    }

    int x = param_1;

    // Aplicar centrado dentro del box [param_1, param_1+param_4].
    // param_1 y param_4 están en unidades del ortho; el extent de GDI viene en
    // píxeles de framebuffer.  Text_MeasureOrthoWidth hace la conversión (es el
    // equivalente correcto del `sz.cx / g_fScreenRate_x` de IDA para nuestro
    // pipeline).  Sin ella el texto quedaba descentrado hacia la izquierda.
    if (param_5 >= 2 && param_4 > 0 && DAT_055c9fec) {
        int textW = Text_MeasureOrthoWidth(param_3);
        if (textW > 0 && textW < param_4) {
            x = param_1 + (param_4 - textW) / 2;
        }
    }

    FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8, x, param_2,
                 (const char*)param_3, (DWORD)param_4);
}

// FUN_004977f0 @ 0x004977F0 — String_FindSubstr(str, pattern, from_start)
// DBCS-aware strstr. param_3!=0 forces search from position 0 only.
// Returns 1 if found, 0 otherwise.
unsigned int __cdecl FUN_004977f0(char *param_1, void *param_2, char param_3) {
    char *pat = (char*)param_2;
    int iVar5 = (int)strlen(pat);
    // BUG-FIX 2026-07-17: patrón vacío = no-match. El IDA devuelve 1 para patrón
    // vacío, pero eso solo es "correcto" porque en el original los strings de filtro
    // están cargados (no vacíos). Varios de esos globals llegan vacíos en runtime en
    // nuestro build (ej. GlobalText[457/458] si el Text.bmd no tiene esas filas) →
    // FindText(nombre,"")=1 rechazaba TODO nombre en el create-char. Un patrón vacío
    // no debe matchear nada.
    if (iVar5 < 1) return 0;
    int iVar6 = (int)strlen(param_1) - iVar5;
    if (param_3 != '\0') iVar6 = 0;
    if (iVar6 < 0) return 0;
    for (int iVar8 = 0; iVar8 <= iVar6; ) {
        int iVar2 = 0;
        if (iVar5 < 1) return 1;
        while ((param_1 + iVar8)[iVar2] == pat[iVar2]) {
            if (++iVar2 >= iVar5) return 1;
        }
        char c = FUN_00541eab((byte*)(param_1 + iVar8));
        iVar8 += (unsigned int)(unsigned char)c;
    }
    return 0;
}

// FUN_0047fe30 @ 0x0047FE30 — Text_SplitAtMiddle(src, dstB_ptr, dstA, len)
// Splits src at middle: first half → dstA (param_3), second half → *(char*)param_2.
// Split point: first space near len/2, or forced at len/2+2.
void __cdecl FUN_0047fe30(void *param_1_v, int param_2, void *param_3_v, int param_4) {
    char *param_1 = (char*)param_1_v;
    char *param_3 = (char*)param_3_v;
    unsigned int uVar4 = 0, uVar3 = 0;
    if (param_4 > 0) {
        do {
            uVar3 = uVar4;
            if ((param_4/2 - 2 <= (int)uVar4 && param_1[uVar4] == ' ') ||
                (param_4/2 + 2 <= (int)uVar4)) break;
            char c = FUN_00541eab((byte*)(param_1 + uVar4));
            uVar4 += (unsigned int)(unsigned char)c;
            uVar3 = 0;
        } while ((int)uVar4 < param_4);
    }
    if ((int)uVar3 > 0) memcpy(param_3, param_1, uVar3);
    param_3[uVar3] = '\0';
    for (unsigned int i = uVar3; (int)i < param_4; i++)
        ((char*)param_2)[i - uVar3] = param_1[i];
    ((char*)param_2)[param_4 - uVar3] = '\0';
}

// FUN_0040f610 @ 0x0040F610 — CUIRenderText::RenderText (vtable dispatcher)
// Original IDA: `(*(vtable[0]+4))(this, x, y, text, ...)` — thiscall through
// CUIRenderText->pSubclass->vtable[1]. El subclass se instancia en OpenFont
// (0x0050f690) vía sub_40F570 y puede ser tipo-0 (simple, TGA font) o tipo-1
// (compleja, bitmap font procesada). El subclass tipo-0 usa `Bitmaps[0/1]`
// (Interface/FontInput.tga) para renderizar cada glyph como un quad.
//
// Port: implementación autónoma usando wglUseFontBitmapsA sobre la HFONT
// GDI ya seleccionada en m_hFontDC. Genera 256 display lists a partir de la
// fuente GDI y emite el texto con glCallLists. Suficiente para UI (login,
// char-select, chat) — el look no matchea la fuente TGA original píxel a
// píxel pero los textos aparecen en su posición con el color y layout
// correctos, que era lo que faltaba.
//
// Coordenadas: callers pasan Y con origen TOP-LEFT (convención game). La GL
// ortho es `gluOrtho2D(0, vw, 0, vh)` Y-bottom (ver GL_Begin2D en
// Render/GL_2D.cpp). GL_DrawTexture Y-flipa via `vh - param_3`. Acá hacemos
// lo mismo para raster pos.
//
// Color: DAT_00559c78 es el COLORREF GDI (0x00BBGGRR). Se convierte a glColor.
// El Alpha del byte alto (cuando está seteado, ej 0xffd2e6ff) se respeta.
// ── Escala "píxeles de framebuffer" → "unidades del ortho 2D" ───────────────
// 2026-07-20.  El punto que hacía fallar todos los recuadros de texto:
//
//   · La GEOMETRÍA 2D (RenderColor / RenderBitmap) se emite en unidades del
//     ortho, que es `gluOrtho2D(0, DAT_0056156c, 0, DAT_00561570)`, y la GPU la
//     estira hasta el viewport.  Un quad de ancho W se ve W * (viewport/ortho).
//   · Los GLIFOS, en cambio, los pinta wglUseFontBitmaps + glBitmap, que hace
//     un blit 1:1 EN PÍXELES DE FRAMEBUFFER desde el raster position.  NO se
//     estiran.  Y `GetTextExtentPointA` mide en esos mismos píxeles.
//
// O sea que el ancho del texto y el ancho de su fondo viven en unidades
// distintas, y hay que dividir el extent por la relación viewport/ortho para
// que el recuadro cubra exactamente las letras.
//
// Deliberadamente NO usamos `g_fScreenRate_x` (_DAT_055c9b70) ni
// `Screen_ToGLX` para esto: son dos fuentes de escala que en nuestro build
// están DESINCRONIZADAS.  `g_fScreenRate_x` sale de `g_ScreenW` (Config_Load),
// mientras que el ortho y el viewport salen de `DAT_0056156c` — y son dos
// variables separadas (globals.cpp:303 vs Config_Load.cpp:49), donde nada
// copia una a la otra.  Preguntarle a OpenGL por su viewport es la única
// fuente que no puede desincronizarse, y sigue siendo correcta si algún día
// se unifican esos globals.
static void Text_PixelToOrthoScale(float* outX, float* outY)
{
    *outX = 1.0f;
    *outY = 1.0f;
    GLint vp[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, vp);
    DWORD ow = DAT_0056156c ? DAT_0056156c : 640;
    DWORD oh = DAT_00561570 ? DAT_00561570 : 480;
    if (vp[2] > 0 && ow > 0) *outX = (float)vp[2] / (float)ow;
    if (vp[3] > 0 && oh > 0) *outY = (float)vp[3] / (float)oh;
    if (*outX <= 0.0f) *outX = 1.0f;
    if (*outY <= 0.0f) *outY = 1.0f;
}

// Ancho del texto EN UNIDADES DEL ORTHO (que es donde vive todo el layout).
// Es el equivalente correcto, para nuestro pipeline, del `sz.cx /
// g_fScreenRate_x` que hace IDA en RenderText (0x47F650) y RenderTipText
// (0x47F7F0).
// Escala pixeles-de-framebuffer -> unidades del ortho en las que dibuja el
// stack de texto (la misma que aplica FUN_0040f610 a los glifos).
//
// OJO, NO es lo mismo que g_fScreenRate_x: ese es el factor del BINARIO, que
// pasa a espacio-640 porque su CUIRenderText reescala internamente. En nuestro
// pipeline el reescalado lo hace el ortho, asi que un ancho medido con
// GetTextExtentPointA hay que dividirlo por ESTE factor para mezclarlo con el
// layout. Usar los dos mezclados deja la caja y el texto a escalas distintas
// (ver el fix del ancho del tooltip en CharMenu_Build.cpp).
extern "C" float Text_GetOrthoScaleX(void)
{
    float sx, sy;
    Text_PixelToOrthoScale(&sx, &sy);
    (void)sy;
    return sx;
}

extern "C" int Text_MeasureOrthoWidth(const char* text)
{
    if (!text || !*text) return 0;
    HDC hFontDC = DAT_055c9fec;
    if (!hFontDC) return 0;
    SIZE sz = {0, 0};
    if (!GetTextExtentPointA(hFontDC, text, (int)strlen(text), &sz)) return 0;
    float sx, sy;
    Text_PixelToOrthoScale(&sx, &sy);
    return (int)((float)sz.cx / sx);
}

// ═══════════════════════════════════════════════════════════════════════════
// Marcadores de estilo embebidos:  "\x02" <byte-de-estilo>
// ---------------------------------------------------------------------------
// Port de sub_4104B0 (parser) + sub_4102E0 (tabla estilo→colores) + la regla de
// selección de color por columna de sub_4105F0.
//
// Dónde viven en el binario:  CUIRenderText (0x40F610) es SOLO un dispatcher
// (`(*pSubclass->vtable[1])(...)`).  El subclass lo instancia `sub_40F570` desde
// OpenFont según `g_iRenderTextType` (registro SOFTWARE\Webzen\Mu\Config
// "TextOut"):
//   · type != 1 → sub_410A90, objeto de 4 bytes (solo vtable).  Su RenderText
//                 (0x410AF0) NO conoce marcadores: rasteriza el string entero
//                 con TextOutA y lo sube como una textura de un solo color.
//   · type == 1 → sub_40F730, objeto de 0x2C4 bytes.  Su RenderText (0x40FB70)
//                 va a sub_40FCD0 → sub_4104B0 (parsea y ELIMINA los
//                 marcadores) → sub_4105F0 (pinta cada columna con el color del
//                 tramo activo), con caché de texturas por string.
//
// DESVIACIÓN CONSCIENTE: nuestro FUN_0040f610 es una reimplementación propia
// basada en glifos (wglUseFontBitmapsA), sin la partición type-0/type-1 del
// original ni su caché de texturas.  Aplicamos siempre el comportamiento del
// type-1, porque el alternativo es dibujar los bytes de control como glifos
// basura.  Consecuencia: un cliente vanilla con "TextOut" != 1 muestra esa
// basura y el nuestro no.
//
// Layout del objeto original (para poder volver a cruzarlo con IDA):
//   this+36 + 16*i → charIndex   (índice en el texto YA limpio)
//   this+40 + 16*i → pixelX      (extent del prefijo − 1; umbral de columna)
//   this+44 + 16*i → fg
//   this+48 + 16*i → bg
//   this+196       → cantidad de marcadores
//   memset(this+36, 0, 0xA0)  ⇒ 10 entradas de 16 bytes
// ═══════════════════════════════════════════════════════════════════════════

#define TEXT_STYLE_MAX_MARKERS 10   // 0xA0 / 16

struct TextStyleMarker {
    int   charIndex;    // this+36+16*i
    int   pixelX;       // this+40+16*i  (tal cual lo guarda el binario)
    int   pixelStart;   // NO está en el binario: el extent crudo (pixelX+1),
                        // que es donde arranca el tramo.  El original solo
                        // necesita el umbral, nosotros necesitamos posicionar.
    DWORD fg;           // this+44+16*i
    DWORD bg;           // this+48+16*i
};

// sub_4102E0 — tabla estilo→colores.  Port literal, incluidas las constantes
// tal como las muestra el decompile (decimales con signo).  Los DWORD están en
// orden de memoria RGBA (0xAABBGGRR), igual que m_dwTextColor/m_dwBackColor.
static void Text_StyleColors(char style, DWORD *fg, DWORD *bg)
{
    DWORD result = 0;
    DWORD v4;
    switch (style) {
        case 1:
        case -12: v4 = (DWORD)-16776961; break;                 // 0xFF0000FF
        case -16: v4 = m_dwTextColor; result = m_dwBackColor; break;  // reset
        case -15: v4 = m_dwBackColor; result = m_dwTextColor; break;  // invertido
        case -14: v4 = (DWORD)-14116;    break;                 // 0xFFFFC8DC
        case -13: v4 = (DWORD)-16711736; break;                 // 0xFF00FFC8
        case -11: v4 = (DWORD)-11521516; break;                 // 0xFF503214
        case -10: v4 = (DWORD)-16751556; break;                 // 0xFF00643C
        case -9:  v4 = (DWORD)-16777116; break;                 // 0xFF000064
        case -8:
            v4 = (DWORD)-3613466;                               // 0xFFC8DCE6
            if (!DAT_005590ac /* g_bUseChatListBox */)
                result = (DWORD)-1778384896;                    // 0x96000000
            break;
        default:  v4 = 0xFFFFFFFFu; break;
    }
    *fg = v4;
    *bg = result;
}

// sub_4104B0 — extrae los marcadores y devuelve el texto sin ellos.
// El original limpia el string IN PLACE sobre el buffer del caller; acá el
// caller nos pasa `const char*`, así que escribimos en `dst` (que debe tener
// al menos 0x100 bytes, igual que el `Destination[292]` de sub_40FCD0).
static int Text_ParseStyleMarkers(const char *src, char *dst, size_t dstCap,
                                  TextStyleMarker *out)
{
    memset(out, 0, TEXT_STYLE_MAX_MARKERS * sizeof(TextStyleMarker));

    int len = (int)strlen(src);
    int rd = 0;          // v5 — índice de lectura
    int consumed = 0;    // v6 — bytes de marcador ya consumidos
    int n = 0;           // v3 — cantidad de marcadores

    while (rd < len) {
        if (consumed >= 18) break;          // tope del original: 9 marcadores
        if (src[rd] == 2) {
            out[n].charIndex = rd - consumed;
            ++rd;
            if (rd >= len) break;           // marcador truncado al final
            Text_StyleColors(src[rd], &out[n].fg, &out[n].bg);
            ++n;
            consumed += 2;
        }
        ++rd;
    }

    if (n == 0) return 0;

    // Strip: copia todo menos los pares "\x02 <estilo>".
    size_t w = 0;
    for (int r = 0; r < len && w + 1 < dstCap; ) {
        char ch = src[r];
        if (ch == 2) { r += 2; continue; }
        dst[w++] = ch;
        ++r;
    }
    dst[w] = '\0';

    // pixelX = GetTextExtentPointA(texto limpio, charIndex).cx − 1  (o 0).
    HDC hFontDC = DAT_055c9fec;
    for (int i = 0; i < n; ++i) {
        SIZE sz = {0, 0};
        int  nch = out[i].charIndex;
        if (nch > (int)w) nch = (int)w;
        if (hFontDC && nch > 0) GetTextExtentPointA(hFontDC, dst, nch, &sz);
        out[i].pixelStart = sz.cx;
        out[i].pixelX     = sz.cx ? sz.cx - 1 : 0;
    }
    return n;
}

void __cdecl FUN_0040f610(HDC /*hdc_unused*/, int x, int y, const char *text, DWORD /*color_unused*/)
{
    if (text == NULL || *text == '\0') return;

    // ── DIAG: log first call per second to see what text is being requested
    {
        static DWORD s_lastTxt = 0;
        DWORD now = GetTickCount();
        if (now - s_lastTxt > 1000) {
            s_lastTxt = now;
            char b[200];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "TEXT call x=%d y=%d str='%s' color=0x%08X",
                x, y, text ? text : "(null)", (unsigned)DAT_00559c78);
            DbgLogPublic(b);
        }
    }

    // ── Marcadores de estilo "\x02<estilo>" (ver bloque de arriba) ───────────
    // El guard `strlen <= 0xFF` es el de sub_40FCD0 (el original solo parsea
    // strings que entran en su Destination[292]).
    char             strippedBuf[0x100];
    TextStyleMarker  markers[TEXT_STYLE_MAX_MARKERS];
    int              nMarkers = 0;
    const char      *drawText = text;
    if (strchr(text, 2) != NULL && strlen(text) <= 0xFF) {
        nMarkers = Text_ParseStyleMarkers(text, strippedBuf, sizeof(strippedBuf), markers);
        if (nMarkers > 0) drawText = strippedBuf;
        if (*drawText == '\0') return;
    }

    HGLRC hRC = wglGetCurrentContext();
    if (!hRC) return;

    HDC hFontDC = DAT_055c9fec;
    if (hFontDC == NULL) return;

    // ── FUENTE ACTIVA (fix 2026-07-20) ──────────────────────────────────────
    // Acá había `hFont = DAT_055ca00c` HARDCODEADO (la fuente regular), pero
    // los callers seleccionan fuentes DISTINTAS en este mismo DC antes de
    // llamarnos: g_hFontBold (chat, HUD_Pass2/3/4, sub_40CE20 vía
    // CUIRenderText::SetFont) y g_hFontBig (HUD_Pass3:486, doble tamaño).
    //
    // Consecuencia doble:
    //   1. Los glifos salían SIEMPRE en regular — bold y big nunca se veían.
    //   2. El recuadro de fondo se medía con GetTextExtentPointA usando la
    //      fuente que el caller seleccionó (bold/big = más ancha) mientras las
    //      letras se dibujaban en regular (más angosta) → el fondo excedía al
    //      texto.  Y cuando el caller sí había dejado la regular, calzaba.
    //      De ahí el "a veces sobra, a veces no" que quedaba después de
    //      arreglar la escala viewport/ortho.
    //
    // Ahora la fuente sale del DC (que es la que efectivamente usó el caller
    // para medir), y cacheamos las display lists POR fuente.
    HFONT hFont = (HFONT)GetCurrentObject(hFontDC, OBJ_FONT);
    if (hFont == NULL) hFont = (HFONT)(uintptr_t)DAT_055ca00c;
    if (hFont == NULL) return;

    // Cache de hasta 4 fuentes (regular / bold / big / repuesto).  Antes era
    // una sola entrada, así que alternar fuentes entre llamadas habría
    // reconstruido 256 display lists en CADA llamada.
    struct FontLists { HGLRC rc; HFONT font; GLuint base; int ascent; };
    static FontLists s_cache[4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };
    static int       s_next = 0;

    int slot = -1;
    for (int i = 0; i < 4; ++i) {
        if (s_cache[i].base != 0 && s_cache[i].rc == hRC && s_cache[i].font == hFont) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        slot = s_next;
        s_next = (s_next + 1) & 3;
        if (s_cache[slot].base != 0) {
            glDeleteLists(s_cache[slot].base, 256);
            s_cache[slot].base = 0;
        }
        GLuint base = glGenLists(256);
        if (base == 0) return;
        // hFont ya está seleccionada en el DC (de ahí la sacamos), así que no
        // hace falta SelectObject — el que había acá encima pisaba la elección
        // del caller y contaminaba las mediciones posteriores de otro código.
        if (!wglUseFontBitmapsA(hFontDC, 0, 256, base)) {
            glDeleteLists(base, 256);
            return;
        }
        TEXTMETRICA tm;
        int ascent = 11;
        if (GetTextMetricsA(hFontDC, &tm)) ascent = tm.tmAscent;
        s_cache[slot].rc     = hRC;
        s_cache[slot].font   = hFont;
        s_cache[slot].base   = base;
        s_cache[slot].ascent = ascent;
    }
    const GLuint s_fontListBase = s_cache[slot].base;
    const int    s_ascent       = s_cache[slot].ascent;

    // ── CLAMP A LA PANTALLA (port de sub_47F4C0 L18-46) ─────────────────────
    // El binario NO deja que la caja de texto se salga: antes de dibujarla
    // corre el origen para que entre.  Con a7 = 0 (que es como la llama
    // sub_410AF0) las ramas que aplican son:
    //     if (x < 0) x = 0;
    //     if (Width + x > WindowWidth)  x = WindowWidth - Width;
    //     if (byte_7E11D6E) {
    //         if (y < 0) y = 0;
    //         v12 = WindowHeight - 47 * WindowHeight / 640;
    //         if (Height + y > v12) y = v12 - Height;
    //     }
    // Width/Height son el extent en píxeles; acá los pasamos a unidades del
    // ortho con la misma escala que ya usa el fondo.
    //
    // 2026-08-21: sin esto, al llegar al borde el quad de fondo se dibujaba
    // (glVertex2f se clipea normal) pero los glifos no, porque glRasterPos
    // fuera del viewport invalida la posición y glCallLists no emite nada →
    // quedaba un recuadro negro vacío en el borde.
    extern DWORD DAT_0056156c;  // ancho del ortho 2D
    extern DWORD DAT_00561570;  // alto  del ortho 2D
    DWORD vh = DAT_00561570 ? DAT_00561570 : 480;
    DWORD vw = DAT_0056156c ? DAT_0056156c : 640;
    {
        float csx, csy;
        Text_PixelToOrthoScale(&csx, &csy);
        SIZE tot = {0, 0};
        GetTextExtentPointA(hFontDC, drawText, (int)strlen(drawText), &tot);
        float wOrtho = (float)tot.cx / csx;
        float hOrtho = (float)tot.cy / csy;
        if (x < 0) x = 0;
        if ((float)x + wOrtho > (float)vw) x = (int)((float)vw - wOrtho);
        if (DAT_07e11d6e) {
            if (y < 0) y = 0;
            float bottom = (float)vh - (47.0f * (float)vh) / 640.0f;
            if ((float)y + hOrtho > bottom) y = (int)(bottom - hOrtho);
        }
        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }

    // Y-flip: game pasa y con top=0, ortho GL usa bottom=0.
    int   rasterY = (int)vh - y - s_ascent;

    // Color base desde DAT_00559c78 (COLORREF 0x00BBGGRR + opcional alpha en
    // byte 3).  Con marcadores presentes esto es solo el color del PRIMER tramo;
    // el resto sale de markers[].fg (ver el loop de abajo).
    //
    // 2026-05-04: respetar el alpha que el CALLER setea via glColor4f(...,α)
    // antes de RenderText. Antes pisábamos con `glColor4ub(R,G,B,A)` y se
    // perdía el cross-fade del banner clase/zona (los dos textos siempre a
    // α=1 → solapaban). Multiplicamos los alphas para que tanto el del
    // texto-color (DAT_00559c78 byte 3) como el del caller respeten su
    // contribución.
    GLfloat curColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glGetFloatv(GL_CURRENT_COLOR, curColor);
    GLubyte callerA = (GLubyte)(curColor[3] * 255.0f + 0.5f);

    // 2026-08-21: el color del caller (glColor3f) también MODULA el RGB, no
    // sólo el alpha.  En el binario el subclass por defecto de CUIRenderText
    // (sub_410AF0, g_iRenderTextType != 1) hace TextOut a un DIB, copia los
    // píxeles a una textura pintándolos con m_dwTextColor / m_dwBackColor
    // (sub_40F6C0) y dibuja el quad con RenderBitmap (0x5125A0) — que NO llama
    // a glColor, así que la textura sale modulada por el color que dejó el
    // caller.  O sea: color final = m_dwTextColor × glColor.
    //
    // Sin esto, los `glColor3f` de RenderItemName (0x4C9E70) no hacían nada y
    // los nombres del suelo salían todos con m_dwTextColor: el Zen sin su
    // dorado y los items +N sin su color por nivel.
    GLfloat callerR = curColor[0], callerG = curColor[1], callerB = curColor[2];

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LIST_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── FONDO + GLIFOS, POR TRAMOS DE COLOR ──────────────────────────────────
    // Sin marcadores hay un único tramo con (m_dwTextColor, m_dwBackColor), o
    // sea exactamente el comportamiento previo.
    //
    // El original (sub_4105F0) resuelve el color por COLUMNA de píxel: busca el
    // último marcador cuyo pixelX sea menor que la columna.  Como pixelX es el
    // extent del prefijo, ese corte cae justo en el borde del carácter, así que
    // partir por charIndex — que es lo natural para un renderer de glifos — da
    // el mismo resultado.
    //
    // FONDO: 2026-07-19 — faltaba por completo.  En el original el subclass
    // pinta el fondo (m_dwBackColor / bg del marcador) en los píxeles NO-texto
    // del tramo; por eso los mensajes del chat salían sin su recuadro.
    // Formato 0xAABBGGRR igual que el color de texto.  Alpha 0 = sin fondo.
    {
        float sx, sy;
        Text_PixelToOrthoScale(&sx, &sy);

        const int drawLen = (int)strlen(drawText);
        const int nRuns   = nMarkers + 1;

        glListBase(s_fontListBase);

        for (int run = 0; run < nRuns; ++run) {
            int startChar = (run == 0)        ? 0        : markers[run - 1].charIndex;
            int endChar   = (run == nMarkers) ? drawLen  : markers[run].charIndex;
            if (startChar < 0)        startChar = 0;
            if (startChar > drawLen)  startChar = drawLen;
            if (endChar   > drawLen)  endChar   = drawLen;
            if (endChar <= startChar) continue;

            DWORD fg    = (run == 0) ? DAT_00559c78 : markers[run - 1].fg;
            DWORD bc    = (run == 0) ? DAT_00559c80 : markers[run - 1].bg;
            int   runPx = (run == 0) ? 0            : markers[run - 1].pixelStart;
            float runX  = (float)x + (float)runPx / sx;

            SIZE bsz = {0, 0};
            BOOL haveExtent = GetTextExtentPointA(hFontDC, drawText + startChar,
                                                  endChar - startChar, &bsz);

            GLubyte bA = (GLubyte)((bc >> 24) & 0xFF);
            if (bA != 0 && haveExtent && bsz.cx > 0) {
                GLubyte bR = (GLubyte)( bc        & 0xFF);
                GLubyte bG = (GLubyte)((bc >>  8) & 0xFF);
                GLubyte bB = (GLubyte)((bc >> 16) & 0xFF);
                GLubyte bFinal = (GLubyte)((unsigned)bA * (unsigned)callerA / 255u);
                bR = (GLubyte)((float)bR * callerR + 0.5f);
                bG = (GLubyte)((float)bG * callerG + 0.5f);
                bB = (GLubyte)((float)bB * callerB + 0.5f);
                // glRasterPos deja el ORIGEN DEL GLIFO en la baseline, así que el
                // texto ocupa [rasterY - descent, rasterY + ascent]. Antes usaba
                // `rasterY-2 .. rasterY+cy-2`, que corría la caja hacia arriba y
                // dejaba aire abajo. descent = cy - ascent.
                int descent = (int)bsz.cy - s_ascent;
                if (descent < 0) descent = 0;
                // bsz está en píxeles de framebuffer (así mide GDI y así
                // blitea glBitmap); el quad se emite en unidades del ortho y la
                // GPU lo estira.  Sin esta división el fondo salía más ancho
                // que las letras exactamente por la relación viewport/ortho.
                float wOrtho    = (float)bsz.cx  / sx;
                float ascOrtho  = (float)s_ascent / sy;
                float descOrtho = (float)descent  / sy;
                float x0 = runX - 1.0f;
                float x1 = runX + wOrtho + 1.0f;
                float y0 = (float)rasterY - descOrtho;
                float y1 = (float)rasterY + ascOrtho;
                glColor4ub(bR, bG, bB, bFinal);
                glBegin(GL_QUADS);
                    glVertex2f(x0, y0);
                    glVertex2f(x1, y0);
                    glVertex2f(x1, y1);
                    glVertex2f(x0, y1);
                glEnd();
            }

            GLubyte R = (GLubyte)( fg        & 0xFF);
            GLubyte G = (GLubyte)((fg >>  8) & 0xFF);
            GLubyte B = (GLubyte)((fg >> 16) & 0xFF);
            GLubyte A = (GLubyte)((fg >> 24) & 0xFF);
            if (A == 0) A = 0xFF;
            GLubyte finalA = (GLubyte)((unsigned)A * (unsigned)callerA / 255u);

            // Modulación por el color del caller (ver la nota de arriba).
            R = (GLubyte)((float)R * callerR + 0.5f);
            G = (GLubyte)((float)G * callerG + 0.5f);
            B = (GLubyte)((float)B * callerB + 0.5f);

            glColor4ub(R, G, B, finalA);
            glRasterPos2f(runX, (float)rasterY);
            glCallLists((GLsizei)(endChar - startChar), GL_UNSIGNED_BYTE,
                        drawText + startChar);
        }
    }
    glPopAttrib();
}
