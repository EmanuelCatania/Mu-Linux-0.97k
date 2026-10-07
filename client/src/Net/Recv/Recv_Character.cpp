// Recv_Character.cpp — paquetes del server: el personaje propio: lista, creación, entrada al mapa, stats, vida y maná.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "UI/EventTimer.h"
#include "Net/Recv/NetRecv.h"

void Recv_NewCharacterInfo(const BYTE* Msg)
{
    // CharacterAttribute global ya declarada en globals.h como DAT_07cf1ff4.
    // Es un void*. Castear a BYTE* para offsets.
    BYTE* CA = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!CA) return;

    // Skip header (4 bytes: C1, len, F3, E0). Body starts at +4.
    const BYTE* p = Msg + 4;
    DWORD Level          = *(const DWORD*)(p + 0);
    DWORD LevelUpPoint   = *(const DWORD*)(p + 4);
    DWORD Experience     = *(const DWORD*)(p + 8);
    DWORD NextExperience = *(const DWORD*)(p + 12);
    DWORD Strength       = *(const DWORD*)(p + 16);
    DWORD Dexterity      = *(const DWORD*)(p + 20);
    DWORD Vitality       = *(const DWORD*)(p + 24);
    DWORD Energy         = *(const DWORD*)(p + 28);
    DWORD Life           = *(const DWORD*)(p + 32);
    DWORD MaxLife        = *(const DWORD*)(p + 36);
    DWORD Mana           = *(const DWORD*)(p + 40);
    DWORD MaxMana        = *(const DWORD*)(p + 44);
    (void)Experience; (void)NextExperience;

    *(WORD*)(CA + 0x0E) = ClampToWord(Level);
    *(WORD*)(CA + 0x54) = ClampToWord(LevelUpPoint);
    *(WORD*)(CA + 0x14) = ClampToWord(Strength);
    *(WORD*)(CA + 0x16) = ClampToWord(Dexterity);
    *(WORD*)(CA + 0x18) = ClampToWord(Vitality);
    *(WORD*)(CA + 0x1A) = ClampToWord(Energy);
    // Life/Mana también van acá (algunas versions no mandan F3/E1):
    *(WORD*)(CA + 0x1C) = ClampToWord(Life);
    *(WORD*)(CA + 0x20) = ClampToWord(MaxLife);
    *(WORD*)(CA + 0x1E) = ClampToWord(Mana);
    *(WORD*)(CA + 0x22) = ClampToWord(MaxMana);
    // Experience offsets (per HUD_Pass2.cpp:521-522: curExp=+16, maxExp=+52).
    *(DWORD*)(CA + 0x10) = Experience;
    *(DWORD*)(CA + 0x34) = NextExperience;
}

BYTE s_PendingSkillKey[10];

bool s_HasPendingSkillKey = false;

void ApplySkillKeyMap(void)
{
    if (!s_HasPendingSkillKey || !CharacterAttribute) return;
    const int hero = (int)DAT_005616ac;                  // SelectedHero
    if (hero < 0 || hero > 4) return;

    BYTE* attr   = (BYTE*)(uintptr_t)CharacterAttribute;
    BYTE* keyMap = attr + 215 + (hero << 6);
    memset(keyMap, 0xFF, 0x40);
    int applied = 0;
    for (int i = 0; i < 10; ++i) {
        const BYTE sk = s_PendingSkillKey[i];
        if (sk == 255) continue;
        for (int j = 0; j < 64; ++j) {
            if (sk == attr[j + 87]) { keyMap[j] = (BYTE)i; ++applied; break; }
        }
    }
    NetLog("NET:    skill-keys aplicadas: %d de 10 (hero=%d)", applied, hero);
}

// ── F3/E1 PMSG_NEW_CHARACTER_CALC_RECV ───────────────────────────────────────
// Del DLL de inyeccion (Protocol.cpp GCNewCharacterCalcRecv).  Trae los stats
// ya calculados por el server (MuEmu, con resets y sus propias formulas).
//
// Layout real, PMSG_NEW_CHARACTER_CALC_SEND (Protocol.h:566 del server):
// header(4) + 17 DWORDs.
//    p+0  CurHP          p+4  MaxHP          p+8  CurMP         p+12 MaxMP
//    p+16 CurBP          p+20 MaxBP          p+24 PhysiSpeed    p+28 MagicSpeed
//    p+32 PhysiDmgMin    p+36 PhysiDmgMax    p+40 MagicDmgMin   p+44 MagicDmgMax
//    p+48 MagicDmgRate   p+52 AttackSuccessRate                 p+56 DamageMultiplier
//    p+60 Defense        p+64 DefenseSuccessRate
void Recv_NewCharacterCalc(const BYTE* Msg, int Size)
{
    BYTE* CA = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!CA) return;
    if (Size < 4 + 17 * 4) return;   // paquete corto: no leer fuera

    const BYTE* p = Msg + 4;
    DWORD ViewCurHP            = *(const DWORD*)(p + 0);
    DWORD ViewMaxHP            = *(const DWORD*)(p + 4);
    DWORD ViewCurMP            = *(const DWORD*)(p + 8);
    DWORD ViewMaxMP            = *(const DWORD*)(p + 12);
    DWORD ViewCurBP            = *(const DWORD*)(p + 16);
    DWORD ViewMaxBP            = *(const DWORD*)(p + 20);
    DWORD ViewPhysiSpeed       = *(const DWORD*)(p + 24);
    DWORD ViewMagicSpeed       = *(const DWORD*)(p + 28);
    DWORD ViewMagicDamageMin   = *(const DWORD*)(p + 40);
    DWORD ViewMagicDamageMax   = *(const DWORD*)(p + 44);
    DWORD ViewAttackSuccessRate= *(const DWORD*)(p + 52);
    DWORD ViewDefense          = *(const DWORD*)(p + 60);
    DWORD ViewDefenseSuccess   = *(const DWORD*)(p + 64);

    *(WORD*)(CA + 0x1C) = ClampToWord(ViewCurHP);
    *(WORD*)(CA + 0x20) = ClampToWord(ViewMaxHP);
    *(WORD*)(CA + 0x1E) = ClampToWord(ViewCurMP);
    *(WORD*)(CA + 0x22) = ClampToWord(ViewMaxMP);
    *(WORD*)(CA + 0x24) = ClampToWord(ViewCurBP);
    *(WORD*)(CA + 0x26) = ClampToWord(ViewMaxBP);
    *(WORD*)(CA + 0x38) = ClampToWord(ViewPhysiSpeed);
    *(WORD*)(CA + 0x44) = ClampToWord(ViewMagicSpeed);
    *(WORD*)(CA + 0x3A) = ClampToWord(ViewAttackSuccessRate);
    *(WORD*)(CA + 0x4E) = ClampToWord(ViewDefense);
    *(WORD*)(CA + 0x4C) = ClampToWord(ViewDefenseSuccess);
    // Los dos campos que el DLL escribe ademas (GCNewCharacterCalcRecv,
    // Protocol.cpp).  El dano FISICO no viaja por aca -- el DLL tampoco lo
    // escribe, lo sigue calculando el cliente.
    *(WORD*)(CA + 0x46) = ClampToWord(ViewMagicDamageMin);
    *(WORD*)(CA + 0x48) = ClampToWord(ViewMagicDamageMax);
}

// ── Globals del state machine de login (mapping IDA → nuestro codebase) ────
// IDA                            | nuestro
// -------------------------------|----------------------------
// CurrentProtocolState           | DAT_05826cb0 (server response code)
// dword_83A7C14                  | DAT_083a7c14 (login sub-state)
// HeroKey                        | g_HeroKey (nuevo)

unsigned short g_HeroKey = 0;

// Compatibilidad de ciclo de vida: MuEmu puede entregar 0x5C mientras F3/03
// todavía está dentro de OpenWorld. El binario procesa el viewport sobre un
// héroe ya reconstruido; en nuestro pump reentrante la asociación podía caer
// en el slot anterior y el reinicio del pool la borraba. Se conserva sólo la
// fila ya creada por FUN_00434DC0 y se aplica al nuevo héroe al terminarlo.
bool s_IsRebuildingHero = false;

bool s_HasPendingHeroGuildMark = false;

short s_PendingHeroGuildMarkRow = -1;

// ---------------------------------------------------------------------------
// F3/00 — ReceiveCharacterList  (@ 0x00424240)
// Lista de personajes del account después de login OK.
//
// PACKET LAYOUT (post-C3 decrypt; Msg = full C1-framed buffer):
//   Msg[0] = 0xC1
//   Msg[1] = plainLen (size byte)
//   Msg[2] = 0xF3 (opcode)
//   Msg[3] = 0x00 (sub)
//   Msg[4] = count (número de chars en la cuenta, 0..5)
//   Msg[5+k*26 .. Msg[5+k*26+25]] = record k
//
// PER-RECORD (stride 26 bytes):
//   +0     slot (BYTE)        — índice de slot 0..4
//   +1..10 Name[10]           — nombre ASCII (puede no estar null-terminated)
//   +11    reserved
//   +12,13 Level (WORD LE)
//   +14    CtlCode (BYTE)     — bit 4 (0x10) = AccountBlockItem
//   +15..25 CharSet[11]       — equipment data; CharSet[0] codifica clase
//
// Acciones:
//   1. Reset entity slot active flags (DAT_07abf5d0+0x2D2 = 0)
//   2. Para cada record: CreateHero(slot, class, 0, x, y, rotate)
//      donde x=slot*100, y=slot*50-50, rotate=(slot-1)*15
//   3. Escribe Level (entity+0x1BE), CtlCode (entity+0x1C0), Name (entity+0x1C1..)
//   4. (TODO) ChangeCharacterExt(slot, &CharSet[1]) — visualiza equipment
//   5. DAT_05826cb0 = 51 (entra a estado char-select activo)
// ---------------------------------------------------------------------------
void Recv_CharList(const BYTE* Msg, int Size)
{
    const int CHAR_STRIDE  = 0x394;
    const int CHAR_SLOT_AT = 0x2D2;       // entity+0x2D2 = "selected" flag
    const int MAX_PREVIEW  = 5;            // slots renderizados en char-select

    // 1) Limpiar flags de los 5 slots previos
    for (int i = 0; i < MAX_PREVIEW; ++i) {
        BYTE* slot = (BYTE*)(uintptr_t)(DAT_07abf5d0 + i * CHAR_STRIDE);
        // El campo +0 (active) lo (re)setea CreateHero. Aquí limpiamos el flag
        // de "char seleccionado" que el F3/01 ChangeCharacter marca con 1.
        slot[CHAR_SLOT_AT] = 0;
    }

    BYTE count = Msg[4];
    NetLog("NET:    F3/00 char count=%d", count);

    if (count == 0) {
        DAT_05826cb0 = 51;
        return;
    }

    if (count > MAX_PREVIEW) count = MAX_PREVIEW;

    const BYTE* rec = Msg + 5;
    for (int i = 0; i < (int)count; ++i, rec += 26) {
        BYTE  slot     = rec[0];
        WORD  level    = (WORD)(rec[12] | (rec[13] << 8));
        BYTE  ctlCode  = rec[14];
        BYTE  csByte0  = rec[15];

        // class = ((CharSet[0] >> 4) | (CharSet[0] & 0x10)) >> 1
        int   klass    = ((csByte0 >> 4) | (csByte0 & 0x10)) >> 1;

        // Posición de preview por slot
        float x        = (float)slot * 100.0f;
        float y        = (float)slot * 50.0f - 50.0f;
        float rotate   = (float)((int)slot - 1) * 15.0f;

        unsigned char* c = CreateHero((int)slot, klass, 0, x, y, rotate);
        if (!c) {
            NetLog("NET:    F3/00 slot=%d CreateHero FAILED", slot);
            continue;
        }

        // entity+0x1BE = Level (WORD)
        *(WORD*)(c + 0x1BE) = level;
        // entity+0x1C0 = CtlCode (BYTE)
        c[0x1C0] = ctlCode;
        // entity+0x1C1..0x1CA = Name (10 bytes), entity+0x1CB = NUL
        for (int k = 0; k < 10; ++k) c[0x1C1 + k] = rec[1 + k];
        c[0x1CB] = 0;
        // entity+444 (0x1BC) = clase (BYTE) — lo necesita Recv_JoinMapServer para
        // propagarlo a la entidad del héroe al entrar al mundo.
        c[444] = (BYTE)klass;

        char name[11]; for (int k = 0; k < 10; ++k) name[k] = (char)rec[1+k]; name[10] = 0;
        NetLog("NET:    F3/00 slot=%d class=%d level=%d name='%s'",
               slot, klass, level, name);

        // Equipment visuals: CharSet[1..10] = rec[16..25] (10 bytes packed).
        // Pipeline ya completo (LevelConvert + DeleteBug + CreateBug + ChangeCharacterExt).
        ChangeCharacterExt((int)slot, (BYTE*)&rec[16]);

        // CreateHero llamó SetPlayerStop con las alas todavía en -1, así que
        // dejó action=1 (idle pegado al piso). Re-llamamos ahora que las alas
        // están equipadas para que action pase a 9 (float-idle) y los chars
        // con alas aparezcan flotando como en el cliente original.
        SetPlayerStop((int)(uintptr_t)c);
    }

    // CurrentProtocolState = 51 → Scene_CharSelect lo usa como gate de render
    DAT_05826cb0 = 51;
}

// ---------------------------------------------------------------------------
// F3/01 — ReceiveCreateCharacter  (@ 0x00424390)
// Server response al "create character" lanzado desde Scene_CharSelect.
// Layout (C1):
//   [0]=C1 [1]=len [2]=F3 [3]=01 [4]=result
//     result==1 → success. payload at [5..]:
//       [5..14]  Name (10B)
//       [15]     SlotIndex
//       [16-17]  PositionX/Y
//       [18]     Class/skin byte
//     result==2 → blocked (CurrentProtocolState=55)
//     other     → failed  (CurrentProtocolState=54)
// Si tiene éxito: CreateHero en el slot, setea los flags de la entidad, copia el nombre, CurrentProtocolState=53.
// Ported verbatim from IDA reference 00424390_ReceiveCreateCharacter.c.
// ---------------------------------------------------------------------------
void Recv_CreateChar(const BYTE* Msg)
{
    BYTE result = Msg[4];
    if (result == 1) {
        BYTE slot = Msg[15];
        // Posición preview: y = slot*50 - 50, x = slot*100
        float x = (float)slot * 100.0f;
        float y = (float)slot * 50.0f - 50.0f;
        // dword_7ABF20C: low byte = class, high byte = skin
        int klass = (int)(DAT_07abf20c & 0xFF);
        int skin  = (int)((DAT_07abf20c >> 8) & 0xFF);
        unsigned char* c = CreateHero((int)slot, klass, skin, x, y, 0.0f);
        DAT_05826cb0 = 53;  // CurrentProtocolState — char created OK
        if (!c) {
            NetLog("NET:    F3/01 slot=%d CreateHero FAILED", slot);
            return;
        }
        // Entity field writes (mirrors IDA layout):
        //   +446 (WORD) = 1               — selected/visible flag
        //   +449..+458  = Name(10B) + 0   — copy from Msg+5
        const BYTE* src = Msg + 5;
        *(WORD*)(c + 446) = 1;
        // 10 bytes name + NUL at +459
        for (int k = 0; k < 10; ++k) c[449 + k] = src[k];
        c[459] = 0;
        NetLog("NET:    F3/01 slot=%d class=%d created", slot, klass);
    } else if (result == 2) {
        DAT_05826cb0 = 55;  // blocked / name-taken
        NetLog("NET:    F3/01 create blocked (result=2)");
    } else {
        DAT_05826cb0 = 54;  // generic failure
        NetLog("NET:    F3/01 create failed (result=%d)", result);
    }
}

// ---------------------------------------------------------------------------
// F3/02 — ReceiveDeleteCharacter (inline en ProtocolCore @ 0x004389A0:1581)
// Layout (C1): [0]=C1 [1]=len [2]=F3 [3]=02 [4]=result
//   result==1 → success → CurrentProtocolState = 57 (0x39)
//   else      → failed  → DAT_05826d20 = result; CurrentProtocolState = 58 (0x3A)
// La eliminación visual del slot ocurre en respuesta al state 57 vía Scene_CharSelect.
// ---------------------------------------------------------------------------
void Recv_DeleteChar(const BYTE* Msg)
{
    BYTE result = Msg[4];
    if (result == 1) {
        DAT_05826cb0 = 57;
        NetLog("NET:    F3/02 delete OK");
    } else {
        DAT_05826d20 = result;
        DAT_05826cb0 = 58;
        NetLog("NET:    F3/02 delete failed reason=%d", result);
    }
}

// ---------------------------------------------------------------------------
// F3/03 — ReceiveJoinMapServer  (@ 0x00425840, IDA: ReceiveJoinMapServer/3204B)
// El servidor confirma la entrada al mundo. Layout del packet (rama bEncrypted):
//   Msg[ 4] = PosX (grid)
//   Msg[ 5] = PosY (grid)
//   Msg[ 6] = World (map number, 0..16)
//   Msg[ 7] = Direction (0..7) → Rotation = (Direction-1)*45°
//   Msg[ 8.. 9] = stat dword (strength offset)
//   Msg[16..17] = HP / first stat word
//   Msg[44]     = anti-skill bonus
//   Msg[45]     = magic bonus
//   ... (resto: stats, monedas, atributos)
//
// Pipeline IDA (líneas 124-386):
//   1. Hash-table re-key sobre CharacterMachine / CharacterAttribute (anti-tamper, no-op).
//   2. Volcar palabras stat de Msg+8..+50 a CharacterAttribute+0x10..+0x30.
//   3. World = Msg[6]; OpenWorld(World) — carga mapa, terrain, tiles.
//   4. HeroIndex = rand() % 400; v50 = CharactersClient + 916*HeroIndex.
//   5. CreateCharacterPointer(v50, 390, PosX, PosY, (Direction-1)*45);
//   6. Hero = v50; Hero+0x1DC = HeroKey; Hero+444 = char.class; Hero+445 = 0;
//      Hero+746 = Msg[44]; Hero+448 = Msg[45]; Hero+132 = 1; SetCharacterClass(Hero).
//   7. Copia 11 bytes de CharacterAttribute → Hero+449.
//   8. Reset 12 quest slots (Hero+540..+540+12*68).
//   9. Hero+459 = 0; CreateEffect(1265 = teleport-in, &Hero.pos, &Hero.angle, &Hero+232,
//      0, Hero, -1, 0, 0).
//  10. CurrentProtocolState = 61; LockInputStatus = 0; CheckIME_Status(1, 0).
//  11. Si World < 11 || > 16: StopBuffer(110, 1) (silenciar dungeon BGM).
//
// La rama bEncrypted=false es un re-handshake GameGuard (build/firma de packet
// 5 bytes XOR); sin GG real basta con el state advance.
// ---------------------------------------------------------------------------
void Recv_JoinMapServer(const BYTE* Msg, int bEncrypted)
{
    // El F3/03 que envía el server MuEmu (Protocol.cpp GDCharacterInfoSend →
    // DataServer → DGCharacterInfoRecv → cliente) llega con bEncrypted=false: el
    // dispatcher nunca lo setea. El IDA original 0.97K diferenciaba ambos paths
    // para soportar re-handshakes de GameGuard; nuestro server MuEmu no usa GG,
    // así que ejecutamos siempre el world-load path (no hacer early-return).
    (void)bEncrypted;

    // Debe activarse antes de OpenWorld: ése es el tramo que permite que el
    // socket procese 0x5C de forma reentrante mientras el pool aún contiene
    // la entidad anterior del héroe.
    s_IsRebuildingHero = true;
    s_HasPendingHeroGuildMark = false;

    const BYTE PosX      = Msg[4];
    const BYTE PosY      = Msg[5];
    const BYTE world     = Msg[6];
    const BYTE direction = Msg[7];

    // (1-2) Stat parse — populate CharacterAttribute from packet bytes 8..50.
    // Sin esto el panel C aparece con todo en 0 (stat points, HP cur, mana,
    // damage, etc.).  Layout per IDA ReceiveJoinMapServer (lines 166-200):
    //
    //   ReceiveBuffer offset → CharacterAttribute offset
    //   ----------------------------------------------------
    //    +8  (dword)  → CA+16  (current experience)
    //   +12  (dword)  → CA+52  (next-level experience)
    //   +16  (word)   → CA+84  (zone level)
    //   +18  (word)   → CA+20  (Strength)
    //   +20  (word)   → CA+22  (Agility / Dexterity)
    //   +22  (word)   → CA+24  (Vitality)
    //   +24  (word)   → CA+26  (Energy)
    //   +26  (word)   → CA+28  (HP current)
    //   +28  (word)   → CA+32  (HP max)
    //   +30  (word)   → CA+30  (MP current)
    //   +32  (word)   → CA+34  (MP max)
    //   +34  (word)   → CA+36
    //   +36  (word)   → CA+38
    //   +46  (word)   → CA+46  (LevelUpPoint = available stat points)
    //   +48  (word)   → CA+48  (max LevelUpPoint)
    //
    //   CA+40, CA+42, CA+44 cleared to 0.
    if (CharacterAttribute) {
        BYTE* CA = (BYTE*)CharacterAttribute;
        const BYTE* RB = Msg;

        // Experience
        *(DWORD*)(CA + 16) = *(const DWORD*)(RB +  8);   // current
        *(DWORD*)(CA + 52) = *(const DWORD*)(RB + 12);   // next

        // Zone level + stats
        *(WORD*)(CA + 84) = *(const WORD*)(RB + 16);
        *(WORD*)(CA + 20) = *(const WORD*)(RB + 18);     // Strength
        *(WORD*)(CA + 22) = *(const WORD*)(RB + 20);     // Agility
        *(WORD*)(CA + 24) = *(const WORD*)(RB + 22);     // Vitality
        *(WORD*)(CA + 26) = *(const WORD*)(RB + 24);     // Energy

        // HP / MP
        *(WORD*)(CA + 28) = *(const WORD*)(RB + 26);     // HP cur
        *(WORD*)(CA + 32) = *(const WORD*)(RB + 28);     // HP max
        *(WORD*)(CA + 30) = *(const WORD*)(RB + 30);     // MP cur
        *(WORD*)(CA + 34) = *(const WORD*)(RB + 32);     // MP max

        // Misc stats
        *(WORD*)(CA + 36) = *(const WORD*)(RB + 34);
        *(WORD*)(CA + 38) = *(const WORD*)(RB + 36);

        // Reset PvP/etc fields
        *(BYTE*)(CA + 40) = 0;
        *(WORD*)(CA + 42) = 0;
        *(WORD*)(CA + 44) = 0;

        // ** LevelUpPoint (stat points to allocate) **
        *(WORD*)(CA + 46) = *(const WORD*)(RB + 46);     // available
        *(WORD*)(CA + 48) = *(const WORD*)(RB + 48);     // max

        // PMSG_CHARACTER_INFO_SEND (F3/03) trae Money (DWORD) en offset 40 — la
        // struct NO es pack(1): tras MaxBP (WORD@36) hay 2 bytes de padding porque
        // Money (DWORD) se alinea a 4 → offset 40. El zen se muestra desde
        // CharacterMachine+1352 (= DAT_07cf1ffc+1352).
        if (DAT_07cf1ffc != 0) {
            DWORD money = *(const DWORD*)(RB + 40);
            *(DWORD*)((BYTE*)(uintptr_t)DAT_07cf1ffc + 1352) = money;
            NetLog("NET:    F3/03 Money=%u -> CharacterMachine+1352", money);
        }

        // PMSG_CHARACTER_INFO_SEND trae PKLevel (BYTE) justo después de Money →
        // offset 44. El render aplica el tinte rojo (1.0,0.1,0.1) cuando
        // entity+0x2EA >= 6 (Entity_UpdateRender.cpp). Entity_Spawn inicializa ese
        // campo en 3 para los mobs, pero el HÉROE no pasa por ese path, así que se
        // guarda el PKLevel real que manda el server.
        if (DAT_07abf5d8) {
            BYTE pk = RB[44];
            if (pk > 6) pk = 0;                   // valor fuera de rango → normal
            *(BYTE*)((BYTE*)(uintptr_t)DAT_07abf5d8 + 0x2ea) = pk;
            NetLog("NET:    F3/03 PKLevel=%u -> hero+0x2EA", (unsigned)pk);
        }

        NetLog("NET:    F3/03 stats: Str=%u Agi=%u Vit=%u Ene=%u HP=%u/%u MP=%u/%u "
               "LevelUpPts=%u(CA+84) ResetPts=%u/%u(CA+46/48)",
               *(WORD*)(CA + 20), *(WORD*)(CA + 22),
               *(WORD*)(CA + 24), *(WORD*)(CA + 26),
               *(WORD*)(CA + 28), *(WORD*)(CA + 32),
               *(WORD*)(CA + 30), *(WORD*)(CA + 34),
               *(WORD*)(CA + 84),
               *(WORD*)(CA + 46), *(WORD*)(CA + 48));

        // Volcado hex de la región del paquete con los stats, para verificación
        char hexBuf[256];
        int hexN = 0;
        hexN = wsprintfA(hexBuf, "NET:    F3/03 RB[8..50] hex:");
        for (int i = 8; i <= 50 && hexN < (int)sizeof(hexBuf) - 4; ++i) {
            hexN += wsprintfA(hexBuf + hexN, " %02X", RB[i]);
        }
        NetLog("%s", hexBuf);
    }

    // (3) World setup: mapa, terreno, tiles.
    World = world;

    // WIPE del entity pool de slots stale del CharSelect ANTES de OpenWorld +
    // hero spawn: si no, los slots de chars del CharSelect quedan activos con sus
    // nombres en +0x1C1 y aparecen como "NPCs" fantasma cuando el hover detect
    // los recoge.
    //
    // Wipea TODOS los slots (FULL: slot[0]=0 + 0x84=0 + 0x160=0 + 0x1C1=0) — el
    // nuevo hero se crea via CreateCharacterPointer a unas líneas más abajo
    // (paso 4-5) en un slot random. Los mobs/players del world via 0x12/0x13
    // viewport packets llegan DESPUÉS del OpenWorld y populan el pool limpio.
    if (DAT_07abf5d0) {
        // Full memset del pool para garantizar todos los bytes limpios.
        // 400 slots × 0x394 = ~366 KB. CreateCharacterPointer + viewport spawn
        // packets repopulan los campos relevantes después.
        memset((void*)(uintptr_t)DAT_07abf5d0, 0, 400 * 0x394);
        // Reset hero pointer too — el siguiente CreateCharacterPointer setea uno nuevo.
        DAT_07abf5d8 = nullptr;
    }

    // OpenWorld bloquea ~2 segundos cargando BMDs. Los dos pumps de mensajes
    // alrededor de la carga quedan desactivados (`false &&`): el drenaje del socket
    // durante la carga lo hace AccessModel (ver `g_WorldLoading` abajo).
    {
        MSG msg;
        // Se mantiene estructuralmente sólo para diagnóstico. 00425840 no despacha
        // los paquetes de viewport encolados antes de crear al héroe, más abajo.
        for (int i = 0; false && i < 32 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // OpenWorld tarda ~2 s cargando BMDs y, para que el server no cierre por
    // backpressure, AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese
    // pump entrega WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los
    // handlers correrian RE-ENTRANTES en mitad de la carga (p.ej. un
    // `0x13 ViewportMonster` creando monstruos cuyo modelo todavia no esta abierto).
    //
    // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
    // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
    // cola y se procesan al terminar la carga.
    ++g_WorldLoading;
    OpenWorld();              // OpenWorld(World) — BMD load (~2s)
    --g_WorldLoading;

    // Pump messages POST-load para drenar lo que llegó durante el bloqueo.
    {
        MSG msg;
        for (int i = 0; false && i < 64 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // (4-5) Random hero slot + spawn at packet position.
    HeroIndex = (DWORD)(_rand() % 400);
    unsigned int heroIndex = HeroIndex;
    unsigned char* heroPtr = (unsigned char*)(uintptr_t)DAT_07abf5d0 + (heroIndex * 0x394);
    float Rotation = ((float)direction - 1.0f) * 45.0f;
    CreateCharacterPointer(heroPtr, 390, PosX, PosY, Rotation);

    // CreateCharacterPointer setea cached_wp (+0x388/0x38c) pero NO
    // target_grid (+0x306/0x307). El per-frame walker Entity_AdvancePath en
    // Player_InputTick lee target_grid: con 0,0 el hero caminaría solo a la
    // esquina del mapa.
    // Forzar target == position para que el walker quede idle hasta el primer click.
    heroPtr[0x306] = PosX;
    heroPtr[0x307] = PosY;
    heroPtr[0x305] = 0;             // move-pending OFF
    heroPtr[0x354] = 0;             // path_current_wp = 0
    heroPtr[0x355] = 0;             // path_substep = 0
    heroPtr[0x356] = 0;             // path_wp_count = 0
    // Init de anim_state post-wipe: el wipe deja 0x105=0 y anim 0 puede no ser
    // "idle" para algunos models. anim_state=1 es el idle1 standard.
    heroPtr[0x105] = 1;             // anim_state = idle
    heroPtr[0x106] = 1;             // anim_state_prev
    *(float*)(heroPtr + 0x108) = 0.0f;  // anim_frame

    // (6) Hero binding + class/skill bonus bytes from packet.
    *(unsigned short*)(heroPtr + 0x1DC) = g_HeroKey;
    BYTE* charAttr = (BYTE*)CharacterAttribute;
    heroPtr[444] = charAttr[11];

    // El ReceiveJoinMapServer de IDA NO espeja el cuerpo/equipo desde
    // las entidades del char-select. Asocia al Hero, copia clase/flags, y después reconstruye
    // desde CharacterMachine vía SetCharacterClass(). Acá dejamos sólo la
    // siembra de luz, porque CreateCharacterPointer deja Light en 0.
    *(unsigned int*)(heroPtr + 0xe8) = 0x3e99999a; // 0.3f
    *(unsigned int*)(heroPtr + 0xec) = 0x3e99999a;
    *(unsigned int*)(heroPtr + 0xf0) = 0x3e99999a;
    heroPtr[445] = 0;
    heroPtr[746] = Msg[44];      // skill bonus / curse-of-equipment slot
    heroPtr[448] = Msg[45];      // magic bonus / second-pwd flag
    heroPtr[132] = 1;            // alive flag
    DAT_07abf5d8 = (char*)heroPtr;   // bind global Hero pointer

    // Si C1:5C llegó durante OpenWorld, la tabla de marks ya contiene la
    // marca real; falta únicamente restaurar su fila sobre el nuevo Character.
    if (s_HasPendingHeroGuildMark) {
        *(short*)(heroPtr + 474) = s_PendingHeroGuildMarkRow;
        s_HasPendingHeroGuildMark = false;
        s_PendingHeroGuildMarkRow = -1;
        GuildWar_UpdateEntityRelation((int)(uintptr_t)heroPtr, 0, 0, 0);
    }
    s_IsRebuildingHero = false;
    HeroEquipWatchdog((int)(uintptr_t)heroPtr);
    *(DWORD*)(heroPtr + 449) = *(DWORD*)(charAttr + 0);
    *(DWORD*)(heroPtr + 453) = *(DWORD*)(charAttr + 4);
    *(WORD*)(heroPtr + 457)  = *(WORD*)(charAttr + 8);
    NetLog("NET:    F3/03 hero spawn: heroSlot=%d heroClass=%d",
           (int)heroIndex, (int)heroPtr[444]);

    // Dejamos intacto el equipo de CharacterMachine. En nuestro cliente 97k ésa es la
    // fuente viva tanto para reconstruir el cuerpo en el mundo como para sincronizar el panel/HUD.
    // Limpiarlo acá estaba arrancando casco/armadura/pantalones/guantes/botas justo
    // después de que SetCharacterClass reconstruyera al héroe.

    // Drop stale post-select / PvP outline bit on world enter.
    *(DWORD*)(heroPtr + 0x78) &= ~2u;

    // (9) entity+459 = 0 (status flag) + teleport-in effect 1265.
    heroPtr[459] = 0;
    // CreateEffect(1265, Hero.Position, Hero.Angle, Hero+232, 0, Hero, -1, 0, 0)
    // Effect_Spawn signature: see src/Render/Particle_Spawn.cpp / Particle_Spawn.
    // Wire-up TODO: nuestro Effect_Spawn aún no está parameterizado igual; la
    // entrada al mundo funciona sin el efecto visual.

    // (10) Avanzar state machine.
    DAT_05826cb0 = 61;           // CurrentProtocolState → enter-world fade
    // IDA ReceiveJoinMapServer L380: LockInputStatus = 0 (0x07E11D6F, el gate
    // del IME que WndProc pone en 1 al abrir el chat).  CheckIME_Status(1, 0)
    // queda pendiente: nuestro CheckIME_Status no reproduce todavia el
    // guardado/restaurado del estado de conversion.
    DAT_07e11d6f = 0;            // LockInputStatus

    // Envíos post-F3/03: el binario original 0.97k (FUN_00425840 Recv_JoinMapServer)
    // en el path bEncrypted=TRUE (= C3 packet, como manda MuEmu) NO envía
    // NINGÚN packet post-F3/03. Solo procesa los datos del char y carga el
    // mundo. El send de F3/12 ViewportEnable solo ocurre en el path
    // bEncrypted=FALSE (no aplica con MuEmu C3).
    //
    // No agregar acá 0x0E keepalive ni F3/12: el keepalive 1Hz en Game_MainLoop
    // ya cubre el liveness check post-F3/03, y en MuEmu el server-tick lleva
    // RegenOk (=2 normalmente seteado por F3/12) a 0 (OBJECT_PLAYING) sin
    // necesidad del F3/12 desde cliente.

    // (11) Stop dungeon BGM 110 si no estamos en mapa-evento (11..16).
    if (world < 11 || world > 16) {
        // FUN_00404c60(110, 1) — StopBuffer; existe en Sound.cpp
        // FUN_00404c60(110);  // (firma simplificada en nuestro port)
    }

    NetLog("NET:    F3/03 JoinMapServer world=%d pos=(%d,%d) dir=%d rot=%.1f heroIdx=%d",
           world, PosX, PosY, direction, Rotation, (int)heroIndex);
}

// ---------------------------------------------------------------------------
// F3/04 — ReceiveRevival  (@ 0x004264D0)
//
// Respawn tras la muerte.  El server MuEmu lo manda SOLO, por timer: al morir
// pone `DieRegen = 1` (ObjectManager.cpp:2759 / Monster.cpp), y el tick
// `ObjectSetStateCreate` (ObjectManager.cpp:80) lo pasa a 2 cuando venció
// `MaxRegenTime + 1000`; ahí `CObjectManager::Run` (ObjectManager.cpp:262-341)
// restaura Life/Mana/BP, reubica al pj (`CharacterGetRespawnLocation`) y llama
// `GCCharacterRegenSend`.  El cliente NO pide nada: sólo tiene que procesar
// este paquete.
//
// PMSG_CHARACTER_REGEN_SEND (Protocol.h:426, `setE` → frame C3), con el
// padding de MSVC:
//    +0..3   PSBMSG_HEAD (C3, size, F3, 04)
//    +4      BYTE  X
//    +5      BYTE  Y
//    +6      BYTE  Map
//    +7      BYTE  Dir
//    +8      WORD  Life
//    +10     WORD  Mana
//    +12     WORD  BP
//    +14,15  padding (Experience DWORD se alinea a 4)
//    +16     DWORD Experience
//    +20     DWORD Money
//    +24     DWORD ViewCurHP   (GAMESERVER_EXTRA)
//    +28     DWORD ViewCurMP
//    +32     DWORD ViewCurBP
// Los offsets +8/+10/+12/+16/+20 coinciden exactamente con los que lee IDA
// (`*((_WORD *)ReceiveBuffer + 4/5/6)`, `*((_DWORD *)ReceiveBuffer + 4/5)`).
//
// El ruido de hash-table (STRUCT_DECRYPT/ENCRYPT sobre CharacterMachine, ~60%
// del decompile) se omite por policy del proyecto.
// ---------------------------------------------------------------------------
void Recv_Revival(const BYTE* Msg, int Size)
{
    // El paquete sin los View* mide 24 bytes (header 4 + hasta Money en +20).
    if (Size < 24) {
        NetLog("NET:    F3/04 Revival paquete corto (Size=%d) - descartado", Size);
        return;
    }

    const BYTE PosX      = Msg[4];
    const BYTE PosY      = Msg[5];
    const BYTE map       = Msg[6];
    const BYTE direction = Msg[7];

    NetLog("NET:  → F3/04 Revival map=%d pos=(%d,%d) dir=%d HP=%u MP=%u",
           map, PosX, PosY, direction,
           (unsigned)*(const WORD*)(Msg + 8), (unsigned)*(const WORD*)(Msg + 10));

    // (1) Reset de input + estado de teleport, y baja del slot del héroe viejo.
    DAT_083a42c4 = 0;                                  // MouseLButton = 0
    Teleport = 0;                                  // Teleport = 0
    if (DAT_07abf5d8) *(BYTE*)DAT_07abf5d8 = 0;        // *(BYTE *)Hero = 0

    // (2) Stats desde el paquete.
    if (CharacterAttribute) {
        BYTE* CA = (BYTE*)CharacterAttribute;
        *(WORD*)(CA + 28) = *(const WORD*)(Msg +  8);  // Life  → HP actual
        *(WORD*)(CA + 30) = *(const WORD*)(Msg + 10);  // Mana  → MP actual
        *(WORD*)(CA + 36) = *(const WORD*)(Msg + 12);  // BP
        *(DWORD*)(CA + 16) = *(const DWORD*)(Msg + 16);// Experience
    }
    if (DAT_07cf1ffc) {
        // Money → CharacterMachine + 1352 (mismo campo que puebla el F3/03).
        *(DWORD*)((BYTE*)(uintptr_t)DAT_07cf1ffc + 1352) = *(const DWORD*)(Msg + 20);
    }

    if (!DAT_07abf5d0) return;

    // (3) Desactiva las 400 entidades del pool (el server reenvía el viewport).
    for (int i = 0; i < 400; ++i)
        *((BYTE*)(uintptr_t)DAT_07abf5d0 + i * 0x394) = 0;

    // (4) Se preservan dos campos del héroe viejo antes de re-crearlo:
    //     +474 (WORD) y +746 (BYTE, skill/curse slot que llega en el F3/03).
    WORD savedW474 = 0;
    BYTE saved746  = 0;
    if (DAT_07abf5d8) {
        savedW474 = *(const WORD*)((BYTE*)DAT_07abf5d8 + 474);
        saved746  = *((const BYTE*)DAT_07abf5d8 + 746);
    }

    // (5) Re-crea al héroe EN EL MISMO SLOT (HeroIndex no cambia).
    unsigned char* heroPtr =
        (unsigned char*)(uintptr_t)DAT_07abf5d0 + (HeroIndex * 0x394);
    const float Rotation = ((float)direction - 1.0f) * 45.0f;
    CreateCharacterPointer(heroPtr, 390, PosX, PosY, Rotation);

    *(WORD*)(heroPtr + 476) = g_HeroKey;

    // (6) Clase / flags.
    heroPtr[445] = 0;
    if (CharacterAttribute)
        heroPtr[444] = *((const BYTE*)CharacterAttribute + 11);
    heroPtr[746] = saved746;
    heroPtr[132] = 1;                                  // vivo
    *(WORD*)(heroPtr + 474) = savedW474;
    heroPtr[846] = 1;                                  // SafeZone (ver +0x34E)

    DAT_07abf5d8 = (char*)heroPtr;                     // re-bind del puntero Hero

    // (7) Reconstruye el cuerpo desde CharacterMachine y corta la animación
    //     de muerte; después el efecto visual de aparición.
    SetCharacterClass((int)(uintptr_t)heroPtr);             // SetCharacterClass
    SetPlayerStop((int)(uintptr_t)heroPtr);             // SetPlayerStop
    CreateEffect(1265, (float*)(heroPtr + 16), (float*)(heroPtr + 28),
                 (float*)(heroPtr + 232), nullptr, (float*)heroPtr,
                 (float*)-1, nullptr, 0);              // CreateEffect(1265)

    // (8) Si había una tienda abierta, se cierra el inventario asociado.
    if (ShopOpened) {
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
    }

    ClearItems();
    ClearCharacters(g_HeroKey);

    // (9) Cambio de mapa (respawn en otro mapa) + altura del terreno.
    if ((int)World == (int)map) {
        DAT_05826d24 = 0;   // SummonLife = 0
        return;
    }

    World = map;
    // OpenWorld tarda ~2 s cargando BMDs y, para que el server no cierre por
    // backpressure, AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese
    // pump entrega WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los
    // handlers correrian RE-ENTRANTES en mitad de la carga (p.ej. un
    // `0x13 ViewportMonster` creando monstruos cuyo modelo todavia no esta abierto).
    //
    // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
    // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
    // cola y se procesan al terminar la carga.
    ++g_WorldLoading;
    OpenWorld();                                    // OpenWorld(World)
    --g_WorldLoading;

    float z;
    if ((int)World == -1 ||
        *(const WORD*)(heroPtr + 696) != 819 ||        // sin Dinorant
        heroPtr[846] != 0) {
        z = RequestTerrainHeight(*(float*)(heroPtr + 16), *(float*)(heroPtr + 20));
    } else if (World == 8 || World == 10) {
        z = RequestTerrainHeight(*(float*)(heroPtr + 16), *(float*)(heroPtr + 20)) + 90.0f;
    } else {
        z = RequestTerrainHeight(*(float*)(heroPtr + 16), *(float*)(heroPtr + 20)) + 30.0f;
    }
    *(float*)(heroPtr + 24) = z;
    DAT_05826d24 = 0;       // SummonLife = 0
}

// ---------------------------------------------------------------------------
// F3/05 — ReceiveLevelUp  (@ 0x00431180)
//
// Sin este handler el panel de personaje se quedaba clavado en el nivel que
// traía el F3/03: seguía diciendo "Nivel: 1" con la experiencia por encima de
// la requerida, y los puntos de stat no aparecían hasta salir a char-select y
// volver (que re-pide el F3/03 y repuebla CharacterAttribute).
//
// PMSG_LEVEL_UP_SEND (Protocol.h:445, `header.set` → frame C1 plano), con el
// padding de MSVC:
//    +0..3   PSBMSG_HEAD (C1, size, F3, 05)
//    +4      WORD Level
//    +6      WORD LevelUpPoint
//    +8      WORD MaxLife
//    +10     WORD MaxMana
//    +12     WORD MaxBP
//    +14     WORD FruitAddPoint
//    +16     WORD MaxFruitAddPoint
//    +18     WORD FruitSubPoint
//    +20     WORD MaxFruitSubPoint
//    +22,23  padding
//    +24     DWORD ViewPoint / +28 ViewMaxHP / +32 ViewMaxMP / +36 ViewMaxBP
//    +40     DWORD ViewExperience / +44 DWORD ViewNextExperience
// Los offsets +4..+16 coinciden con los que lee IDA
// (`*((_WORD *)ReceiveBuffer + 2..8)`).
//
// Detalle fiel a IDA: la vida y el maná ACTUALES se igualan al máximo
// (CA+28 = CA+32, CA+30 = CA+34) — al subir de nivel el pj queda full.
// Y la experiencia del próximo nivel NO viene en el paquete: la recalcula el
// cliente con CharData_CalcNextLevelExp (0x47E350) a partir del nivel nuevo.
// Ese es el "Experiencia: 6280 / 63" de la captura del bug: CA+52 quedaba con
// el valor viejo porque nadie lo recalculaba.
//
// El ruido de hash-table (STRUCT_DECRYPT/ENCRYPT sobre CharacterMachine, ~75%
// del decompile) se omite por policy del proyecto.
// ---------------------------------------------------------------------------
// CharData_CalcNextLevelExp (0x0047E350) — definida en Scene/Scene_CharSelect_Nav.cpp
// y sin entrada en functions.h.
void Recv_LevelUp(const BYTE* Msg, int Size)
{
    // El paquete sin los View* mide 22 bytes (header 4 + hasta +20 inclusive).
    if (Size < 18 || !CharacterAttribute) {
        NetLog("NET:    F3/05 LevelUp paquete corto (Size=%d) - descartado", Size);
        return;
    }

    BYTE* CA = (BYTE*)CharacterAttribute;

    const WORD level        = *(const WORD*)(Msg +  4);
    const WORD levelUpPoint = *(const WORD*)(Msg +  6);
    const WORD maxLife      = *(const WORD*)(Msg +  8);
    const WORD maxMana      = *(const WORD*)(Msg + 10);
    const WORD maxBP        = *(const WORD*)(Msg + 12);

    *(WORD*)(CA + 14) = level;          // CharacterLevel
    *(WORD*)(CA + 84) = levelUpPoint;   // puntos de stat disponibles

    *(WORD*)(CA + 32) = maxLife;        // MaxLife
    *(WORD*)(CA + 34) = maxMana;        // MaxMana
    *(WORD*)(CA + 28) = maxLife;        // Life  = MaxLife  (full al subir)
    *(WORD*)(CA + 30) = maxMana;        // Mana  = MaxMana
    *(WORD*)(CA + 38) = maxBP;          // MaxBP

    if (Size >= 22) {
        *(WORD*)(CA + 46) = *(const WORD*)(Msg + 14);   // FruitAddPoint
        *(WORD*)(CA + 48) = *(const WORD*)(Msg + 16);   // MaxFruitAddPoint
    }

    // Experiencia del próximo nivel.
    //
    // DESVIACIÓN DELIBERADA respecto de IDA: el binario la deriva del nivel con
    // CharData_CalcNextLevelExp (0x47E350, `10 * lvl^2 * (lvl+9)`), pero MuEmu
    // usa su propia curva y manda el valor ya resuelto en ViewNextExperience.
    // Para nivel 5 la fórmula del 0.97k da 3500 y el server dice 7969 — o sea la
    // fórmula dejaría el panel en desacuerdo con el server y con lo que el propio
    // F3/03 muestra al volver de char-select.  Mismo criterio que el F3/06 de
    // stats y el 0xA3 de quest prize: si vienen los View*, mandan ellos.
    //
    // CONFIRMADO contra el DLL de inyección: `CProtocol::GCLevelUpRecv`
    // (Source/Client/Main/Protocol.cpp:816-817) hace exactamente esto —
    // `ViewExperience`/`ViewNextExperience` directo del paquete, sin tocar la
    // fórmula del 0.97k.  Idem su handler del F3/03 (L871).  O sea la referencia
    // toma la misma decisión, no hay una tercera variante que contemplar.
    if (Size >= 48) {
        *(DWORD*)(CA + 16) = *(const DWORD*)(Msg + 40);              // ViewExperience
        *(DWORD*)(CA + 52) = *(const DWORD*)(Msg + 44);              // ViewNextExperience
        *(WORD*)(CA + 84) = ClampToWord(*(const DWORD*)(Msg + 24));  // ViewPoint
        *(WORD*)(CA + 32) = ClampToWord(*(const DWORD*)(Msg + 28));  // ViewMaxHP
        *(WORD*)(CA + 34) = ClampToWord(*(const DWORD*)(Msg + 32));  // ViewMaxMP
        *(WORD*)(CA + 38) = ClampToWord(*(const DWORD*)(Msg + 36));  // ViewMaxBP
        *(WORD*)(CA + 28) = *(WORD*)(CA + 32);   // Life = MaxLife (full al subir)
        *(WORD*)(CA + 30) = *(WORD*)(CA + 34);   // Mana = MaxMana
    } else {
        CalculateNextExperince((int)(uintptr_t)CharacterMachine);
    }

    NetLog("NET:  -> F3/05 LevelUp lvl=%u pts=%u HP=%u MP=%u BP=%u next=%u",
           (unsigned)level, (unsigned)levelUpPoint, (unsigned)maxLife,
           (unsigned)maxMana, (unsigned)maxBP,
           (unsigned)*(DWORD*)(CA + 52));

    // Efecto visual de subida de nivel: 15 joints 1249 + el aura 1264.
    if (DAT_07abf5d8) {
        BYTE* hero = (BYTE*)DAT_07abf5d8;
        for (int i = 0; i < 15; ++i) {
            Joint_Create(1249, (float*)(hero + 16), (float*)(hero + 16),
                         (float*)(hero + 28), 0, (int)(uintptr_t)hero,
                         40.0f, 2, 0);
        }
        CreateEffect(1264, (float*)(hero + 16), (float*)(hero + 28),
                     (float*)(hero + 232), nullptr, (float*)hero,
                     (float*)-1, nullptr, 0);
    }
}

// 0xF3
void NetRecv_F3(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    switch (sub) {
        case 0x00:
            NetLog("NET:  → F3/00 CharList count=%d", Msg[4]);
            Recv_CharList(Msg, Size);
            break;
        case 0x01:
            NetLog("NET:  → F3/01 CreateChar result=%d", Msg[4]);
            Recv_CreateChar(Msg);
            break;
        case 0x02:
            NetLog("NET:  → F3/02 DeleteChar result=%d", Msg[4]);
            Recv_DeleteChar(Msg);
            break;
        case 0x03:
            NetLog("NET:  → F3/03 JoinMapServer world=%d", Msg[6]);
            Recv_JoinMapServer(Msg, (int)bEncrypted);
            break;
        case 0x04:
            // Respawn tras la muerte.  Ver Recv_Revival.
            Recv_Revival(Msg, Size);
            break;
        case 0x05:
            // Subida de nivel.  Ver Recv_LevelUp.
            Recv_LevelUp(Msg, Size);
            break;
        case 0x06: {
            // ── F3/06 PMSG_LEVEL_UP_POINT_SEND ───────────────────
            // El struct real del server (Protocol.h:467, GAMESERVER_EXTRA=1 en
            // stdafx.h:8) es, con el padding de MSVC:
            //    +0..3  PSBMSG_HEAD  (C1, size, F3, 06)
            //    +4     BYTE result  (= 16 + type, 0 = rechazado)
            //    +5     padding
            //    +6     WORD MaxLifeAndMana
            //    +8     WORD MaxBP
            //    +10    padding (alineación a 4)
            //    +12    DWORD ViewPoint      (LevelUpPoint restante)
            //    +16    DWORD ViewMaxHP
            //    +20    DWORD ViewMaxMP
            //    +24    DWORD ViewMaxBP
            //    +28    DWORD ViewStrength
            //    +32    DWORD ViewDexterity
            //    +36    DWORD ViewVitality
            //    +40    DWORD ViewEnergy
            //    sizeof = 44
            // Los campos View* son autoritativos (el server manda el
            // estado COMPLETO), así que no hace falta decrementar
            // LevelUpPoint ni incrementar el stat a mano.
            // No hay nada que desencriptar: el server usa
            // `header.set` (no `setE`), o sea C1 plano.
            if (Size < 10 || !CharacterAttribute) {
                NetLog("NET:  → F3/06 AddPoint short pkt (Size=%d)", Size);
                break;
            }
            BYTE result = Msg[4];
            if (result == 0) {
                NetLog("NET:  → F3/06 AddPoint REJECTED");
                break;
            }
            BYTE* CA = (BYTE*)CharacterAttribute;
            int slot = result & 0x0F;    // 0=Str 1=Agi 2=Vit 3=Ene

            if (Size >= 44) {
                // Server con GAMESERVER_EXTRA: usamos los DWORD View*.
                DWORD ViewPoint  = *(DWORD*)(Msg + 12);
                DWORD ViewMaxHP  = *(DWORD*)(Msg + 16);
                DWORD ViewMaxMP  = *(DWORD*)(Msg + 20);
                DWORD ViewStr    = *(DWORD*)(Msg + 28);
                DWORD ViewDex    = *(DWORD*)(Msg + 32);
                DWORD ViewVit    = *(DWORD*)(Msg + 36);
                DWORD ViewEne    = *(DWORD*)(Msg + 40);
                *(WORD*)(CA + 0x54) = ClampToWord(ViewPoint);
                *(WORD*)(CA + 0x14) = ClampToWord(ViewStr);
                *(WORD*)(CA + 0x16) = ClampToWord(ViewDex);
                *(WORD*)(CA + 0x18) = ClampToWord(ViewVit);
                *(WORD*)(CA + 0x1A) = ClampToWord(ViewEne);
                *(WORD*)(CA + 0x20) = ClampToWord(ViewMaxHP);
                *(WORD*)(CA + 0x22) = ClampToWord(ViewMaxMP);
                *(WORD*)(CA + 0x26) = ClampToWord(*(DWORD*)(Msg + 24));   // MaxBP
                NetLog("NET:  → F3/06 AddPoint OK slot=%d pts=%u str=%u agi=%u vit=%u ene=%u",
                       slot, ViewPoint, ViewStr, ViewDex, ViewVit, ViewEne);
            } else {
                // Server sin EXTRA: sólo llegan MaxLifeAndMana/MaxBP,
                // así que el stat y los puntos se ajustan localmente.
                WORD maxLifeMana = *(WORD*)(Msg + 6);
                if (*(WORD*)(CA + 0x54) > 0) (*(WORD*)(CA + 0x54))--;
                switch (slot) {
                case 0: (*(WORD*)(CA + 0x14))++; break;
                case 1: (*(WORD*)(CA + 0x16))++; break;
                case 2: (*(WORD*)(CA + 0x18))++; *(WORD*)(CA + 0x20) = maxLifeMana; break;
                case 3: (*(WORD*)(CA + 0x1A))++; *(WORD*)(CA + 0x22) = maxLifeMana; break;
                }
                *(WORD*)(CA + 0x26) = *(WORD*)(Msg + 8);                 // IDA v4[19] = MaxBP
                NetLog("NET:  → F3/06 AddPoint OK (no-extra) slot=%d maxLifeMana=%u",
                       slot, maxLifeMana);
            }
            // IDA ReceiveAddPoint (0x431480) termina con sub_47E3C0
            // (CharData_RecalcStats).  Sin esto dano, defensa y
            // velocidad quedaban viejos hasta cambiar el equipo.
            CalculateAll((int)(uintptr_t)CharacterMachine, 0, 0);
            break;
        }

        case 0x07: {
            // PMSG_MONSTER_DAMAGE_SEND (Protocol.h:485) - dano que
            // nos hace un monstruo.  IDA lo resuelve inline:
            //   v51 = RB[5] + (RB[4] << 8);
            //   if (CA+28 < v51) CA+28 = 0; else CA+28 -= v51;
            // Layout con el padding de MSVC:
            //   +4,+5 BYTE damage[2]  .  +6,+7 padding
            //   +8  DWORD ViewCurHP   .  +12 DWORD ViewDamageHP
            // Con GAMESERVER_EXTRA el server manda la vida que le
            // queda al pj ya resuelta: es autoritativa y evita que
            // el cliente se desincronice restando de su propia copia.
            if (Size < 6 || !CharacterAttribute) break;
            BYTE* CA = (BYTE*)CharacterAttribute;
            if (Size >= 16) {
                *(WORD*)(CA + 28) = ClampToWord(*(const DWORD*)(Msg + 8));
            } else {
                const WORD dmg = (WORD)(Msg[5] + (Msg[4] << 8));
                WORD hp = *(WORD*)(CA + 28);
                *(WORD*)(CA + 28) = (hp < dmg) ? 0 : (WORD)(hp - dmg);
            }
            NetLog("NET:  -> F3/07 MonsterDamage hp=%u",
                   (unsigned)*(WORD*)(CA + 28));
            break;
        }

        case 0x08: {
            // ReceivePK (0x00431DC0) - PMSG_PK_LEVEL_SEND
            // (Protocol.h:495): index[2] en +4,+5 y PKLevel en +6.
            // Hasta ahora el PKLevel solo llegaba con el F3/03, o
            // sea no se actualizaba en vivo.  El campo es +746
            // (0x2EA), el mismo que gatea el tinte rojo del render
            // (Entity_UpdateRender: `>= 6`).
            if (Size < 7 || !DAT_07abf5d0) break;
            const int pkKey = Msg[5] + (Msg[4] << 8);
            const int pkIdx = FindCharacterIndex(pkKey);
            if (pkIdx < 0 || pkIdx >= 400) break;
            BYTE* pkEnt = (BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)pkIdx * 0x394;
            const BYTE pkLevel = Msg[6];
            pkEnt[746]            = pkLevel;
            *(WORD*)(pkEnt + 446) = (WORD)(pkLevel >= 6);
            NetLog("NET:  -> F3/08 PKLevel key=%d idx=%d lvl=%u",
                   pkKey, pkIdx, (unsigned)pkLevel);
            // Aviso en el chat.  El decompile perdio los break de
            // cada case (se leen como fall-through), pero cada uno
            // elige su texto y su color y cae en la MISMA llamada.
            int pkColor = 0;
            int pkText  = -1;
            switch (pkLevel) {
                case 2: pkColor = 1; pkText = 487; break;
                case 3: pkColor = 1; pkText = 488; break;
                case 4: pkColor = 2; pkText = 489; break;
                case 5: pkColor = 2; pkText = 490; break;
                case 6: pkColor = 2; pkText = 491; break;
                default: break;
            }
            if (pkText >= 0 && GlobalText[pkText] && GlobalText[pkText][0]) {
                UIChatLogWindow_AddText((char*)(pkEnt + 449),
                                        GlobalText[pkText], pkColor);
            }
            break;
        }

        case 0x13: {
            // PMSG_ITEM_EQUIPMENT_SEND (ItemManager.h:162) - cambio
            // de equipo de OTRO jugador del viewport.
            //   +4,+5 index[2]  .  +6..+16 CharSet[11]
            // IDA pasa `ReceiveBuffer + 7`, que es `&CharSet[1]`:
            // los otros dos callers de ChangeCharacterExt
            // (ReceiveCharacterList L68, Combat_PacketDispatch L317)
            // pasan literalmente `&CharSet[1]`, o sea la funcion
            // espera el CharSet SIN el byte de clase.  Coincide 1:1
            // con el layout del server.
            if (Size < 17) break;
            const int eqKey = Msg[5] + (Msg[4] << 8);
            const int eqIdx = FindCharacterIndex(eqKey);
            NetLog("NET:  -> F3/13 ItemEquipment key=%d idx=%d", eqKey, eqIdx);
            if (eqIdx < 0 || eqIdx >= 400) break;
            ChangeCharacterExt(eqIdx, (BYTE*)Msg + 7);
            break;
        }

        case 0x14: {
            // PMSG_ITEM_MODIFY_SEND (ItemManager.h:169) - el server
            // reescribe una celda del inventario.
            //   +4 slot  .  +5.. ItemInfo
            if (Size < 6) break;
            NetLog("NET:  -> F3/14 ItemModify slot=%d", Msg[4]);
            DAT_07e91388 = 0;            // suelta el item agarrado
            InsertInventoryItem(OffsetInventoryItems, 8, 8, Msg[4],
                         (BYTE*)Msg + 5, 0);
            PlayBuffer(49, 0, 0);
            break;
        }

        case 0x20:
            // PMSG_SUMMON_LIFE_SEND (Protocol.h:502) - HP % de la
            // mascota invocada.  Es el UNICO productor del valor;
            // sin el, la barra del monstruo invocado nunca se
            // dibuja (el gate del HUD es `if (SummonLife)`).
            if (Size < 5) break;
            DAT_05826d24 = Msg[4];       // SummonLife
            NetLog("NET:  -> F3/20 SummonLife=%u", (unsigned)Msg[4]);
            break;

        case 0x22:
            // PMSG_TIME_VIEW_SEND (Protocol.h:508) - `WORD time` en
            // +4.  IDA lo llama `SoccerTime` (0x05826C08) y lo lee
            // igual: `*((WORD *)ReceiveBuffer + 2)`.
            if (Size < 6) break;
            DAT_05826c08 = *(const WORD*)(Msg + 4);   // SoccerTime
            NetLog("NET:  -> F3/22 TimeView=%u", (unsigned)DAT_05826c08);
            break;

        case 0x23: {
            // Marcador de guild war / soccer.  MuEmu no manda este
            // sub-opcode (no hay ningun sender con 0xF3,0x23), asi
            // que hoy es codigo inerte; se porta por completitud.
            //   +4..+11  nombre equipo 0   .  +12 score equipo 0
            //   +13..+20 nombre equipo 1   .  +21 score equipo 1
            // (0xFF en el score = no hay partido en curso)
            if (Size < 23) break;
            memcpy(&SoccerTeamName[0][0], Msg + 4, 8);
            *(WORD*)&SoccerTeamName[0][8] = *(const WORD*)(Msg + 12);
            memcpy(&SoccerTeamName[1][0], Msg + 13, 8);
            *(WORD*)&SoccerTeamName[1][8] = *(const WORD*)(Msg + 21);
            SoccerTeamName[0][8] = 0;    // el WORD de arriba escribe
            SoccerTeamName[1][8] = 0;    // 2 bytes; aca se corta en NUL
            GuildWarScore[0] = Msg[12];
            GuildWarScore[1] = Msg[21];
            DAT_05826d33 = (char)(Msg[12] != 255);   // SoccerObserver
            NetLog("NET:  -> F3/23 GuildWarScore %d-%d obs=%d",
                   GuildWarScore[0], GuildWarScore[1], (int)DAT_05826d33);
            break;
        }

        case 0x40: {
            // ReceiveServerCommand (0x00436550).  Dos senders del
            // server comparten este sub-opcode y se distinguen por
            // el byte `type` en +4: GCFireworksSend (type 0, con x/y
            // en +5/+6) y GCServerCommandSend (el resto).
            if (Size < 5) break;
            NetLog("NET:  -> F3/40 ServerCommand type=%d arg=%d",
                   Msg[4], (Size >= 6) ? Msg[5] : -1);
            switch (Msg[4]) {
                case 0: {   // fuegos artificiales sobre una celda
                    if (Size < 7) break;
                    float Position[3];
                    float Angle[3] = { 0.0f, 0.0f, 0.0f };
                    float Light[3] = { 1.0f, 1.0f, 1.0f };
                    Position[0] = ((float)Msg[5] + 0.5f) * 100.0f;
                    Position[1] = ((float)Msg[6] + 0.5f) * 100.0f;
                    Position[2] = RequestTerrainHeight(Position[0], Position[1]);
                    CreateEffect(1248, Position, Angle, Light,
                                 nullptr, nullptr, (float*)-1, nullptr, 0);
                    break;
                }
                case 1: {   // aviso de texto (dos bloques de GlobalText)
                    if (Size < 6) break;
                    const int gt = (Msg[5] < 20) ? (Msg[5] + 650)
                                                 : (Msg[5] + 810);
                    if (GlobalText[gt] && GlobalText[gt][0])
                        CreateOkMessageBox(GlobalText[gt]);
                    break;
                }
                case 2:
                    PlayBuffer(70, 0, 0);
                    break;
                case 3: {
                    if (Size < 6) break;
                    const int gt = Msg[5] + 710;
                    if (GlobalText[gt] && GlobalText[gt][0])
                        CreateOkMessageBox(GlobalText[gt]);
                    break;
                }
                case 5:
                    if (Size < 6) break;
                    ItemList_Select(Msg[5]);      // avanza el dialogo
                    break;
                case 6:
                    if (GlobalText[449] && GlobalText[449][0])
                        CreateOkMessageBox(GlobalText[449]);
                    break;
                default:
                    break;
            }
            break;
        }

        case 0x10: {
            // F3/10: server snapshot of full inventory + equipment.
            // Portado del sub_404830 de la 0.52 en src/Item/Item_Inventory.cpp.
            NetLog("NET:  → F3/10 Inventory snapshot");
            Recv_Inventory(Msg);
            break;
        }
        case 0x11: {
            // Port FIEL desde server source
            // Mu-linux-97K/Source/MuServer/GameServer/SkillManager.cpp:2256
            // GCSkillListSend. Wire format:
            //   [C1][size][F3][11][count] [slot][skill][level]×count
            //
            // Per PMSG_SKILL_LIST (server SkillManager.h:161):
            //   slot:  position in skill array (0..MAX_SKILL_LIST-1)
            //   skill: skill ID
            //   level: (m_level << 3) | (m_index & 7)  ← packed
            //
            // CharacterAttribute.Skill[] layout (per IDA):
            //   CA[86] = count
            //   CA[87..86+count] = skill IDs (byte each)
            //
            // Hero+913 = SelectedSkill (default 0 = first skill).
            if (Size < 5) {
                NetLog("NET:  → F3/11 SkillList size=%d (too small)", Size);
                break;
            }
            BYTE count = Msg[4];
            NetLog("NET:  → F3/11 SkillList count=%d size=%d", count, Size);
            if (!DAT_07cf1ff4) break;
            BYTE* CA = (BYTE*)(uintptr_t)DAT_07cf1ff4;

            // The server uses the same F3:11 packet for a delta:
            // count=0xFE adds one skill and count=0xFF removes one.
            // Treating those values as an oversized full list made a
            // learned scroll visible only after the next login.
            if (count == 0xFE || count == 0xFF) {
                if (Size < 8) break;
                BYTE slot = Msg[5];
                BYTE skill = Msg[6];
                if (slot >= 20) break;
                CA[87 + slot] = (count == 0xFE) ? skill : 0;

                int total = 0;
                for (int i = 0; i < 20; ++i) {
                    if (CA[87 + i] != 0) ++total;
                }
                CA[86] = (BYTE)total;
                if (DAT_07abf5d8) {
                    BYTE* hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
                    if (hero[913] >= 20) hero[913] = 0;
                }
                NetLog("NET:    F3/11 Skill%s slot=%d skill=%d total=%d",
                       (count == 0xFE) ? "Add" : "Del", slot, skill, total);
                break;
            }

            if (count > 20) break;

            // Primero limpia los 60 slots para evitar basura vieja que
            // crashearía el tooltip al pasar el mouse (RenderSkillIcon
            // lee el byte en 0x57+idx — basura fuera de rango = OOB
            // on GetSkillInformation).
            for (int i = 0; i < 60; ++i) CA[87 + i] = 0;

            int written = 0;
            for (int n = 0; n < count; ++n) {
                int off = 5 + n * 3;
                if (off + 2 >= Size) break;
                BYTE slot   = Msg[off];
                BYTE skill  = Msg[off + 1];
                // BYTE level  = Msg[off + 2];  // packed level — TODO: store separately
                if (slot < 20) {
                    CA[87 + slot] = skill;
                    if (skill != 0) written++;
                }
            }
            CA[86] = count;
            if (DAT_07abf5d8) {
                BYTE* hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
                if (hero[913] >= 20) hero[913] = 0;
            }
            if ((DWORD)DAT_005616ac > 4) DAT_005616ac = 0;   // 5 slots: 0..4 (era >= 4, pisaba el quinto)
            // La lista recien ahora esta completa: re-traducir las
            // teclas de skill que llegaron en el F3/30 (MuEmu lo
            // manda antes que este paquete).
            ApplySkillKeyMap();
            NetLog("NET:    F3/11 stored %d skills: %d %d %d %d %d %d %d %d %d %d",
                   written, CA[87], CA[88], CA[89], CA[90], CA[91],
                   CA[92], CA[93], CA[94], CA[95], CA[96]);
            break;
        }
        case 0xE0: {
            // F3/E0 PMSG_NEW_CHARACTER_INFO_RECV: Level/Stats/HP/MP.
            // Port FIEL desde DLL injection (Protocol.cpp:849).
            NetLog("NET:  → F3/E0 NewCharacterInfo");
            Recv_NewCharacterInfo(Msg);
            break;
        }
        case 0xE1: {
            // F3/E1 PMSG_NEW_CHARACTER_CALC_RECV: HP/MP/Defense/Attack.
            NetLog("NET:  → F3/E1 NewCharacterCalc");
            Recv_NewCharacterCalc(Msg, Size);
            break;
        }
        case 0xE3: {  // lista de apilado (DLL CItemStack)
            extern void Recv_ItemStackList(const BYTE* Msg, int Size);
            Recv_ItemStackList((const BYTE*)Msg, Size);
            break;
        }
        case 0xE4: {  // precios fijos (DLL CItemValue)
            extern void Recv_ItemValueList(const BYTE* Msg, int Size);
            Recv_ItemValueList((const BYTE*)Msg, Size);
            break;
        }
        case 0xE6: {
            gEventTimer.Receive(Msg, Size);
            break;
        }
        case 0xE2: case 0xE5: {
            // F3/E2 (barras de vida) y F3/E5 (lista de /move): sólo se vuelcan al log.
            // Sus layouts están en Protocol/GameServerProtocol.h.
            char b[400];
            int p = wsprintfA(b, "NET:  → F3/%02X DUMP size=%d: ", sub, Size);
            int dumpN = Size > 64 ? 64 : Size;
            for (int i = 0; i < dumpN && p < 380; ++i)
                p += wsprintfA(b + p, "%02X ", Msg[i]);
            NetLog("%s", b);
            break;
        }
        case 0x30: {
            // ── ReceiveOption (IDA 0x436FB0) — PORT FIEL ─────────────────
            // Layout autoritativo del server MuEmu (Protocol.h,
            // PMSG_OPTION_DATA_SEND, header C1:F3:30). Coincide 1:1 con los
            // offsets que lee el IDA ReceiveOption:
            //   +4..13  SkillKey[10]  → mapa de skill-keys del héroe
            //   +14     GameOption    (bit0=AutoAttack, bit2=WhisperSound)
            //   +15/16/17  QKey/WKey/EKey  (item type = byte + 448)
            //   +18     ChatWindow    (nibble alto*3 = líneas, bajo = transparencia)
            const BYTE* p = Msg + ((hdr == 0xC1) ? 4 : 5);
            NetLog("NET:  → F3/30 Option size=%d", Size);
            if (Size < 19) { NetLog("NET:    F3/30 too short — skip"); break; }

            // 1) Teclas de skill: se guardan y se traducen en
            //    ApplySkillKeyMap, que tambien corre al final del
            //    F3/11 porque MuEmu manda este paquete antes que la
            //    lista de skills (ver la nota del helper).
            memcpy(s_PendingSkillKey, p, sizeof(s_PendingSkillKey));
            s_HasPendingSkillKey = true;
            ApplySkillKeyMap();

            // 2) Opciones de juego.
            DAT_07e11e18 = ((p[10] & 1) == 1);          // m_bAutoAttack
            m_bWhisperSound = (char)((p[10] & 4) == 4);    // m_bWhisperSound (0x07E11D80); antes un DAT_07e11e26 sin xrefs en IDA
            DAT_00559c60 = p[11] + 448;                 // QKey  (item type)
            DAT_00559c64 = p[12] + 448;                 // WKey
            DAT_00559c68 = p[13] + 448;                 // EKey

            // 3) Chat window: líneas visibles + transparencia + on/off del listbox.
            //    IDA: si ChatWindow==0xFF usa 36 (=0x24 → 6 líneas, alpha 0.4).
            BYTE cw = p[14];
            if (cw == 0xFF) cw = 36;
            int chatLines = 3 * (cw >> 4);
            int transp    = cw & 0x0F;
            if (chatLines) {
                g_bUseChatListBox = 1;
                if (DAT_055c9ff0) {          // vtable +56 = setVisibleCnt (slot 14)
                    DWORD* cobj = (DWORD*)DAT_055c9ff0;
                    void** cvt  = (void**)*cobj;
                    if (cvt) {
                        typedef int (__fastcall *FnSetCnt)(DWORD*, int, int);
                        ((FnSetCnt)cvt[14])(cobj, 0, chatLines);
                    }
                }
            } else {
                g_bUseChatListBox = 0;            // sin recuadro (modo clásico)
            }
            if (DAT_055c9ff0)
                *(float*)((BYTE*)DAT_055c9ff0 + 188) = (float)transp * 0.1f;

            // 4) POSICIÓN de la ventana de chat según el modo.
            // IDA CheckFunctionButtons (0x4C04A0, handler de F4):
            //     if (g_bUseChatListBox) sub_40C690(obj, 186, 420);  // abajo
            //     else                   sub_40C690(obj, -10,  81);  // ARRIBA-IZQ
            // sub_40C690 = setPosition → obj[11]=x, obj[12]=y.
            // Nuestro ctor hardcodeaba (186,420) fijo, así que en modo clásico
            // los mensajes salían abajo en vez de arriba-izquierda.
            if (DAT_055c9ff0) {
                DWORD* cobj = (DWORD*)DAT_055c9ff0;
                if (g_bUseChatListBox) { cobj[11] = 186;          cobj[12] = 420; }
                else              { cobj[11] = (DWORD)(-10); cobj[12] = 81;  }
            }

            NetLog("NET:    F3/30 opt=%02X Q=%d W=%d E=%d chatWin=%02X lines=%d transp=%d listbox=%d",
                   p[10], DAT_00559c60, DAT_00559c64, DAT_00559c68,
                   cw, chatLines, transp, (int)g_bUseChatListBox);
            break;
        }

        default: {
            // Dump del payload de CUALQUIER F3 sub desconocido, para tener la
            // estructura cuando toque portarlo (p.ej. F3/E6 = horarios de eventos, ver
            // Protocol/GameServerProtocol.h).  El server MuEmu los manda en loop; hoy
            // se ignoran (inofensivo), pero acá queda el hex para identificarlos.
            char b[420];
            int p = wsprintfA(b, "NET:  → F3/%02X UNHANDLED-DUMP size=%d: ", sub, Size);
            int dumpN = Size > 80 ? 80 : Size;
            for (int i = 0; i < dumpN && p < 400; ++i)
                p += wsprintfA(b + p, "%02X ", Msg[i]);
            NetLog("%s", b);
            // También logueá los bytes imprimibles (nombres de evento, etc.)
            char t[100]; int q = 0;
            for (int i = 4; i < dumpN && q < 95; ++i) {
                char ch = (char)Msg[i];
                t[q++] = (ch >= 0x20 && ch < 0x7F) ? ch : '.';
            }
            t[q] = 0;
            NetLog("NET:    F3/%02X ascii: %s", sub, t);
            break;
        }
    }
}

// 0x2C
void NetRecv_2C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ── ReceiveUseStateItem (IDA 0x437F10) — resultado de la fruta ──
    // El case faltaba: sin el, usar una fruta mandaba el 0x26 pero el
    // resultado se descartaba en silencio y el panel de stats (tecla C)
    // no se enteraba de nada.
    //
    // Server: CFruit::UseFruit (Fruit.cpp) -> PMSG_FRUIT_RESULT_SEND,
    // header C1:2C.  Layout con GAMESERVER_EXTRA=1 (sizeof 28):
    //   +3   BYTE  result
    //   +4   DWORD ViewValue       (puntos sumados)
    //   +8   DWORD ViewPoint       (LevelUpPoint)
    //   +12  DWORD ViewStrength
    //   +16  DWORD ViewDexterity
    //   +20  DWORD ViewVitality
    //   +24  DWORD ViewEnergy
    //
    // `result` empaqueta tres campos:
    //   bits 0-3  = cantidad de puntos
    //   bits 4-5  = stat (0 Ene, 1 Vit, 2 Agi, 3 Fue)
    //   bits 6-7  = 0 exito(+) · 1 fallo · 2 exito(-) · 3 no permitido
    const int amount  = Msg[3] & 0x0F;
    const int statIdx = (Msg[3] >> 4) & 0x03;
    const int outcome = Msg[3] >> 6;

    // Indice del nombre del stat en GlobalText, per IDA L88-100.
    static const int kStatText[4] = { 168, 169, 167, 166 };
    // CharacterAttribute: +20 Fuerza, +22 Agilidad, +24 Vitalidad,
    // +26 Energia (mismos offsets que el F3/03 y el F3/06).
    static const int kStatOff[4]  = { 26, 24, 22, 20 };

    BYTE* CA = (BYTE*)(uintptr_t)DAT_07cf1ffc;
    NetLog("NET:  -> 0x2C Fruit result=%02X outcome=%d stat=%d amount=%d size=%d",
           Msg[3], outcome, statIdx, amount, Size);

    if (outcome == 1) {
        CreateOkMessageBox(GlobalText[378]);        // "fallo"
    } else if (outcome == 3) {
        CreateOkMessageBox(GlobalText[446]);        // "no se puede"
    } else if (CA) {
        // outcome 0 = suma, 2 = resta.
        const int sign = (outcome == 0) ? 1 : -1;
        *(WORD*)(CA + kStatOff[statIdx]) =
            (WORD)(*(WORD*)(CA + kStatOff[statIdx]) + sign * amount);
        *(WORD*)(CA + 46) = (WORD)(*(WORD*)(CA + 46) + sign * amount); // FruitAddPoint

        // Con GAMESERVER_EXTRA el server manda el estado completo y
        // autoritativo; mandan esos valores sobre la aritmetica de arriba
        // (misma regla que el F3/05 y el F3/06).
        if (Size >= 28) {
            *(WORD*)(CA + 20) = ClampToWord(*(const DWORD*)(Msg + 12));  // Fuerza
            *(WORD*)(CA + 22) = ClampToWord(*(const DWORD*)(Msg + 16));  // Agilidad
            *(WORD*)(CA + 24) = ClampToWord(*(const DWORD*)(Msg + 20));  // Vitalidad
            *(WORD*)(CA + 26) = ClampToWord(*(const DWORD*)(Msg + 24));  // Energia
            *(WORD*)(CA + 84) = ClampToWord(*(const DWORD*)(Msg + 8));   // LevelUpPoint
        }

        // Recalcula damage/defense/velocidades con los stats nuevos.
        CalculateAll((int)(uintptr_t)CA, 0, 0);

        char buf[300];
        // GlobalText[377] = texto de suma, [379] = texto de resta.
        wsprintfA(buf, GlobalText[outcome == 0 ? 377 : 379],
                  GlobalText[kStatText[statIdx]], amount);
        CreateOkMessageBox(buf);
    }
    EnableUse = 0;
}

// 0x1C
void NetRecv_1C(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Teleport response compatibility: use MuEmu's active server
    // format, not an assumed original-server packet variation.
    // MuEmu GameServer/Move.h::PMSG_TELEPORT_SEND is:
    // C3:size:1C gate,map,x,y,dir (8 bytes). For Teleport skill 6
    // Move.cpp manda gate=0 después de su broadcast normal de skill 0x19.
    // IDA ReceiveTeleport supplies the visual semantics below.
    if (Size < 8 || !Hero) {
        NetLog("NET: 0x1C Teleport malformed size=%d", Size);
        return;
    }

    const BYTE gate = Msg[3], map = Msg[4], gridX = Msg[5];
    const BYTE gridY = Msg[6], direction = Msg[7];
    NetLog("NET: 0x1C Teleport gate=%u map=%u xy=(%u,%u) dir=%u",
           (unsigned)gate, (unsigned)map, (unsigned)gridX,
           (unsigned)gridY, (unsigned)direction);

    // IDA: lo primero es sub_4CD3B0 = devolver el item que se tenga
    // agarrado a su celda (en las dos ramas).
    Item_ReturnPickedItem();

    BYTE* hero = (BYTE*)(uintptr_t)Hero;
    const float worldX = ((float)gridX + 0.5f) * 100.0f;
    const float worldY = ((float)gridY + 0.5f) * 100.0f;
    *(float*)(hero + 16) = worldX;
    *(float*)(hero + 20) = worldY;

    // ReceiveTeleport @ 00428210 calcula la altura del piso en
    // el tile autorizado por el server. Su rama de montado/zona segura
    // preserva el offset vertical original antes de terminar el
    // teleport animation.
    float worldZ = RequestTerrainHeight(worldX, worldY);
    if (World != -1 && *(short*)(hero + 696) == 819 && !hero[846])
        worldZ += (World == 8 || World == 10) ? 90.0f : 30.0f;
    *(float*)(hero + 24) = worldZ;
    // IDA L141-160: solo +904/+908 (la grilla).  El camino viejo no se
    // retoma porque el final del handler hace c+748 = 0 y SetPlayerStop.
    *(DWORD*)(hero + 904) = gridX;
    *(DWORD*)(hero + 908) = gridY;
    *(float*)(hero + 36) = ((float)direction - 1.0f) * 45.0f;

    if (gate != 0) {
        // ReceiveTeleport's gate branch clears the old viewport
        // antes de aceptar el par mapa/posición del server. Esto
        // es deliberadamente distinto del Teleport de mago: los gates
        // no llaman a CreateTeleportEnd; recargan el mundo
        // cuando hace falta y esperan los paquetes de viewport nuevos.
        ClearItems();
        ClearCharacters((int)HeroKey);

        if (map != (BYTE)World) {
            World = map;
            // OpenWorld tarda ~2 s cargando BMDs y, para que el server no cierre por
            // backpressure, AccessModel pumpea la cola de mensajes cada 8 modelos.  Ese
            // pump entrega WM_USER -> Net_Recv -> **Net_ProcessPacket**, o sea los
            // handlers correrian RE-ENTRANTES en mitad de la carga (p.ej. un
            // `0x13 ViewportMonster` creando monstruos cuyo modelo todavia no esta abierto).
            //
            // `g_WorldLoading` deja que el pump siga DRENANDO el socket (que es lo que
            // evita el backpressure) pero suspende el dispatch: los paquetes quedan en la
            // cola y se procesan al terminar la carga.
            ++g_WorldLoading;
            OpenWorld();
            --g_WorldLoading;

            // OpenWorld replaces terrain data, so IDA evaluates
            // la altura de aterrizaje una segunda vez contra el mapa nuevo.
            worldZ = RequestTerrainHeight(worldX, worldY);
            if (World != -1 && *(short*)(hero + 696) == 819 && !hero[846])
                worldZ += (World == 8 || World == 10) ? 90.0f : 30.0f;
            *(float*)(hero + 24) = worldZ;

            // IDA L275-277: aviso "<mapa> ..." en el chat.
            char mapNotice[256];
            sprintf_s(mapNotice, "%s%s", GetMapName(World), GlobalText[484]);
            UIChatLogWindow_AddText("", mapNotice, 1);
        }

        // ── ACK de fin de carga: C1 04 F3 12 ──────────────────
        // IDA `ReceiveTeleport` @0x428210, dentro del branch de gate
        // y DESPUÉS de OpenWorld, arma y envía un paquete
        // (`v118[2]=0xC1 v118[3]=1 v118[4]=0xF3` + chain-XOR) y
        // recién entonces setea `LoadingWorld = 30`.
        //
        // Del lado del server (MuEmu) el ciclo es:
        //   gObjMoveGate OK        → RegenOk = 1  (User.cpp:1964)
        //   cliente manda F3/12    → RegenOk = 2  (Protocol.cpp:1439
        //                            CGCharacterMoveViewportEnableRecv)
        //   tick de ObjectManager  → RegenOk = 3, State = OBJECT_CREATE,
        //                            aplica RegenMapNumber/X/Y y recién
        //                            ahí manda el viewport del mapa nuevo
        //   luego                  → RegenOk = 0
        //
        // Sin el ACK, `RegenOk` se queda en 1 y produce los DOS
        // síntomas a la vez:
        //   · `gObjMoveGate` (User.cpp:1882) hace
        //     `if (lpObj->RegenOk != 0 ...) goto ERROR_JUMP;` — todo
        //     `/move` posterior se rechaza, y el ERROR_JUMP reenvía
        //     la posición ACTUAL, que el cliente interpreta como un
        //     teleport al mismo lugar (de ahí "siempre va al primer
        //     destino").
        //   · nunca se llega a `State = OBJECT_CREATE`, así que el
        //     server no manda las entidades del mapa: sin NPCs ni
        //     mobs.
        {
            BYTE ackPkt[4] = { 0xC1, 0x04, 0xF3, 0x12 };
            gNetwork.Send(ackPkt, 4);
            NetLog("NET:    0x1C gate → F3/12 ViewportEnable ACK enviado");
        }

        // Temporizadores de "cargando mundo" (IDA LABEL_108).
        // DAT_07e11d1c = LoadingWorld: Render_GameFrame saltea el
        // frame mientras sea > 30, que es la "pantalla de carga".
        DAT_07e11dc8 = GetTickCount();
        DAT_07e11dc4 = 0;
        DAT_07e11d1c = 30;

        // ReceiveTeleport @ 00428210: restore the complete UI/
        // el estado de selección recién después de abrir el mundo nuevo. Las
        // direcciones de abajo son los globals originales, no los
        // similarly named 07E119xx input-state variables.
        // (Aca habia un `DAT_05826d04 = 0` sin contraparte: la rama
        //  de gate de IDA, L476-503, no toca Teleport ni 0x5826D04.)
        DAT_07e11d28 = 0;                 // MouseUpdateTime
        DAT_00559bec = 6;                 // MouseUpdateTimeMax
        InventoryOpened = 0;
        ShopOpened = 0;
        WarehouseOpened = 0;
        DAT_00559f5f = 0;
        DAT_07eaa14c = 0;
        EventWindowOpened = 0;
        CreateEffect(1265, (float*)(hero + 16), (float*)(hero + 28),
                     (float*)(hero + 232), nullptr, (float*)hero,
                     (float*)-1, nullptr, 0);
        *(DWORD*)(hero + 0x168) = 0;
        DAT_083a3ff0 = 0;                  // EnableEvent
        SelectedItem = -1;                 // SelectedItem
        SelectedNpc = -1;                 // SelectedNpc
        SelectedCharacter = -1;                 // SelectedCharacter
        SelectedOperate = -1;                 // SelectedOperate
        Attacking = -1;                 // Attacking
        DAT_00559c6d = -1;
        // IDA hace un store de DWORD en 07EAA134. En este port de C++
        // sólo está representado su byte vivo RepairEnable_0; no
        // desbordar los globals host, que no son contiguos.
        DAT_07eaa134 = 0;
        *(BYTE*)&DAT_07eaa138 = 0;         // RepairEnable
    } else {
        // El server usa gate=0 para el teleport de skill. IDA
        // completa ese efecto visual y limpia Teleport acá.
        CreateTeleportEnd((unsigned int)(uintptr_t)hero);
        Teleport = 0;                 // IDA L508: Teleport = 0 (0x05826D14)
    }

    // Este store es común a las dos ramas en el original.
    hero[748] = 0;
    SetPlayerStop((int)(uintptr_t)hero);
}

// 0x26
void NetRecv_26(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveLife @ 0x00431780 (port FIEL).
    // Server pushes HP / MaxHP updates and item-durability decrements
    // por este opcode. El sub-byte en Msg[3] elige:
    //   0xFD     → EnableUse = 0 (item slot lock)
    //   0xFE     → MaxLife (CharacterAttribute+32) = WORD BE
    //   0xFF     → Life    (CharacterAttribute+28) = WORD BE
    //   else     → Inventory slot N decrement: ItemAttribute[N].Durability--
    if (Size < 6) return;
    sub = Msg[3];
    NetLog("NET:  → 0x26 Life sub=0x%02x", sub);
    if (DAT_07cf1ff4 == 0) return;
    BYTE* charAttr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (sub == 0xFD) {
        EnableUse = 0;
    } else if (sub == 0xFE) {
        *(WORD*)(charAttr + 32) = (WORD)((Msg[4] << 8) | Msg[5]);
    } else if (sub == 0xFF) {
        *(WORD*)(charAttr + 28) = (WORD)((Msg[4] << 8) | Msg[5]);
    } else {
        int slot = (int)sub - 12;
        if (slot >= 0 && slot < 64) {
            ITEM* inv = (ITEM*)OffsetInventoryItems;
            ITEM* it = &inv[slot];
            if (it->Durability > 0) {
                it->Durability--;
            }
            if (it->Durability == 0) {
                memset(it, 0, sizeof(ITEM));
                it->Type = -1;
            }
        }
    }
}

// 0x27
void NetRecv_27(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveMana @ 0x00431A90 (port FIEL).
    // Server pushes Mana / BP / MaxMana / MaxBP updates.
    //   0xFE → MaxMana(offset 34) + MaxBP(offset 38), cada uno WORD BE
    //   0xFF → Mana(offset 30)    + BP(offset 36),    cada uno WORD BE
    if (Size < 8) return;
    sub = Msg[3];
    NetLog("NET:  → 0x27 Mana sub=0x%02x", sub);
    if (DAT_07cf1ff4 == 0) return;
    BYTE* charAttr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (sub == 0xFE) {
        *(WORD*)(charAttr + 34) = (WORD)((Msg[4] << 8) | Msg[5]);
        *(WORD*)(charAttr + 38) = (WORD)((Msg[6] << 8) | Msg[7]);
    } else if (sub == 0xFF) {
        *(WORD*)(charAttr + 30) = (WORD)((Msg[4] << 8) | Msg[5]);
        *(WORD*)(charAttr + 36) = (WORD)((Msg[6] << 8) | Msg[7]);
    } else {
        *(WORD*)(charAttr + 30) = (WORD)((Msg[4] << 8) | Msg[5]);
        int slot = (int)sub - 12;
        if (slot >= 0 && slot < 64) {
            BYTE* invBase = (BYTE*)&DAT_07ea9328;
            BYTE* dur = invBase + slot * 68 + 11;
            if (*dur > 0) (*dur)--;
            if (*dur == 0) {
                *(short*)(invBase + slot * 68) = -1;
                memset(invBase + slot * 68 + 4, 0, 64);
            }
        }
    }
}

// ── Character config opcodes (DLL Protocol.cpp:308-327) ─────────
// Las structs PMSG_CHARACTER_*_RECV usan PBMSG_HEAD (3 bytes)
// + member alineado al tipo. WORD se alinea a 2 → +1 PAD entre header
// y data. DWORD se alinea a 4 → +1 PAD igual. Por eso los offsets son
// 4 para WORD/DWORD (no 3) y la size on-wire es 6/4/8 (no 5/4/7).
// Confirmado en debug.log: hdr=C1 op=DD size=6, op=DE size=4, op=DF size=8.
// 0xDD
void NetRecv_DD(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // [C1][06][DD][PAD][WORD level]  → offset 4
    if (Size >= 6) {
        WORD maxDelLvl = *(const WORD*)(Msg + 4);
        NetLog("NET:  → 0xDD CharDeleteMaxLevel=%u", maxDelLvl);
        g_CharDeleteMaxLevel = maxDelLvl;
    }
}

// 0xDE
void NetRecv_DE(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // [C1][04][DE][BYTE result]  → offset 3 (no padding for BYTE)
    if (Size >= 4) {
        BYTE flag = Msg[3];
        NetLog("NET:  → 0xDE CharCreationEnable=%u", flag);
        g_CharCreationEnable = flag;
    }
}

// 0xDF
void NetRecv_DF(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // [C1][08][DF][PAD][DWORD MaxLevel]  → offset 4
    if (Size >= 8) {
        DWORD maxLvl = *(const DWORD*)(Msg + 4);
        NetLog("NET:  → 0xDF MaxCharacterLevel=%u", maxLvl);
        g_MaxCharacterLevel = maxLvl;
    }
}
