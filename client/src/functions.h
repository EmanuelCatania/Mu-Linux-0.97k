#pragma once
#include <intrin.h>   // _ReturnAddress (used por HashTable_GetIndex defensive log)
#include <stdio.h>    // _snprintf_s (used por HashTable_GetIndex defensive log)
// float10 = Ghidra name for x87 80-bit extended float; MSVC uses long double (64-bit) as closest match
typedef long double float10;
// functions.h — Prototypes for all FUN_ (unresolved) functions.
//
// Kayito canonical name index (from main.exe.idb, 2026-01-03):
//   FUN_00466ad0 = MoveEffect       FUN_00470030 = MoveJoint       FUN_004736e0 = MoveJoints
//   FUN_00443e70 = SetAttackSpeed   FUN_004520c0 = MoveCharacterVisual
//   FUN_0043e820 = SetAction        Entity_AdvancePath = MovePath        FUN_00449900 = MoveCharacter
//   FUN_00456770 = RenderCharacter  FUN_0045ab00 = RenderCharactersClient
//   Combat_ProcessQueuedAction = Action           Combat_SendMovePathPacket = SendMove        FUN_004ac140 = CheckGate
//   Player_ProcessInput (IDA: FUN_004acef0) = MoveHero         FUN_004b14f0 = MoveInterface   FUN_004c4650 = RenderItemInfo
//   FUN_004c8d70 = RenderRepairInfo FUN_0045ccf0 = CreateMonster   FUN_0045f930 = CreateHero
//   Path_FindRoute = PathFinding2     Timer_UpdateFrameTiming (IDA: FUN_0043fd70) = CalcFPS         FUN_00440060 = BMD::Animation
//   BMD_TransformPosition = BMD::TransformPosition        FUN_00442090 = BMD::Release
//   FUN_004423e0 = BMD::Open        FUN_00442a60 = BMD::Save       FUN_004430c0 = SetPlayerStop
//   FUN_00443930 = SetPlayerWalk    FUN_0043e050 = CreateAngle
//   FUN_00444410 = SetPlayerAttack  FUN_00444b60 = SetPlayerShock  FUN_00445230 = AttackEffect
//   FUN_00448600 = CharacterAnimation               Combat_UseWarriorSkill = UseSkillWarrior
//   Combat_UseElfSkill = UseSkillElf      FUN_0047e4f0 = CHARACTER_MACHINE::GetMagicSkillDamage
// Signatures are best-effort from call-site analysis; wrong-arity calls will
// produce C4087/C2660 errors — fix by updating the specific prototype.
//
// MSVC CRT stubs — NOT game logic:
//   FUN_005416bc → crt_sprintf
//   FUN_0054519d → crt_output_engine
//   FUN_0054158c → operator_delete
//   FUN_00541c10 → __chkstk_probe (large-frame stack allocator)

// ── HashTable / ref-count obfuscation ─────────────────────────────────────────
// These are scattered throughout as anti-tamper; not game logic.
// Note: all args are void* to accept any pointer without cast errors.
void  __cdecl FUN_00403f80(void *ctx, void *obj, void *key);  // HashTable_Insert
void  __cdecl FUN_00404330(void *a, void *b);                 // HashTable_Release/remove (2-arg)
void* __cdecl FUN_00404280(void *ctx, void *key);             // HashTable_GetNode
uint  __cdecl HashTable_GetIndex(void *ctx, void *key);       // Returns slot index (0xffffffff = not found)
void  __cdecl FUN_00404040(void *ctx, void *key);  // HashTable_Decrement / release ref
void  __cdecl FUN_00403f30(void *a, void *b);
void  __cdecl FUN_00403a30(void);
void  __cdecl FUN_00404370(void *node, void *key);  // HashTable_Retain (ref-count inc on node+key)
void  __cdecl FUN_00404400(void *a, void *b);
void  __cdecl FUN_00409e20(void *key, void *node);            // HashTable_Update
// HashTable_GetIndex — dispatch via vtable at (DAT_055c9bc8+0xC)
// Returns slot index, or 0xFFFFFFFF if not found.
//
// DEFENSIVE (silencioso): anti-tamper invocado desde sistemas per-frame.  Un
// ctx/vtable corrupto producía un fn() a basura imposible de rastrear.
// Validamos y retornamos 0xFFFFFFFF (== "no encontrado") en lugar de morir.
// NO se loggea desde acá — hacerlo re-entra DbgLog cuando Windows bombea
// mensajes durante la escritura, y termina en stack smash.  Si se sospecha
// que la tabla hash está rota, mirar DAT_055c9bc8 en el debugger.
// 2026-05-03: SAFE STUB. Always return 0xFFFFFFFF (= "not found") so all
// callers' `if (idx != 0xffffffff) ...` guards skip the subsequent deref.
// Previously this called the vtable's hash function (g_FakeHashVtable[3] =
// HashFn_Sentinel returning 0) — but FUN_00404280 (HashTable_GetValue) then
// did its own key-match check and returned NULL when the slot's key didn't
// match the lookup key, causing NULL deref in callers like Game_MainLoop:158.
// Returning -1 here makes the table appear empty to callers, which IS our
// desired semantics (no anti-tamper data is actually stored).
inline unsigned int HashTable_GetIndex(void* /*ctx_ptr*/, void* /*key*/) {
    return 0xffffffff;
}

// ── Sound (0x00404xxx) ────────────────────────────────────────────────────────
void  __cdecl FUN_00404a10(int, const char*, int, char);      // Sound_LoadAndPlay(id, filename, type, loop)
// FUN_00404bc0 = PlayBuffer alias (callers use FUN_ name directly).
HRESULT __cdecl FUN_00404bc0(int Buffer, DWORD Object, BOOL bLooped);
uint  __fastcall FUN_00401230(void *param_1, short *slot_data, char flag); // CharSlot_Verify
void  __fastcall FUN_004017e0(int char_handle);  // CharSlot_SetServer
short __fastcall FUN_004011d0(void *pThis, short *param_1, int param_2); // CharSlot_FindFirstActive
void  __fastcall FUN_004016e0(void *pThis, int param_1);                 // CharSlot_DecodePrevState
uint  __fastcall FUN_00401650(void *pThis, short *param_1);              // CharSlot_FindEquipped
void  __fastcall FUN_00401730(void *pThis, char param_1);               // CharSlot_Navigate
int   __cdecl    FUN_00482dd0(int a, int b, uint c);                    // ItemSlot_Check
void  __cdecl Sound_StopBuffer(int);        // IDA: FUN_00404C60
void  __cdecl Sound_Update3DPositions(void);// IDA: FUN_00404CD0

// ── Config / net connect ──────────────────────────────────────────────────────
undefined4 __cdecl FUN_00402fd0(void *);
char  __fastcall FUN_00403150(void *pThis, int edx, char a2, char a3);   // lista de items de la quest (0x403150)
void  __cdecl FUN_00405540(void* ht, const char* msg, ...);
void  __cdecl FUN_004055a0(int);
void  __cdecl FUN_004058b0(void);
void  __cdecl FUN_004065f0(int, int, int, int);
void  __cdecl FUN_00406af0(int, int, int, int);
int   __cdecl FUN_00406b10(int server_idx, int channel);      // PVP flag query
int   __cdecl FUN_00406b30(BYTE* username); // ValidateUsername — returns '\0' if valid
void  __cdecl FUN_00406f50(char *);
void  __cdecl FUN_00406fd0(int, int, int, int);

// ── Network ───────────────────────────────────────────────────────────────────
void  __cdecl FUN_004080f0(int);           // Widget_BaseRelease — releases core widget struct
void  __cdecl FUN_004086e0(int, int, int); // Widget_Release — releases linked list + widget base
void  __cdecl FUN_00409c40(int);           // HashTable tick (1 arg)
void  __cdecl FUN_00409cf0(int);           // post-render hash tick (1 arg)
// FUN_00409e20 signature — canonical (void*,void*) at line 25 above
void  __cdecl FUN_0040c690(void*, undefined4, undefined4);  // Object_SetRectFields
void  __cdecl FUN_0040c7d0(int, int);
void  __cdecl FUN_0040e590(int);
void  __cdecl FUN_0040e990(int, int, int);
void  __cdecl FUN_0040f500(void);
void  __cdecl FUN_0040f610(HDC hdc, int x, int y, const char *text, DWORD color); // Text_DrawColored
void  __cdecl FUN_0040f670(int, int, int);

// ── Input ─────────────────────────────────────────────────────────────────────
void  __cdecl Music_StopTrack(DWORD, int); // IDA: FUN_004127F0
void  __cdecl Music_PlayTrack(DWORD, int); // IDA: FUN_00412890
void  __cdecl FUN_00412a70(int, int, int);
void  __cdecl FUN_00412de0(int, int, int, int);
void  __cdecl FUN_00413900(void);

// ── Combat / Entity interaction ───────────────────────────────────────────────
void  __cdecl FUN_00422df0(void *ctx, void *counter);  // Packet_UpdateCounter(ctx, &counter)
void  __cdecl FUN_00423040(void *ctx, void *chardata);  // CharData_Decode(ctx, g_CharData)
void  __cdecl FUN_00423710(void *a, void *b);  // HashTable_DeleteNode(node, key)
void  __cdecl FUN_00423760(void* node, void* key);
void  __cdecl Net_ConnectServer(const char* ip, unsigned int port); // FUN_00423920 — session TCP connect
void  __cdecl FUN_00423c40(int, int);
void  __cdecl FUN_00423c80(int, int, int);
void  __cdecl FUN_00423ce0(int, int, int, int);
void  __cdecl FUN_00423e10(int, int, int);
void  __cdecl FUN_00424010(int, int);
void  __cdecl FUN_00424240(int, int, int);
void  __cdecl FUN_00424390(int, int, int, int);
void  __cdecl FUN_004247d0(int, int, int);
void  __cdecl FUN_00425840(int, int, int);
void  __cdecl FUN_004264d0(int, int, int);
void  __cdecl FUN_004269f0(int, int, int);
void  __cdecl FUN_00426cf0(int, int, int);
void  __cdecl FUN_00427560(int, int, int);
void  __cdecl FUN_00427a00(int, int, int);
void  __cdecl FUN_00427b90(int, int);
void  __cdecl FUN_00427f40(int, int, int);
void  __cdecl FUN_00428210(int, int, int);
void  __cdecl FUN_00429230(int, int, int);
void  __cdecl FUN_00429690(int opcode, int data_ptr, int len); // PacketHandler_0x12..
void  __cdecl FUN_00429c50(int, int, int);
void  __cdecl FUN_0042a230(int, int, int);
void  __cdecl FUN_0042a530(int, int, int);
void  __cdecl FUN_0042acc0(int, int, int);
void  __cdecl FUN_0042b4f0(int, int, int);
void  __cdecl FUN_0042bc00(int, int, int);
void  __cdecl FUN_0042bca0(int opcode, int data_ptr, int len); // PacketHandler_0x19 (skills)
void  __cdecl FUN_0042cd10(int, int, int);
void  __cdecl FUN_0042d780(int, int, int);
void  __cdecl FUN_0042db60(int, int, int);
void  __cdecl FUN_0042e5c0(int, int, int);
void  __cdecl FUN_0042f030(int, int, int);
void  __cdecl FUN_0042f240(int, int, int);
void  __cdecl FUN_0042f360(int, int, int);
void  __cdecl FUN_0042f690(int, int, int);
void  __cdecl FUN_0042f9a0(int, int, int);
void  __cdecl FUN_00423db0(void);                // Net_SendFlush / packet queue flush

// ── Party / Guild ─────────────────────────────────────────────────────────────
void  __cdecl FUN_004301b0(int, int, int);
void  __cdecl FUN_00431180(int, int, int);
void  __cdecl FUN_00431480(int, int, int);
void  __cdecl FUN_00431780(int, int, int);
void  __cdecl FUN_00431a90(int, int, int);
void  __cdecl FUN_00431dc0(int, int, int);
void  __cdecl FUN_00431ea0(int, int, int);
void  __cdecl FUN_004321f0(int, int, int);
void  __cdecl FUN_004332e0(int, int, int);
void  __cdecl FUN_004337f0(int, int, int);
void  __cdecl FUN_00433900(int opcode, int data_ptr, int len); // PacketHandler_0x44 (party HP)
void  __cdecl FUN_00433a80(int, int, int);

// ── Protocolo Trade / Shop / Guild ────────────────────────────────────────────
void  __cdecl FUN_00434170(int, int, int);
void  __cdecl FUN_00434400(int, int, int);
void  __cdecl FUN_00434450(int, int, int);
void  __cdecl FUN_00434660(int, int, int);
void  __cdecl FUN_00434780(int, int, int);
void  __cdecl FUN_004348b0(int, int, int);
void  __cdecl FUN_00434950(int, int, int);
// IDA: FUN_00434DC0 — GuildMark_UpsertRecord (nombre + tabla de mark 8x8); no pertenece a Trade.
void  __cdecl FUN_00434dc0(int opcode, int data_ptr, int len);
void  __cdecl FUN_00435110(int, int, int);
void  __cdecl FUN_00435280(int, int, int);
// Guerra de guild: ReceiveDeclareWar, ReceiveDeclareWarResult,
// ReceiveGuildBeginWar y ReceiveGuildEndWar.
void  __cdecl FUN_00435390(int, int, int);
void  __cdecl FUN_004353e0(int, int, int);
void  __cdecl FUN_004354f0(int, int, int);
void  __cdecl FUN_00435aa0(int, int, int);
void  __cdecl FUN_00436550(int, int, int);
void  __cdecl FUN_004366c0(int, int, int);
void  __cdecl FUN_004367d0(int, int, int);
void  __cdecl FUN_00436820(int, int, int);
void  __cdecl FUN_00436a80(int, int, int);
void  __cdecl FUN_00436ac0(int, int, int);
void  __cdecl FUN_00436cb0(int, int, int);
void  __cdecl FUN_00436d60(int, int, int);
void  __cdecl FUN_00436e40(int, int, int);
void  __cdecl FUN_00436fb0(int, int, int);
void  __cdecl FUN_004372c0(int, int, int);
void  __cdecl FUN_00437380(int, int, int);
void  __cdecl FUN_004373a0(int, int, int);
void  __cdecl FUN_004373d0(int, int, int);
void  __cdecl FUN_00437400(int, int, int);
// ── Guild UI helpers (called from guild packet handlers) ─────────────────────
void  __cdecl CreateOkMessageBox(char *strMsg);                   // 0x0051D6F0 — CreateOkMessageBox (Offsets.h canonical name)
#define FUN_0051d6f0  CreateOkMessageBox
void  __cdecl FUN_0051d9e0(int count, int param2, void *data);    // GuildMemberList_Update — full member list
void  __cdecl FUN_0051da80(int param1, void *data);               // GuildMemberList_Add — single member record
void  __cdecl FUN_00437450(int, int, int);
void  __cdecl FUN_00437480(int, int, int);
void  __cdecl FUN_004374b0(int, int, int);
void  __cdecl FUN_004374e0(int, int, int);
void  __cdecl FUN_00437f10(int, int, int);
void  __cdecl FUN_0043bde0(int, int, int);
void  __cdecl FUN_0043c070(int, int, int);
void* __cdecl FUN_0043d1d0(void *ctx, void *chardata);  // CharData_Encode(ctx, g_CharData) → ptr
// FUN_0043d3e0: 2-arg form — (HashTable*, int*) used in Ring_ComputeOrbit (see line 840)
void  __cdecl FUN_0043d8a0(void *ctx, void *out);  // HashTable_Timer — updates out from ctx
int   __fastcall FUN_0043dcc0(int);        // Object_GetType (return *(param+8))
void  __cdecl FUN_0043de60(void);  // Net_FlushQueue (no args)
void  __cdecl FUN_0043df90(int, int, int);
float __cdecl FUN_0043e050(float x1, float y1, float x2, float y2); // CreateAngle (atan2-based angle between two points)
float __cdecl Angle_GetDifference(float curAngle, float tgtAngle, char mode); // IDA: FUN_0043e370
void  __cdecl RenderInformation(void);
float __cdecl FUN_0043e1b0(float curAngle, float tgtAngle, float t);   // AngleLerp (smooth angle interpolation)
void  __cdecl FUN_0043e570(float *pos, float *dir, float *vel); // Vector3_Normalize (cross/normalize vel from pos+dir)
void* __cdecl FUN_0043e820(int entity_ptr, int anim_id);   // SetAction — definicion unica en stubs_externs.cpp
void  __cdecl FUN_0043e5c0(int entity_ptr);                // Particle_CommitState
void  __cdecl FUN_0043e680(int entity_ptr, int slot_idx, int table_base, int stride); // Particle_PathUpdate
// Pathfind(src_cached_x, src_cached_y, tgt_x, tgt_y, path_buf, reserved) → 1=ok 0=fail
unsigned int __cdecl Path_FindRoute(int src_x, int src_y, int tgt_x, int tgt_y, unsigned char *path, float reserved); // IDA: FUN_0043f3e0
// PathFinder2_Solve: A* grid search — implemented in Game/PathFinder.cpp
uint  __cdecl FUN_0043f500(void *_this, int sx, float sy, int tx, int ty, int filter, int walkmax, float radius);
// A* BST priority queue helpers — implemented in Game/PathFinder.cpp
undefined4 __cdecl FUN_0043fd30(void *_this, int idx);
void  __cdecl FUN_0043fea0(void *_this, undefined4 data, int cost);
undefined4 __cdecl FUN_004232f0(void *_this, int *pnode);
void  __cdecl FUN_004235d0(void *_this, undefined4 *node);
void  __fastcall FUN_0043ff60(undefined4 *open_set);
void  __cdecl Timer_UpdateFrameTiming(void); // IDA: FUN_0043fd70

// ── Model render helpers (FUN_004400xx / FUN_004410xx / FUN_004414xx) ─────────
// Called on model context pointer (this = DAT_05828d58 + type*0xbc).
void  __cdecl FUN_00440d30(void);                                  // Model_PushMatrix (glPushMatrix-like setup)
// Model_RenderLayer(model, layer, anim_id, scale, height, x, y, z, color)
void  __cdecl FUN_00440d50(void *model, float layer, int anim_id, float scale, int height,
                            float x, float y, float z, unsigned int color);
// Model_Render(model, param2, param3, anim_mode, flags, extra)  — 6-arg form used in Entity_DrawSetup
void  __cdecl FUN_00441e00(void *model, int flags, float f1, int f2, float f3, float f4, float f5, int f6, int rgba);
void  __cdecl FUN_00441be0(void *model, int a, int b);             // Model_KillAnim(model, 0, anim_slot)
void  __cdecl FUN_00441f00(void *model, int height_int, int anim); // Model_RenderShadow(model, height, anim)
void  __cdecl FUN_00442090(int model_ptr);                         // Model_Unload (free BMD model data)
void  __cdecl FUN_00442260(void *model);                           // BMD_PostActionsInit (post-load action setup)
void  __cdecl FUN_00442e00(void *model, char flag);                // BMD_PostLoadInit (post-load finalize)
void  __cdecl FUN_004422f0(void *pThis, int param_1, int param_2, int param_3);  // BMD_BuildAdjacentFaceTable
void  __cdecl FUN_00442e60(int param_1);                           // BMD_ComputeBounds
// VTable helpers used by type 0x13f/0x14a entity dispatch
void* __fastcall FUN_0040a660(void *param_1);                      // WidgetB_CtorFull
void* __fastcall FUN_00409ed0(void *param_1);                      // WidgetB_Ctor
void  __fastcall FUN_00409f10(void *param_1);                      // WidgetB_SetVtable
void  __fastcall FUN_00409f20(int param_1);                        // WidgetB_ZeroFields
void  __cdecl FUN_0040a860(void *obj, int vtable_id);              // VTable_Release
void  __cdecl FUN_0040a8f0(void *obj, float *p1, float *p2);      // VTable_DrawEdge

// ── Sprite / Animation ────────────────────────────────────────────────────────
// Sprite_Draw — Sprite_Draw(model, flags, rot, pos_param, scale, anim_ptr, dir_ptr, a, b)
void  __cdecl FUN_00440060(void *model, int flags, float rot, unsigned int pos_param,
                            unsigned char scale, unsigned int *anim_ptr, float *dir_ptr,
                            char a, char b);
void  __cdecl FUN_004404e0(void *model, int bone, float *anim1, float *anim2, float *anim3, char blend); // Sprite_DrawBone
void  __cdecl BMD_TransformPosition(void *model, float *bone_data, float *out_pos, float *out_col, char flag); // Bone_GetWorldTransform
// Compatibility bridge used only by stubs_IDA_ports.cpp; canonical implementation is BMD_TransformPosition.
void  __cdecl FUN_004409a0(void *model, float *bone_data, float *out_pos, float *out_col, char flag); // IDA: FUN_004409a0
void  __cdecl FUN_00440a30(void *model, float *bone_mat, float *pos_in, float *pos_out); // BoneTransformOffset — implemented in Render/BMD_SetupRender.cpp
int   __cdecl FUN_00502ba0(int param_1); // Entity_Reset — implemented in Entity/Entity_Reset.cpp
void  __cdecl FUN_004423e0(int, int, int, int);
undefined4 __cdecl FUN_00442a60(int thisModel, char *pathDir, char *pathFile);  // BMD_SaveToFile
void  __cdecl FUN_004430c0(int entity);        // Entity_CancelAction (1 arg)
void  __cdecl FUN_00443930(int entity);        // Entity_StartMoveAnim
unsigned int __cdecl Entity_AdvancePath(void *entity, char flag); // IDA: FUN_0043ea20
void  __cdecl FUN_00454ba0(int entity);        // Entity_StopMove
void  __cdecl FUN_0045c130(int entity);        // Entity_CancelTarget
void  __cdecl FUN_0045c720(int entity);        // Character_UpdateEquipSlotAnimations
void  __cdecl FUN_00443e70(void);              // Entity_LockAnim (no args per decompile)
void  __cdecl FUN_00444410(int, int, int, int);
void  __cdecl FUN_00444a80(int param_1);       // SetPlayerMagic — cast magic anim
void  __cdecl FUN_00444b30(int param_1);       // SetPlayerTeleport - anim de casteo de teleport (accion 87 / 5)
void  __cdecl FUN_00444b60(int c, int Hit);    // SetPlayerShock — knockback/grunt
void  __cdecl FUN_00444d90(int entity_idx);    // SetPlayerDie — death anim/FX (was mismapped as Entity_TeleportEnd)
// ── Character/Effect update pool (0x004796xx) ────────────────────────────────
void  __cdecl Render_DrawSprite(int slot);               // IDA: FUN_00479670
void  __cdecl Render_DrawSpritePool(void);               // IDA: FUN_00479730
void  __cdecl FUN_00479790(void);              // Effect_UpdateAll (mark all active)
// GL_SetBlendSrcOver / GL_SetBlendAdditive / GL_SetBlendSrcAlpha — declared below (GL state / sound)
int   __cdecl FUN_004cb520(void);              // GetLoginAnimTime → returns frame counter
void  __cdecl FUN_004f8eb0(float *world_pos);  // Login_CameraUpdate

// ── Entity update / render core ───────────────────────────────────────────────
void  __cdecl FUN_00454fc0(float *);
void* __cdecl FUN_00456770(void *entity, void *vis_entity, void *param_3); // Entity_UpdateRender (2195 lines)
void* __cdecl FUN_00456590(int entity, int type, float scale, int bone_count, float angle, int flag1, float flag2); // Entity_BoneAllParticles
void* __cdecl FUN_00456650(int entity, int bone_start, int bone_end, float scale); // Entity_BoneRangeParticles
void  __cdecl FUN_0045ab00(void);                                    // Entity_RenderAll_3D

// ── Entity render helpers (used by FUN_00456770 / Entity_UpdateRender) ───────
void  __cdecl FUN_00504130(void *model, int entity, int type, float alpha, uint draw_flags); // Entity_DrawSetupBase — implemented in Render/BMD_SetupRender.cpp
void* __cdecl FUN_00504960(void *model, int entity, int entity_type, float scale, int flags, float alpha, int rgba); // Model_RenderSkeleton
void* __cdecl FUN_00504ac0(void *model, int entity, int entity_type, float scale, int flags, float alpha, int rgba); // Model_RenderSkeleton2
void  __cdecl FUN_00455430(float ox, float oy, float oz, int entity, int weapon_slot_ptr, int anim_id, char level, unsigned int option1, char link, char translate, unsigned int render_type); // RenderLinkObject
void  __cdecl FUN_00449840(int entity_a, int entity_b, int flags);   // Entity_DrawOverlayFx
bool  __cdecl FUN_00451b20(int entity);                              // CheckFullSet — sets EquipmentLevelSet
extern int EquipmentLevelSet;                                         // 0=no glow, 9-11=lightning tier
extern int g_bAddDefense;                                             // set completo → bonus de defensa en FUN_0047e160
float __cdecl FUN_004f7500(float xf, float yf);                      // RequestTerrainHeight(xf, yf) → bilinear height
void  __cdecl FUN_004f7960(float grid_x, float grid_y, float *rgb_out); // Terrain_SampleColor → RGB[3]
void  __cdecl FUN_004553c0(void *model, int type, int bone_idx, float scale, float *color, int entity); // Model_BoneParticle
void  __cdecl FUN_00503cf0(int weapon_id, float scale, float half_scale, float *color, char flag); // Weapon_SetColor: fills color[3] from item type + scale factors
void  __cdecl FUN_00503fe0(int weapon_id, float scale, float half_scale, float *color);           // Weapon_SetColorAlt: simpler version (no flag)
// IDA: FUN_004552C0 — compone la matriz del emblema de guild sobre un jugador visible.
void  __cdecl FUN_004552c0(int entity, int shield_id);
// IDA: FUN_004F0100 — CreateGuildMark, compone la textura 34 desde la tabla compartida de marks.
extern "C" void __cdecl CreateGuildMark(int mark_index, bool blend);
void* __cdecl FUN_004f8bb0(int type, float x, float y, float sx, float sy, float *color, float angle, float alpha); // Particle_DrawBillboard

// ── Skill/weapon widget system (UI overlay beams) ─────────────────────────────
void* __fastcall FUN_004093a0(void *param_1);                        // Widget_Ctor
void* __fastcall FUN_00407fe0(void *param_1);                        // Widget_CtorBase
void  __cdecl FUN_00541ec1(void *arr, int size, int elem, void *ctor); // Array_InitWithCtor
void  __cdecl FUN_004093e0(void *widget, int entity, short *slot, int type, int radius, int flags); // Widget_BindEntity
void  __cdecl FUN_00409250(void *widget, float x, float y, float z, float radius, int boneIdx); // Cloth_AddAnchor
int   __cdecl FUN_00408900(int *widget, unsigned int hash, int flags); // Widget_CheckState → 0=done,1=active
void  __cdecl FUN_00408130(void *widget, float entity, int p3, float p4, float p5, int p6, int p7, float p8, float p9, int type_a, int type_b, int flags); // Widget_Init
void  __cdecl FUN_0045ac20(int, int, int);
int   __cdecl FUN_0045ac80(int);             // FindCharacterIndex @ 0045AC80
void  __cdecl FUN_0045adc0(unsigned char *c, int Type, int PositionX, int PositionY, float Rotation); // CreateCharacterPointer
unsigned int __cdecl FUN_0045bfa0(int Key, int Type, unsigned char PosX, unsigned char PosY, float Rotation); // CreateCharacter
void  __cdecl FUN_0045c8c0(int Key, BYTE *Equipment); // ChangeCharacterExt
char* __cdecl FUN_0045ccf0(uint, int, int, int, int);
unsigned char* __cdecl FUN_0045f930(int Index, int Class, int Skin, float x, float y, float Rotate); // CreateHero
void  __cdecl Monster_SaveSetBase(const char*); // FUN_0045fa20 — writes MonsterSetBase position records
float* __cdecl Effect_Create(int type, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, byte flag); // IDA: FUN_00460dc0
// Compatibility bridge used only by stubs_IDA_ports.cpp.
float* __cdecl FUN_00460dc0(int type, float *p1, float *p2, float *p3, float *p4, float *p5, float *p6, float *p7, byte flag); // IDA: FUN_00460dc0
void  __cdecl Effect_SpawnSmokeBurst(float *, char); // IDA: FUN_004660f0
void  __cdecl Effect_SpawnSmokeExplosion(undefined4 *, char); // IDA: FUN_004661f0
void  __cdecl Effect_SpawnLightningBurst(int); // IDA: FUN_00460c30
void  __cdecl Effect_SpawnProximityHit(int); // IDA: FUN_00465e60
void  __cdecl FUN_00466440(int);                        // STUB: HashTable obfuscation — not portable
void  __cdecl FUN_00473d90(int, float *, float);        // Ring_ComputeOrbit

// ── Particle / effect system ──────────────────────────────────────────────────
void  __cdecl Effect_TickAll(void); // IDA: FUN_0046b790
void  __cdecl EffectPool_RenderAll(void); // IDA: FUN_0046bba0
// 2026-05-07: Particle_Render real es void(void) per IDA mu97k-src-IDA/raw/
// 0046BE40_Particle_Render.c. La firma anterior (6 args) era erronea — el
// llamador en Game_RenderTick lo invoca sin args.
void  __cdecl Particle_RenderAll(void); // IDA: FUN_0046be40
void  __cdecl FUN_0046c3e0(void);
void  __cdecl SkillEffects_RenderAll(void); // IDA: FUN_0046cb70
void  __cdecl WeatherParticles_Update(void); // IDA: FUN_0046cc80
void* __cdecl Joint_Create(int, float *, float *, float *, unsigned int, int, float, short, unsigned char); // IDA: FUN_0046d840
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void* __cdecl FUN_0046d840(int, float *, float *, float *, unsigned int, int, float, short, unsigned char); // IDA: FUN_0046d840

// ── Item drop render ──────────────────────────────────────────────────────────
void  __cdecl Joint_TickAll(void); // IDA: FUN_004736e0
void  __cdecl FUN_00473710(void);                                   // ItemDrop_Render
void  __cdecl FUN_00473ea0(int, float *, unsigned int, unsigned int, unsigned int, float, unsigned int, float); // Particle_Spawn
void  __cdecl FUN_004741e0(int, int, int, int);
void  __cdecl FUN_004742b0(int, int, int, int);
void  __cdecl FUN_00474310(int, int, int, int);

// ── Player render ─────────────────────────────────────────────────────────────
void  __cdecl FUN_00474f90(int char_class, float *pos, float *rot, float scale); // Player_DrawInstance
void  __cdecl Effect_TickFade(void); // IDA: FUN_00475090
void  __cdecl FUN_00475110(void);                                   // Player_Render
void  __cdecl FUN_00475170(int particle_slot);                      // Particle_InitDir — sets dir vector from entity pos delta
int   __cdecl Particle_Spawn(int type, float *bone_mat, float *pos, float *size, int flag, float alpha, int mode); // IDA: FUN_00475220

// ── Char list / spawn ─────────────────────────────────────────────────────────
void  __cdecl FUN_00478c00(void);
void  __cdecl FUN_004792c0(int, int, int);
void  __cdecl FUN_00479330(int, int, int, int);
void  __cdecl DamageNumbers_Tick(void); // IDA: FUN_00479380
void  __cdecl Effect_TickFlare(void); // IDA: FUN_004794a0
int   __cdecl FUN_004795c0(unsigned short, float *, float, float *, int, float, int); // Particle_Spawn7
// Render_DrawSprite / Render_DrawSpritePool / FUN_00479790 — implemented in src/stubs.cpp (Character/Effect pool)
void  __cdecl FUN_0047b910(int, int, int);
void  __cdecl FUN_0047cef0(int, int, int);
int   __cdecl FUN_0047cf40(short* a1, int a2, int a3, unsigned short a4);  // Stats_ApplyBonus2
void  __cdecl FUN_0047d330(int, int, int);
void  __cdecl FUN_0047d3d0(int, int, int);
// Stat helpers — ported 2026-05-02. Signatures match IDA decompile.
int   __fastcall FUN_0047d410(int characterMachine);                // Stats_CalcBase (attack damage)
int   __cdecl    FUN_0047dae0(int characterMachine);                // Stats_CalcMagicDmgRange
int   __cdecl    FUN_0047dd50(short* characterMachine_words);       // Stats_CalcAddStrength
int   __cdecl    FUN_0047dd80(int characterMachine);                // CalculateAttackSpeed
int   __cdecl    FUN_0047dfe0(int characterMachine);                // Stats_CalcDefense
int   __cdecl    FUN_0047e160(int characterMachine);                // Stats_CalcCritBase
int   __cdecl    FUN_0047e2e0(short* characterMachine_words);       // Stats_CalcExtraOption1
int   __cdecl    FUN_0047e310(int characterMachine);                // Stats_CalcExtraOption2
int   __cdecl    FUN_0047e3c0(int characterMachine, int, int);      // CharData_RecalcStats
void  __cdecl FUN_0047e7a0(int, int, int);
void  __cdecl FUN_0047ec00(int, int, int);
int   __cdecl Input_IsKeyJustPressed(int);          // IDA: FUN_0047ec20
void  __cdecl Input_ClearState(int mode);           // IDA: FUN_0047ec60
// FUN_0047F0B0 @ 0x0047F0B0
void  __cdecl UI_RenderInputField(int, unsigned int, int);
// FUN_0047F650 @ 0x0047F650
undefined8  __cdecl UI_RenderText(undefined4 x, undefined4 y, LPCSTR str, LPSIZE max_w, char bold, undefined4 extra);
void  __cdecl FUN_0047f6f0(int, int, int);
// FUN_0047F7A0 @ 0x0047F7A0
void  __cdecl UI_DrawText(int x, int y, char* text, int max_width, int style, int extra);
// FUN_0047FAE0 @ 0x0047FAE0
void  __cdecl UI_AddNotice(char *, unsigned char);
void  __cdecl Chat_TickNoticeTimer(void);                // IDA: FUN_0047fcb0
// FUN_0047FCE0 @ 0x0047FCE0
void  __cdecl UI_RenderNotices(void);
void  __cdecl FUN_004801c0(int, int, int);
void  __cdecl UIChatLogWindow_AddText(const char* strID, const char* strText, int MsgType); // 0x00480620
#define FUN_00480620  UIChatLogWindow_AddText
#define UI_ShowDialog UIChatLogWindow_AddText
void  __cdecl Chat_TickMessageTimer(void);               // IDA: FUN_00480950
// FUN_00480980 @ 0x00480980
void  __cdecl UI_RenderChatLogOverlay(void);
void  __cdecl FUN_00480c60(int, int, int);
void  __cdecl FUN_00480e00(int, int, int);
void  __cdecl FUN_00481ba0(char* ID, char* Text, DWORD entity, int Flag, int SetColor); // CreateChat
// FUN_004821A0 @ 0x004821A0
void  __cdecl UI_TickHoverBubbles(void);
int   __cdecl FUN_00482be0(int category);  // GetItemSlot — returns inventory slot index or -1
void  __cdecl FUN_00483780(int, int, int);
char  __cdecl Item_Equip(DWORD character, DWORD object);            // 0x00483B30, Attack selected-skill executor

// ── UI / HUD ──────────────────────────────────────────────────────────────────
uint  __cdecl FUN_004977f0(char* src, void* key, char flags);  // String_CompareKey (returns non-zero if match)
void  __cdecl FUN_0047fe30(void*, int, void*, int);            // Chat_SplitLine
void  __cdecl FUN_00497870(int, int, int, int);
void  __cdecl FUN_004ac140(void);                                    // Player_ActionSubTick
char  __cdecl Path_IsLineClear(int src_x, int src_y, int tgt_x, int tgt_y); // IDA: FUN_004830b0
// IDA: FUN_00491c40
void  __cdecl Combat_SendMovePathPacket(int entity, int entity2);                 // sends the local hero's path packet
// IDA: FUN_0049cbf0
void  __cdecl Combat_DispatchHeroSkillAttack(void *entity);                       // hero skill/attack dispatcher
// IDA: FUN_0048ba70
char  __cdecl Combat_CheckArrowRequirement(void);                                  // validates arrow/bolt availability
// Combat_ProcessQueuedAction @ 0x0048D640 — Action(c, o) — IDA name "Action".
// Despachador real de acciones (pickup/equip/attack/skill/walk) basado en
// `*(c+749)` queue. Implementación en stubs.cpp.
// IDA: FUN_0048d640
void  __cdecl Combat_ProcessQueuedAction(DWORD c, DWORD o);
// Send_MovePacket_Player_legacy_stub — el viejo stub zero-arg que vivía en
// Combat_ProcessQueuedAction. Sigue mandando packet 0x10 movement. Los call-sites
// existentes que esperaban el comportamiento viejo siguen llamando a éste.
void  __cdecl Send_MovePacket_Player_legacy_stub(void);
unsigned int __cdecl FUN_00483160(void);                             // Entity_CanAct (returns 0 if locked)
void  __cdecl Player_ProcessInput(void); // IDA: FUN_004acef0
void  __cdecl Mouse_UpdateHoverTargets(void);                                    // IDA: FUN_004b0310
void  __cdecl FUN_004b0e80(void);                                    // Hotkey_Trigger — executes hotkey slot action
void  __cdecl FUN_004b14f0(void);
void  __cdecl FUN_004bbdd0(int, int, int, int);
void  __cdecl Cursor_Render(void);                                               // IDA: FUN_004bffa0
void  __cdecl Input_ProcessFunctionKeys(void);      // IDA: FUN_004c04a0
void  __cdecl UI_UpdateFpsCounter(void);                                         // IDA: FUN_004c14e0
void  __cdecl FUN_004c3530(void);
unsigned int __cdecl Item_CalculateMaxDurability(void* item, int attrBase, int level); // IDA: FUN_004C45C0
int          __cdecl Item_CalculateValue(void* item, int sellMode);                    // IDA: FUN_0047C690
unsigned int __cdecl Item_CalculateRepairCost(int Gold, int Durability, int MaxDurability, short Type, char* Text); // IDA: FUN_004C3EF0
// FUN_004cb520 — declared above as int(void) GetLoginAnimTime
void  __cdecl FUN_004cb6f0(int, int, int, int);
void  __cdecl FUN_004cba60(void);                                    // CharPreview_Reset (resets char/NPC preview UI)
char  __cdecl FUN_004e3d60(void *ctx, int p1, int p2);              // Connection_Check → '\0'=ok else error
DWORD __cdecl FUN_00494520(void *key_val, BYTE *buf, char flag);    // IME_InputChar — input char into buffer; returns '\0' = no-enter
char  __cdecl FUN_00513440(char *buf);                               // Chat_Validate — validate text buf; '\0'=ok
void  __cdecl FUN_00497c70(void);                                    // UI_SendFacingPacket (called on Enter from non-elf class)
void  __cdecl Net_ProcessReceiveQueue(void); // FUN_004cbdf0
void  __cdecl FUN_004cc530(int, int, int, int);
void  __cdecl FUN_004cc660(BYTE* Inv, int Width, int Height,
                           int Index, BYTE* Item, int First);   // InsertInventoryItem
void  __cdecl FUN_004cce00(int, int, int, int);                    // UI_Main
void  __cdecl FUN_004cd3b0(void);  // CharPreview_Refresh (no args per call-site)

// ── 3D / terrain / world ──────────────────────────────────────────────────────
void  __cdecl FUN_004e1be0(float x, float y, float w, float h, int entity_slot, uint type, byte class_id, char flag);  // Draw char preview
void  __cdecl FUN_004e13a0(int effect_id, unsigned int type, unsigned char class_id, unsigned char ext_option, float *pos, int in_bounds, char flag); // ItemDrop_SpawnEffect
int   __cdecl FUN_004e9250(int mode);            // SecondPassword_Shuffle (10-element array shuffle)
void  __cdecl FUN_004e9300(int, int, int, int);
void  __cdecl FUN_004eb070(int, int, int, int);
void  __cdecl Scene_ProcessPacketUpdates(void); // FUN_004ecb00
void  __cdecl FUN_004ecc60(int, int, int, int);
void  __cdecl FUN_004ef160(int, int, int, int);
// FUN_004f0100 — declared above as (int entity_id, char flag) NPC_SetFlag
void  __cdecl FUN_004f0480(int, int, int, int);
void  __cdecl FUN_004f0810(int, int, int, int);
void  __cdecl FUN_004f0a50(int, int, int, int);
void  __cdecl FUN_004f1270(int, int, int, int);
void  __cdecl FUN_004f1f50(int, int, int, int);
void  __cdecl FUN_004f27f0(int, int, int, int);
void  __cdecl FUN_004f3170(int, int, int, int);
void  __cdecl FUN_004f3c50(int, int, int, int);
void  __cdecl FUN_004f46a0(int, int, int, int);
void  __cdecl FUN_004f54b0(int, int, int, int);
void  __cdecl FUN_004f5570(int, int, int, int);
void  __cdecl FUN_004f5ce0(int, int, int, int);
void  __cdecl FUN_004f6050(int, int, int, int);
void  __cdecl FUN_004f6420(int, int, int, int);
void  __cdecl FUN_004f64d0(void);
int   __cdecl FUN_004f6c30(int grid_x, int grid_y);               // Terrain_GetAttrDirect(grid_x, grid_y) → attr byte
int   __cdecl FUN_004f6c40(unsigned int, unsigned int);            // Terrain_GetAttr(grid_x, grid_y) → attr index
void  __cdecl Terrain_SetTileAttributeBits(int, int, int); // IDA: FUN_004f6ef0
void  __cdecl Terrain_ClearTileAttributeBits(int, int, int); // IDA: FUN_004f6f10
void  __cdecl Terrain_UpdateTileAttributeRect(int, int, int, int, int, int); // IDA: FUN_004f6f30
// FUN_004f7500 — canonical float10(void) declared above
void  __cdecl FUN_004f7960(float world_x, float world_y, float *out_angle3);  // Terrain_GetAngle
void  __cdecl FUN_004f8740(float x, float y, float scale, int flags, int corners_ptr, char blend, float alpha); // Particle_DrawTile
void  __cdecl FUN_004f8980(int, int, int, float);  // Terrain_SpawnObject(type, x, y, height)
// FUN_004f8bb0 — Particle_Draw (see declaration above in Entity render section)
void  __cdecl FUN_004f8eb0(float *cam_pos);                         // Camera_SetMatrix
unsigned short __cdecl FUN_004f8ff0(float x, float y, float z);    // Frustum_IsVisible

// ── Camera ────────────────────────────────────────────────────────────────────
void  __cdecl FUN_004f9050(float fov_w, float *cam_pos);           // Camera_SetupFrustum
// FUN_004F9590
int   __cdecl Frustum_TestSphere(float *pos, float cull_dist);     // LOD/visibility cull
int   __cdecl FUN_004f95e0(void);  // Terrain_Water_Tick — returns int
void  __cdecl FUN_004f9ac0(char flag);  // RayCast_Init(flag): init screen-to-world ray
int   __cdecl FUN_004f9c70(int, int, int, int);
float __cdecl FUN_004f9c40(float *vec);  // Vec3_Length (physics variant — returns length, does NOT normalize)
void  __cdecl FUN_004f9ce0(float *cam_pos, float factor, float *in_rel, float *out_pos); // Camera_ProjectRelative
void  __cdecl FUN_004f9f70(float *parent, float *rot, float *out);  // Bone_CombineMatrices
void  __cdecl Matrix_BuildFromEuler(float *angles, float *out_mat12);       // Matrix_FromEuler
float* __cdecl Vector_Rotate(float *pt, float *mat12, float *out);    // Matrix_TransformPoint
void  __cdecl Vector_Transform(float *pt, float *mat12, float *out);    // Matrix_TransformPoint (alt)
void  __cdecl Vector_InverseRotate(float *vec, float *mat12, float *out);   // Vec3_Transform
void  __cdecl FUN_004fa1d0(int, int, int, int);
void  __cdecl FUN_004fa270(int, int, int, int);
void  __cdecl FUN_004fa350(int, int, int, int);
void  __cdecl Triangle_ComputeNormal(float *origin, float *A, float *B, float *out_normal); // Vec3_ComputePlaneNormal
void  __cdecl FUN_004fa5c0(int, int, int, int);
void  __cdecl FUN_004fa930(int entity, int model);  // Entity_ProjectToScreen
int   __cdecl FUN_004faa70(int entity, char param2, int param3);   // Entity_IsRenderable
void  __cdecl FUN_004fae00(void *, int, int, char); // Entity_SetupGL

// ── Entity render pipeline ────────────────────────────────────────────────────
void  __cdecl FUN_004fc030(unsigned char *, unsigned int, int, char); // Entity_PrepareRender
float* __cdecl FUN_004fc070(int entity); // Entity_SpawnEffects / Entity_Render_3D
void  __cdecl FUN_004fd800(void);              // Terrain_Render
void *__cdecl FUN_004ff5a0(int type, float *world_pos, float *target_pos, float extra); // Entity_New
void *__cdecl FUN_004ff580(void *entity);                           // Entity_InitRenderState
void  __cdecl FUN_004fffa0(DWORD Owner);                              // DeleteBug
void  __cdecl FUN_004fffd0(int, void *, void *, int);                 // CreateBug (LinkBone arg unused)
uint  __cdecl FUN_00500970(void);  // returns entity count
void  __cdecl FUN_00500a80(int, int, int, int);
void  __cdecl FUN_00500aa0(void);   // RenderBoids (decoration/animal entity tick+render)
uint  __cdecl FUN_00500e80(void);
void  __cdecl FUN_00502200(int, int, int, int);
void  __cdecl AmbientParticles_Update(void); // IDA: FUN_00502320
void  __cdecl FUN_005032f0(int, int, int, int);
void  __cdecl FUN_00503760(void);
void  __cdecl FUN_00503830(int entity_class, int model_ptr);        // Sprite_SetupAnimation
void  __cdecl FUN_005038e0(void);                                   // Entity_Render (sprite loop)
void  __cdecl FUN_00504b50(int, int, float *, float, unsigned int, unsigned char, unsigned int, unsigned int);
void  __cdecl FUN_00505970(void *, void *, int, char, int);
// Entity_DrawAt(entity_ptr, class, slot, angle_ptr, rot, state_flags, byte, a,b,c, d, mode)
void  __cdecl FUN_00505a10(int, int, unsigned int, float *, float, unsigned int, unsigned char, char, unsigned char, char, int, unsigned int);
void  __cdecl Model_SetAnimationSlots(int, int, int, int, int, int); // FUN_00509810 — writes model animation slots
// ── Map / terrain loaders (called from World_Load / Map_LoadResources) ─────────
void  __cdecl FUN_004ffd50(void);                                  // Terrain_ResetObjects
void  __cdecl FUN_004ffe70(const char *path);                      // Terrain_LoadObjects
void  __cdecl FUN_004f6f90(const char *path);                      // Terrain_LoadMap
int   __cdecl FUN_004f6ce0(const char *FileName);                  // OpenTerrainAttribute
void  __cdecl FUN_004f7250(const char *path);                      // Terrain_LoadLight
void  __cdecl FUN_004f7270(const char *path);                      // Terrain_LoadHeight
void  __cdecl FUN_00502b80(void);                                  // Map_InitEntities
void  __cdecl FUN_00509190(void);                                  // Terrain_InitLayers
void  __cdecl FUN_00509880(void);                                  // Terrain_InitWater
void  __cdecl FUN_0050c4d0(void);                                  // Map_InitLighting
void  __cdecl FUN_0045abb0(int map_id);                            // Map_SetupEntities
// ── Font / UI init ───────────────────────────────────────────────────────────────
void  __cdecl PathFinder_ResetContext(void);                                  // IDA: FUN_0043f2d0
void  __cdecl FUN_0050f700(const char *map_name);                  // Map_Load(name)
void  __cdecl FUN_0050f7a0(void);                                  // Map_Unload
// FUN_0050f5f0 @ 0x0050F5F0 (IDA)
void  __cdecl Font_CreateTextDib(int dc);
// FUN_0040f570 @ 0x0040F570 (IDA)
void  __cdecl Font_CreateRenderer(int hwnd, int data, int dc);
// ── Model / data loaders (called from UI_LoadTextures) ───────────────────────────
void  __cdecl Model_LoadPlayerAndItemMeshes(void); // IDA: FUN_00506170
void  __cdecl Model_LoadPlayerEquipmentTextures(void); // IDA: FUN_00507610
void  __cdecl Model_LoadItemMeshes(void); // IDA: FUN_005079d0
void  __cdecl Model_AssignItemTexturePrefixes(void); // IDA: FUN_00508d10
void  __cdecl Model_LoadSkillEffectAssets(void); // IDA: FUN_0050b710
void  __cdecl UI_LoadInterfaceTextures(void); // IDA: FUN_0050eb80
void  __cdecl Sound_LoadGameSamples(void); // IDA: FUN_0050f030
void  __cdecl Item_LoadTextData(const char *path);                 // IDA: FUN_0047B130
void  __cdecl Item_SaveBMD(const char *path);                      // IDA: FUN_0047B650
void  __cdecl Item_LoadBMD(const char *path);                      // IDA: FUN_0047B740
void  __cdecl Skill_LoadTextData(const char *path);                // IDA: FUN_0047A5B0
void  __cdecl Skill_SaveBMD(const char *path);                     // IDA: FUN_0047A970
void  __cdecl Skill_LoadBMD(const char *path);                     // IDA: FUN_0047AC50
void  __cdecl Gate_LoadTextData(const char *path);                 // IDA: FUN_0047A010
void  __cdecl Gate_SaveBMD(const char *path);                      // IDA: FUN_0047A170
void  __cdecl Gate_LoadBMD(const char *path);                      // IDA: FUN_0047A4D0
void  __cdecl FUN_004799d0(const char *path);                      // Filter_LoadData
void  __cdecl FUN_00479a50(const char *path);                      // Filter_LoadBMD
void  __cdecl FUN_00479b30(const char *path);                      // Filter_LoadBMD_Alt
void  __cdecl FUN_00479cf0(const char *path);                      // FilterName_LoadData
void  __cdecl FUN_00479d70(const char *path);                      // FilterName_LoadBMD
void  __cdecl FUN_00479e50(const char *path);                      // FilterName_LoadBMD_Alt
void  __cdecl Dialog_LoadBMD(const char *path);                    // IDA: FUN_0047B020
uint  __cdecl FUN_00401040(int handle, const char *path);          // Quest_LoadBMD
void  __cdecl NPCName_LoadTextData(const char *path);              // IDA: FUN_0047D120
// FUN_00404a10 — declared above as Sound_LoadAndPlay(int type, int flags)
void  __cdecl FUN_0050e5a0(void);              // World_Load
void  __cdecl FUN_0050f690(void);              // World_Init
void  __cdecl Scene_LoadAccountResources(void); // FUN_0050fcf0 (IDA)
void  __cdecl Scene_UnloadAccountResources(void); // FUN_0050ff10 (IDA)
void  __cdecl Scene_LoadCharSelectResources(void); // FUN_0050ff50 (IDA)

// ── GL helper layer ───────────────────────────────────────────────────────────
void  __cdecl Scene_UnloadCharSelectResources(void); // FUN_005102c0 (IDA)
void  __cdecl Scene_LoadGameAssets(void); // IDA: FUN_00510320
void  __cdecl FUN_00510fe0(int, int, int, int);
void  __cdecl Monster_LoadStartupData(void); // IDA: FUN_00511060
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
unsigned int __cdecl FUN_00513260(float *viewport, float *projection); // Entity_ViewportCheck(vp, proj) → 1=visible
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
void  __cdecl FUN_00511d00(int type, float *pos, float r, float g, // SkillEffect_Draw2D
                            float *extra, float, float, float,
                            float, float);
void  __cdecl FUN_005120c0(int, int, int, int);

// ── Texture system ────────────────────────────────────────────────────────────
void  __cdecl GL_Begin2D(void);                                  // GL_SetupOrtho2D
void  __cdecl GL_End2D(void);                                  // GL_End2D
void  __cdecl GL_DrawRect(float, float, float, float);  // FillRect(x,y,w,h)
void  __cdecl FUN_005126e0(int id, float x, float y, float w, float h, undefined4 color); // GL_DrawColoredRect
int   __cdecl FUN_0051ddf0(void);   // GuildLeaderboard_Render
int   __cdecl FUN_0051db00(void);   // GuildMemberList_Render2
void  __cdecl GL_DrawTexture(int id, float x, float y, float w,     // Texture_Draw2D
                            float h, float u0, float v0,
                            float u1, float v1, char fx, char fy);
// FUN_005128c0 = RenderBitmapUV(int Texture, float x, float y, float Width,
//   float Height, float u, float v, float uWidth, float vHeight).  La firma
//   `(int,int,int,int)` de antes estaba mal y por eso nadie la usaba.
//   Implementada en Render/Render_Frame.cpp (static).  NO es intercambiable con
//   RenderBitmap (0x5125A0): esta mapea un cuadrilatero SESGADO en V, aquella un
//   rectangulo.
void  __cdecl FUN_005128c0(int, int, int, int);   // (firma vieja, sin uso)
float10 __cdecl FUN_005129f0(float angle);  // |a| (abs)
double  __cdecl FUN_00512a10(float a, float b); // min(a,b)
double  __cdecl FUN_00512a30(float a, float b); // max(a,b)
int     __cdecl FUN_00512a50(float a1, float a2, float a3, int a4, int a5, int a6, int a7, int a8, int a9, float a10); // PointInPolygon
// FUN_00513570 @ 0x00513570
char  __cdecl Chat_ValidateInputCommand(void);
void  __cdecl CharSelect_SendSelectPacket(void);                  // FUN_00513c10
void  __cdecl FUN_00514270(int, int, int, int);
void  __cdecl SetErrorMessage(int message); // 0x005142D0 — SetErrorMessage (Offsets.h canonical name)
#define FUN_005142d0  SetErrorMessage
void  __cdecl FUN_00514310(void);          // UI_InGameMenu state machine
int   __cdecl FUN_0051af50(void);  // UI_StatsPanel_Render
int   __cdecl SeparateTextIntoLines(const char *lpszText, char *lpszSeparated, int iMaxLine, int iLineSize); // 0x0051D600
#define FUN_0051d600  SeparateTextIntoLines
int   __cdecl FUN_0051d840(int slot);                                     // ItemList_Select
void  __cdecl FUN_0051e0c0(void);
void  __cdecl FUN_0051e7e0(void);

// ── Scene functions ───────────────────────────────────────────────────────────
void  __cdecl FUN_005239a0(void);                                  // CharSelect_UpdateInput
bool  __cdecl FUN_00524cb0(void);   // MoveMainCamera (per-frame camera follow + Hero offset)
void  __cdecl FUN_00529130(int, int, int, int);
int   __cdecl FUN_00529740(const char *path, int id,               // Texture_Load (OZJ/JPEG)
                            int w, int h, int flags, char mipmap);
int   __cdecl FUN_00529bd0(const char *path, int id,               // Texture_LoadTGA (TGA/OZT)
                            int w, int h, int flags, char mipmap);
void  __cdecl FUN_0052a050(int id);                                // Texture_Unload
int   __cdecl FUN_0053cc30(int mode, BYTE* src, int len);  // Packet_Encode (3 args)
int   __cdecl FUN_0053cca0(int, int, int, int);  // CSimpleModulus_Decode → returns plaintext byte count
void  __cdecl FUN_0053d410(int, int, int, int);
void  __cdecl FUN_0053d470(int, int, int, int);
uint  __cdecl FUN_0053d5c0(char *);  // Texture_GetSlot — returns slot index
float10 __cdecl FUN_005433b0(void);  // GetTickCount-based time accumulator → float10
void  __cdecl FUN_005524c0(int, int, int, int);

// ── MSVC CRT stubs (obfuscation / anti-tamper wrappers) ───────────────────────
int   __cdecl FUN_005416bc(char *buf, const char *fmt, ...);      // crt_sprintf
#define crt_sprintf FUN_005416bc
void  __cdecl FUN_0054519d(int, int, int, int);                   // crt_output_engine
void  __cdecl FUN_0054158c(void *ptr);                            // operator_delete
#define operator_delete(p) FUN_0054158c(p)                         // alias
void  __cdecl FUN_00541c10(int frame_size);                       // __chkstk_probe

// operator_new — MSVC global new (routes through CRT malloc)
// Ghidra decompiles show `operator_new(size)` as the standard C++ new.
#ifndef operator_new
#include <new>
inline void* operator_new(size_t n) { return ::operator new(n); }
#endif
FILE* __cdecl FUN_0054173f(LPCSTR, const void*);    // fopen-wrapper
int   __cdecl FUN_00541eab(byte *str);   // IsLeadByte — DBCS lead-byte check
void  __cdecl FUN_00542457(int, int, int, int);
void  __cdecl FUN_00542762(int, int, int, int);
void  __cdecl FUN_0054283e(int, int, int, int);
void  __cdecl FUN_00543037(int, int, int, int);

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
int   __cdecl FUN_004f8480(int, int, int, int, float, int, int); // Cursor_HitTest
int   __cdecl Entity_SelectNearest(int type); // IDA: FUN_004afdc0; find hovered entity of given type
int   __cdecl ItemOnGround_HoverTest(void);   // IDA: FUN_004afa40
int   __cdecl SpecialObject_HoverTest(void);  // IDA: FUN_004b0240
void  __cdecl Party_MatchEntityNames(void); // IDA: FUN_004afb00
char  __cdecl FUN_004e5980(void);        // SecondPassword_IsActive

// ── Char menu builder helpers (FUN_004c3530) ──────────────────────────────────
void  __cdecl FUN_004c2420(int, int, int, int, int, int);  // DrawItemInfoBox(x, y, count, fixedWidth, iSort, drawBox)
void  __cdecl FUN_004c2880(int class_data_ptr);            // build class info block
void  __cdecl FUN_004c2c10(int row, unsigned char *color, int *value, const char *label, int x, int flags); // draw stat row
void  __cdecl FUN_004c2d50(int row, int value, int col_width); // draw value column
void  __cdecl FUN_004c2e20(int class_id);                  // prepare class data

// ── Second password ───────────────────────────────────────────────────────────
unsigned int  __cdecl FUN_004e93a0(void);  // Scene_CheckReady — returns connection-ready flag

// ── Weather / BMD animation helpers ──────────────────────────────────────────
bool  __cdecl FUN_00440aa0(void* pvVar9, float* rot, float* scale, void* extra, float fVar); // BMD_AnimTick
extern "C" bool __cdecl CharacterAnimation(int c, int o);    // 0x00448600 — per-char anim tick
// ── Effect_Create helpers ─────────────────────────────────────────────────────
void  __cdecl FUN_004f76c0(float, float, int, int, int);          // AddTerrainLight (0x4F76C0)
// ^ es `AddTerrainLight(float xf, float yf, float Light[3], int Range, float Buffer[3])`.
//   Los dos punteros quedaron tipados como `int` por el decompile; el wrapper
//   `AddTerrainLight` de structs.h le pone la firma real.  NO confundir con
//   `AddTerrainLightClip` (0x4F7800), que es otra funcion y clampea a 1.0.
float* __cdecl FUN_0045fec0(uint, float*, float, int, short);     // Particle_SpawnFX

// ── Monster_Data sub-functions ────────────────────────────────────────────────
void  __cdecl Monster_LoadScriptTable(void); // FUN_0050b510 — loads monster script/model table
void  __cdecl Monster_ParseSetBase2(LPCSTR param_1); // FUN_0047d020 — parses MonsterSetBase2 spawn definitions
void  __cdecl FUN_00505bd0(int count);                          // MonsterTable_Init / Model_SetSlotIndex
void  __cdecl FUN_00505e90(int id, const char* dir, const char* file); // Monster_RegisterBMD / Model_LoadSMD
void  __cdecl FUN_005098c0(int monster_idx);                    // Monster_SetupSoundAnim
int   __cdecl TextParser_GetToken(void);                         // FUN_0047A1F0 — text-data tokenizer (returns record type)
void  __cdecl FUN_00479910(int buf, int len);                   // XOR-cipher buffer in-place (key: FC CF AB, 3-byte cycle)
void  __cdecl FUN_0047ea70(void *dst, void *src);               // Skill_HashTable_SerializeEntry (encode + insert)
void  __cdecl FUN_0047eaf0(void *entry, void *key);             // Skill_HashTable_FreeEntry (decode + remove)
void  __cdecl FUN_00401120(int buf, int size);                  // Quest_DecryptBuf (quest record XOR)
uint  __cdecl FUN_005430f0(char *buf, uint size, uint count, int *fp); // fwrite-wrapper (locked)
void  __cdecl FUN_00543264(int ch, int *fp);                     // fputc-wrapper (writes single byte to file)
void  __cdecl FUN_0054150f(FILE* fp);                           // fclose-wrapper
void  __cdecl FUN_005060b0(int id, const char* dir, const char* file, int idx); // Monster_LoadBase
void  __cdecl FUN_00505c80(int id, const char* prefix, int flags, char loop);   // Monster_LoadSound
// ── GL_State sub-functions ────────────────────────────────────────────────────
void  __cdecl GL_DisableCullFace(void);           // GL_SetTextureState
void  __cdecl GL_DisableDepthWrites(void);           // GL_SetDepthState
void  __cdecl GL_EnableCullFace(void);           // GL_TexEnable
unsigned int __cdecl FUN_00529000(const char* path, int width, int height, void* pixelBuf, int flip); // PNG_WriteFile (BMP replacement)
// ── Particle path-finding helpers ────────────────────────────────────────────
int   __cdecl Math_GetAngleFromPoints(float x1, float y1, float x2, float y2); // IDA: FUN_0043e430
int   __cdecl FUN_0043e120(int a, int b, int c);                     // Angle_Clamp

// ── Joint_Create helpers ──────────────────────────────────────────────────────
void  __cdecl FUN_0046fe90(int joint_ptr, float *mat);  // Joint_SegmentTick
// MoveHumming(Position, Angle, TargetPosition, Turn) — gira Angle hacia el
// target y DEVUELVE la distancia (Hex-Rays la tipa `void`: el retorno va en st0).
float __cdecl FUN_0043e4a0(float *Position, float *Angle, float *TargetPosition, float Turn);

// ── GL_2D helpers ─────────────────────────────────────────────────────────────
void  __cdecl GL_DisableDepthTest(void);           // GL_End2D / restore state
void  __cdecl GL_SetAlphaTest(char mode);      // GL_SetBlendMode (1 arg)
void  __cdecl GL_EnableLightMap(void);           // GL_EnableLightMap (lightmap blend state)

// ── Entity_Render helpers ─────────────────────────────────────────────────────
void  __cdecl FUN_00454cd0(int entity_ptr, int param2);  // Entity_UpdateAnim
void  __cdecl FUN_00449900(int entity_ptr);              // Entity_ComputeScreenPos
void  __cdecl FUN_004520c0(int entity_ptr);              // Entity_UpdateState

// ── Effect_Tick helpers ───────────────────────────────────────────────────────
void  __cdecl FUN_00466ad0(float *, int);         // MoveEffect — implemented in Render/MoveEffect.cpp
char * __cdecl FUN_00470030(undefined1 *param_1, uint param_2); // MoveJoint — implemented in Render/MoveJoint.cpp
void  __cdecl FUN_00511bf0(float*, float, int);  // sincos helper (dest, angle, scale)
void  __cdecl FUN_004f9e90(float*, float*);      // EulerToMatrix(angles, out_mat)
void  __cdecl FUN_00465fe0(int, int);

// ── Net_Connect helpers ───────────────────────────────────────────────────────
void  __cdecl FUN_0043db30(int);
void  __cdecl FUN_0043dbf0(void*, int);  // Socket_Init(ctx, hwnd)
void  __cdecl FUN_0043dc90(int);         // NetContext_Disconnect (close socket + cleanup)

// ── Entity_Init helpers ───────────────────────────────────────────────────────
void  __cdecl FUN_0045c050(int);  // Entity_SetupAnimSlots(entity_ptr)
void  __cdecl FUN_00543274(void*, void*);  // fprintf-like helper

// ── Sound helpers ─────────────────────────────────────────────────────────────
void  __cdecl FUN_00404bb0(void);  // Sound_BufferUnlock helper

// ── Net_PacketSession helpers ─────────────────────────────────────────────────
void  __cdecl FUN_004df410(undefined4, uint);  // packet handler with ECX/EDX args
void  __cdecl FUN_004e4760(void);
void  __cdecl FUN_004e5500(void);
void  __cdecl FUN_004e5de0(void);
void  __cdecl FUN_004e6550(void);
void  __cdecl FUN_004e6c40(void);
void  __cdecl FUN_004e7ac0(void);
void  __cdecl FUN_004e8b70(void);
void  __cdecl FUN_004e9050(void);
void  __cdecl FUN_004eb5d0(void);
void  __cdecl FUN_004eb7f0(void);
void  __cdecl FUN_004ec330(void);
// SecondPassword UI helper calls
undefined4 __cdecl FUN_0051e240(int mode, int param_2, int param_3);  // UI_OpenDialog (open named dialog by id)
uint  __cdecl FUN_004e3db0(int ctx, int p1, int p2, int p3, int p4);   // SecondPassword_AuthSend — returns non-zero on success
undefined4 __cdecl FUN_004f6850(void);        // SecondPassword_CancelReturn (nav back, clear state)
uint  __cdecl FUN_004f6a70(void);             // Net_Disconnect_Clean (sends disconnect + returns)
void  __cdecl FUN_004d1fc0(void);             // SecondPassword_RenderGrid_Empty
// 2026-05-08: real signature per IDA `004D23B0_sub_4D23B0.c` (Inventory grid
// render + click dispatcher). See Item/Item_ClickHandler.cpp for the port.
void  __cdecl FUN_004d23b0(char *origin_x, int origin_y, short *inv_base,
                            int grid_w, int grid_h, char mode_flag);

// ── Scene_MapTick helpers ─────────────────────────────────────────────────────
void  __cdecl FUN_004c4650(void*, void*, void*, int);  // RenderItemInfo(sx, sy, ITEM*, bSell)
void  __cdecl FUN_004c8d70(void*, int,   void*);        // RenderItemInfo_Shop(sx, sy, ITEM*)
void  __cdecl FUN_004c9730(float a1, int a2, int a3);   // Skill_RenderTooltip @ 0x004C9730

// ── Texture helpers ───────────────────────────────────────────────────────────
void  __cdecl FUN_0053cd20(void *ctx, undefined4 *src, ushort *dst, byte n);
uint  __cdecl Pipe_SendQuery(void *mgr, CHAR *name); // FUN_0053ed00
void  __cdecl FUN_0053ed30(void *mgr, CHAR *name);      // Pipe_SetTarget — copies name, sends 0x613
void  __cdecl FUN_005404a0(LPCVOID param_1, int param_2, CHAR *param_3);  // Pipe_Write wrapper
void  __cdecl FUN_005403a0(LPCVOID param_1, int param_2, CHAR *param_3);  // Pipe_WriteFile — WriteFile to named pipe

// ── WinMain helpers ───────────────────────────────────────────────────────────
void  __cdecl Window_FatalError(HWND hwnd, char shutdown);  // 0x004129F0
UINT  __cdecl FUN_0053d5a0(char* filename);  // Resource_Load — loads a resource file

// ── Misc/Util helpers ─────────────────────────────────────────────────────────
void  __cdecl FUN_005030c0(int);     // Entity_UpdateGravitySlot
void  __cdecl Entity_UpdateSparkleEffect(int); // FUN_00503650 — updates periodic entity sparkle particles
int   __cdecl FUN_00402850(void *);  // Packet_ParseType1
undefined4 __cdecl FUN_00402f40(void *);  // Packet_ParseType2
void  __cdecl FUN_0040e330(DWORD);   // (undocumented)
int   __cdecl FUN_00408940(int *, float);   // cloth: paso de simulación (0x408940)

// ── Scene-entity grid helpers ─────────────────────────────────────────────────
void  __cdecl FUN_004ffcc0(void *node, int cell_ptr);  // Entity_GridUnlink — unlinks node from its doubly-linked list

// ── Sound slot helpers ────────────────────────────────────────────────────────
HRESULT __cdecl FUN_00404ad0(int channel);  // Sound_FreeChannel — releases DirectSound buffers for channel

// ── File I/O CRT helpers ──────────────────────────────────────────────────────
void  __cdecl FUN_00543037(int *fp, int offset, int whence);  // CRT fseek wrapper
int   __cdecl FUN_00542eb4(char *fp);                         // CRT ftell wrapper
int   __cdecl FUN_00541597(void *dst, int size, int count, int *fp); // CRT fread wrapper
int   __cdecl FUN_00541eab(unsigned char *str);               // IsLeadByte — already in stubs.cpp

// ── Terrain helpers ───────────────────────────────────────────────────────────
void  __cdecl Terrain_Clear(void);  // FUN_004f6c60 (IDA) — resets terrain tile buffers
void  __cdecl FUN_004f6cb0(void);   // Terrain_ReadFallback
void  __cdecl FUN_004f6eb0(int data, int size); // Terrain_ProcessBlock
void  __cdecl CreateTerrainNormal(void); // FUN_004f70b0 (IDA)
void  __cdecl Terrain_FinalizeLighting(void); // FUN_004f71c0 (IDA)
uint  __cdecl OpenTerrainHeight(char *path); // FUN_004f7290 (IDA)
void  __cdecl FUN_004f9c20(void);   // TerrainHeight_Flush
void  __cdecl FUN_00529360(char *path, int dst); // Texture_LoadToBuf
void  __cdecl FUN_00512d30(void);   // Map_InitRayCast
int   __cdecl FUN_004f98c0(int, int, int, int, int); // RayCast_Setup / terrain light setup (sub_4F98C0)
void  __cdecl FUN_004f7060(void);   // Terrain_SpawnAmbientObjects (sub_4F7060)
void  __cdecl FUN_00479540(void);   // RenderTerrainAlphaBitmaps (sub_479540)
int   __cdecl FUN_004f9a30(int a1); // Terrain light double-buffer commit (sub_4F9A30)
float10 __cdecl FUN_00454b00(int entity_ptr);  // Entity_GetMoveRate

// ── Scene_MapTick helpers ─────────────────────────────────────────────────────
void  __cdecl FUN_005050d0(void);   // Entity_UpdateAll

// ── Model/BGM loader helpers ──────────────────────────────────────────────────
// FUN_00505e90 — same as Monster_RegisterBMD above (int first arg), see line 653
void  __cdecl FUN_00505bd0(int slot);   // Model_SetSlotIndex (same as MonsterTable_Init above)
void  __cdecl FUN_00505060(int slot, const char *dir, const char *file); // alternate SMD loader

// ── Model mesh/anim loaders (used by Model_Load* functions) ──────────────────
// FUN_0040b280: loads main BMD mesh into model slot
//   param_1 = model id (as ptr cast to int)
//   param_2 = path to .smd/.bmd file
//   param_3 = num anims
//   param_4 = flag
void  __cdecl FUN_0040b280(int model_id, const char *path, int num_anims, char flag);
// FUN_0040b310: adds an animation to a loaded model
//   param_1 = model id (int)
//   param_2 = path to animation .smd file
//   param_3 = loop flag (0=no loop, 1=loop)
void  __cdecl FUN_0040b310(int model_id, const char *path, char loop);
// FUN_00506050: loads a numbered model file (format: "prefix%s%02d.smd" or "%s%d.smd")
//   param_1 = model id
//   param_2 = path prefix string
//   param_3 = index number
void  __cdecl FUN_00506050(int model_id, const char *prefix, int index);

// ── Terrain tile pick helpers ─────────────────────────────────────────────────
void  __cdecl FUN_004f7fb0(float xf, float yf, int xi, int yi, float lodf); // RenderTerrainFace (0x004F7FB0)
unsigned int __cdecl FUN_00512d40(float *Position, float *Target, int Polygon, float *v1, float *v2, float *v3, float *v4, float *Normal, char Collision); // CollisionDetectLineToFace — IDA-activated 2026-04-26 audit #7

// ── Item inventory helpers (from Offsets.h) ───────────────────────────────────
#define FUN_00482ff0 GetItemCount
#define FUN_00482d70 GetItemSlot
int   __cdecl GetItemCount(int siType, int iLevel);  // 0x00482FF0
int   __cdecl GetItemSlot(int siType, int iLevel);   // 0x00482D70

// ── SecondPassword Screen6/7 helpers ─────────────────────────────────────────
void  __cdecl FUN_0051d780(int a, int b);  // UI_ShowBitmapMsg (bitmap message overlay, 2 args)
void  __cdecl Item_RecalculateRepairCost(void); // IDA: FUN_004C4080
void  __cdecl FUN_004233e0(int, int);      // HashTable_Unlock (2-arg variant, release lock)

// ── Map / Scene helpers ───────────────────────────────────────────────────────
int   __cdecl ParseNextToken(void);        // FUN_0050E2C0 — reads next token from ParserFileHandle into ParserTokenString; returns token type (2=EOF)

// ── BMD skin helpers ─────────────────────────────────────────────────────────
void  __cdecl FUN_004f9d60(float *vec);    // Vec3_Normalize — normalises a 3-float vector in-place

// ── NPC Script helpers ────────────────────────────────────────────────────────
void  __cdecl FUN_0043d3e0(int, int*);     // HashTable_LockRead (lock + read value into *out)
void  __cdecl FUN_00409e20(void*, void*);  // HashTable_Retain2 (2-arg ref-count inc variant)
void  __cdecl FUN_00423760(void*, void*);  // HashTable_Release2 (2-arg ref-count dec variant)

// ── BMD bone-draw helper ──────────────────────────────────────────────────────
void  __cdecl FUN_004414d0(void *model, char a, int b, float frame, int flags,
                            float f3, int f4, float f5, float f6, float f7, unsigned int rgba);
                            // BMD_DrawBoneSlot_Anim (animated variant, 11 args)

// ── Character animation/attack helpers (Kayito names, called from large stubs) ──
// FUN_00444410 — already declared above (line 266) with 4 args: (int, int, int, int)
void  __cdecl FUN_00444b60(int entity, int type);            // SetPlayerShock
void  __cdecl FUN_00445230(int entity); // AttackEffect
void  __cdecl FUN_00448600(int entity, int index);           // CharacterAnimation
// IDA: FUN_00485780
void  __cdecl Combat_UseWarriorSkill(int entity, int skillType);
// IDA: FUN_0048a180
void  __cdecl Combat_UseElfSkill(int entity, int object);
int   __cdecl FUN_0047e4f0(int machinePtr, int skillType, int level); // CHARACTER_MACHINE::GetMagicSkillDamage

// ── Sound: PlayBuffer ─────────────────────────────────────────────────────────
HRESULT __cdecl PlayBuffer(int Buffer, DWORD Object, BOOL bLooped);  // 0x00404bc0

// ── Cipher / XOR aliases ──────────────────────────────────────────────────────
// BuxConvert_0 = FUN_00479910  (XOR-cipher, (int buf, int len), 3-byte key FC CF AB)
#define BuxConvert_0 FUN_00479910

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
void  __cdecl SetPlayerStop(void *ch);         // 0x004430c0 (CHARACTER* param)
// SetCharacterScale = FUN_0045c050  (already declared as Entity_SetupAnimSlots)
#define SetCharacterScale FUN_0045c050

// ── Error reporting ───────────────────────────────────────────────────────────
void  __cdecl CErrorReport__Write(DWORD This, char *lpszFormat, ...); // 0x00405540

// ── Forward declarations for small unmapped functions ─────────────────────────
// CRT internals
void  __cdecl FUN_005414ce(void *pFunc);                             // CRT atexit registration
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
void  __fastcall FUN_00406de0(void *This);                           // HashTable_Init
void  __fastcall PacketCipher_Initialize(void *This);                // IDA: FUN_00406bd0
void  __fastcall FUN_00405240_init(void *This);                      // ErrorReport_GlobalInit
void  __fastcall FUN_00403ea0(void *This);                           // Quest table init
void  __fastcall FUN_0040a600(void *This);                           // Sound device init B

// Destructor chains (virtual ~dtor pattern: call deinit, conditional delete)
void  __fastcall FUN_00406cd0(void *This);                           // HashWidget deinit
void  __fastcall FUN_00409b80(void *This);                           // SoundWidgetB deinit
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
void  __cdecl DeleteObjects(void);
void  __cdecl DeleteNpcs(void);
void  __cdecl DeleteMonsters(void);
void  __cdecl ClearItems(void);
void  __cdecl ClearCharacters(int param);
void  __cdecl EndOpengl(void);                                       // 0x00511bc0
void  __cdecl StopMusic(void);                                       // 0x00513420
void  __cdecl DeleteBug(int Owner);                                  // 0x004fffa0

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
void  __cdecl FUN_00405540(DWORD This, char *fmt);                   // CErrorReport::WriteDebugInfoStr
int   __cdecl FUN_0053d170(int param_1);                             // shift right 3

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
void  __cdecl CloseInventoryRelatedWindows(void);                                          // 0x004CBA60

// SMD parser
int   __stdcall FUN_0040b350(void);                                   // SMD tokenizer
void  __cdecl ParseNodes(void);                                      // SMD node parser
void  __cdecl ParseTriangles(bool Flip);                             // SMD triangle parser
void  __cdecl ParseSkeleton(void);                                   // SMD skeleton parser

// Text filter
bool  __cdecl FindText(char *src, char *pattern, bool caseSensitive);// 0x0040e690

// Camera / math
void  __cdecl AngleMatrix(float angles[3], float matrix[3][4]);      // 0x00440c80
void  __cdecl VectorIRotate(float in1[3], float in2[3][4], float out[3]); // 0x00440e00

// BMD transform
void  __fastcall BMD__TransformPosition(void *This, float (*BoneMatrix)[4], float *Pos, float *WorldPos, bool Translate); // 0x004409a0

// Sound helpers (FUN_0040a0a0 chain)
void  __cdecl FUN_004f9d60(float *param);                            // float init
// int   __fastcall FUN_00409f30(void *This, int p1, int p2, int p3, char p4); // open file — duplicate, correct 6-param version at line 1156
// void  __fastcall FUN_0040a300(void *This, int p1);                   // cleanup after open — duplicate, correct 3-param version at line 1159

// Render helpers
void  __cdecl EnableAlphaBlend(void);                                // 0x004f8830
void  __cdecl EnableAlphaTest(bool enable);                          // 0x004f8870
void  __cdecl RenderTerrainAlphaBitmap(int tex, float x, float y, float sx, float sy, float *light, float alpha, float size); // 0x004fd100

// Codec / JPEG internal helpers (batch 3)
void  __cdecl FUN_00543839(int param);                               // CRT init
void *__cdecl FUN_00543d81(void);                                    // CRT alloc
void  __cdecl _strncpy(char *dst, char *src, int n);                 // strncpy wrapper
// FUN_005430f0 already declared above (line ~713) as fwrite-wrapper
void  __cdecl FUN_005436a6(int *fp);                                 // fflush wrapper

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
void    __cdecl LoadWaveFile(int Buffer, const char* strFileName, int MaxChannel, bool Enable);       // 0x00404a10
void  __stdcall FUN_00405340(void);                                  // CErrorReport: rotate log
char* __stdcall CErrorReport_FindLogMarker(char* param_1);           // IDA: FUN_00405420
void  __fastcall FUN_00405620(void* param_1);                        // CErrorReport: system info
void  __fastcall FUN_004056b0(void* param_1);                        // CErrorReport: OpenGL info
void  __fastcall FUN_00405760(void* ecx, void* edx, HWND param_1);  // CErrorReport: IME info
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
void* __fastcall FUN_00409ad0(void* param_1);                        // CSQuest constructor
void  __fastcall FUN_00409b80(void* param_1);                        // CSQuest destructor
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
int   __stdcall FUN_0053cf90(int param_1, unsigned int param_2, int param_3, unsigned int param_4, int param_5); // Crypto bit-field extract
void  __stdcall FUN_0053d0d0(unsigned char *param_1, int param_2, int param_3); // Crypto byte-shift
int   __cdecl FUN_0053d1c0(void *self, const char *filename, short magic, int k0, int k1, int k2, int k3); // Crypto key file loader
// FUN_0053d430 — already declared above (GameGuard init)
void  __fastcall FUN_0053d620(DWORD param_1);                           // GameGuard cleanup
int   __cdecl FUN_0053d7d0(void *self, char *param_1);               // GameGuard main check
// FUN_0053d890 — implemented in GameGuard_Init2.cpp
void* __cdecl FUN_0053e8c0(void *param_1);                              // GG encrypted string decoder
void  __fastcall FUN_0053e930(int *param_1);                            // GG log flush+close
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
// FUN_005404a0 — already declared above (Pipe_Write wrapper)
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
void  __cdecl SendRequestEquipmentItem_stub(int iSrcType, int iSrcIndex, ITEM *pItem, int iDstType, int iDstIndex); // 0x0043C250
int   __stdcall FindHotKey_stub(int Skill);                              // 0x004B1170
void  __cdecl RenderSkillIcon_stub(int iIndex, float x, float y, float width, float height); // 0x004BB940
void  __cdecl SendChat(char *Text);                                      // 0x004C1B90
int   __cdecl ConvertGold64_stub(int Zen, char *Buffer);                 // 0x004C3E10
void  __cdecl RenderItemName_stub(int i, DWORD o, int ItemLevel, int ItemOption, bool Sort); // 0x004C9E70
void  __cdecl InsertWarehouseItem_stub(unsigned int param_1, unsigned char *param_2); // 0x004CC0E0
void  __cdecl SetItemGridFlag_stub(int param_1, int param_2, unsigned char param_3); // 0x004CC1E0
int   __cdecl CompareItems_stub(short param_1, int param_2, int param_3); // 0x004CC270
void  __cdecl SortInventory_stub(short *param_1);                        // 0x004CC3C0
unsigned long long __cdecl CheckInventorySpace_stub(int p1, int p2, unsigned short *p3, int p4, int p5); // 0x004D5D70
int   __cdecl FindEmptySlot_stub(int p1, int p2, int p3, int p4);       // 0x004D5F20
unsigned int __cdecl FindEmptySlotNearMouse_stub(int p1, int p2, int p3, int p4, int p5); // 0x004D6020
unsigned int __stdcall Inventory_DropItem_stub(void);                    // 0x004D6470
long long __fastcall CalculateInventoryValue_stub(int p1, unsigned int p2, short *p3, int p4, short *p5); // 0x004DF330
void  __cdecl InventoryColor_stub(ITEM *p);                              // 0x004E2420
void  __stdcall RenderEquipmentBox_stub(void);                           // 0x004E25A0
void  __cdecl RenderEquipmentPart3D_stub(int Index, float sx, float sy, float Width, float Height); // 0x004E2E40
void  __stdcall RenderEquipment3D_stub(void);                            // 0x004E3100
void  __cdecl RenderItemsBoxes_stub(float fPosX, float fPosY, DWORD Inventory, int iMaxWidth, int iMaxHeight); // 0x004E37B0
void  __cdecl RenderItems3D_stub(float p1, float p2, short *p3, int p4, int p5, char p6); // 0x004E38B0
int   __cdecl CheckMixRecipe(short *p1, int p2, int p3);                // 0x004E40F0
void  __cdecl RenderInventoryInterface_stub(int StartX, int StartY, int Flag); // 0x004ECBA0
void  __cdecl RenderGuildMark_stub(float p1, float p2, float p3, float p4, int p5); // 0x004F02F0
void  __cdecl AddTerrainLightClip_stub(float xf, float yf, float Light[3], int Range, float Buffer[3]); // 0x004F7800
void  __cdecl RenderTerrainBlock_stub(float xf, float yf, int xi, int yi, bool EditFlag); // 0x004F9720
void  __cdecl RenderTerrainFrustrum_stub(bool EditFlag);                 // 0x004F97E0
void  __cdecl MoveObject_Special_stub(int param_1); // legacy alias of FUN_004fa5f0 (0x004FA5F0)
char* __stdcall PickObject_Mouse_stub(void);                             // 0x004FA7C0
float* __cdecl MoveObject_PerWorld(float param_1);                       // 0x004FDC00
int   __stdcall MoveHeavenThunder_stub(void);                            // 0x004FED90
void  __stdcall MoveObjects_stub(void);                                  // 0x004FF260
void  __stdcall MoveBugs_stub(void);                                     // 0x005001F0
void  __cdecl OpenNpc_stub(int Type);                                    // 0x005091D0
void  __stdcall EnableAlphaBlend2_stub(void);                            // 0x00511810
void  __cdecl RenderSpriteUV_stub(int Texture, float Position[3], float Width, float Height, float (*UV)[2], float Light[3][4], float Alpha); // 0x00511FB0
double __cdecl RenderNumber2D_stub(float x, float y, int Num, float Width, float Height); // 0x005122F0
void  __stdcall MoveCamera_stub(void);                                   // 0x0051E4E0
void  __cdecl Combat_UseWizardSkill(DWORD c, DWORD o);                  // 0x004889D0
bool  __stdcall Combat_UseElfSkillItem(DWORD c, DWORD pItem);           // 0x0048BD70

// Batch 17 — Entity, combat, rendering, IME, chat, particles
void  __cdecl FUN_0043ce50(unsigned char param_1, int param_2);          // action request packet (0x0043CE50)
void  __cdecl LookAtTarget_stub(DWORD o, DWORD TargetCharacter);         // 0x0043E890
void  __cdecl MoveHead_stub(int param_1);                                // 0x0043E940
bool  __cdecl BMD__CollisionDetectLineToMesh_stub(DWORD pThis, float Position[3], float Target[3], bool Collision, int Mesh, int Triangle); // 0x00440BE0
bool  __cdecl AttackStage_stub(DWORD c, DWORD o);                       // 0x00448930
// IDA: FUN_00451f30
void  __cdecl Combat_SpawnDeathDustParticles(int entity);
// IDA: FUN_00452030
void  __cdecl Combat_SpawnIdleAmbientParticle(int entity);
void  __stdcall MoveCharactersClient_stub(void);                         // 0x00455010
unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char *param_1);   // hash read 1-byte (0x0045FAE0)
void  __cdecl Effect_SpawnBombRing(float *param_1); // IDA: FUN_00466300
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void  __cdecl FUN_00466300(float *param_1); // IDA: FUN_00466300
void  __cdecl RenderWheelWeapon_stub(DWORD o);                           // 0x0046B7C0
void  __cdecl ItemDrop_RenderGroundWeapon(int param_1); // IDA: FUN_0046b980
// Compatibility bridge used only by stubs_IDA_ports.cpp.
void  __cdecl FUN_0046b980(int param_1); // IDA: FUN_0046b980
void  __cdecl FUN_0046b7c0(DWORD o);                                     // RenderWheelWeapon (0x0046B7C0) — efecto 239
void  __cdecl FUN_0046c5a0(int p1, int p2, float *p3, float *p4);       // skill impact particles (0x0046C5A0)
void  __cdecl CreateBlood_stub(DWORD o);                                 // 0x0046C680
void  __cdecl FUN_0046c7f0(int p1, int p2, float p3, float p4, float p5); // directional blood (0x0046C7F0)
void  __cdecl CreateArrow_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey); // 0x00474370
void  __cdecl CreateArrows_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey); // 0x00474BD0
void  __stdcall MoveParticles_stub(void);                                // 0x00477090
unsigned int __cdecl FUN_0047cfb0(DWORD ecx, short *param_1);            // item special option (0x0047CFB0)
void  __stdcall SetPositionIME_Wnd_stub(float x, float y);              // 0x0047ECAF
bool  __cdecl CheckIME_Status_stub(bool change, int mode);              // 0x0047EDC0
void  __stdcall RenderIME_Status_stub(void);                             // 0x0047EE52
// FUN_0047f360 @ 0x0047F360 (IDA)
void  __cdecl Font_RenderTextToBitmap(int p1, int p2, LPCSTR p3, int p4, int p5, int p6, int p7, int p8, LPCSTR p9);
// FUN_0047f4c0 @ 0x0047F4C0 (IDA)
void  __cdecl Font_RenderBitmapText(int p1, int p2, float p3, float p4, int p5, int p6, float p7, int p8);
void  __cdecl RenderTipText_stub(int sx, int sy, char *Text);           // 0x0047F7F0
int   __stdcall FUN_0047fed0(void);                                      // hotbar skill data (0x0047FED0)
void  __cdecl FUN_00481a40(int param_1, char *param_2, int param_3);    // assign chat text (0x00481A40)
void  __cdecl AssignChat_stub(char *ID, char *Text, int Flag);          // 0x00482090
int   __stdcall Item_FindElfWeaponInventorySlot(void);                   // IDA: FUN_004824C0
int   __stdcall Item_CountElfWeaponInventorySlots(void);                 // IDA: FUN_00482850
int   __cdecl Item_CountWeaponGroupItems(int param_1);                   // IDA: FUN_00482e40
void  __cdecl FUN_0048b680(int weaponType);                              // elf weapon validation/swap (0x0048B680)
unsigned int __cdecl FUN_004942e0(int param_1);                          // chat command parser (0x004942E0)
bool  __cdecl CheckTarget_stub(DWORD c);                                 // 0x0049CAE0

// Missing forward declarations (compilation fixes)
int   __cdecl Xor_ConvertBuffer(void *lpBuffer, DWORD nBytes, int key); // XOR encode buffer
void  __cdecl FUN_0054337b(int *fp, char *fmt);                          // CRT fscanf wrapper
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
void  __cdecl FUN_0047eb80(void);                                        // 0x0047EB80 clearMatchInfo
int   __cdecl FUN_0045acc0(int key);                                     // 0x0045ACC0 FindCharacterIndex + limpia el flag +744 de todas
void  __cdecl FUN_0045ad10(int Action);                                  // 0x0045AD10 SetAction sobre todos los jugadores visibles
void  __cdecl FUN_00433830(void);                                        // 0x00433830 — InitPartyList
void  __stdcall SendCheck(void);                                         // 0x004220A0 — anti-tamper checksum packet

// Batch 20 — forward declarations for implemented stubs
void  __cdecl RenderText(int x, int y, char *text, int p1, int p2, void *p3); // Text render

// Batch 20 — OpenNpc, MoveCamera, RenderEquipment3D, RenderItems3D, LookAtTarget
void  __cdecl AccessModel(int id, char *path, char *name, int param);   // 0x005060B0
void  __cdecl OpenTexture(int id, void *path, int flags, bool param);   // 0x00505C80
// LoadWaveFile declared above (line 1088) with real signature — real impl in src/Sound/Sound.cpp.
void  __cdecl OpenModel(int id, char *path, ...);                       // 0x00505E90 (varargs: texName, normName, etc.)
void  __cdecl RenderItem3D(float sx, float sy, float w, float h, int Type, int Level, int Option1, int ExtOption, bool PickUp); // 0x004E1BE0
// InventoryColor = InventoryColor_stub (declared at line 1301)
// RenderEquipmentPart3D = RenderEquipmentPart3D_stub (declared at line 1303)
void  __cdecl RenderNumber2D_fn(float x, float y, int Num, float Width, float Height); // 0x005122F0
// DisableAlphaBlend = GL_ResetState (declared at line 578)
// CreateAngle = FUN_0043e050 (4 floats → float). Line 207 has wrong sig; cast in callers.
// FarAngle = Angle_GetDifference (already declared at line 208)
// RenderBitmap = GL_DrawTexture (declared at line 601)

// Missing function stubs (linker fix)
bool  __cdecl FindTextA(char *haystack, char *needle, bool caseSensitive);
void* __fastcall FUN_0040f730(void* param_1);                              // UI widget constructor (0x2C4 bytes)
void  __fastcall FUN_00410a90(int* param_1);                               // Simple widget constructor (4 bytes)

// Batch 21 — forward declarations for MoveObjects, CollisionDetectLineToMesh, CheckMixRecipe
void  __cdecl FUN_004fa5f0(int pObj);                                      // Object_AnimUpdate (per-frame anim tick)
void  __cdecl FUN_004fdc00(float pObj);                                    // Object_RenderUpdate (terrain render per object)
void  __cdecl FaceNormalize(float v[3], float out[3], float v2[3], float normal[3]); // 0x00440A60 approx
bool  __cdecl CollisionDetectLineToFace(float pos[3], float target[3], int normalIdx,
              float localC[3], float* posZ, float* v3, float* v4, float normal[3], char flag); // 0x00440C90 approx
int   __cdecl ItemValue(ITEM* ip, unsigned int goldType);                  // 0x0047C690

// ── SkillElf dependencies ────────────────────────────────────────────────────
void  __cdecl GetSkillInformation(int iType, int iLevel, char* lpszName, int* piMana, int* piDistance, int* piSkillMana); // 0x0047E7A0
// CheckArrow = Combat_CheckArrowRequirement (already declared as char __cdecl Combat_CheckArrowRequirement(void))
// SetPlayerAttack = FUN_00444410 (already declared; Ghidra shows 1-arg DWORD — cast in caller)
// CreateAngle = FUN_0043e050 (already declared line 207; Ghidra shows 4 floats → float — cast in caller)
