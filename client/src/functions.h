#pragma once

// 2026-09-25: se borraron 40 declaraciones FUN_ huerfanas de este header --
// sin definicion, sin un solo call site y casi todas con la firma generica
// inventada (int, int, int, int).  Sus direcciones ya tienen su funcion real
// declarada con el nombre de IDA (RenderParty, RenderGuildList, RenderTrade,
// Camera_SetupFrustum, OpenTextData, ...), asi que lo unico que hacian era
// ofrecer un SEGUNDO simbolo C++ para la misma direccion del binario: la forma
// exacta en que quedo muerto el puente de AccessModel.
#include <intrin.h>   // _ReturnAddress (used por HashTable_GetIndex defensive log)
#include <stdio.h>    // _snprintf_s (used por HashTable_GetIndex defensive log)
// float10 = Ghidra name for x87 80-bit extended float; MSVC uses long double (64-bit) as closest match
typedef long double float10;
// functions.h — Prototypes for all FUN_ (unresolved) functions.
//
// Kayito canonical name index (from main.exe.idb, 2026-01-03):
// Nombres del port que NO coinciden con el de IDA (el resto ya se renombro
// al nombre de IDA y la equivalencia es trivial):
//   MoveJoint = MoveJoint     MoveJoints = MoveJoints     Entity_AdvancePath = MovePath
//   Entity_RenderAll_3D = RenderCharactersClient     Combat_SendMovePathPacket = SendMove
//   Player_ProcessInput (IDA: Player_InputTick) = MoveHero     Chat_InputTick = MoveInterface
//   Path_FindRoute = PathFinding2     Timer_UpdateFrameTiming (IDA: CalcFPS) = CalcFPS
//   BMD_Animation = BMD::Animation     BMD_TransformPosition = BMD::TransformPosition
//   BMD__Release = BMD::Release     BMD__Open = BMD::Open     BMD__Save = BMD::Save
//   FUN_00448600 = CharacterAnimation     Combat_UseWarriorSkill = UseSkillWarrior
//   Combat_UseElfSkill = UseSkillElf     CHARACTER_MACHINE_GetMagicSkillDamage = CHARACTER_MACHINE::GetMagicSkillDamage
// Signatures are best-effort from call-site analysis; wrong-arity calls will
// produce C4087/C2660 errors — fix by updating the specific prototype.
//
// MSVC CRT stubs — NOT game logic:
//   crt_sprintf → crt_sprintf
//   output → crt_output_engine
//   operator_delete → operator_delete
//   __chkstk_probe → __chkstk_probe (large-frame stack allocator)

// ── HashTable / ref-count obfuscation ─────────────────────────────────────────
// These are scattered throughout as anti-tamper; not game logic.
// Note: all args are void* to accept any pointer without cast errors.
void  __cdecl HashTable_Insert(void *ctx, void *obj, void *key); // IDA: HashTable_Insert (0x00403F80)
void  __cdecl Packet_DecryptByte(void *a, void *b); // IDA: Packet_DecryptByte (0x00404330)
void* __cdecl HashTable_GetNode(void *ctx, void *key); // IDA: HashTable_GetNode (0x00404280)
uint  __cdecl HashTable_GetIndex(void *ctx, void *key);       // Returns slot index (0xffffffff = not found)
void  __cdecl PACKET_ENCRYPT(void *ctx, void *key); // IDA: PACKET_ENCRYPT (0x00404040)
void  __cdecl FUN_00403f30(void *a, void *b);
void  __cdecl CWsctlc__LogPrintOn(void);   // IDA: CWsctlc__LogPrintOn (0x00403A30)
void  __cdecl Packet_DecryptBuffer(void *node, void *key); // IDA: Packet_DecryptBuffer (0x00404370)
void  __cdecl Packet_EncryptBuffer(void *a, void *b); // IDA: Packet_EncryptBuffer (0x00404400)
void  __cdecl Packet_DecryptDword(void *key, void *node); // IDA: FUN_00409E20 (0x00409E20)
// HashTable_GetIndex — dispatch via vtable at (MAIN_HASH_CLASS+0xC)
// Returns slot index, or 0xFFFFFFFF if not found.
//
// DEFENSIVE (silencioso): anti-tamper invocado desde sistemas per-frame.  Un
// ctx/vtable corrupto producía un fn() a basura imposible de rastrear.
// Validamos y retornamos 0xFFFFFFFF (== "no encontrado") en lugar de morir.
// NO se loggea desde acá — hacerlo re-entra DbgLog cuando Windows bombea
// mensajes durante la escritura, y termina en stack smash.  Si se sospecha
// que la tabla hash está rota, mirar MAIN_HASH_CLASS en el debugger.
// 2026-05-03: SAFE STUB. Always return 0xFFFFFFFF (= "not found") so all
// callers' `if (idx != 0xffffffff) ...` guards skip the subsequent deref.
// Previously this called the vtable's hash function (g_FakeHashVtable[3] =
// HashFn_Sentinel returning 0) — but HashTable_GetNode (HashTable_GetValue) then
// did its own key-match check and returned NULL when the slot's key didn't
// match the lookup key, causing NULL deref in callers like Game_MainLoop:158.
// Returning -1 here makes the table appear empty to callers, which IS our
// desired semantics (no anti-tamper data is actually stored).
inline unsigned int HashTable_GetIndex(void* /*ctx_ptr*/, void* /*key*/) {
    return 0xffffffff;
}

// ── Sound (0x00404xxx) ────────────────────────────────────────────────────────
void  __cdecl LoadWaveFile(int, const char*, int, char); // IDA: FUN_00404A10; wrapper to LoadWaveFile
// CSQuest navigation/state helpers.  Names are from 5.2; retain IDA addresses.
uint  __fastcall CSQuest_CheckRequestCondition(void *pThis, short *quest, char last_check); // IDA: FUN_00401230
void  __fastcall CSQuest_ShowDialogText(int dialog_index);                                  // IDA: FUN_004017E0
short __fastcall CSQuest_FindQuestContext(void *pThis, short *quest, int context_column);   // IDA: FUN_004011D0
void  __fastcall CSQuest_GetQuestState(void *pThis, int quest_index);                       // IDA: FUN_004016E0
uint  __fastcall CSQuest_CheckActCondition(void *pThis, short *quest);                      // IDA: FUN_00401650
void  __fastcall CSQuest_CheckQuestState(void *pThis, char state);                          // IDA: FUN_00401730
int   __cdecl    CSQuest_FindQuestItemsInInven(int a, int b, uint c);                    // ItemSlot_Check
void  __cdecl Sound_StopBuffer(int);        // IDA: FUN_00404C60
void  __cdecl Sound_Update3DPositions(void);// IDA: FUN_00404CD0

// ── Config / net connect ──────────────────────────────────────────────────────
undefined4 __cdecl Packet_ParseReceived(void *);
char  __fastcall FUN_00403150(void *pThis, int edx, char a2, char a3);   // lista de items de la quest (0x403150)
void  __cdecl CErrorReport_Write(void* ht, const char* msg, ...); // IDA: CErrorReport::Write (0x00405540)
void  __cdecl CErrorReport_WriteCurrentTime(int verbose); // IDA: FUN_004055A0
void  __cdecl FUN_004058b0(void);
void  __cdecl Client_GetSystemInfo(int, int, int, int); // IDA: FUN_004065F0; avoid WinAPI GetSystemInfo collision
void  __cdecl FUN_00406af0(int, int, int, int);
int   __cdecl Packet_IsValidSockType(int server_idx, int channel);      // PVP flag query
bool  __cdecl CheckSpecialText(const BYTE* text); // IDA: FUN_00406B30; 5.2: CheckSpecialText
void  __cdecl FUN_00406f50(char *);

// ── Network ───────────────────────────────────────────────────────────────────
void  __cdecl Widget_NodeInit(int);           // Widget_BaseRelease — releases core widget struct
void  __cdecl Widget_Release(int, int, int); // Widget_Release — releases linked list + widget base
void  __cdecl CPhysicsManager_Move(void* physics_manager, float fTime);   // IDA: CPhysicsManager::Move (0x00409C40)
void  __cdecl CPhysicsManager_Render(void* physics_manager); // IDA: FUN_00409CF0
// Packet_DecryptDword signature — canonical (void*,void*) at line 25 above
void  __cdecl Object_SetRectFields(void*, undefined4, undefined4);  // Object_SetRectFields
void  __cdecl FUN_0040c7d0(int, int);
void  __cdecl Object_ClearMembers(int);
void  __cdecl FUN_0040e990(int, int, int);
void  __cdecl exception_ctor(void); // IDA: FUN_0040F500 (0x0040F500), ??0exception@@QAE@XZ
void  __cdecl CUIRenderText_RenderText(HDC hdc, int x, int y, const char *text, DWORD color); // Text_DrawColored

// ── Input ─────────────────────────────────────────────────────────────────────
void  __cdecl Music_StopTrack(DWORD, int); // IDA: FUN_004127F0
void  __cdecl Music_PlayTrack(DWORD, int); // IDA: PlayMp3
void  __cdecl CheckHack(int, int, int); // IDA: CheckHack (0x00412A70)
void  __cdecl GetCheckSum(int, int, int, int); // IDA: GetCheckSum (0x00412DE0)

// ── Combat / Entity interaction ───────────────────────────────────────────────
void  __cdecl PACKET_DECRYPT(void *ctx, void *counter); // IDA: FUN_00422DF0 (0x00422DF0)
void  __cdecl STRUCT_DECRYPT(void *ctx, void *chardata); // IDA: STRUCT_DECRYPT (0x00423040)
void  __cdecl Packet_EncryptByte(void *a, void *b); // IDA: Packet_EncryptByte (0x00423710)
void  __cdecl Packet_EncryptDword(void* node, void* key); // IDA: Packet_EncryptDword (0x00423760)
void  __cdecl CreateSocket(const char* ip, unsigned int port); // IDA: CreateSocket (0x00423920)
void  __cdecl Buffer_XorKey3(int, int);
int   __cdecl FindGuildName(char *Name); // IDA: FUN_00423c80 (0x00423C80)
void  __cdecl GuildWar_UpdateEntityRelation(int, int, int, int); // IDA: FUN_00423CE0 (0x00423CE0)
void  __cdecl ReceiveServerList(int, int, int); // IDA: ReceiveServerList (0x00423E10)
void  __cdecl ReceiveJoinServer(int, int); // IDA: ReceiveJoinServer (0x00424010)
void  __cdecl ReceiveCharacterList(int, int, int); // IDA: ReceiveCharacterList (0x00424240)
void  __cdecl ReceiveCreateCharacter(int, int, int, int); // IDA: ReceiveCreateCharacter (0x00424390)
void  __cdecl ReceiveLogOut(int, int, int); // IDA: ReceiveLogOut (0x004247D0)
void  __cdecl ReceiveJoinMapServer(int, int, int); // IDA: ReceiveJoinMapServer (0x00425840)
void  __cdecl ReceiveRevival(int, int, int); // IDA: ReceiveRevival (0x004264D0)
void  __cdecl ReceiveMagicList(int, int, int); // IDA: ReceiveMagicList (0x004269F0)
void  __cdecl ReceiveInventory(int, int, int); // IDA: ReceiveInventory (0x00426CF0)
void  __cdecl ReceiveTradeInventory(int, int, int); // IDA: ReceiveTradeInventory (0x00427560)
void  __cdecl ReceiveNotice(int, int, int); // IDA: ReceiveNotice (0x00427A00)
void  __cdecl ReceiveMoveCharacter(int, int); // IDA: ReceiveMoveCharacter (0x00427B90)
void  __cdecl ReceiveMovePosition(int, int, int); // IDA: ReceiveMovePosition (0x00427F40)
void  __cdecl ReceiveTeleport(int, int, int); // IDA: ReceiveTeleport (0x00428210)
void  __cdecl ReceiveChangePlayer(int, int, int); // IDA: ReceiveChangePlayer (0x00429230)
void  __cdecl Combat_PacketDispatch(int opcode, int data_ptr, int len); // IDA: Combat_PacketDispatch (0x00429690)
void  __cdecl ReceiveCreateTransformViewport(int, int, int); // IDA: ReceiveCreateTransformViewport (0x00429C50)
void  __cdecl ReceiveCreateMonsterViewport(int, int, int); // IDA: ReceiveCreateMonsterViewport (0x0042A230)
void  __cdecl ReceiveCreateSummonViewport(int, int, int); // IDA: ReceiveCreateSummonViewport (0x0042A530)
void  __cdecl ReceiveAttackDamage(int, int, int); // IDA: ReceiveAttackDamage (0x0042ACC0)
void  __cdecl ReceiveAction(int, int, int); // IDA: ReceiveAction (0x0042B4F0)
void  __cdecl SetPlayerBow(int, int, int); // IDA: FUN_0042BC00 (0x0042BC00)
void  __cdecl Skills_PacketHandler(int opcode, int data_ptr, int len); // IDA: Skills_PacketHandler (0x0042BCA0)
void  __cdecl ReceiveMagicContinue(int, int, int); // IDA: ReceiveMagicContinue (0x0042CD10)
void  __cdecl ReceiveMagicPosition(int, int, int); // IDA: ReceiveMagicPosition (0x0042D780)
void  __cdecl ReceiveDieExp(int, int, int); // IDA: ReceiveDieExp (0x0042DB60)
void  __cdecl ReceiveDieExpLarge(int, int, int); // IDA: ReceiveDieExpLarge (0x0042E5C0)
void  __cdecl ReceiveDie(int, int, int); // IDA: ReceiveDie (0x0042F030)
void  __cdecl ReceiveCreateItemViewport(int, int, int); // IDA: ReceiveCreateItemViewport (0x0042F240)
void  __cdecl ReceiveGetItem(int, int, int); // IDA: ReceiveGetItem (0x0042F360)
void  __cdecl ReceiveDropItem(int, int, int); // IDA: ReceiveDropItem (0x0042F690)
void  __cdecl ReceiveEquipmentItem(int, int, int); // IDA: ReceiveEquipmentItem (0x0042F9A0)
void  __cdecl InitGuildWar(void); // IDA: FUN_00423DB0 (0x00423DB0)

// ── Party / Guild ─────────────────────────────────────────────────────────────
void  __cdecl ReceiveTalk(int, int, int); // IDA: ReceiveTalk (0x004301B0)
void  __cdecl ReceiveLevelUp(int, int, int); // IDA: ReceiveLevelUp (0x00431180)
void  __cdecl ReceiveAddPoint(int, int, int); // IDA: ReceiveAddPoint (0x00431480)
void  __cdecl ReceiveLife(int, int, int); // IDA: ReceiveLife (0x00431780)
void  __cdecl ReceiveMana(int, int, int); // IDA: ReceiveMana (0x00431A90)
void  __cdecl ReceivePK(int, int, int); // IDA: ReceivePK (0x00431DC0)
void  __cdecl ReceiveDurability(int, int, int); // IDA: ReceiveDurability (0x00431EA0)
void  __cdecl ReceiveHelperItem(int, int, int); // IDA: ReceiveHelperItem (0x004321F0)
void  __cdecl ReceiveTradeResult(int, int, int); // IDA: ReceiveTradeResult (0x004332E0)
void  __cdecl ReceiveTradeExit(int, int, int); // IDA: ReceiveTradeExit (0x004337F0)
void  __cdecl Party_PacketHandler(int opcode, int data_ptr, int len); // IDA: Party_PacketHandler (0x00433900)
void  __cdecl ReceiveGGAuth(int, int, int); // IDA: ReceiveGGAuth (0x00433A80)

// ── Protocolo Trade / Shop / Guild ────────────────────────────────────────────
void  __cdecl ReceiveStorageGold(int, int, int); // IDA: ReceiveStorageGold (0x00434170)
void  __cdecl ReceiveStorageExit(int, int, int); // IDA: ReceiveStorageExit (0x00434400)
void  __cdecl ReceiveStorageStatus(int, int, int); // IDA: ReceiveStorageStatus (0x00434450)
void  __cdecl ReceivePartyList(int, int, int); // IDA: ReceivePartyList (0x00434660)
void  __cdecl ReceiveGuildResult(int, int, int); // IDA: ReceiveGuildResult (0x00434780)
void  __cdecl ReceiveGuildList(int, int, int); // IDA: ReceiveGuildList (0x004348B0)
void  __cdecl ReceiveGuildLeave(int, int, int); // IDA: ReceiveGuildLeave (0x00434950)
// IDA: FUN_00434DC0 — GuildMark_UpsertRecord (nombre + tabla de mark 8x8); no pertenece a Trade.
void  __cdecl Trade_PacketHandler(int opcode, int data_ptr, int len); // IDA: Trade_PacketHandler (0x00434DC0)
void  __cdecl ReceiveCreateGuildResult(int, int, int); // IDA: ReceiveCreateGuildResult (0x00435280)
// Guerra de guild: ReceiveDeclareWar, ReceiveDeclareWarResult,
// ReceiveGuildBeginWar y ReceiveGuildEndWar.
void  __cdecl ReceiveDeclareWar(int, int, int); // IDA: ReceiveDeclareWar (0x00435390)
void  __cdecl ReceiveDeclareWarResult(int, int, int); // IDA: ReceiveDeclareWarResult (0x004353E0)
void  __cdecl ReceiveGuildBeginWar(int, int, int); // IDA: ReceiveGuildBeginWar (0x004354F0)
void  __cdecl ReceiveGuildEndWar(int, int, int); // IDA: ReceiveGuildEndWar (0x00435AA0)
void  __cdecl ReceiveServerCommand(int, int, int); // IDA: ReceiveServerCommand (0x00436550)
void  __cdecl ReceiveMix(int, int, int); // IDA: ReceiveMix (0x004366C0)
void  __cdecl ReceiveMixExit(int, int, int); // IDA: ReceiveMixExit (0x004367D0)
void  __cdecl ReceiveMoveToDevilSquareResult(int, int, int); // IDA: ReceiveMoveToDevilSquareResult (0x00436820)
void  __cdecl ReceiveDevilSquareRank(int, int, int); // IDA: ReceiveDevilSquareRank (0x00436A80)
void  __cdecl ReceiveMoveToEventMatchResult(int, int, int); // IDA: ReceiveMoveToEventMatchResult (0x00436AC0)
void  __cdecl ReceiveEventZoneOpenTime(int, int, int); // IDA: ReceiveEventZoneOpenTime (0x00436CB0)
void  __cdecl ReceiveSetAttribute(int, int, int); // IDA: ReceiveSetAttribute (0x00436D60)
void  __cdecl ReceiveMatchGameCommand(int, int, int); // IDA: ReceiveMatchGameCommand (0x00436E40)
void  __cdecl ReceiveOption(int, int, int); // IDA: ReceiveOption (0x00436FB0)
void  __cdecl ReceiveEventChipInfomation(int, int, int); // IDA: ReceiveEventChipInfomation (0x004372C0)
void  __cdecl ReceiveEventChip(int, int, int); // IDA: ReceiveEventChip (0x00437380)
void  __cdecl ReceiveMutoNumber(int, int, int); // IDA: ReceiveMutoNumber (0x004373A0)
void  __cdecl ReceiveServerImmigration(int, int, int); // IDA: ReceiveServerImmigration (0x004373D0)
void  __cdecl ReceiveScratchResult(int, int, int); // IDA: ReceiveScratchResult (0x00437400)
// ── Guild UI helpers (called from guild packet handlers) ─────────────────────
void  __cdecl CreateOkMessageBox(char *strMsg);                   // 0x0051D6F0 — CreateOkMessageBox (Offsets.h canonical name)
void  __cdecl GuildMemberList_Update(int count, int param2, void *data);    // GuildMemberList_Update — full member list
void  __cdecl GuildMemberList_Add(int param1, void *data);               // GuildMemberList_Add — single member record
void  __cdecl ReceiveQuestHistory(int, int, int); // IDA: ReceiveQuestHistory (0x00437450)
void  __cdecl ReceiveQuestState(int, int, int); // IDA: ReceiveQuestState (0x00437480)
void  __cdecl ReceiveQuestResult(int, int, int); // IDA: ReceiveQuestResult (0x004374B0)
void  __cdecl ReceiveQuestPrize(int, int, int); // IDA: ReceiveQuestPrize (0x004374E0)
void  __cdecl ReceiveUseStateItem(int, int, int); // IDA: ReceiveUseStateItem (0x00437F10)
void  __cdecl InsertBuffPhysicalEffect(int, int, int); // IDA: InsertBuffPhysicalEffect (0x0043BDE0)
void  __cdecl ClearBuffPhysicalEffect(int, int, int); // IDA: ClearBuffPhysicalEffect (0x0043C070)
void* __cdecl STRUCT_ENCRYPT(void *ctx, void *chardata); // IDA: STRUCT_ENCRYPT (0x0043D1D0)
// FUN_0043d3e0: 2-arg form — (HashTable*, int*) used in Ring_ComputeOrbit (see line 840)
void  __cdecl HashTable_Insert_Short(void *ctx, void *out);  // HashTable_Timer — updates out from ctx
int   __fastcall CWsctlc_GetSocket(int);   // IDA: CWsctlc::GetSocket (0x0043DCC0)
int   __fastcall CWsctlc_FDWriteSend(int); // IDA: sub_43DDD0; 5.2: CWsctlc::FDWriteSend (0x0043DDD0)
void  __cdecl FUN_0043de60(void);  // Net_FlushQueue (no args)
void  __cdecl CPacketQueue_PushPacket(int, int, int); // IDA: sub_43DF90; 5.2: CPacketQueue::PushPacket (0x0043DF90)
int   __fastcall CWsctlc_nRecv(void*);                // IDA: sub_43DE70; 5.2: CWsctlc::nRecv (0x0043DE70)
int   __fastcall CWsctlc_GetReadMsg(int);             // IDA: CWsctlc::GetReadMsg (0x0043E010)
float __cdecl CreateAngle(float x1, float y1, float x2, float y2); // IDA: Movement_Tick (0x0043E050)
float __cdecl FarAngle(float curAngle, float tgtAngle, char mode); // IDA: FUN_0043e370 (0x0043E370)
void  __cdecl RenderInformation(void);
float __cdecl TurnAngle2(float curAngle, float tgtAngle, float t); // IDA: TurnAngle2 (0x0043E1B0)
void  __cdecl Vector_AddRotated(float *pos, float *dir, float *vel); // Vector3_Normalize (cross/normalize vel from pos+dir)
void* __cdecl SetAction(int entity_ptr, int anim_id); // IDA: SetAction (0x0043E820)
void  __cdecl Alpha(int entity_ptr); // IDA: Alpha (0x0043E5C0)
void  __cdecl FUN_0043e680(int entity_ptr, int slot_idx, int table_base, int stride); // Particle_PathUpdate
// Pathfind(src_cached_x, src_cached_y, tgt_x, tgt_y, path_buf, reserved) → 1=ok 0=fail
unsigned int __cdecl Path_FindRoute(int src_x, int src_y, int tgt_x, int tgt_y, unsigned char *path, float reserved); // IDA: PathFinding2
// PathFinder2_Solve: A* grid search — implemented in Game/PathFinder.cpp
uint  __cdecl PATH_FindPath(void *_this, int sx, float sy, int tx, int ty, int filter, int walkmax, float radius); // IDA: PATH_FindPath (0x0043F500)
// A* BST priority queue helpers — implemented in Game/PathFinder.cpp
undefined4 __cdecl PATH_AddClearPos(void *_this, int idx); // IDA: PATH_AddClearPos (0x0043FD30)
void  __cdecl CBTree_Add(void *_this, undefined4 data, int cost); // IDA: CBTree_Add (0x0043FEA0)
undefined4 __cdecl CBTree_RemoveNode(void *_this, int *pnode); // IDA: CBTree_RemoveNode (0x004232F0)
void  __cdecl CBTree_RemoveFrom(void *_this, undefined4 *node); // IDA: CBTree_RemoveFrom (0x004235D0)
void  __fastcall CBTree_RemoveAll(undefined4 *open_set); // IDA: CBTree_RemoveAll (0x0043FF60)
void  __cdecl Timer_UpdateFrameTiming(void); // IDA: CalcFPS

// ── Model render helpers (FUN_004400xx / FUN_004410xx / FUN_004414xx) ─────────
// Called on model context pointer (this = DAT_05828d58 + type*0xbc).
void  __cdecl BMD__BeginRender(void);                                  // Model_PushMatrix (glPushMatrix-like setup)
// Model_RenderLayer(model, layer, anim_id, scale, height, x, y, z, color)
void  __cdecl BMD__RenderMesh(void *model, float layer, int anim_id, float scale, int height,
                            float x, float y, float z, unsigned int color);
// Model_Render(model, param2, param3, anim_mode, flags, extra)  — 6-arg form used in Entity_DrawSetup
void  __cdecl BMD__RenderBody(void *model, int flags, float f1, int f2, float f3, float f4, float f5, int f6, int rgba);
void  __cdecl BMD__RenderMeshEffect(void *model, int a, int b);             // Model_KillAnim(model, 0, anim_slot)
void  __cdecl BMD__RenderBodyShadow(void *model, int height_int, int anim); // Model_RenderShadow(model, height, anim)
void  __cdecl BMD__Release(int model_ptr);                         // Model_Unload (free BMD model data)
void  __cdecl BMD__FindNearTriangle(void *model);                           // BMD_PostActionsInit (post-load action setup)
void  __cdecl BMD__Init(void *model, char flag);                // BMD_PostLoadInit (post-load finalize)
void  __cdecl BMD__FindTriangleForEdge(void *pThis, int param_1, int param_2, int param_3);  // BMD_BuildAdjacentFaceTable
void  __cdecl BMD_CreateBoundingBox(int param_1);                           // BMD_ComputeBounds
// VTable helpers used by type 0x13f/0x14a entity dispatch
void* __fastcall WidgetB_CtorFull(void *param_1);                      // WidgetB_CtorFull
void* __fastcall WidgetB_Ctor(void *param_1);                      // WidgetB_Ctor
void  __fastcall WidgetB_SetVtable(void *param_1);                      // WidgetB_SetVtable
void  __fastcall WidgetB_ZeroFields(int param_1);                        // WidgetB_ZeroFields
void  __cdecl VTable_Release(void *obj, int vtable_id);              // VTable_Release
void  __cdecl FUN_0040a8f0(void *obj, float *p1, float *p2);      // VTable_DrawEdge

// ── Sprite / Animation ────────────────────────────────────────────────────────
// IDA: BMD_Animation (0x00440060)
void  __cdecl BMD_Animation(void *model, int flags, float rot, unsigned int pos_param,
                            unsigned char scale, unsigned int *anim_ptr, float *dir_ptr,
                            char a, char b);
void  __cdecl Skeleton_Transform(void *model, int bone, float *anim1, float *anim2, float *anim3, char blend); // Sprite_DrawBone
void  __cdecl BMD_TransformPosition(void *model, float *bone_data, float *out_pos, float *out_col, char flag); // IDA: TransformPosition (0x004409A0)
void  __cdecl BMD__RotationPosition(void *model, float *bone_mat, float *pos_in, float *pos_out); // BoneTransformOffset — implemented in Render/BMD_SetupRender.cpp
int   __cdecl ItemObjectAttribute(int param_1); // IDA: ItemObjectAttribute (0x00502BA0)
void  __cdecl BMD__Open(int, int, int, int);   // IDA: BMD__Open (0x004423E0)
undefined4 __cdecl BMD__Save(int thisModel, char *pathDir, char *pathFile);  // BMD_SaveToFile
void  __cdecl SetPlayerStop(int entity); // IDA: SetPlayerStop (0x004430C0)
void  __cdecl SetPlayerWalk(int entity); // IDA: SetPlayerWalk (0x00443930)
unsigned int __cdecl Entity_AdvancePath(void *entity, char flag); // IDA: MovePath
void  __cdecl MoveCharacterPosition(int entity); // IDA: MoveCharacterPosition (0x00454BA0)
void  __cdecl SetCharacterClass(int entity); // IDA: SetCharacterClass (0x0045C130)
void  __cdecl Character_UpdateEquipSlotAnimations(int entity);        // Character_UpdateEquipSlotAnimations
void  __cdecl SetAttackSpeed(void);              // IDA: SetAttackSpeed (0x00443E70)
void  __cdecl SetPlayerAttack(int, int, int, int); // IDA: SetPlayerAttack (0x00444410)
void  __cdecl SetPlayerMagic(int param_1); // IDA: SetPlayerMagic (0x00444A80)
void  __cdecl SetPlayerTeleport(int param_1);       // SetPlayerTeleport - anim de casteo de teleport (accion 87 / 5)
void  __cdecl SetPlayerShock(int c, int Hit); // IDA: SetPlayerShock (0x00444B60)
void  __cdecl SetPlayerDie(int entity_idx); // IDA: SetPlayerDie (0x00444D90)
// ── Character/Effect update pool (0x004796xx) ────────────────────────────────
void  __cdecl Render_DrawSprite(int slot);               // IDA: RenderSprite
void  __cdecl Render_DrawSpritePool(void);               // IDA: RenderSprites
void  __cdecl CheckSprites(void);              // Effect_UpdateAll (mark all active)
// GL_SetBlendSrcOver / GL_SetBlendAdditive / GL_SetBlendSrcAlpha — declared below (GL state / sound)
extern "C" int __cdecl GetScreenWidth(void); // IDA: GetScreenWidth (0x004CB520)

// ── Entity update / render core ───────────────────────────────────────────────
void  __cdecl MoveCharacterClient(float *); // IDA: MoveCharacterClient (0x00454FC0)
void* __cdecl RenderCharacter(void *entity, void *vis_entity, void *param_3); // IDA: RenderCharacter (0x00456770)
void* __cdecl Entity_SpawnBoneEffect(int entity, int type, float scale, int bone_count, float angle, int flag1, float flag2); // Entity_BoneAllParticles
void* __cdecl Entity_SpawnBoneRangeEffect(int entity, int bone_start, int bone_end, float scale); // Entity_BoneRangeParticles
void  __cdecl Entity_RenderAll_3D(void); // IDA: Entity_RenderAll_3D (0x0045AB00)

// ── Entity render helpers (used by RenderCharacter / Entity_UpdateRender) ───────
void  __cdecl FUN_00504130(void *model, int entity, int type, float alpha, uint draw_flags); // Entity_DrawSetupBase — implemented in Render/BMD_SetupRender.cpp
void* __cdecl RenderPartObjectBodyColor(void *model, int entity, int entity_type, float scale, int flags, float alpha, int rgba); // IDA: RenderPartObjectBodyColor (0x00504960)
void* __cdecl Entity_SetModelColorAlt(void *model, int entity, int entity_type, float scale, int flags, float alpha, int rgba); // Model_RenderSkeleton2
void  __cdecl RenderLinkObject(float ox, float oy, float oz, int entity, int weapon_slot_ptr, int anim_id, char level, unsigned int option1, char link, char translate, unsigned int render_type); // IDA: RenderLinkObject (0x00455430)
void  __cdecl DeleteCloth(int entity_a, int entity_b, int flags); // IDA: DeleteCloth (0x00449840)
bool  __cdecl CheckFullSet(int entity); // IDA: CheckFullSet (0x00451B20)
extern int EquipmentLevelSet;                                         // 0=no glow, 9-11=lightning tier
extern int g_bAddDefense;                                             // set completo → bonus de defensa en FUN_0047e160
float __cdecl RequestTerrainHeight(float xf, float yf); // IDA: RequestTerrainHeight (0x004F7500)
void  __cdecl RequestTerrainLight(float grid_x, float grid_y, float *rgb_out); // IDA: RequestTerrainLight (0x004F7960)
void  __cdecl Model_BoneParticle(void *model, int type, int bone_idx, float scale, float *color, int entity); // Model_BoneParticle
void  __cdecl PartObjectColor(int weapon_id, float scale, float half_scale, float *color, char flag); // IDA: PartObjectColor (0x00503CF0)
void  __cdecl Weapon_SetColorAlt(int weapon_id, float scale, float half_scale, float *color);           // Weapon_SetColorAlt: simpler version (no flag)
// IDA: FUN_004552C0 — compone la matriz del emblema de guild sobre un jugador visible.
void  __cdecl FUN_004552c0(int entity, int shield_id);
// IDA: FUN_004F0100 — CreateGuildMark, compone la textura 34 desde la tabla compartida de marks.
extern "C" void __cdecl CreateGuildMark(int mark_index, bool blend);
void  __cdecl RenderTerrainAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, float *Light, float Rotation, float Alpha); // IDA: RenderTerrainAlphaBitmap (0x004F8BB0)

// ── Skill/weapon widget system (UI overlay beams) ─────────────────────────────
void* __fastcall Widget_Ctor(void *param_1); // IDA: Widget_Ctor (0x004093A0)
void* __fastcall Widget_CtorBase(void *param_1); // IDA: Widget_CtorBase (0x00407FE0)
void  __cdecl L_YGXPAXIHP6EX0_Z1_Z(void *arr, int size, int elem, void *ctor); // Array_InitWithCtor
void  __cdecl SpringMesh_Create(void *widget, int entity, short *slot, int type, int radius, int flags); // Widget_BindEntity
void  __cdecl VerletNode_AddToSystem(void *widget, float x, float y, float z, float radius, int boneIdx); // Cloth_AddAnchor
int   __cdecl Widget_CheckState(int *widget, unsigned int hash, int flags); // IDA: Widget_CheckState (0x00408900)
void  __cdecl GridSpring_Create(void *widget, float entity, int p3, float p4, float p5, int p6, int p7, float p8, float p9, int type_a, int type_b, int flags); // Widget_Init
extern "C" void __cdecl DeleteCharacter(int Key); // IDA: DeleteCharacter (0x0045AC20)
int   __cdecl FindCharacterIndex(int); // IDA: FindCharacterIndex (0x0045AC80)
void  __cdecl CreateCharacterPointer(unsigned char *c, int Type, int PositionX, int PositionY, float Rotation); // CreateCharacterPointer
unsigned int __cdecl CreateCharacter(int Key, int Type, unsigned char PosX, unsigned char PosY, float Rotation); // IDA: CreateCharacter (0x0045BFA0)
void  __cdecl ChangeCharacterExt(int Key, BYTE *Equipment); // ChangeCharacterExt
char* __cdecl CreateMonster(uint, int, int, int, int); // IDA: CreateMonster (0x0045CCF0)
unsigned char* __cdecl CreateHero(int Index, int Class, int Skin, float x, float y, float Rotate); // CreateHero
void  __cdecl Monster_SaveSetBase(const char*); // SaveMonsters — writes MonsterSetBase position records
float* __cdecl CreateEffect(int type, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, byte flag); // IDA: CreateEffect (0x00460DC0)
// Tamachan (0.98j, efecto 193): control por el opcode 0x0B tipo 2.
void __cdecl Tamachan_Clear(void);     // IDA 0.98j: sub_46C220
void __cdecl Tamachan_Spawn(void);     // IDA 0.98j: sub_46C190
void __cdecl Tamachan_Dismiss(void);   // IDA 0.98j: sub_46C250
void  __cdecl Effect_SpawnSmokeBurst(float *, char); // IDA: FUN_004660f0
void  __cdecl Effect_SpawnSmokeExplosion(undefined4 *, char); // IDA: FUN_004661f0
void  __cdecl Effect_SpawnLightningBurst(int); // IDA: FUN_00460c30
void  __cdecl Effect_SpawnProximityHit(int); // IDA: FUN_00465e60
void  __cdecl FUN_00466440(int);                        // STUB: HashTable obfuscation — not portable
void  __cdecl Ring_ComputeOrbit(int, float *, float);        // Ring_ComputeOrbit

// ── Particle / effect system ──────────────────────────────────────────────────
void  __cdecl Effect_TickAll(void); // IDA: MoveEffects
void  __cdecl EffectPool_RenderAll(void); // IDA: RenderEffects
// 2026-05-07: Particle_Render real es void per IDA mu97k-src-IDA/raw/
// 0046BE40_Particle_Render.c. La firma anterior (6 args) era erronea — el
// llamador en Game_RenderTick lo invoca sin args.
void  __cdecl Particle_RenderAll(void); // IDA: FUN_0046be40
void  __cdecl Trail_RenderAll(void); // IDA: Trail_RenderAll (0x0046C3E0)
void  __cdecl SkillEffects_RenderAll(void); // IDA: FUN_0046cb70
void  __cdecl WeatherParticles_Update(void); // IDA: MoveLeaves
void* __cdecl Joint_Create(int, float *, float *, float *, unsigned int, int, float, short, unsigned char); // IDA: CreateJoint
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void* __cdecl CreateJoint(int, float *, float *, float *, unsigned int, int, float, short, unsigned char); // IDA: CreateJoint

// ── Item drop render ──────────────────────────────────────────────────────────
void  __cdecl Joint_TickAll(void); // IDA: MoveJoints
void  __cdecl ItemDrop_Render(void); // IDA: ItemDrop_Render (0x00473710)
void  __cdecl FUN_00473ea0(int, float *, unsigned int, unsigned int, unsigned int, float, unsigned int, float); // Particle_Spawn
void  __cdecl CreateMagicShiny(int, int, int, int);   // IDA: CreateMagicShiny (0x004741E0)

// ── Player render ─────────────────────────────────────────────────────────────
void  __cdecl RenderPlane(int char_class, float *pos, float *rot, float scale); // Player_DrawInstance
void  __cdecl Effect_TickFade(void); // IDA: MovePlanes
void  __cdecl ItemDrop_SetupRenderRef(int particle_slot);                      // Particle_InitDir — sets dir vector from entity pos delta
int   __cdecl Particle_Spawn(int type, float *bone_mat, float *pos, float *size, int flag, float alpha, int mode); // IDA: FUN_00475220

// ── Char list / spawn ─────────────────────────────────────────────────────────
void  __cdecl RenderParticles(void);   // IDA: RenderParticles (0x00478C00)
void  __cdecl RenderPoints(int, int, int, int);   // IDA: RenderPoints (0x00479330)
void  __cdecl DamageNumbers_Tick(void); // IDA: MovePoints
void  __cdecl Effect_TickFlare(void); // IDA: MovePointers
int   __cdecl CreateSprite(unsigned short, float *, float, float *, int, float, int); // IDA: CreateSprite (0x004795C0)
// Render_DrawSprite / Render_DrawSpritePool / CheckSprites — implemented in src/stubs.cpp (Character/Effect pool)
void  __cdecl ItemConvert(int, int, int);
int   __cdecl FUN_0047cf40(short* a1, int a2, int a3, unsigned short a4);  // Stats_ApplyBonus2
void  __cdecl CHARACTER_MACHINE_Init(int, int, int);   // IDA: CHARACTER_MACHINE_Init (0x0047D3D0)
// Stat helpers — ported 2026-05-02. Signatures match IDA decompile.
int   __fastcall FUN_0047d410(int characterMachine);                // Stats_CalcBase (attack damage)
int   __cdecl    FUN_0047dae0(int characterMachine);                // Stats_CalcMagicDmgRange
int   __cdecl    FUN_0047dd50(short* characterMachine_words);       // Stats_CalcAddStrength
int   __cdecl    CalculateAttackSpeed(int characterMachine); // IDA: CHARACTER_MACHINE::CalculateAttackSpeed (0x0047DD80)
int   __cdecl    FUN_0047dfe0(int characterMachine);                // Stats_CalcDefense
int   __cdecl    FUN_0047e160(int characterMachine);                // Stats_CalcCritBase
int   __cdecl    FUN_0047e2e0(short* characterMachine_words);       // Stats_CalcExtraOption1
int   __cdecl    FUN_0047e310(int characterMachine);                // Stats_CalcExtraOption2
int   __cdecl    CalculateAll(int characterMachine, int, int); // IDA: FUN_0047E3C0 (0x0047E3C0)
void  __fastcall CalculateNextExperince(int characterMachine); // IDA: FUN_0047E350 (0x0047E350)
void  __cdecl StartMatchCountDown(int, int, int); // IDA: StartMatchCountDown (0x0047EC00)
int   __cdecl PressKey(int); // IDA: PressKey (0x0047EC20)
void  __cdecl ClearInput(int mode); // IDA: ClearInput (0x0047EC60)
// FUN_0047F0B0 @ 0x0047F0B0
void  __cdecl UI_RenderInputField(int, unsigned int, int);
// FUN_0047F650 @ 0x0047F650
undefined8  __cdecl UI_RenderText(undefined4 x, undefined4 y, LPCSTR str, LPSIZE max_w, char bold, undefined4 extra);
void  __cdecl FUN_0047f6f0(int, int, int);
// FUN_0047F7A0 @ 0x0047F7A0
void  __cdecl UI_DrawText(int x, int y, char* text, int max_width, int style, int extra);
// FUN_0047FAE0 @ 0x0047FAE0
void  __cdecl UI_AddNotice(char *, unsigned char);
void  __cdecl Chat_TickNoticeTimer(void);                // IDA: MoveNotices
// FUN_0047FCE0 @ 0x0047FCE0
void  __cdecl UI_RenderNotices(void);
void  __cdecl UIChatLogWindow_AddText(const char* strID, const char* strText, int MsgType); // IDA: UIChatLogWindow_AddText (0x00480620)
#define UI_ShowDialog UIChatLogWindow_AddText
void  __cdecl Chat_TickMessageTimer(void);               // IDA: FUN_00480950
// FUN_00480980 @ 0x00480980
void  __cdecl UI_RenderChatLogOverlay(void);
void  __cdecl FUN_00480c60(int, int, int);
void  __cdecl RenderBoolean(int x, int y, DWORD c);   // IDA: RenderBoolean (0x00480E00)
void  __cdecl CreateChat(char* ID, char* Text, DWORD entity, int Flag, int SetColor); // CreateChat
// FUN_004821A0 @ 0x004821A0
void  __cdecl UI_TickHoverBubbles(void);
int   __cdecl Item_FindQuickSlotByCategory(int category);  // GetItemSlot — returns inventory slot index or -1
char  __cdecl Item_Equip(DWORD character, DWORD object);            // 0x00483B30, Attack selected-skill executor

// ── UI / HUD ──────────────────────────────────────────────────────────────────
bool  __cdecl FindTextA(char* src, char* pattern, bool caseSensitive); // IDA: FindText (0x004977F0)
void  __cdecl CutText(void*, int, void*, int);            // Chat_SplitLine
void  __cdecl SetActionClass(int, int, int, int);
void  __cdecl CheckGate(void); // IDA: CheckGate (0x004AC140)
char  __cdecl Path_IsLineClear(int src_x, int src_y, int tgt_x, int tgt_y); // IDA: CheckWall
// IDA: SendMove
void  __cdecl Combat_SendMovePathPacket(int entity, int entity2);                 // sends the local hero's path packet
// IDA: Attack
void  __cdecl Combat_DispatchHeroSkillAttack(void *entity);                       // hero skill/attack dispatcher
// IDA: CheckArrow
char  __cdecl Combat_CheckArrowRequirement(void);                                  // validates arrow/bolt availability
// IDA: Action (0x0048D640)
// Despachador real de acciones (pickup/equip/attack/skill/walk) basado en
// `*(c+749)` queue. Implementación en stubs.cpp.
// IDA: FUN_0048d640
void  __cdecl Action(DWORD c, DWORD o);
unsigned int __cdecl CheckAttack(void); // IDA: CheckAttack (0x00483160)
void  __cdecl Player_ProcessInput(void); // IDA: Player_InputTick
void  __cdecl Mouse_UpdateHoverTargets(void);                                    // IDA: FUN_004b0310
char  __cdecl SelectSkillByHotkey(int number);                              // SelectSkillByHotkey (0x4B0E80)
void  __cdecl Chat_InputTick(void);
void  __cdecl Cursor_Render(void);                                               // IDA: RenderCursor
void  __cdecl Input_ProcessFunctionKeys(void);      // IDA: CheckFunctionButtons
void  __cdecl UI_UpdateFpsCounter(void);                                         // IDA: RenderDebugWindow
void  __cdecl RenderHelpWindow(void);
unsigned int __cdecl Item_CalculateMaxDurability(void* item, int attrBase, int level); // IDA: FUN_004C45C0
int          __cdecl Item_CalculateValue(void* item, int sellMode);                    // IDA: FUN_0047C690
unsigned int __cdecl Item_CalculateRepairCost(int Gold, int Durability, int MaxDurability, short Type, char* Text); // IDA: FUN_004C3EF0
// GetScreenWidth — declared above as int(void) GetLoginAnimTime
void  __cdecl FUN_004cb6f0(int, int, int, int);
void  __cdecl CloseInventoryRelatedWindows(void); // IDA: CloseInventoryRelatedWindows (0x004CBA60)
char  __cdecl Connection_Check(void *ctx, int p1, int p2);              // Connection_Check → '\0'=ok else error
// IDA sub_494520(texto, 1): valida/consume el buffer de texto; 0 = seguir.
// 2026-09-24: la firma tenia un tercer parametro inventado (el estado de
// Enter); los dos unicos call sites son el tick de macros y pasan 2 args.
DWORD __cdecl FUN_00494520(void *buf, char flag);
char  __cdecl Chat_Validate(char *buf);                               // Chat_Validate — validate text buf; '\0'=ok
void  __cdecl CheckChatText(char* text);                             // IDA: sub_497C70 (0x00497C70) gestos por texto del chat
void  __cdecl SendRequestAction(BYTE actionType); // [C1][05][18][dir][tipo], inline en IDA
char* __cdecl GetMapName(int iMap); // IDA: GetMapName (0x004EF120)
void  __cdecl ClearInventory(void); // IDA: ClearInventory (0x004CBDF0)
extern "C" void __cdecl Item_TradeHistoryAdd(int slot, BYTE* pool);  // IDA: sub_4CC530 (0x004CC530)
extern "C" void __cdecl InsertInventoryItem(BYTE* Inv, int Width, int Height,
                                              int Index, BYTE* Item, bool First); // IDA: InsertInventoryItem (0x004CC660)
void  __cdecl Item_ReturnPickedItem(void);  // CharPreview_Refresh (no args per call-site)

// ── 3D / terrain / world ──────────────────────────────────────────────────────
void  __cdecl RenderObjectScreen(int effect_id, unsigned int type, unsigned char class_id, unsigned char ext_option, float *pos, int in_bounds, char flag); // ItemDrop_SpawnEffect
int   __cdecl SecondPassword_Shuffle(int mode);            // SecondPassword_Shuffle (10-element array shuffle)
void  __cdecl UpdateWindowsMouse(void); // IDA: UpdateWindowsMouse (0x004ECB00)
// FUN_004f0100 — declared above as (int entity_id, char flag) NPC_SetFlag
void  __cdecl FUN_004f5ce0(int, int, int, int);
void  __cdecl FUN_004f6420(int, int, int, int);
void  __cdecl Scene_MapTick(void);   // IDA: Scene_MapTick (0x004F64D0)
int   __cdecl TERRAIN_INDEX(int grid_x, int grid_y); // IDA: TERRAIN_INDEX (0x004F6C30)
int   __cdecl Terrain_GetTileIndex(unsigned int, unsigned int);            // IDA: Terrain_GetTileIndex (0x004F6C40)
void  __cdecl Terrain_SetTileAttributeBits(int, int, int); // IDA: FUN_004f6ef0
void  __cdecl Terrain_ClearTileAttributeBits(int, int, int); // IDA: SubTerrainAttribute
void  __cdecl Terrain_UpdateTileAttributeRect(int, int, int, int, int, int); // IDA: FUN_004f6f30
void  __cdecl FUN_004f8740(float x, float y, float scale, int flags, int corners_ptr, char blend, float alpha); // Particle_DrawTile
void  __cdecl FUN_004f8980(int, int, int, float);  // Terrain_SpawnObject(type, x, y, height)
// RenderTerrainAlphaBitmap — Particle_Draw (see declaration above in Entity render section)
void  __cdecl CreateFrustrum2D(float *cam_pos); // IDA: CreateFrustrum2D (0x004F8EB0)
unsigned short __cdecl TestFrustrum2D(float x, float y, float z);    // Frustum_IsVisible

// ── Camera ────────────────────────────────────────────────────────────────────
// FUN_004F9590
int   __cdecl Frustum_TestSphere(float *pos, float cull_dist);     // LOD/visibility cull
int   __cdecl Terrain_Water(void);  // IDA: Terrain_Water (0x004F95E0)
void  __cdecl RenderTerrain(char flag); // IDA: RenderTerrain (0x004F9AC0)
int   __cdecl Terrain_QuadEqual(int, int, int, int);
float __cdecl Vec3_Length(float *vec);  // Vec3_Length (physics variant — returns length, does NOT normalize)
void  __cdecl VectorMA(float *cam_pos, float factor, float *in_rel, float *out_pos); // Camera_ProjectRelative
void  __cdecl R_ConcatTransforms(float *parent, float *rot, float *out);  // Bone_CombineMatrices
void  __cdecl Matrix_BuildFromEuler(float *angles, float *out_mat12);       // Matrix_FromEuler
float* __cdecl Vector_Rotate(float *pt, float *mat12, float *out);    // Matrix_TransformPoint
void  __cdecl Vector_Transform(float *pt, float *mat12, float *out);    // Matrix_TransformPoint (alt)
void  __cdecl Vector_InverseRotate(float *vec, float *mat12, float *out);   // Vec3_Transform
void  __cdecl EulerToQuat(int, int, int, int);
void  __cdecl QuatToMatrix(int, int, int, int);
void  __cdecl QuatSlerp(int, int, int, int);
void  __cdecl Triangle_ComputeNormal(float *origin, float *A, float *B, float *out_normal); // Vec3_ComputePlaneNormal
void  __cdecl SetActionObject(int, int, int, int); // IDA: SetActionObject (0x004FA5C0)
void  __cdecl Entity_GetLightScale(int entity, int model);  // Entity_ProjectToScreen
int   __cdecl Calc_RenderObject(int entity, char param2, int param3);   // IDA: Calc_RenderObject (0x004FAA70)
void  __cdecl Draw_RenderObject(void *, int, int, char); // Entity_SetupGL

// ── Entity render pipeline ────────────────────────────────────────────────────
void Entity_PrepareRender(void *entity, int slot, int flag, char mode);  // IDA: Entity_PrepareRender (0x004FC030) — firma identica a Render/Entity_PrepareRender.h
float* __cdecl Entity_SpawnEffects(int entity); // IDA: Entity_SpawnEffects (0x004FC070)
void  __cdecl Terrain_Render(void);              // Terrain_Render
void *__cdecl CreateObject(int type, float *world_pos, float *target_pos, float extra); // IDA: CreateObject (0x004FF5A0)
void *__cdecl Entity_InitRenderState(void *entity);                           // Entity_InitRenderState
void  __cdecl DeleteBug(DWORD Owner); // IDA: DeleteBug (0x004FFFA0)
void  __cdecl CreateBug(int, void *, void *, int); // IDA: CreateBug (0x004FFFD0)
uint  __cdecl RenderBugs(void);  // returns entity count
void  __cdecl DeleteBoids(int, int, int, int); // IDA: DeleteBoids (0x00500A80)
void  __cdecl RenderBoids(void);   // RenderBoids (decoration/animal entity tick+render)
uint  __cdecl Weather_Update(void); // IDA: Weather_Update (0x00500E80)
void  __cdecl RenderFishs(int, int, int, int); // IDA: RenderFishs (0x00502200)
void  __cdecl AmbientParticles_Update(void); // IDA: Ambient_ParticleUpdate
void  __cdecl CreateItem(int, int, int, int);   // IDA: CreateItem (0x005032F0)
void  __cdecl MoveItems(void); // IDA: MoveItems (0x00503760)
void  __cdecl Entity_SetGravity(int entity_class, int model_ptr);        // Sprite_SetupAnimation
void  __cdecl Entity_Render(void);                                   // Entity_Render (sprite loop)
void  __cdecl RenderPartObjectEffect(int, int, float *, float, unsigned int, unsigned char, unsigned int, unsigned int); // IDA: RenderPartObjectEffect (0x00504B50)
void  __cdecl Entity_RenderSlotWith(void *, void *, int, char, int);
// Entity_DrawAt(entity_ptr, class, slot, angle_ptr, rot, state_flags, byte, a,b,c, d, mode)
void  __cdecl RenderPartObject(int, int, unsigned int, float *, float, unsigned int, unsigned char, char, unsigned char, char, int, unsigned int); // IDA: RenderPartObject (0x00505A10)
void  __cdecl DivineSkirt_Apply(int entity, int modelType, int part, void *model);   // desviacion: Physics/Cloth_MeshDivine.cpp
void  __cdecl Model_SetAnimationSlots(int, int, int, int, int, int); // SetMonsterSound — writes model animation slots
// ── Map / terrain loaders (called from World_Load / Map_LoadResources) ─────────
void  __cdecl DeleteObjects(void);                                  // Terrain_ResetObjects
void  __cdecl OpenObjectsEnc(const char *path);                      // Terrain_LoadObjects
void  __cdecl OpenTerrainMapping(const char *path);                      // Terrain_LoadMap
int   __cdecl OpenTerrainAttribute(const char *FileName);                  // OpenTerrainAttribute
void  __cdecl OpenTerrainLight(const char *path);                      // Terrain_LoadLight
void  __cdecl CreateTerrain(const char *path);                      // Terrain_LoadHeight
void  __cdecl ClearItems(void);                                  // Map_InitEntities
void  __cdecl DeleteNpcs(void);                                  // Terrain_InitLayers
void  __cdecl DeleteMonsters(void);                                  // Terrain_InitWater
void  __cdecl OpenWorldModels(void);                                  // Map_InitLighting
void  __cdecl ClearCharacters(int map_id);                            // Map_SetupEntities
// ── Font / UI init ───────────────────────────────────────────────────────────────
void  __cdecl PathFinder_ResetContext(void);                                  // IDA: FUN_0043f2d0
void  __cdecl FUN_0050f700(const char *FileName);                  // SaveMacro (Data/Macro.txt)
void  __cdecl OpenMacro(char *FileName);                            // 0x0050F750
// FUN_0050f7a0 NO es "Map_Unload": es sub_50F7A0, el envio de opciones F3/30.
// Portada como SaveOptionsToServer97k en UI/UI_InGameMenu.cpp (2026-09-21).
// FUN_0050f5f0 @ 0x0050F5F0 (IDA)
void  __cdecl Font_CreateTextDib(int dc);
// FUN_0040f570 @ 0x0040F570 (IDA)
void  __cdecl Font_CreateRenderer(int hwnd, int data, int dc);
// ── Model / data loaders (called from UI_LoadTextures) ───────────────────────────
void  __cdecl Model_LoadPlayerAndItemMeshes(void); // IDA: OpenPlayers
void  __cdecl Model_LoadPlayerEquipmentTextures(void); // IDA: OpenPlayerTextures
void  __cdecl Model_LoadItemMeshes(void); // IDA: OpenItems
void  __cdecl Model_AssignItemTexturePrefixes(void); // IDA: OpenItemTextures
void  __cdecl Model_LoadSkillEffectAssets(void); // IDA: OpenSkills
void  __cdecl UI_LoadInterfaceTextures(void); // IDA: OpenImages
void  __cdecl Sound_LoadGameSamples(void); // IDA: OpenSounds
void  __cdecl Item_LoadTextData(const char *path);                 // IDA: FUN_0047B130
void  __cdecl Item_SaveBMD(const char *path);                      // IDA: FUN_0047B650
void  __cdecl Item_LoadBMD(const char *path);                      // IDA: FUN_0047B740
void  __cdecl Skill_LoadTextData(const char *path);                // IDA: FUN_0047A5B0
void  __cdecl Skill_SaveBMD(const char *path);                     // IDA: FUN_0047A970
void  __cdecl Skill_LoadBMD(const char *path);                     // IDA: FUN_0047AC50
void  __cdecl Gate_LoadTextData(const char *path);                 // IDA: FUN_0047A010
void  __cdecl Gate_SaveBMD(const char *path);                      // IDA: FUN_0047A170
void  __cdecl Gate_LoadBMD(const char *path);                      // IDA: FUN_0047A4D0
void  __cdecl Filter_LoadTextFile(const char *path);                      // Filter_LoadData
void  __cdecl Filter_SaveBMD(const char *path);                      // Filter_LoadBMD
void  __cdecl OpenFilterFile(const char *path);                      // Filter_LoadBMD_Alt
void  __cdecl FilterName_LoadData(const char *path);                      // FilterName_LoadData
void  __cdecl FilterName_SaveBMD(const char *path);                      // FilterName_LoadBMD
void  __cdecl OpenNameFilterFile(const char *path);                      // FilterName_LoadBMD_Alt
void  __cdecl Dialog_LoadBMD(const char *path);                    // IDA: FUN_0047B020
uint  __cdecl CSQuest_OpenQuestScript(int handle, const char *path);          // Quest_LoadBMD
void  __cdecl NPCName_LoadTextData(const char *path);              // IDA: FUN_0047D120
// LoadWaveFile — declared above as Sound_LoadAndPlay(int type, int flags)
void  __cdecl OpenWorld(void); // IDA: OpenWorld (0x0050E5A0)
void  __cdecl OpenFont(void);              // IDA: OpenFont (0x0050F690)
void  __cdecl Scene_LoadAccountResources(void); // OpenLogoSceneData (IDA)
void  __cdecl Scene_UnloadAccountResources(void); // ReleaseLogoSceneData (IDA)
void  __cdecl Scene_LoadCharSelectResources(void); // OpenCharacterSceneData (IDA)

// ── GL helper layer ───────────────────────────────────────────────────────────
void  __cdecl ReleaseCharacterSceneData(void); // IDA: ReleaseCharacterSceneData (0x005102C0)
void  __cdecl Scene_LoadGameAssets(void); // IDA: OpenBasicData
void  __cdecl Monster_LoadStartupData(void); // IDA: Monster_Data_Load
int   __cdecl GL_CaptureScreenshot(void);          // Screenshot capture; returns counter/10000
void  __cdecl GL_GetModelViewMatrix(unsigned int *);                         // Matrix_GetViewRotation
void  __cdecl GL_SetPerspective(int fov, float aspect, int near_clip, float far_clip);  // GL_SetPerspective
// FUN_005112F0
void  __cdecl Camera_BuildMouseRay(int mouseX, int mouseY, float *out_ray); // Camera_MouseRay
// FUN_005113F0
void  __cdecl Camera_ProjectWorldToScreen(float *world_pos, int *out_x, int *out_y); // World_ToScreen
void  __cdecl GL_BindTextureSlot(int texture_type);                      // Particle_SetTexture (glBindTexture)
void  __cdecl GL_EnableDepthTest(void);  // GL_Setup2D (no args)
void  __cdecl GL_EnableDepthWrites(void);
void  __cdecl GL_ResetState(void);
bool __cdecl FUN_00513260(float *rayOrigin, float *rayTarget, const float *box12); // 0x513260 SAT segmento-vs-OBB (box = objeto+0x130)
void  __cdecl GL_SetBlendSrcOver(char mode);                             // GL_SetMode(1=blend)
void  __cdecl GL_SetBlendAdditive(void);                                  // Frame_UpdateTimer
void  __cdecl GL_SetBlendSrcAlpha(void);
void  __cdecl GL_SetViewport(int, int, int, int);
float10 __cdecl Screen_ToGLX(float);  // GL_sin or trig helper
float10 __cdecl Screen_ToGLY(float);  // GL_cos or trig helper
// FUN_005119B0
void  __cdecl GL_BeginViewport(int x, int y, int w, int h);       // Viewport_Set
void  __cdecl GL_DrawBillboard(float w, float h, float *rot_mat);     // SkillEffect_DrawBillboard
void  __cdecl GL_BeginSprite(void);
void  __cdecl GL_EndOpenGL(void);                                  // EndOpengl
void  __cdecl RenderSprite_0(int type, float *pos, float r, float g, // SkillEffect_Draw2D
                            float *extra, float, float, float,
                            float, float);

// ── Texture system ────────────────────────────────────────────────────────────
void  __cdecl GL_Begin2D(void);                                  // GL_SetupOrtho2D
void  __cdecl GL_End2D(void);                                  // GL_End2D
void  __cdecl GL_DrawRect(float, float, float, float);  // FillRect(x,y,w,h)
void  __cdecl GL_DrawRotatedRect(int id, float x, float y, float w, float h, undefined4 color); // GL_DrawColoredRect
int   __cdecl FUN_0051ddf0(void);   // GuildLeaderboard_Render
int   __cdecl GuildOverview_Render(void);   // GuildMemberList_Render2
void  __cdecl GL_DrawTexture(int id, float x, float y, float w,     // Texture_Draw2D
                            float h, float u0, float v0,
                            float u1, float v1, char fx, char fy);
// FUN_005128c0 = RenderBitmapUV(int Texture, float x, float y, float Width,
//   float Height, float u, float v, float uWidth, float vHeight).  La firma
//   `(int,int,int,int)` de antes estaba mal y por eso nadie la usaba.
//   Implementada en Render/Render_Frame.cpp (static).  NO es intercambiable con
//   RenderBitmap (0x5125A0): esta mapea un cuadrilatero SESGADO en V, aquella un
//   rectangulo.
float10 __cdecl FUN_005129f0(float angle);  // |a| (abs)
double  __cdecl Math_Fmin(float a, float b); // min(a,b)
double  __cdecl Math_Fmax(float a, float b); // max(a,b)
int     __cdecl Collision_PointInPolygon(float a1, float a2, float a3, int a4, int a5, int a6, int a7, int a8, int a9, float a10); // PointInPolygon
// FUN_00513570 @ 0x00513570
char  __cdecl Chat_ValidateInputCommand(void);
void  __cdecl CharSelect_SendSelectPacket(void); // IDA: FUN_00513c10 (0x00513C10)
void  __cdecl SetErrorMessage(int message); // IDA: SetErrorMessage (0x005142D0)
void  __cdecl UI_InGameMenu(void);          // UI_InGameMenu state machine
int   __cdecl RenderErrorMessage(void);  // UI_StatsPanel_Render
int   __cdecl SeparateTextIntoLines(const char *lpszText, char *lpszSeparated, int iMaxLine, int iLineSize); // 0x0051D600
#define FUN_0051d600  SeparateTextIntoLines
int   __cdecl ItemList_Select(int slot);                                     // ItemList_Select
void  __cdecl RenderInfomation3D(void); // IDA: RenderInfomation3D (0x0051E0C0)
void  __cdecl CServerSelWin_UpdateWhileActive(void); // IDA: CServerSelWin::UpdateWhileActive (0x0051E7E0)

// ── Scene functions ───────────────────────────────────────────────────────────
void  __cdecl FUN_005239a0(void);                                  // CharSelect_UpdateInput
bool  __cdecl MoveMainCamera(void);   // MoveMainCamera (per-frame camera follow + Hero offset)
void  __cdecl SaveImage(int, int, int, int);   // IDA: SaveImage (0x00529130)
int   __cdecl OpenJPG(const char *path, int id,               // Texture_Load (OZJ/JPEG)
                            int w, int h, int flags, char mipmap);
int   __cdecl OpenTGA(const char *path, int id,               // Texture_LoadTGA (TGA/OZT)
                            int w, int h, int flags, char mipmap);
void  __cdecl UnloadImage(int id); // IDA: UnloadImage (0x0052A050)
int   __cdecl CSimpleModulus_Encode(int mode, BYTE* src, int len); // IDA: FUN_0053CC30 (0x0053CC30)
int   __cdecl CSimpleModulus_Decode(int, int, int, int); // IDA: FUN_0053CCA0 (0x0053CCA0)
uint  __cdecl Pipe_QueryResource(char *);  // Texture_GetSlot — returns slot index
float10 __cdecl CIsin(void);  // GetTickCount-based time accumulator → float10

// ── MSVC CRT stubs (obfuscation / anti-tamper wrappers) ───────────────────────
int   __cdecl crt_sprintf(char *buf, const char *fmt, ...); // IDA: crt_sprintf (0x005416BC)
void  __cdecl crt_output_engine(int, int, int, int); // IDA: output (0x0054519D)
void  __cdecl operator_delete(void *ptr); // IDA: operator_delete (0x0054158C)
void  __cdecl __chkstk_probe(int frame_size); // IDA: __chkstk_probe (0x00541C10)

// operator_new — MSVC global new (routes through CRT malloc)
// Ghidra decompiles show `operator_new(size)` as the standard C++ new.
#ifndef operator_new
#include <new>
inline void* operator_new(size_t n) { return ::operator new(n); }
#endif
FILE* __cdecl crt_fopen(LPCSTR, const void*);    // fopen-wrapper
int   __cdecl mbclen(const byte *str);   // IsLeadByte — DBCS lead-byte check
void  __cdecl crt_toupper(int, int, int, int);
void  __cdecl crt_time(int, int, int, int);
void  __cdecl setlocale(int, int, int, int);   // IDA: setlocale (0x0054283E)
// crt_fseek: su unica declaracion buena esta mas abajo (int*, int, int).  Aca
// habia una SEGUNDA con firma generica de 4 ints, sin ningun call site: dos
// declaraciones del mismo simbolo del binario con firmas distintas son dos
// simbolos C++ distintos, que es como quedo muerto el puente de AccessModel.

// ── GL_PopMatrixAll ───────────────────────────────────────────────────────────
unsigned int __cdecl GL_PopMatrixAll(void);

// ── Named game functions (called by name in .cpp files) ───────────────────────
// These alias or wrap FUN_ addresses but are called by symbolic name.
void  __cdecl Object_MoveUpdate(void);        // @ 0x0043e050 (movement tick)
void  __cdecl Particle_Update(void);          // @ 0x0046c3e0 (particle system tick)
void  __cdecl Character_UpdateAll(void);      // @ 0x00479730 (character update loop)
void  __cdecl Effect_UpdateAll(void);         // @ 0x00479790 (effect system tick)
void  __cdecl Login_CameraUpdate(void);       // @ 0x004f8eb0 alias (login camera)
float __cdecl Screen_ToGLx(float x);          // @ 0x00511950
float __cdecl Screen_ToGLy(float y);          // @ 0x00511980

// ── Mouse hover tick helpers (Mouse_UpdateHoverTargets) ───────────────────────────────────
int   __cdecl RenderTerrainTile(int, int, int, int, float, int, int); // IDA: RenderTerrainTile (0x004F8480)
int   __cdecl Entity_SelectNearest(int type); // IDA: FUN_004afdc0; find hovered entity of given type
int   __cdecl ItemOnGround_HoverTest(void);   // IDA: FUN_004afa40
int   __cdecl SpecialObject_HoverTest(void);  // IDA: FUN_004b0240
void  __cdecl Party_MatchEntityNames(void); // IDA: FUN_004afb00
char  __cdecl Party_HPBar_HoverCheck(void);        // SecondPassword_IsActive

// ── Char menu builder helpers (RenderHelpWindow) ──────────────────────────────────
void  __cdecl FUN_004c2420(int, int, int, int, int, int);  // DrawItemInfoBox(x, y, count, fixedWidth, iSort, drawBox)
void  __cdecl ItemHelp_RequireClass(int class_data_ptr);            // build class info block
void  __cdecl FUN_004c2c10(int column, unsigned char *format, int *value, const char *widthRef, int y, int kind); // IDA: sub_4C2C10 columna de valores de la ayuda F1
void  __cdecl CharMenu_AppendSkillDesc(int row, int value, int col_width); // draw value column
void  __cdecl CharMenu_BuildStatRequirements(int class_id);                  // prepare class data

// ── Second password ───────────────────────────────────────────────────────────
unsigned int  __cdecl SecondPassword_Handler(void); // IDA: SecondPassword_Handler (0x004E93A0)

// ── Weather / BMD animation helpers ──────────────────────────────────────────
bool  __cdecl BMD__PlayAnimation(void* pvVar9, float* rot, float* scale, void* extra, float fVar); // BMD_AnimTick
extern "C" bool __cdecl CharacterAnimation(int c, int o);    // 0x00448600 — per-char anim tick
// ── CreateEffect helpers ─────────────────────────────────────────────────────
void  __cdecl AddTerrainLight(float, float, float*, int, float*);          // AddTerrainLight(0x4F76C0)
//   NO confundir con `AddTerrainLightClip` (0x4F7800), que es otra funcion y
//   ademas clampea la luz a 1.0.
float* __cdecl Entity_FindNearby_SendPacket(uint, float*, float, int, short);     // Particle_SpawnFX

// ── Monster_Data sub-functions ────────────────────────────────────────────────
void  __cdecl Monster_LoadScriptTable(void); // FUN_0050b510 — loads monster script/model table
void  __cdecl Monster_ParseSetBase2(LPCSTR param_1); // FUN_0047d020 — parses MonsterSetBase2 spawn definitions
void  __cdecl SetMaxTextures(int count); // IDA: SetMaxTextures (0x00505BD0)
void  __cdecl OpenMonsterModel(int monster_idx);                    // Monster_SetupSoundAnim
int   __cdecl TextParser_GetToken(void);                         // FUN_0047A1F0 — text-data tokenizer (returns record type)
void  __cdecl BuxConvert_0(int buf, int len);   // IDA: BuxConvert_0 (0x00479910) -- clave FC CF AB en DAT_00559bb4
void  __cdecl Skill_HashTable_SerializeEntry(void *dst, void *src);               // Skill_HashTable_SerializeEntry (encode + insert)
void  __cdecl Skill_HashTable_FreeEntry(void *entry, void *key);             // Skill_HashTable_FreeEntry (decode + remove)
void  __cdecl BuxConvert_1(void* buffer, int size);               // IDA: BuxConvert_1 (0x00401120)
uint  __cdecl FUN_005430f0(char *buf, uint size, uint count, int *fp); // fwrite-wrapper (locked)
void  __cdecl putc(int ch, int *fp);                     // fputc-wrapper (writes single byte to file)
void  __cdecl crt_fclose(FILE* fp);                           // fclose-wrapper
void  __cdecl AccessModel(int id, const char* dir, const char* file, int idx); // Monster_LoadBase
// DESVIACION DEL PORT (no existe en IDA): AccessModel + OpenTexture + siembra de
// velocidades de animacion.  Definida en Render/Render_LegacyLinker.cpp.
void  __cdecl AccessModelWithTextures(int id, char* path, char* name, int param);
void  __cdecl OpenTexture(int id, const char* prefix, int flags, char loop);   // Monster_LoadSound
// ── GL_State sub-functions ────────────────────────────────────────────────────
void  __cdecl GL_DisableCullFace(void);           // GL_SetTextureState
void  __cdecl GL_DisableDepthWrites(void);           // GL_SetDepthState
void  __cdecl GL_EnableCullFace(void);           // GL_TexEnable
unsigned int __cdecl WriteJpeg(const char* path, int width, int height, void* pixelBuf, int quality); // IDA: WriteJpeg (0x00529000) -- libjpeg, filas invertidas
// ── Particle path-finding helpers ────────────────────────────────────────────
int   __cdecl Math_GetAngleFromPoints(float x1, float y1, float x2, float y2); // IDA: FUN_0043e430
int   __cdecl Angle_Clamp(int a, int b, int c);                     // Angle_Clamp

// ── Joint_Create helpers ──────────────────────────────────────────────────────
void  __cdecl Joint_SegmentTick(int joint_ptr, float *mat);  // Joint_SegmentTick
// MoveHumming(Position, Angle, TargetPosition, Turn) — gira Angle hacia el
// target y DEVUELVE la distancia (Hex-Rays la tipa `void`: el retorno va en st0).
float __cdecl FUN_0043e4a0(float *Position, float *Angle, float *TargetPosition, float Turn);

// ── GL_2D helpers ─────────────────────────────────────────────────────────────
void  __cdecl GL_DisableDepthTest(void);           // GL_End2D / restore state
void  __cdecl GL_SetAlphaTest(char mode);      // GL_SetBlendMode (1 arg)
void  __cdecl GL_EnableLightMap(void);           // GL_EnableLightMap (lightmap blend state)

// ── Entity_Render helpers ─────────────────────────────────────────────────────
void  __cdecl MoveMonsterClient(int entity_ptr, int param2); // IDA: MoveMonsterClient (0x00454CD0)
void  __cdecl MoveCharacter(int entity_ptr); // IDA: MoveCharacter (0x00449900)
void  __cdecl MoveCharacterVisual(int entity_ptr); // IDA: MoveCharacterVisual (0x004520C0)

// ── Effect_Tick helpers ───────────────────────────────────────────────────────
void  __cdecl MoveEffect(float *, int); // IDA: MoveEffect (0x00466AD0)
char * __cdecl MoveJoint(undefined1 *param_1, uint param_2); // MoveJoint — implemented in Render/MoveJoint.cpp
void  __cdecl TEXCOORD(float*, float, int);  // sincos helper (dest, angle, scale)
void  __cdecl EulerToMatrix(float*, float*);      // EulerToMatrix(angles, out_mat)
void  __cdecl Joint_BoneOffsetApply(int, int);

// ── Net_Connect helpers ───────────────────────────────────────────────────────
extern "C" int __cdecl CWsctlc_Connect(DWORD This, const char* ip_addr,
                                        unsigned short port, DWORD WinMsgNum); // IDA: CWsctlc::Connect (0x0043DCD0)
void  __cdecl CWsctlc_Startup(int);       // IDA: CWsctlc::Startup (0x0043DB30)
void  __cdecl CWsctlc_Create(void*, int); // IDA: CWsctlc::Create (0x0043DBF0)
void  __cdecl CWsctlc_Close(int);         // IDA: CWsctlc::Close (0x0043DC90)

// ── Entity_Init helpers ───────────────────────────────────────────────────────
void  __cdecl SetCharacterScale(int); // IDA: SetCharacterScale (0x0045C050)
void  __cdecl crt_fprintf(void*, void*);  // fprintf-like helper

// ── Sound helpers ─────────────────────────────────────────────────────────────

// ── Net_PacketSession helpers ─────────────────────────────────────────────────
void  __cdecl Inventory_DropDispatch(undefined4, uint);  // packet handler with ECX/EDX args
void  __cdecl SecondPassword_Screen1(void);
void  __cdecl FUN_004e5500(void);
void  __cdecl SecondPassword_Screen3(void);
void  __cdecl SecondPassword_Screen4(void);
void  __cdecl SecondPassword_Screen5(void);
void  __cdecl CheckGoldenArcherWindow(void); // IDA: CheckGoldenArcherWindow (0x004E7AC0)
void  __cdecl FUN_004e8b70(void);
void  __cdecl FUN_004e9050(void);
void  __cdecl SecondPassword_Screen9(void);
void  __cdecl FUN_004eb7f0(void);
void  __cdecl FUN_004ec330(void);
// SecondPassword UI helper calls
void  __cdecl ShowCheckBox(int num, int index, int message);  // IDA: ShowCheckBox (0x0051E240)
uint  __cdecl SecondPassword_GridSlotAvail(int ctx, int p1, int p2, int p3, int p4);   // SecondPassword_AuthSend — returns non-zero on success
undefined4 __cdecl SecondPassword_CancelReturn(void);        // SecondPassword_CancelReturn (nav back, clear state)
uint  __cdecl Net_Disconnect_Clean(void);             // Net_Disconnect_Clean (sends disconnect + returns)
void  __cdecl FUN_004d1fc0(void);             // SecondPassword_RenderGrid_Empty
// 2026-05-08: real signature per IDA `004D23B0_sub_4D23B0.c` (Inventory grid
// render + click dispatcher). See Item/Item_ClickHandler.cpp for the port.
void  __cdecl FUN_004d23b0(char *origin_x, int origin_y, short *inv_base,
                            int grid_w, int grid_h, char mode_flag);

// ── Scene_MapTick helpers ─────────────────────────────────────────────────────
void  __cdecl RenderItemInfo(void*, void*, void*, int);  // RenderItemInfo(sx, sy, ITEM*, bSell)
void  __cdecl RenderRepairInfo(void*, int,   void*);        // RenderItemInfo_Shop(sx, sy, ITEM*)
void  __cdecl RenderSkillTooltip(float a1, int a2, int a3);   // Skill_RenderTooltip @ 0x004C9730

// ── Texture helpers ───────────────────────────────────────────────────────────
uint  __cdecl Pipe_SendQuery(void *mgr, CHAR *name); // IDA: FUN_0053ed00 (0x0053ED00)
void  __cdecl Pipe_SetTarget(void *mgr, CHAR *name);      // Pipe_SetTarget — copies name, sends 0x613
void  __cdecl Pipe_Write(LPCVOID param_1, int param_2, CHAR *param_3);  // Pipe_Write wrapper
void  __cdecl Pipe_WriteFile(LPCVOID param_1, int param_2, CHAR *param_3);  // Pipe_WriteFile — WriteFile to named pipe

// ── WinMain helpers ───────────────────────────────────────────────────────────
void  __cdecl Window_FatalError(HWND hwnd, char shutdown);  // 0x004129F0
UINT  __cdecl Resource_Load(char* filename);  // Resource_Load — loads a resource file

// ── Misc/Util helpers ─────────────────────────────────────────────────────────
void  __cdecl ItemAngle(int); // IDA: ItemAngle (0x005030C0)
void  __cdecl Entity_UpdateSparkleEffect(int); // FUN_00503650 — updates periodic entity sparkle particles
int   __cdecl FUN_00402850(void *);  // Packet_ParseType1
undefined4 __cdecl Packet_ParseType2(void *);  // Packet_ParseType2
void  __cdecl FUN_0040e330(DWORD);   // (undocumented)
int   __cdecl Sound_UpdateChannel3D_Tick(int *, float);   // cloth: paso de simulación (0x408940)

// ── Scene-entity grid helpers ─────────────────────────────────────────────────
void  __cdecl Entity_GridUnlink(void *node, int cell_ptr);  // Entity_GridUnlink — unlinks node from its doubly-linked list

// ── Sound slot helpers ────────────────────────────────────────────────────────
HRESULT __cdecl Sound_ReleaseBuffer(int buffer); // IDA: FUN_00404AD0; 5.2: ReleaseBuffer

// ── File I/O CRT helpers ──────────────────────────────────────────────────────
void  __cdecl crt_fseek(int *fp, int offset, int whence);  // CRT fseek wrapper
int   __cdecl crt_ftell(char *fp);                         // CRT ftell wrapper
int   __cdecl crt_fread(void *dst, int size, int count, int *fp); // CRT fread wrapper
int   __cdecl mbclen(const unsigned char *str);         // IsLeadByte — already in stubs.cpp

// ── Terrain helpers ───────────────────────────────────────────────────────────
void  __cdecl Terrain_Clear(void);  // InitTerrainMappingLayer (IDA) — resets terrain tile buffers
void  __cdecl ExitProgram(void);   // Terrain_ReadFallback
void  __cdecl BuxConvert(int data, int size);   // IDA: BuxConvert (0x004F6EB0) -- clave en CERO a proposito, ver globals.cpp
void  __cdecl CreateTerrainNormal(void); // FUN_004f70b0 (IDA)
void  __cdecl CreateTerrainLight(void); // IDA: CreateTerrainLight (0x004F71C0)
uint  __cdecl OpenTerrainHeight(char *path); // FUN_004f7290 (IDA)
void  __cdecl Terrain_SetupCulling(void);   // TerrainHeight_Flush
void  __cdecl OpenJpegBuffer(char *path, int dst); // Texture_LoadToBuf
void  __cdecl Map_InitRayCast(void);   // Map_InitRayCast
int   __cdecl FUN_004f98c0(int, int, int, int, int); // RayCast_Setup / terrain light setup (sub_4F98C0)
void  __cdecl Terrain_SpawnAmbientObjects(void);   // Terrain_SpawnAmbientObjects (sub_4F7060)
void  __cdecl FUN_00479540(void);   // RenderTerrainAlphaBitmaps (sub_479540)
int   __cdecl Terrain_WaterWaveUpdate(int a1); // Terrain light double-buffer commit (sub_4F9A30)
float10 __cdecl CharacterMoveSpeed(int entity_ptr); // IDA: CharacterMoveSpeed (0x00454B00)

// ── Scene_MapTick helpers ─────────────────────────────────────────────────────

// ── Model/BGM loader helpers ──────────────────────────────────────────────────
// OpenModel — same as Monster_RegisterBMD above (int first arg), see line 653
void  __cdecl SetMaxTextures(int slot); // IDA: SetMaxTextures (0x00505BD0)

// ── Model mesh/anim loaders (used by Model_Load* functions) ──────────────────
// OpenSMDModel: loads main BMD mesh into model slot
//   param_1 = model id (as ptr cast to int)
//   param_2 = path to .smd/.bmd file
//   param_3 = num anims
//   param_4 = flag
void  __cdecl OpenSMDModel(int model_id, const char *path, int num_anims, char flag);   // IDA: OpenSMDModel (0x0040B280)
// OpenSMDAnimation: adds an animation to a loaded model
//   param_1 = model id (int)
//   param_2 = path to animation .smd file
//   param_3 = loop flag (0=no loop, 1=loop)
void  __cdecl OpenSMDAnimation(int model_id, const char *path, char loop);   // IDA: OpenSMDAnimation (0x0040B310)
// OpenModels: loads a numbered model file (format: "prefix%s%02d.smd" or "%s%d.smd")
//   param_1 = model id
//   param_2 = path prefix string
//   param_3 = index number
void  __cdecl OpenModels(int model_id, const char *prefix, int index); // IDA: OpenModels (0x00506050)

// ── Terrain tile pick helpers ─────────────────────────────────────────────────
void  __cdecl RenderTerrainFace(float xf, float yf, int xi, int yi, float lodf); // RenderTerrainFace (0x004F7FB0)
unsigned int __cdecl CollisionDetectLineToFace(float *Position, float *Target, int Polygon, float *v1, float *v2, float *v3, float *v4, float *Normal, char Collision); // IDA: CollisionDetectLineToFace (0x00512D40)

// ── Item inventory helpers (from Offsets.h) ───────────────────────────────────
// GetItemCount (0x00482FF0) y GetItemSlot (0x00482D70) se implementan en
// Item/Item_LegacyHelpers.cpp.  Aca habia dos #define que mapeaban sus FUN_ a
// esos nombres; nadie los usaba y eran una trampa: los bloques IDA-only de
// stubs_IDA_ports.cpp DEFINEN esos FUN_, asi que al activar su gate el define
// los convertia en una redefinicion de la funcion real.
int   __cdecl GetItemCount(int siType, int iLevel);  // 0x00482FF0
int   __cdecl GetItemSlot(int siType, int iLevel);   // 0x00482D70

// ── SecondPassword Screen6/7 helpers ─────────────────────────────────────────
void  __cdecl CreateDialogInterface(int a, int b);  // UI_ShowBitmapMsg (bitmap message overlay, 2 args)
void  __cdecl Item_RecalculateRepairCost(void); // IDA: FUN_004C4080
void  __cdecl FUN_004233e0(int, int);      // HashTable_Unlock (2-arg variant, release lock)

// ── Map / Scene helpers ───────────────────────────────────────────────────────
int   __cdecl ParseNextToken(void);        // FUN_0050E2C0 — reads next token from ParserFileHandle into ParserTokenString; returns token type (2=EOF)

// ── BMD skin helpers ─────────────────────────────────────────────────────────
void  __cdecl Vec3_Normalize(float *vec);    // Vec3_Normalize — normalises a 3-float vector in-place

// ── NPC Script helpers ────────────────────────────────────────────────────────
void  __cdecl FUN_0043d3e0(int, int*);     // HashTable_LockRead (lock + read value into *out)
void  __cdecl Packet_DecryptDword(void*, void*); // IDA: FUN_00409E20 (0x00409E20)
void  __cdecl Packet_EncryptDword(void*, void*); // IDA: Packet_EncryptDword (0x00423760)

// ── BMD bone-draw helper ──────────────────────────────────────────────────────
void  __cdecl BMD__RenderMeshTranslate(void *model, char a, int b, float frame, int flags,
                            float f3, int f4, float f5, float f6, float f7, unsigned int rgba);
                            // BMD_DrawBoneSlot_Anim (animated variant, 11 args)

// ── Character animation/attack helpers (Kayito names, called from large stubs) ──
// SetPlayerAttack — already declared above (line 266) with 4 args: (int, int, int, int)
void  __cdecl SetPlayerShock(int entity, int type); // IDA: SetPlayerShock (0x00444B60)
void  __cdecl AttackEffect(int entity); // IDA: AttackEffect (0x00445230)
// IDA: UseSkillWarrior
void  __cdecl Combat_UseWarriorSkill(int entity, int skillType);
// IDA: UseSkillElf
void  __cdecl Combat_UseElfSkill(int entity, int object);
int   __cdecl CHARACTER_MACHINE_GetMagicSkillDamage(int machinePtr, int skillType, int level); // CHARACTER_MACHINE::GetMagicSkillDamage

// ── Sound: PlayBuffer ─────────────────────────────────────────────────────────
HRESULT __cdecl PlayBuffer(int Buffer, DWORD Object, BOOL bLooped); // IDA: PlayBuffer (0x00404BC0)

// ── Cipher / XOR aliases ──────────────────────────────────────────────────────

// ── Token parser aliases ──────────────────────────────────────────────────────
// GetToken = TextParser_GetToken — lee el proximo token del archivo abierto y lo deja en
// TokenString (= TextParserTokenString, IDA @0x07CF1EF0).
// OJO: hay DOS tokenizers con buffers DISTINTOS y el arbol los tenia mezclados
// (corregido 2026-08-22):
//   GetToken        0x0047A1F0 -> TokenString    0x07CF1EF0  (Item/Monster/Skill/
//                                                             NPC/Gate/Filter.txt)
//   Parse_NextToken 0x0050E2C0 -> ParserTokenString   0x083A3FF4  (OpenWorldModels)
#define GetToken    TextParser_GetToken
#define TokenString TextParserTokenString // char[256] — GetToken output (DAT_07CF1EF0)

// ── Character helpers ─────────────────────────────────────────────────────────

// ── Error reporting ───────────────────────────────────────────────────────────
void  __cdecl CErrorReport__Write(DWORD This, char *lpszFormat, ...); // 0x00405540

// ── Forward declarations for small unmapped functions ─────────────────────────
// CRT internals
void  __cdecl crt_atexit(void *pFunc);                             // CRT atexit registration
void  __cdecl FUN_00543c98(void *ptr);                               // CRT free wrapper
void  __cdecl FUN_0053d430(BYTE *ptr);                               // GameGuard string cleanup
int   __cdecl FUN_0053ea90(void *ptr);                               // GameGuard query

// Vtable init / class chains (thiscall emulated as fastcall)
int   __cdecl    FUN_00405280(HANDLE *This);                         // CErrorReport vtable+init
int   __cdecl    FUN_00405290(int param_1);                          // CErrorReport field init
void  __cdecl    FUN_004052b0(void *This, const char *filename);     // CErrorReport open log
void  __cdecl    FUN_00407de0(DWORD *This);                          // CWaveFile vtable set
void  __cdecl    FUN_00407ec0(DWORD *This);                          // CWaveFile vtable+chain
void  __fastcall FUN_004093c0(void *This);                           // SoundWidget vtable+chain
void  __fastcall FUN_00408070(void *This);                           // SoundWidget base init
void  __fastcall FUN_0040a6e0(void *This);                           // WidgetC vtable+chain
void  __fastcall FUN_00410ad0(void *This);                           // StreamBase vtable+chain
void  __fastcall FUN_0040f680(void *This);                           // StreamBase vtable set (dup)
void  __fastcall FUN_0040f690(void *This);                           // StreamBase vtable set
void  __fastcall PacketCipher_Initialize(void *This);                // IDA: FUN_00406bd0
void  __fastcall FUN_00405240_init(void *This);                      // ErrorReport_GlobalInit
void  __fastcall FUN_00403ea0(void *This);                           // Quest table init
void  __fastcall FUN_0040a600(void *This);                           // Sound device init B

// Destructor chains (virtual ~dtor pattern: call deinit, conditional delete)
void  __fastcall FUN_00406cd0(void *This);                           // HashWidget deinit
void  __fastcall Locimp_dtor(void *This);                           // SoundWidgetB deinit
void  __fastcall FUN_0040dba0(void *This);                           // DSBuffer deinit
void  __fastcall FUN_0040eae0(void *This);                           // DSBufferB deinit
void  __fastcall FUN_0040f540(void *This);                           // Stream deinit+release
void  __fastcall FUN_0040f950(void *This);                           // StreamB deinit
// void  __fastcall FUN_00412510(void *This);                           // GG module deinit — duplicate, correct DWORD* version at line 1276
// void  __fastcall FUN_00412610(void *This);                           // GG module2 deinit — duplicate, correct DWORD* version at line 1277
void  __fastcall FUN_0040d550(void *This);                           // DSDevice deinit

// Sound system
HRESULT __cdecl FillBuffer(int Buffer, int MaxChannel, BOOL Enable); // 0x00404a00
void  __cdecl AllStopSound(void);                                    // 0x00404ca0
void  __cdecl FreeDirectSound(void);                                 // 0x00404b80
void  __cdecl StopBuffer(int Buffer, int Object);                    // 0x00404bc0 (from PlayBuffer)

// Music
void  __cdecl StopMp3(char *filename, int param);                    // 0x00513330

// Party / UI
void  __cdecl InitPartyList(void);                                   // 0x004e54f0
void  __cdecl ClearNotice(void);                                     // 0x0047fac0
void  __cdecl ClearWhisperID(void);                                  // 0x004804d0

// Mouse
void  __cdecl CenterMouseX(void);                                    // 0x005110d0
void  __cdecl CenterMouseY(void);                                    // 0x005110f0

// Terrain / objects
void  __cdecl ReleaseMainData(void);                                 // 0x005110a0
void  __cdecl EndOpengl(void);                                       // 0x00511bc0
void  __cdecl StopMusic(void);                                       // 0x00513420

// Crypto
WORD  __cdecl DecryptCheckSumKey(WORD wSource);                      // 0x00412d30

// Entity / item helpers
int   __cdecl GetHandOfWeapon(int obj);                              // 0x00448900
short __cdecl FUN_0047cfe0(short *param_1);                          // Item field accessor (+0x14)
short __cdecl FUN_0047d000(short *param_1);                          // Item field accessor (+0x18)
char* __cdecl getMonsterName(int type);                              // 0x0047d200
bool  __cdecl getEqualMonster(int addV);                             // 0x0047ebd1
void  __cdecl SetMatchInfo(BYTE byType, int iMaxTime, int iTime, int iMaxMonster, int iKillMonster); // 0x0047eba0
void  __cdecl Quest_InitializeStaticState(void); // IDA: FUN_00401010
char  __fastcall FUN_00403f30(void *pCSQuest);   // dispatcher del render de quest (0x403F30)
void  __fastcall CSQuest__setQuestList(int ecx, int edx, int index, int result); // 0x004011b0
void  __fastcall CSQuest__setQuestLists(int ecx, int edx, BYTE *byList, int num, int Class); // 0x00401160

// Blur / trail
void  __cdecl MoveBlurs(void);                                       // 0x0046c3b0

// JPEG codec helpers

// Misc internal
int   __cdecl CSimpleModulus_GetByteOfBit(int param_1); // IDA: FUN_0053D170 (0x0053D170)

// ── Batch 3 forward declarations ─────────────────────────────────────────────

// Quest system (CSQuest class, entries 0x00401040 .. 0x00401960)
// Offsets below are verified against IDA raw files and Ghidra backup.
// Earlier drafts incorrectly listed 0x004019a0 / 0x00401a40 — those addresses do not exist.
short __fastcall CSQuest__FindQuestContext(DWORD This, DWORD pQuest, int index);          // 0x004011D0
BOOL  __fastcall CSQuest__CheckRequestCondition(void *This, DWORD pQuest);                 // 0x00401230
BOOL  __fastcall CSQuest__CheckActCondition(void *This, DWORD pQuest);                     // 0x00401650
int   __fastcall CSQuest__getQuestState(void *This, int index);                            // 0x004016E0
void  __fastcall CSQuest__CheckQuestState(void *This, int param);                          // 0x00401730
void  __cdecl CSQuest__ShowDialogText(int questId, int param);                             // 0x004017E0
void  __fastcall CSQuest__ShowQuestNpcWindow(void *This, int /*edx*/, int index);          // 0x00401910
void  __fastcall CSQuest__clearQuest(void *This);                                          // 0x00401960
int   __cdecl CSQuest__FindQuestItemsInInven(void);                                        // 0x00482DD0

// SMD parser
int   __stdcall FUN_0040b350(void);                                   // SMD tokenizer
void  __cdecl ParseNodes(void);                                      // SMD node parser
void  __cdecl ParseTriangles(bool Flip);                             // SMD triangle parser
void  __cdecl ParseSkeleton(void);                                   // SMD skeleton parser

// Camera / math
void  __cdecl AngleMatrix(float angles[3], float matrix[3][4]);      // 0x00440c80
void  __cdecl VectorIRotate(float in1[3], float in2[3][4], float out[3]); // 0x00440e00

// BMD transform
void  __fastcall BMD__TransformPosition(void *This, float (*BoneMatrix)[4], float *Pos, float *WorldPos, bool Translate); // 0x004409a0

// Sound helpers (FUN_0040a0a0 chain)
void  __cdecl Vec3_Normalize(float *param);                            // float init
// int   __fastcall FUN_00409f30(void *This, int p1, int p2, int p3, char p4); // open file — duplicate, correct 6-param version at line 1156
// void  __fastcall FUN_0040a300(void *This, int p1);                   // cleanup after open — duplicate, correct 3-param version at line 1159

// Render helpers
void  __cdecl EnableAlphaBlend(void);                                // 0x004f8830
void  __cdecl EnableAlphaTest(bool enable);                          // 0x004f8870
// RenderTerrainAlphaBitmap se declara mas arriba.  Aca habia una SEGUNDA
// declaracion que ademas mentia en dos cosas: la direccion (0x004FD100 cae
// dentro de Entity_SpawnEffects) y el orden de los dos ultimos parametros
// (son Rotation y Alpha, no alpha y size).

// Codec / JPEG internal helpers (batch 3)
void  __cdecl crt_exit(int param);                               // CRT init
void *__cdecl crt_tmpfile(void);                                    // CRT alloc
void  __cdecl _strncpy(char *dst, char *src, int n);                 // strncpy wrapper
// FUN_005430f0 already declared above (line ~713) as fwrite-wrapper
void  __cdecl crt_fflush(int *fp);                                 // fflush wrapper

// BST / RB-tree operations
void  __cdecl FUN_00411420(int *param_1);                            // BST recursive delete
void  __fastcall FUN_004112b0(int *param_1);                         // BST iterator increment
void  __fastcall FUN_00411870(int *param_1);                         // BST iterator decrement
void  __fastcall FUN_004118d0(int param_1);                          // list clear all
void  __fastcall FUN_004122c0(int param_1);                          // list trim excess
void  __fastcall FUN_00410d90(int param_1);                          // list destroy + free sentinel

// Hash cleanup (already have FUN_00406d40 as cdecl)
void  __cdecl FUN_00406d40(int param_1);                             // free hash buckets

// ── Batch 5-7 forward declarations (codec marker read/write, setup) ──


// Sub-functions called by batch 8 init functions

// Batch 10 — Sound/BST/Misc range: Quest UI, ErrorReport, SystemInfo, HashTable, GameGuard
void  __fastcall FUN_00402ff0(int param_1);                          // quest dialog answers render
void  __fastcall FUN_00403320(void* param_1);                        // quest NPC dialog render
UINT  __fastcall FUN_00403700(void* ecx, void* edx, UINT param_1);  // quest list by state
void  __fastcall FUN_00403a40(void* param_1);                        // quest UI main panel
UINT  __fastcall HashTable_GetIndex(void* ecx, void* edx, DWORD param_1); // IDA: FUN_004041e0
HRESULT __cdecl InitDirectSound(HWND hDlg);                          // 0x004044A0 — DirectSound8 init (src/Sound/Sound.cpp)
HRESULT __cdecl CreateStaticBuffer(int Buffer, const char* strFileName, int MaxChannel, bool Enable); // 0x00404650
void  __stdcall FUN_00405340(void);                                  // CErrorReport: rotate log
char* __stdcall CErrorReport_FindLogMarker(char* param_1);           // IDA: FUN_00405420
void  __fastcall CErrorReport_WriteSystemInfo(void* report);           // IDA: CErrorReport__WriteSystemInfo
void  __fastcall CErrorReport_WriteOpenGLInfo(void* report);           // IDA: FUN_004056B0
void  __fastcall CErrorReport_WriteImeInfo(void* report, void* edx, HWND hwnd); // IDA: CErrorReport__WriteImeInfo
void  __cdecl GetOSVersion(DWORD si);                                // detect Windows version
long long __cdecl Cpu_MeasureClockRate(DWORD param_1);               // IDA: FUN_00405e20
void  __cdecl GetCPUInfo(DWORD si);                                  // detect CPU vendor+model
DWORD __stdcall GetDXVersion_stub(void);                             // detect DirectX version
void  __stdcall CErrorReport_DumpCallStack(void);                    // IDA: FUN_00406660
// PacketCipher_Initialize (IDA: FUN_00406bd0) is declared above.
UINT  __fastcall PacketCipher_HashKey83(int param_1);                // IDA: FUN_00406e90
UINT  __fastcall PacketCipher_HashKeyConfigured(int param_1);        // IDA: FUN_00406ef0
int   __stdcall FUN_004070d0(int param_1, int param_2);              // GameGuard packet handler

// Batch 11 — BMD/SMD model loading, mesh rendering, shadow volumes, quest list
void  __fastcall FUN_00408ff0(void* param_1);                        // BMD mesh render prepare+draw
void  __fastcall FUN_004090b0(void* ecx, void* edx, int p1, float p2, int p3); // BMD emit quads
void  __fastcall FUN_004091d0(void* ecx, void* edx, int p1, int p2, float p3); // BMD emit vertex
void* __fastcall FUN_00409ad0(void* param_1);                        // IDA: sub_409AD0 (0x00409AD0), ctor de CPhysicsManager
void  __fastcall Locimp_dtor(void* param_1);                        // CSQuest destructor
void  __fastcall FUN_00409d20(int param_1);                          // CSQuest clear all nodes
int   __fastcall FUN_00409f30(void* ecx, void* edx, int p1, int p2, int p3, char p4); // BMD visible tri list
void  __fastcall FUN_0040a110(void* ecx, void* edx, short p1, short p2, short p3, int p4, int p5, int p6); // shadow edge add
void  __fastcall FUN_0040a1c0(void* ecx, void* edx, short p1, int p2, short p3, int p4); // shadow backface cull
void  __fastcall FUN_0040a300(void* ecx, void* edx, int param_1);   // shadow volume build
void  __cdecl ParseNodes(void);                                      // SMD parse nodes
void  __cdecl ParseSkeleton(void);                                   // SMD parse skeleton
void  __cdecl ParseTriangles(bool Flip);                             // SMD parse triangles
// FUN_0040b350 already declared above (line ~1017) as SMD skip-line
void  __stdcall FixupSMD_stub(void);                                 // SMD fixup post-process

// ─── Batch 12: JPEG codec (statically-linked libjpeg IJG) ───────────────────
// IAT thunks at 0x0052A0D0-0x0052A100 are linker-generated (IMM32 + dsound), no decl needed.

// Batch 13 — GameGuard / Crypto / IAT thunks (0x0053CB86–0x005406F0)
// IAT thunks (0x0053CB86..0x0053CBA4) resolved by linker — no declarations needed.
unsigned int __cdecl FUN_0053ce30(void *self, unsigned short *param_1, int param_2); // Crypto decrypt block
int   __stdcall CSimpleModulus_AddBits(int param_1, unsigned int param_2, int param_3, unsigned int param_4, int param_5); // IDA: FUN_0053CF90 (0x0053CF90)
void  __stdcall CSimpleModulus_Shift(unsigned char *param_1, int param_2, int param_3); // IDA: FUN_0053D0D0 (0x0053D0D0)
int   __cdecl CSimpleModulus_LoadKey(void *self, const char *filename, short magic, int k0, int k1, int k2, int k3); // IDA: FUN_0053D1C0 (0x0053D1C0)
// FUN_0053d430 — already declared above (GameGuard init)
void  __fastcall FUN_0053d620(DWORD param_1);                           // GameGuard cleanup
int   __cdecl FUN_0053d7d0(void *self, char *param_1);               // GameGuard main check
// FUN_0053d890 — implemented in GameGuard_Init2.cpp
void* __cdecl FUN_0053e8c0(void *param_1);                              // GG encrypted string decoder
// FUN_0053ea90 — already declared above (GameGuard query)
void  __cdecl FUN_0053eba0(int *param_1, void *param_2);                // GG encrypted log writer
int   __fastcall FUN_0053ed80(char *param_1);                           // GG full shutdown
int   __cdecl FUN_0053efa0(void *self, int param_1);                    // GG DLL loader + thread launcher
int   __fastcall FUN_0053f290(int param_1);                             // GG monitoring thread stop
void  __fastcall FUN_0053f680(int param_1);                             // GG crash reporter
int   __stdcall FUN_0053fcf0(void);                                     // Toolhelp32 API resolver
void  __cdecl FUN_0053fdd0(void);                                       // GG process snapshot
void  __cdecl FUN_0053feb0(void);                                       // GG module snapshot
unsigned int __stdcall FUN_005400d0(int param_1, unsigned int param_2);  // GG error dispatch
// Pipe_Write — already declared above (Pipe_Write wrapper)
// FUN_005404c0, FUN_00540a70, FUN_00540ac0 — already implemented in batch 10/11
int   __fastcall FUN_005406f0(int param_1);                             // GG pipe/handle cleanup

// Batch 15 — Font/Text rendering, BST cache, ListBox, KillGLWindow, GenerateCheckSum, DestroyWindow
void  __cdecl FUN_0040fcd0(void *self, unsigned char *param_1, int param_2, LONG param_3, int param_4); // Font_RenderText (GL texture)
void  __cdecl FUN_004102e0(void *self, int param_1, unsigned char param_2);  // Font_SetColorCode
void  __cdecl FUN_004104b0(void *self, char *param_1);               // Font_ParseColorMarkup
void  __cdecl FUN_004105f0(void *self, DWORD *param_1, int param_2, int param_3, int param_4); // Font_BuildColorBitmap
int   __cdecl FUN_004106d0(void *self, unsigned char *param_1, int param_2, int param_3, float param_4); // Font_CacheLookupAndRender
void  __stdcall  FUN_004108b0(int param_1, int param_2, int param_3, int param_4); // Font_RenderCachedBitmap
void  __cdecl FUN_00410e50(void *self, DWORD *param_1, int *param_2); // BST_EraseNode (red-black tree)
void  __cdecl FUN_00411360(void *self, int *param_1, int *param_2);   // BST_UnlinkNode
void  __cdecl FUN_00411460(void *self, DWORD *param_1, int param_2, int *param_3, int *param_4); // BST_InsertNode
void  __fastcall FUN_00411920(int *param_1);                             // ListBox_RenderItems
void  __cdecl FUN_004119a0(void *self, int param_1);                  // ListBox_ScrollUp
int   __fastcall FUN_00411a20(int *param_1);                             // ListBox_HandleInput
int   __fastcall FUN_00412180(int *param_1);                             // ListBox_HandleInput2
void  __fastcall FUN_00412510(DWORD *param_1);                           // ListBox_Destructor_A
void  __fastcall FUN_00412610(DWORD *param_1);                           // ListBox_Destructor_B
// KillGLWindow @ 0x00412AF0 — already in WinMain.cpp as OpenGL_Release
DWORD __cdecl GenerateCheckSum(BYTE *pbyBuffer, DWORD dwSize, WORD wKey); // Packet checksum (0x00412D50)
void  Game_DestroyWindow(void);                                          // Full cleanup (0x004145C0)
// WinMain @ 0x0041E8A0 — already documented in WinMain.cpp

// Batch 16 — Inventory, equipment, item management, skills, chat, terrain, NPC, GL
void  __cdecl SendRequestEquipmentItem(int iSrcType, int iSrcIndex, ITEM *pItem, int iDstType, int iDstIndex); // 0x0043C250
int   __stdcall FindHotKey(int Skill);                              // 0x004B1170
void  __cdecl RenderSkillIcon(int iIndex, float x, float y, float width, float height); // 0x004BB940
void  __cdecl SendChat(char *Text);                                      // 0x004C1B90
int   __cdecl ConvertGold64(int Zen, char *Buffer);                 // 0x004C3E10
void  __cdecl RenderItemName(int i, DWORD o, int ItemLevel, int ItemOption, bool Sort); // 0x004C9E70
void  __cdecl InsertWarehouseItem(unsigned int param_1, unsigned char *param_2); // 0x004CC0E0
void  __cdecl SetItemGridFlag(int param_1, int param_2, unsigned char param_3); // 0x004CC1E0
int   __cdecl CompareItems(short param_1, int param_2, int param_3); // 0x004CC270
void  __cdecl SortInventory(short *param_1);                        // 0x004CC3C0
unsigned long long __cdecl CheckInventorySpace(int p1, int p2, unsigned short *p3, int p4, int p5); // 0x004D5D70
int   __cdecl FindEmptySlot(int p1, int p2, int p3, int p4);       // 0x004D5F20
unsigned int __stdcall Inventory_DropItem(void);                    // 0x004D6470
long long __fastcall CalculateInventoryValue(int p1, unsigned int p2, short *p3, int p4, short *p5); // 0x004DF330
void  __cdecl InventoryColor(ITEM *p);                              // 0x004E2420
void  __stdcall RenderEquipmentBox(void);                           // 0x004E25A0
void  __cdecl RenderEquipmentPart3D(int Index, float sx, float sy, float Width, float Height); // 0x004E2E40
void  __stdcall RenderEquipment3D(void);                            // 0x004E3100
extern "C" void __cdecl RenderItemsBoxes(float fPosX, float fPosY, DWORD Inventory, int iMaxWidth, int iMaxHeight); // IDA: RenderItemsBoxes (0x004E37B0)
void  __cdecl RenderItems3D(float p1, float p2, short *p3, int p4, int p5, char p6); // 0x004E38B0
int   __cdecl CheckMixRecipe(short *p1, int p2, int p3);                // 0x004E40F0
extern "C" void __cdecl RenderInventoryInterface(int StartX, int StartY, int Flag); // IDA: RenderInventoryInterface (0x004ECBA0)
void  __cdecl RenderGuildMark(float p1, float p2, float p3, float p4, int p5); // 0x004F02F0
void  __cdecl AddTerrainLightClip(float xf, float yf, float Light[3], int Range, float Buffer[3]); // 0x004F7800
void  __cdecl RenderTerrainBlock(float xf, float yf, int xi, int yi, bool EditFlag); // 0x004F9720
void  __cdecl RenderTerrainFrustrum(bool EditFlag);                 // 0x004F97E0
void  __cdecl MoveObject_Special(int param_1); // legacy alias of FUN_004fa5f0 (0x004FA5F0)
char* __stdcall PickObject_Mouse(void);                             // 0x004FA7C0
float* __cdecl MoveObject_PerWorld(float param_1);                       // 0x004FDC00
int   __stdcall MoveHeavenThunder(void);                            // 0x004FED90
void  __stdcall MoveObjects(void);                                  // 0x004FF260
void  __stdcall MoveBugs(void);                                     // 0x005001F0
void  __cdecl OpenNpc(int Type);                                    // 0x005091D0
void  __stdcall GL_SetBlendInvSrcColor(void);                            // 0x00511810
void  __cdecl RenderSpriteUV(int Texture, float Position[3], float Width, float Height, float (*UV)[2], float Light[3][4], float Alpha); // 0x00511FB0
extern "C" double __cdecl RenderNumber2D(float x, float y, int Num, float Width, float Height); // IDA: RenderNumber2D (0x005122F0)
void  __stdcall MoveCamera(void);                                   // 0x0051E4E0
void  __cdecl Combat_UseWizardSkill(DWORD c, DWORD o);                  // 0x004889D0
bool  __stdcall Combat_UseElfSkillItem(DWORD c, DWORD pItem);           // 0x0048BD70

// Batch 17 — Entity, combat, rendering, IME, chat, particles
void  __cdecl FUN_0043ce50(unsigned char param_1, int param_2);          // action request packet (0x0043CE50)
void  __cdecl LookAtTarget(DWORD o, DWORD TargetCharacter);         // 0x0043E890
void  __cdecl MoveHead(int param_1);                                // 0x0043E940
bool  __cdecl BMD__CollisionDetectLineToMesh(DWORD pThis, float Position[3], float Target[3], bool Collision, int Mesh, int Triangle); // 0x00440BE0
bool  __cdecl AttackStage(DWORD c, DWORD o);                       // 0x00448930
// IDA: FUN_00451f30
void  __cdecl Combat_SpawnDeathDustParticles(int entity);
// IDA: FUN_00452030
void  __cdecl Combat_SpawnIdleAmbientParticle(int entity);
void  __stdcall MoveCharactersClient(void);                         // 0x00455010
unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char *param_1);   // hash read 1-byte (0x0045FAE0)
void  __cdecl Effect_SpawnBombRing(float *param_1); // IDA: FUN_00466300
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void  __cdecl FUN_00466300(float *param_1); // IDA: FUN_00466300
void  __cdecl ItemDrop_RenderGroundWeapon(int param_1); // IDA: FUN_0046b980
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void  __cdecl FUN_0046b980(int param_1); // IDA: FUN_0046b980
void  __cdecl RenderWheelWeapon(DWORD o); // IDA: RenderWheelWeapon (0x0046B7C0)
void  __cdecl FUN_0046c5a0(int p1, int p2, float *p3, float *p4);       // skill impact particles (0x0046C5A0)
void  __cdecl CreateBlood(DWORD o);                                 // 0x0046C680
void  __cdecl FUN_0046c7f0(int p1, int p2, float p3, float p4, float p5); // directional blood (0x0046C7F0)
void  __cdecl CreateArrow(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey); // 0x00474370
void  __cdecl CreateArrows(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey); // 0x00474BD0
void  __stdcall MoveParticles(void);                                // 0x00477090
unsigned int __cdecl Item_GetDefenseWithSpecial(DWORD ecx, short *param_1);            // item special option (0x0047CFB0)
void  __stdcall SetPositionIME_Wnd(float x, float y);              // 0x0047ECAF
bool  __cdecl CheckIME_Status(bool change, int mode);              // 0x0047EDC0
void  __stdcall RenderIME_Status(void);                             // 0x0047EE52
// FUN_0047f360 @ 0x0047F360 (IDA)
void  __cdecl Font_RenderTextToBitmap(int p1, int p2, LPCSTR p3, int p4, int p5, int p6, int p7, int p8, LPCSTR p9);
// FUN_0047f4c0 @ 0x0047F4C0 (IDA)
void  __cdecl Font_RenderBitmapText(int p1, int p2, float p3, float p4, int p5, int p6, float p7, int p8);
extern "C" void __cdecl RenderTipText(int sx, int sy, const char *Text);   // IDA: RenderTipText (0x0047F7F0)
int   __cdecl FUN_0047fed0(int lvl, const char* name);                 // IDA: sub_47FED0 (0x0047FED0) — gate de envio de susurro
void  __cdecl RegistWhisperID(int lvl, const char* text);              // IDA: RegistWhisperID (0x004801C0)
void  __cdecl FUN_00481a40(int param_1, char *param_2, int param_3);    // assign chat text (0x00481A40)
void  __cdecl AssignChat(char *ID, char *Text, int Flag);          // 0x00482090
int   __stdcall Item_FindElfWeaponInventorySlot(void);                   // IDA: FUN_004824C0
int   __stdcall Item_CountElfWeaponInventorySlots(void);                 // IDA: FUN_00482850
int   __cdecl Item_CountWeaponGroupItems(int param_1);                   // IDA: FUN_00482e40
void  __cdecl Item_AutoEquipAmmo(int weaponType);                              // elf weapon validation/swap (0x0048B680)
unsigned int __cdecl FUN_004942e0(int param_1);                          // chat command parser (0x004942E0)
bool  __cdecl CheckTarget(DWORD c);                                 // 0x0049CAE0

// Missing forward declarations (compilation fixes)
int   __cdecl Xor_ConvertBuffer(void *lpBuffer, DWORD nBytes, int key); // XOR encode buffer
void  __cdecl fscanf(int *fp, char *fmt);                          // CRT fscanf wrapper
void  __cdecl RenderCenterText(int x, int y, char *text);               // Quest UI center text

// Batch 18 — final 5 game functions
void  __stdcall InitGame(void);                                          // 0x00424490 — reset all game state on new session
void  __cdecl ReceiveChat(BYTE *ReceiveBuffer);                          // 0x00427630 — chat packet handler (opcode 0x00)
void  __fastcall FUN_00422c50(int param_1);                              // 0x00422C50 — BST node cleanup for session obj
void  __cdecl AppearMonster(DWORD c);                                    // 0x0042A0C0 — monster spawn animation+sound
void  __stdcall FUN_00422074(void);                                      // 0x00422074 — SEH __finally handler (DestroyWindow)
void  __cdecl FUN_0051d740(void);                                        // 0x0051D740 — ClearNotice
void  __cdecl FUN_00482350(void);                                        // 0x00482350 — ClearWhisperID
void  __cdecl FUN_004827a0(void);                                        // 0x004827A0 (mismap: NO es clearMatchInfo, esa direccion cae dentro de sub_4824C0)
void  __cdecl clearMatchInfo(void); // IDA: clearMatchInfo (0x0047EB80)
int   __cdecl Character_FindByKey_WithClear(int key);                                     // 0x0045ACC0 FindCharacterIndex + limpia el flag +744 de todas
void  __cdecl Characters_SetActionAll(int Action);                                  // 0x0045AD10 SetAction sobre todos los jugadores visibles
void  __cdecl FUN_00433830(void);                                        // 0x00433830 — InitPartyList
void  __stdcall SendCheck(void);                                         // 0x004220A0 — anti-tamper checksum packet

// Batch 20 — forward declarations for implemented stubs
void  __cdecl RenderText(int x, int y, char *text, int p1, int p2, void *p3); // Text render

// Batch 20 — OpenNpc, MoveCamera, RenderEquipment3D, RenderItems3D, LookAtTarget
void  __cdecl OpenTexture(int id, void *path, int flags, bool param);   // 0x00505C80
// LoadWaveFile declared above (line 1088) with real signature — real impl in src/Sound/Sound.cpp.
void  __cdecl OpenModel(int id, char *path, ...);                       // 0x00505E90 (varargs: texName, normName, etc.)
void  __cdecl RenderItem3D(float sx, float sy, float w, float h, int Type, int Level, int Option1, int ExtOption, bool PickUp); // IDA: RenderItem3D (0x004E1BE0)
// InventoryColor = InventoryColor (declared at line 1301)
// RenderEquipmentPart3D = RenderEquipmentPart3D (declared at line 1303)
void  __cdecl RenderNumber2D_fn(float x, float y, int Num, float Width, float Height); // 0x005122F0
// DisableAlphaBlend = GL_ResetState (declared at line 578)
// CreateAngle (4 floats → float). Line 207 has wrong sig; cast in callers.
// FarAngle está declarado arriba con su dirección IDA.
// RenderBitmap = GL_DrawTexture (declared at line 601)

// Missing function stubs (linker fix)
void* __fastcall FUN_0040f730(void* param_1);                              // UI widget constructor (0x2C4 bytes)
void  __fastcall FUN_00410a90(int* param_1);                               // Simple widget constructor (4 bytes)

// Batch 21 — forward declarations for MoveObjects, CollisionDetectLineToMesh, CheckMixRecipe
void  __cdecl FUN_004fa5f0(int pObj);                                      // Object_AnimUpdate (per-frame anim tick)
void  __cdecl FUN_004fdc00(float pObj);                                    // Object_RenderUpdate (terrain render per object)
void  __cdecl FaceNormalize(float v[3], float out[3], float v2[3], float normal[3]); // 0x00440A60 approx
// CollisionDetectLineToFace se declara mas arriba (0x00512D40).  Aca habia una
// SEGUNDA declaracion, la del stub, que decia ser "0x00440C90 approx" -- esa
// direccion cae DENTRO de sub_440BE0, que es su caller, no ella misma.
int   __cdecl ItemValue(ITEM* ip, unsigned int goldType);                  // 0x0047C690

// ── SkillElf dependencies ────────────────────────────────────────────────────
void  __cdecl GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana); // IDA: GetSkillInformation (0x0047E7A0)
// CheckArrow = Combat_CheckArrowRequirement (already declared as char __cdecl Combat_CheckArrowRequirement(void))
// SetPlayerAttack (already declared; Ghidra shows 1-arg DWORD — cast in caller)
// CreateAngle (already declared line 207; Ghidra shows 4 floats → float — cast in caller)
