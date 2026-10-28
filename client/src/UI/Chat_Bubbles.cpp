#include <cstdint>
// Chat_Bubbles.cpp

#include "stdafx.h"
#include "Text/TextCore.h"
#include "Entity/EntityName.h"
#include "Net/NetText.h"
#include "UI/ChatTypes.h"
#include <string>
#include "Platform/Text.h"
#include "globals.h"
#include "functions.h"

extern "C" const char* Guild_GetMarkName(int row);

// Anti-spam de susurros para personajes de nivel bajo.  Las dos funciones leen
// el nivel del heroe de CharacterAttribute + 14 (WORD) y recorren el anillo
// WhisperRegistID (0x07DB9310): 10 filas de 10 bytes.  El cifrado de
// CharacterMachine con la hash-table que las envuelve es anti-tamper y se omite.

// Nivel del heroe tal cual lo leen sub_47FED0 y RegistWhisperID.
static int Whisper_HeroLevel() {
    const BYTE* ca = (const BYTE*)(uintptr_t)DAT_07cf1ff4;   // IDA: CharacterAttribute
    return ca ? *(const unsigned short*)(ca + 14) : 0;
}

// IDA: sub_47FED0 (0x0047FED0) — gate de ENVIO del susurro.  Lo llama WndProc
// (L2079) con lvl 6: con el heroe por debajo de ese nivel solo se le puede
// escribir a alguien que ya te susurro (quedo anotado por RegistWhisperID); si
// no, avisa con GlobalText[479] y devuelve 0.
int __cdecl FUN_0047fed0(int lvl, const char* name) {
    if (Whisper_HeroLevel() < lvl) {
        for (int row = 0; row < 10; row++) {
            if (!strcmp(name, WhisperRegistID[row]))
                return 1;
        }
        UIChatLogWindow_AddText("", GlobalText[479], 1);   // IDA: byte_7E11DD4
        return 0;
    }
    return 1;
}

// IDA: RegistWhisperID (0x004801C0) — lo llama ProtocolCore case 2 (susurro
// recibido) con lvl 10: con el heroe de nivel < 10 anota al remitente en la
// siguiente fila del anillo, salvo que ya este.
// DESVIACION (0.97.40): IDA hace strcpy sobre filas de 10 bytes; con nombres
// de hasta 40 bytes cada fila mide 41 y se copia con tope.
void __cdecl RegistWhisperID(int lvl, const char* text) {
    if (!text || Whisper_HeroLevel() >= lvl) return;
    for (int row = 0; row < 10; row++) {
        if (!strcmp(text, WhisperRegistID[row]))
            return;
    }
    TextEngine::CopyTruncated(WhisperRegistID[WhisperID_Num], sizeof(WhisperRegistID[0]), text);
    if (++WhisperID_Num >= 10)
        WhisperID_Num = 0;
}

// Asigna texto a una entrada; lógica inline de CreateChat (IDA 0x00481BA0).
// Texto largo (>19 caracteres): CutText lo parte en Text[0] y Text[1].
// Texto corto: va entero a Text[0].
// param_3==1: system message via vtable call on DAT_055c9ff0.
void __cdecl FUN_00481a40(CHAT* param_1, char* param_2, int param_3) {
    // param_1: entrada de chat; param_2: texto.
    // param_3: 0 para texto normal, 1 para mensaje mediante vtable.

    int Length = (int)strlen(param_2);

    char* displayEnd;
    if (param_3 == 0) {
        displayEnd = (char*)((unsigned int)Length + (unsigned int)Length + 0xa0);
    } else if (param_3 == 1) {
        // System message: dispatch slot 28 (vtable +0x70) = ChatLB_AddText
        // (__fastcall this + 4 args).  Previous cdecl form left ECX = junk
        // and corrupted the chat list.
        if (DAT_055c9ff0 && *(int*)DAT_055c9ff0) {
            DWORD* obj = (DWORD*)DAT_055c9ff0;
            void** vt  = (void**)*obj;
            typedef void (__fastcall *FnAddText)(DWORD*, int /*edx*/,
                                                 const char*, const char*,
                                                 int, int);
            ((FnAddText)vt[28])(obj, 0, (const char*)param_1,
                                (const char*)param_2, 3, 0);
        }
        displayEnd = (char*)0x3e8;
    } else {
        displayEnd = param_2;
    }

    if (Length > 0x13) {
        // Long text (>19 chars): split into 2 lines
        CutText(param_2, param_1->Text[0], param_1->Text[1], Length);
        param_1->LifeTime[0] = (int)(uintptr_t)displayEnd;
        param_1->LifeTime[1] = (int)(uintptr_t)displayEnd;
        return;
    }

    // Short text: direct memcpy to offset 0x2c
    memcpy(param_1->Text[0], param_2, Length + 1);
    param_1->LifeTime[0] = (int)(uintptr_t)displayEnd;
}

// CreateChat @ 0x00481BA0 (~396 lines) — creates chat bubble above entity
// Pool de burbujas: 100 CHAT (UI/ChatTypes.h) en DAT_07e016f8.
// Two paths: (1) find existing slot for same owner and update, (2) find empty slot and create.
// IDA: CreateChat (0x00481BA0)
// IDA: CreateChat (0x00481BA0), literales 0x559D60/64: "[" y "] ". El prefijo
// completo determina el corte de color en sub_47F360.
static void ChatFormatGuild(char* out, std::size_t capacity, const char* guild)
{
    std::string text = "[";
    text += guild ? guild : "";
    if (capacity < 4) { if (capacity) out[0] = 0; return; }
    const std::size_t room = capacity - 1 - 3;   // NUL + "[" + "] "
    text.resize(1 + TextEngine::FitBytes(std::string_view(text).substr(1), room));
    text += "] ";
    TextEngine::CopyTruncated(out, capacity, text);
}

const char* ChatDisplayName(const CHAT* c)
{
    if (!c) return "";
    if (!c->Owner || !c->ID[0]) return c->ID;
    const char* full = EntityName::Get(c->Owner);
    return strncmp(full, c->ID, strlen(c->ID)) == 0 ? full : c->ID;
}

const char* ChatDisplayGuild(const CHAT* c)
{
    static char buffer[NetText::GuildNameSize + 4];
    if (!c || !c->Guild[0] || !c->Owner || c->Owner->GuildMarkIndex < 0) return c ? c->Guild : "";
    ChatFormatGuild(buffer, sizeof(buffer), Guild_GetMarkName(c->Owner->GuildMarkIndex));
    return buffer;
}

void __cdecl CreateChat(char* ID, char* Text, DWORD entity, int Flag, int SetColor) {
    // Guard: entity must be alive and visible
    // IDA CreateChat @0x481BA0:
    //     if ( *(_BYTE *)Owner && *(_BYTE *)(Owner + 352) )
    //     v5 = *(std::uint8_t *)(Owner + 746);          // PK
    //     if ( *(_BYTE *)(Owner + 132) == 4 ) v5 = 0;      // Kind
    if (*(BYTE *)entity == 0) return;              // Object.Live   → +0
    if (AsCharacter(entity)->Object.Visible == 0) return;      // Object.Visible → +352 (0x160)

    // Resolve SetColor: -1 means use PK color, but Kind==4 forces 0
    if (SetColor == -1) {
        SetColor = (int)AsCharacter(entity)->PK;    // Owner->PK      → +746 (0x2EA)
        if (AsCharacter(entity)->Object.Kind == 4) {         // Object.Kind==4 → +132 (0x84)
            SetColor = 0;
        }
    }

    char *pool_base = DAT_07e016f8;
    const int STRIDE = (int)sizeof(CHAT);
    const int POOL_SLOTS = (int)(sizeof(DAT_07e016f8) / sizeof(CHAT));   // 100

    // ── Path 1: Find existing slot owned by this entity ──
    char *slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (*(DWORD *)&AsChat(slot)->Owner == entity)
            goto found_existing;
    }

    // ── Path 2: Find empty slot (both timers <= 0) ──
    slot = pool_base;
    for (int si = 0; si < POOL_SLOTS; ++si, slot += STRIDE) {
        if (AsChat(slot)->IDLifeTime <= 0 && AsChat(slot)->LifeTime[0] <= 0)
            goto found_new;
    }
    return;  // no free slot

found_new:
    *(DWORD *)&AsChat(slot)->Owner = entity;
    // Copy ID
    // 0.97.40: CHAT::ID mide 24 bytes; el nombre completo se toma del Owner
    // al mostrar la burbuja (ChatDisplayName).
    TextEngine::CopyTruncated(slot, sizeof(AsChat(slot)->ID), ID);
    // Set color and guild info
    AsChat(slot)->Color = (char)SetColor;
    AsChat(slot)->GuildColor = '\0';

    {
        short guildIdx = AsCharacter(entity)->GuildMarkIndex;  // GuildMarkIndex — IDA +474 (0x1DA), no 0x1d8
        if (guildIdx < 0) {
            AsChat(slot)->Guild[0] = '\0';
        } else {
            AsChat(slot)->GuildColor = AsCharacter(entity)->GuildTeam;  // GuildTeam
            // IDA: CreateChat (0x00481BA0), literales 0x559D60/64: "[" y "] ".
            // El prefijo completo determina el corte de color en sub_47F360.
            ChatFormatGuild(AsChat(slot)->Guild, sizeof(AsChat(slot)->Guild), Guild_GetMarkName(guildIdx));
        }
    }

    // Set text: empty text → timer=100, return
    if (strlen(Text) == 0) {
        AsChat(slot)->IDLifeTime = 100;
        return;
    }

    // Assign text using FUN_00481a40 logic
    FUN_00481a40(AsChat(slot), Text, Flag);
    return;

found_existing:
    // PORT FIEL a IDA CreateChat @0x481BA0, camino "ya existe": para Text vacío
    // hace SOLO `v6[10]=10` (refresca timer); el shift text1→text2 + owner +
    // set-text SOLO ocurren cuando Text NO está vacío.  Target_Render llama esto
    // cada frame con Text="" durante el hover sobre un NPC.

    // Copy ID (name) — siempre
    // 0.97.40: CHAT::ID mide 24 bytes; el nombre completo se toma del Owner
    // al mostrar la burbuja (ChatDisplayName).
    TextEngine::CopyTruncated(slot, sizeof(AsChat(slot)->ID), ID);
    // Set color and guild info — siempre
    AsChat(slot)->Color = (char)SetColor;
    AsChat(slot)->GuildColor = '\0';

    {
        short guildIdx2 = AsCharacter(entity)->GuildMarkIndex;
        if (guildIdx2 < 0) {
            AsChat(slot)->Guild[0] = '\0';
        } else {
            AsChat(slot)->GuildColor = AsCharacter(entity)->GuildTeam;
            // IDA: mismo prefijo al actualizar una etiqueta existente.
            ChatFormatGuild(AsChat(slot)->Guild, sizeof(AsChat(slot)->Guild), Guild_GetMarkName(guildIdx2));
        }
    }

    if (strlen(Text) != 0) {
        // Texto nuevo: shift la línea actual a "previa", set owner, asignar texto.
        if (AsChat(slot)->LifeTime[0] > 0) {                     // disp1 > 0
            strcpy(AsChat(slot)->Text[1], AsChat(slot)->Text[0]);                // text1 → text2
            AsChat(slot)->LifeTime[1] = AsChat(slot)->LifeTime[0];  // disp1 → disp2
        }
        *(DWORD *)&AsChat(slot)->Owner = entity;
        FUN_00481a40(AsChat(slot), Text, Flag);
    } else {
        // Texto vacío (hover de nombre NPC/target): SOLO refrescar el timer.
        AsChat(slot)->IDLifeTime = 10;
    }
}

// AssignChat @ 0x00482090 (~78 lines) — finds character by ID, creates chat bubble
// Pass 1: search Kind==1 (players). Pass 2: search Kind==2 (NPCs).
// On match: CreateChat(ID, Text, character, Flag, -1).
void __cdecl AssignChat(char* ID, char* Text, int Flag) {
    // 0x00482090 — Find character by ID, create chat bubble
    //
    // IDA AssignChat @0x482090:
    //     if ( *(_BYTE *)v4 && *(_BYTE *)(v4 + 132) == 1 )
    //   → activo en **+0** (byte), kind en **+132 (0x84)** (byte).
    // (+0x02 es `entity_type`, 390 para jugadores: no es el kind.)
    // Entity stride 0x394 (=916, coincide con IDA). ID string en +0x1C1 (=449).

    DWORD base = DAT_07abf5d0;  // CharactersClient

    // Pass 1: search players (Kind == 1) — por nombre
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 1) {
            if (strcmp(EntityName::Get(c), ID) == 0) {
                CreateChat(ID, Text, c, Flag, -1);
                return;
            }
        }
    }

    // Pass 2: PRIMERA entidad con Kind == 2 (monstruo), SIN comparar nombre.
    // Fiel a IDA: hace `strcmp(...)` pero DESCARTA el resultado y llama igual
    // (rareza del original — el strcmp quedó sin usar).
    for (int i = 0; i < 400; i++) {
        DWORD c = base + i * 0x394;
        if (*(BYTE*)c != 0 && *(BYTE*)(c + 132) == 2) {
            CreateChat(ID, Text, c, Flag, -1);
            return;
        }
    }
}
