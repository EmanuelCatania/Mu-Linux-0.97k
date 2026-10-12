// Net_Process.cpp
// Net_ProcessPacket @ 0x004389A0  (1824 lines, 489 basic blocks)
// (server-config globals g_MaxCharacterLevel/g_CharDeleteMaxLevel/g_CharCreationEnable
//  declarados en globals.h con extern "C" — no necesitamos redeclararlos acá)

// Dispatcher de paquetes entrantes server→cliente. Corre en un loop do-while.
// Cada iteración desencola un paquete vía Net_GetFreeBuffer, despacha por opcode,
// y sigue hasta vaciar el pool.
//
// 72 phantom param_N / in_stack_ "parameters" are anti-tamper obfuscation noise.
//
// ── PACKET FORMAT ─────────────────────────────────────────────────────────────
//
//   Header byte 0:
//     0xC1 = 1-byte length (byte[1] = total len)
//     0xC2 = 2-byte length (byte[1-2] = len, big-endian)
//     0xC3 = encrypted 1-byte length
//     0xC4 = encrypted 2-byte length
//
//   After header:
//     byte[opcode_offset] = main opcode
//       C1: byte[2] = opcode,  byte[3..] = payload
//       C2: byte[3] = opcode,  byte[4..] = payload
//
//   Decryption: CSimpleModulus_Decode(&g_SimpleModulusSC, ...) — RC4-like stream cipher for C3/C4
//
// ── BUFFER POOL ───────────────────────────────────────────────────────────────
//
//   Net_GetFreeBuffer @ 0x0043E010
//     Pool base: SocketClient (300 entries, stride 0x2008)
//     Status flag: entry+0x401C (0 = free)
//     Returns: entry+0x4024 (packet data start), or NULL
//
//   puVar8   = current packet pointer
//   puVar9   = packet length (ushort)
//   param_1  = opcode (main switch key)
//
//   TotalPacketSize += byte[opcode] each packet (running byte counter)
//
// ── MAIN OPCODE SWITCH (param_1 = opcode byte) ────────────────────────────────
//
//   case 0x00:  Net_SendPacket(puVar8)         — re-queue/echo packet
//
//   case 0x01:  entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               CreateChat(entity+0x1c1, puVar8+5, entity, 0, -1)
//               → entity name/class update (entity stride 0x394 at DAT_07abf5d0)
//
//   case 0x02:  World-enter / spawn position:
//               Copies position payload into locals (0xf dwords)
//               FUN_004801c0()           — world state init
//               if m_bWhisperSound: PlayBuffer(0x26, 0, 0)
//               UIChatLogWindow_AddText(posData, nameData, 0)
//
//   case 0x03:  XOR handshake:
//               32-byte key = {0xe7,0x6d,0x3a,0x89,...} (same global key)
//               FUN_00412de0(byte[2])    — process auth challenge byte
//               Builds response + send() with full WSAEWOULDBLOCK retry path
//
//   case 0x07:  Entity flag update:
//               entity = FindCharacterIndex(byte[3]*256 + byte[7])
//               if byte[3]==1 && !(entity->flags & byte[2]): FUN_0043bde0(byte[2], entity)
//               else: FUN_0043c070(flags, entity)
//
//   case 0x0b:  Packet buffer slot management (max 9 slots, DAT_07e11db4):
//               if DAT_07e11db4 > 9: shift buffer array down
//               Guarda puVar8 en DAT_07e016c8[slot], copia los datos a DAT_07e109cc[slot*0x100]
//               FUN_00500a80()           — process buffered data
//
//   case 0x0c:  if byte[3]==0: UIChatLogWindow_AddText(ChatWhisperID, DAT_07d4d1fc, 2)
//                              → draw login/char-select widget
//
//   case 0x0d:  FUN_00427a00(puVar8)
//
//   case 0x0f:  DAT_07c74ae0 = (byte[3] & 0xf) != 0 ? (byte[3]&0xf)*6 : 0
//               → chat/channel color/type mapping
//
//   case 0x10:  FUN_00427b90(puVar8)
//   case 0x11:  FUN_00427f40(puVar8)
//
//   case 0x12:  El mismo manejo de slot-buffer que 0x0b
//               FUN_00429690(puVar8, puVar9)
//
//   case 0x13:  FUN_0042a230(puVar8, ..., puVar9)
//   case 0x14:  Loop DeleteCharacter(entityId) por la cantidad en byte[3] — lista de destrucción del viewport
//   case 0x15:  FUN_0042acc0(puVar8)
//   case 0x16:  FUN_0042db60(puVar8, iVar20)
//   case 0x17:  FUN_0042f030(puVar8)
//   case 0x18:  FUN_0042b4f0(puVar8)
//   case 0x19:  Skills_PacketHandler(puVar8, puVar9, iVar20)
//   case 0x1a:  FUN_0042d780(puVar8)
//
//   case 0x1b:  Entity state switch on byte[3]:
//               entity = FindCharacterIndex(entityId)
//               1  → FUN_0043c070(flag_1, entity)
//               7  → FUN_0043c070(flag_2, entity)
//               0x10→ FUN_0043c070(0x100, entity)
//               0x40→ FUN_0043c070(0x40,  entity)
//
//   case 0x1c:  Manejo de slot buffer (máx 9, igual que 0x0b)
//               FUN_00428210(puVar8, iVar20)
//
//   case 0x1e:  FUN_0042cd10(puVar8, puVar9, iVar20)
//   case 0x1f:  FUN_0042a530(puVar8)
//   case 0x20:  FUN_0042f240(puVar8)
//
//   case 0x21:  Entity removal list:
//               Loop on byte[2] count: entityId = byte[5+n*2]*256 + byte[4+n*2]
//               if entityId < 1000: DAT_07e12840[entityId*0x204] = 0
//
//   case 0x22:  FUN_0042f360(puVar8)
//   case 0x23:  FUN_0042f690(puVar8)
//
//   case 0x24:  Manejo de slot buffer (igual que 0x1c)
//               FUN_0042f9a0(puVar8, iVar20)
//
//   case 0x25:  FUN_00429230(puVar8)
//   case 0x26:  FUN_00431780()          — equip item response handler
//   case 0x27:  FUN_00431a90()
//
//   case 0x28:  FUN_004cce00(byte[3], &DAT_07ea8410, 8)   — item table slot update
//               if byte[2] != 0: DAT_05826d1c = 0         — reset equip cooldown
//
//   case 0x29:  FUN_004321f0(puVar8, iVar20)
//
//   case 0x2a:  Slot buffer management
//               FUN_00431ea0(puVar8)
//
//   case 0x2c:  FUN_00437f10(puVar8)
//   case 0x30:  FUN_004301b0(puVar8, iVar20)
//   case 0x31:  FUN_00427560(puVar8)
//
//   case 0x32:  InsertInventoryItem(&DAT_07ea8410, 8, 8, byte[3], puVar8+2, 0)
//               PlayBuffer(0x1d, 0, 0)    — inventory update + UI refresh
//
//   case 0x33:  if byte[3] != 0:
//                 DAT_07e91388 = 0
//                 STRUCT_DECRYPT(&MAIN_HASH_CLASS, DAT_07cf1ffc)  — decode g_CharData
//                 DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 STRUCT_ENCRYPT(&MAIN_HASH_CLASS, puVar23)       — re-encode g_CharData
//                 PlayBuffer(0x1d, 0, 0)
//
//   case 0x34:  if packet[2] != 0:
//                 STRUCT_DECRYPT; DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 CalculateAll(puVar23)
//                 STRUCT_ENCRYPT; PlayBuffer(0x25, 0, 0)
//
//   case 0x36:  Server-triggered re-login:
//               Stores PIN buffer: DAT_07ea9834/38/3c ← puVar8[3/7/0xb]
//               DAT_07ea983e = 0
//               SetErrorMessage(0x79)  — ShowErrorDialog(0x79)
//               Después (si DAT_07e91388 >= 1) corta, si no: cae al 0x37
//               NOTA: el bloque XOR de envío de este case es el camino de
//               respuesta al NACK de re-login del server — misma clave de 32 bytes, mismo loop de reintento.
//
//   case 0x37:  FUN_004332e0(puVar8)
//
//   case 0x38:  FUN_004cce00(byte[3], &DAT_07ea5298, 8)
//               PlayBuffer(0x1d, 0, 0)   — secondary inventory (bag/warehouse?)
//
//   case 0x39:  InsertInventoryItem(&DAT_07ea5298, 8, 4, byte[3], puVar8+2, 1)
//               PlayBuffer(0x1d, 0, 0)
//
//   case 0x3a:  DAT_07eaa0f4 = -(byte[3]!=0) & m_nTempMyTradeGold — toggle effect bit
//
//   case 0x3b:  DAT_07eaa0f0 = *(puVar8+2)   — 4-byte misc update
//
//   case 0x3c:  PIN/SecondPassword UI control:
//               0 → m_bYourConfirm = 0
//               1 → m_bYourConfirm = 1; UI_SetScene(0x19)
//               2 → m_bMyConfirm = 0; UI_SetScene(0x19)
//               * → UI_SetScene(0x19)
//
//   case 0x3d:  FUN_004337f0(puVar8)
//
//   case 0x40:  DAT_07eaa0e4 = byte[3]*0x100 + byte[2]
//               SetErrorMessage(0x78)   — ShowErrorDialog(0x78)
//
//   case 0x41:  Shop slot display — inner switch byte[3] (0-5):
//               pcVar21/pcVar26 → Widget_Draw(pcVar21, pcVar26, 2)
//               SetErrorMessage(0)
//
//   case 0x42:  FUN_00434660(puVar8)
//
//   case 0x43:  DAT_07eaa0e0 = 0
//               Widget_Draw(&DAT_05826d78, &DAT_07d4e96c, 2)
//
//   case 0x44:  Party/group HP bars:
//               Loop byte[3] count: byte = puVar8[2+n]
//               DAT_07e11e98[(upper nibble)*0x24] = min(lower nibble, 10)
//
//   case 0x45:  FUN_00429c50((float)puVar8)
//   case 0x46:  FUN_00436d60(puVar8)
//
//   case 0x50:  DAT_07eaa0d8 = byte[3]*0x100 + byte[2]
//               SetErrorMessage(0x77)   — ShowErrorDialog(0x77)
//
//   case 0x51:  FUN_00434780(puVar8)
//   case 0x52:  FUN_004348b0(puVar8)
//   case 0x53:  FUN_00434950(puVar8)
//
//   case 0x54:  Second password / PIN full reset:
//               DAT_07eaa114-117 = 0
//               HashTable_Insert_Short(&MAIN_HASH_CLASS, &DAT_07eaa118); DAT_07eaa118=0
//               PACKET_ENCRYPT; DAT_07eaa119=0; DAT_00559f5f=0; DAT_07eaa14c=0
//               HashTable_Insert_Short(&MAIN_HASH_CLASS, &DAT_07eaa11b); DAT_07eaa11b=0
//               PACKET_ENCRYPT; DAT_07eaa124=1; DAT_07eaa144=0
//
//   case 0x55:  Character list change (delete/create result):
//               DAT_07eaa124=1; DAT_07eaa144=1; GuildInputEnable=1
//               DAT_00559c84=0; ClearInput(0)
//               InputTextMax=8; InputNumber=0
//               *(DAT_07abf5d8+0x1da) = 999
//
//   case 0x56:  FUN_00435280(puVar8)
//
//   case 0x5a:  NPC / trade item list:
//               Loop byte[2] count; stride 0x2a per entry
//               FUN_00434dc0(entityId, puVar23, puVar23+2)
//
//   case 0x5b:  FUN_00435110(puVar8)
//
//   case 0x5c:  Entity trade/duel update:
//               entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               iVar20 = FUN_00434dc0(0xffffffff, puVar8+5, puVar8+0xd)
//               entity[0x1da] = (short)iVar20
//               GuildWar_UpdateEntityRelation(entity)
//
//   case 0x5d:  entity = FindCharacterIndex(byte[3]*256 + byte[2])
//               DAT_07eaa114 = 0
//               *(DAT_07abf5d0 + entity*0x394 + 0x1da) = 0xffff
//               DAT_07eaa0d0 = 0xffffffff
//
//   case 0x60:  FUN_004353e0(puVar8)
//   case 0x61:  FUN_00435390(puVar8)
//   case 0x62:  FUN_004354f0(puVar8)
//   case 0x63:  FUN_00435aa0(puVar8)
//
//   case 0x64:  DAT_05826ca4 = byte[3]; DAT_05826ca8 = byte[2]; DAT_05826d30 = 1
//
//   case 0x71:  Party_PacketHandler()
//   case 0x73:  FUN_00433a80(puVar8, iVar20)
//   case 0x81:  FUN_00434170(puVar8)
//   case 0x82:  FUN_00434400()
//   case 0x83:  FUN_00434450(puVar8)   — second password response (0x83 server ack)
//   case 0x86:  FUN_004366c0(puVar8)
//   case 0x87:  FUN_004367d0()
//   case 0x90:  FUN_00436820(puVar8)
//   case 0x91:  FUN_00436cb0(puVar8)
//   case 0x92:  StartMatchCountDown(byte[3] + 1)
//   case 0x93:  FUN_00436a80(puVar8)
//   case 0x94:  FUN_004372c0(puVar8)
//   case 0x95:  FUN_00437380(puVar8)
//   case 0x96:  FUN_004373a0(puVar8)
//   case 0x99:  FUN_004373d0(puVar8)
//   case 0x9a:  FUN_00436ac0(puVar8)
//   case 0x9b:  FUN_00436e40(puVar8)
//   case 0x9c:  FUN_0042e5c0(puVar8, iVar20)
//   case 0x9d:  FUN_00437400(puVar8)
//   case 0xa0:  FUN_00437450(puVar8)
//   case 0xa1:  FUN_00437480(puVar8)
//   case 0xa2:  FUN_004374b0(puVar8)
//   case 0xa3:  FUN_004374e0(puVar8)
//
// ── OPCODE 0xF1 — LOGIN RESPONSE ─────────────────────────────────────────────
//
//   Outer: switch on byte[3] (sub-opcode):
//
//   F1/00: FUN_00424010(puVar8)         — account list
//
//   F1/01: Login result — inner switch on byte[2]:
//     0x00 → DAT_05826cb0=0x15, state=3  (login rejected)
//     0x01 → DAT_05826cb0=0x14, LogIn=2, FUN_00412a70(), state=3
//                                        (login OK, request char list)
//     0x02 → DAT_05826cb0=0x16, state=3  (wrong password)
//     0x03 → DAT_05826cb0=0x17, state=3  (account banned)
//     0x04 → DAT_05826cb0=0x18, state=3
//     0x05 → DAT_05826cb0=0x19, state=3
//     0x06 → DAT_05826cb0=0x1a, state=3; CErrorReport_Write("Version_dismatch")
//     0x07 → DAT_05826cb0=0x1b (default), state=3
//     0x08 → DAT_05826cb0=0x1c, state=3
//     0x09 → DAT_05826cb0=0x25, state=3
//     0x0a → DAT_05826cb0=0x1d, state=3
//     0x0b → DAT_05826cb0=0x1e, state=3
//     0x0c → DAT_05826cb0=0x1f, state=3
//     0x0d → DAT_05826cb0=0x20, state=3
//     0x11 → DAT_05826cb0=0x26, state=3
//     0xc0/0xd0 → DAT_05826cb0=0x22, state=3
//     0xc1/0xd1 → DAT_05826cb0=0x23, state=3
//     0xc2/0xd2 → DAT_05826cb0=0x24, state=3
//
//   F1/02: FUN_004247d0(puVar8, iVar20)  — server list data
//
//   F1/03: Character list result:
//     byte[2]==0 → DAT_05826cb0=0x3f
//     byte[2]==1 → XOR decrypt 30 bytes with PacketXorKey3[i%3]
//                  DAT_05826cb0=0x3e; copy decrypted name → DAT_05826bdc
//
//   F1/04: Version/token result:
//     byte[2]==0 → DAT_05826cb0=0x41
//     byte[2]==1 → XOR decrypt 10 bytes, DAT_05826cb0=0x40, copy → DAT_055ca050
//     byte[2]==2 → DAT_05826cb0=0x42
//     byte[2]==3 → DAT_05826cb0=0x43
//
//   F1/05:
//     byte[2]==0 → DAT_05826cb0=0x45
//     byte[2]==1 → DAT_05826cb0=0x44
//     byte[2]==2 → DAT_05826cb0=0x46
//     byte[2]==3 → DAT_05826cb0=0x47
//
//   F1/12: Login/char-select gate:
//     byte[2]==0 → DAT_05826cb0=0x0c
//     byte[2]==1 → DAT_05826cb0=0x0b   (char select OK)
//     byte[2]==2 → DAT_05826cb0=0x0d
//
// ── OPCODE 0xF3 — CHAR LIST / ENTER WORLD ────────────────────────────────────
//
//   Dispatch on byte[3] (C1) or byte[2] (C2):
//
//   F3/00: FUN_00424240(puVar8)          — receive character list entries
//   F3/01: FUN_00424390(puVar8)          — receive character detail
//   F3/02: byte[2]==1 → DAT_05826cb0=0x39; else → DAT_05826d20=byte[2], 0x3a
//   F3/03: FUN_00425840(puVar8, iVar20)
//   F3/04: FUN_004264d0()
//   F3/05: FUN_00431180()
//   F3/06: FUN_00431480(puVar8)
//   F3/07: EXP update:
//          XOR decode puVar8+2 (3-byte key PacketXorKey3[i%3])
//          STRUCT_DECRYPT → decode g_CharData; g_CharData[0x1c] -= decoded_exp
//          STRUCT_ENCRYPT → re-encode g_CharData
//   F3/08: FUN_00431dc0(puVar8)
//   F3/10: FUN_00426cf0(puVar8, iVar20)
//   F3/11: FUN_004269f0(puVar8)
//   F3/13: entity=FindCharacterIndex(byte[2]*256+byte[5]); ChangeCharacterExt(entity, puVar8+7)
//   F3/14: DAT_07e91388=0; InsertInventoryItem(&DAT_07ea8410,8,8,byte[2],puVar8+5,0)
//          PlayBuffer(0x31, 0, 0)
//   F3/20: DAT_05826d24 = byte[2]
//   F3/22: DAT_05826c08 = puVar8[2]
//   F3/23: Server info block:
//          DAT_05826cc0-cc4 ← puVar8+2 (8B)
//          DAT_05826cc9-ccd ← puVar8+0xd (8B)
//          DAT_05826ca4=byte[6]; DAT_05826ca8=byte[0x15]
//          DAT_05826d33 = (byte[6] != 0xff)
//          DAT_05826cc8 = 0
//   F3/30: FUN_00436fb0()
//   F3/40: FUN_00436550(puVar8)
//
// ── OPCODE 0xF4 — SERVER REDIRECT ────────────────────────────────────────────
//
//   Dispatch on byte[3] (C1) or byte[2] (C2):
//
//   F4/02: FUN_00423e10(puVar8)          — reconecta a otro puerto/IP
//   F4/03: Server redirect:
//          Parse IP from puVar8+2; Net_Disconnect(SocketClient)
//          CreateSocket(ip, port) — connect to new server
//          if result != 0: g_bGameServerConnected = 1
//          crt_sprintf + Widget_Draw — show "connecting" UI
//   F4/05: DAT_05826cb0=1; DAT_083a7c14=1   — back to Connecting state
//
// ── DEFAULT ───────────────────────────────────────────────────────────────────
//
//   Unrecognized opcode → Item_ReturnPickedItem() (log/discard)
//
// ── C2 / ENCRYPTED PACKET PATH ───────────────────────────────────────────────
//
//   if (byte[0] == 0xC2):  puVar9 = byte[1]*256+byte[2]; opcode = byte[3]
//   if (byte[0] == 0xC3):  CSimpleModulus_Decode → decrypt 1-byte-len; goto LAB_00439505
//   if (byte[0] == 0xC4):  CSimpleModulus_Decode → decrypt 2-byte-len; goto LAB_00439505
//   Los dos caminos desencriptados vuelven al mismo LAB_00439505 → switch de opcodes
//
//   Packet sequence tracking (anti-replay / dedup):
//     g_byPacketSerialRecv = rolling sequence counter
//     HashTable_GetIndex / operator_new(2) / HashTable_Insert / HashTable_Remove
//     → trackea los IDs de secuencia de paquetes en vuelo, manda NACK si no coinciden
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_0043E010  → Net_GetFreeBuffer(pool)
//   FindCharacterIndex  → Entity_GetIndex(entityId)  — returns 0-based entity slot
//   DeleteCharacter  → Entity_Spawn(entityId)     — create or update entity slot
//   CreateChat  → Entity_UpdateNameData(name, data, entity, 0, -1)
//   FUN_004801c0  → World_StateInit()          — inicializa el estado in-world después del 0x02
//   FUN_00412de0  → Auth_ProcessChallenge(byte) — handshake response for opcode 0x03
//   FUN_0043bde0  → Entity_SetFlag(flag, entity)
//   FUN_0043c070  → Entity_UpdateFlags(flags, entity)
//   FUN_00500a80  → BufferedPacket_Process()   — processes 0x0b slot buffer
//   FUN_00428210  → FUN_00428210(puVar8, iVar20) — 0x1c handler
//   FUN_00427a00  → PacketHandler_0x0d(puVar8)
//   FUN_00427b90  → PacketHandler_0x10(puVar8)
//   FUN_00427f40  → PacketHandler_0x11(puVar8)
//   FUN_00429690  → PacketHandler_0x12(puVar8, puVar9)
//   FUN_0042a230  → PacketHandler_0x13(puVar8)
//   FUN_0042acc0  → PacketHandler_0x15(puVar8)
//   FUN_0042db60  → PacketHandler_0x16(puVar8, iVar20)
//   FUN_0042f030  → PacketHandler_0x17(puVar8)
//   FUN_0042b4f0  → PacketHandler_0x18(puVar8)
//   Skills_PacketHandler  → PacketHandler_0x19(puVar8, puVar9, iVar20)
//   FUN_0042d780  → PacketHandler_0x1a(puVar8)
//   FUN_0042cd10  → PacketHandler_0x1e(puVar8, puVar9, iVar20)
//   FUN_0042a530  → PacketHandler_0x1f(puVar8)
//   FUN_0042f240  → PacketHandler_0x20(puVar8)
//   FUN_0042f360  → PacketHandler_0x22(puVar8)
//   FUN_0042f690  → PacketHandler_0x23(puVar8)
//   FUN_0042f9a0  → PacketHandler_0x24(puVar8, iVar20)
//   FUN_00429230  → PacketHandler_0x25(puVar8)
//   FUN_00431780  → Equip_HandleResponse()
//   FUN_00431a90  → PacketHandler_0x27()
//   FUN_004cce00  → ItemTable_SetSlot(slot, table, stride)
//   FUN_004321f0  → PacketHandler_0x29(puVar8, iVar20)
//   FUN_00431ea0  → PacketHandler_0x2a(puVar8)
//   FUN_00437f10  → PacketHandler_0x2c(puVar8)
//   FUN_004301b0  → PacketHandler_0x30(puVar8, iVar20)
//   FUN_00427560  → PacketHandler_0x31(puVar8)
//   InsertInventoryItem  → ItemTable_UpdateSlot(table, stride, size, slot, data, flag)
//   STRUCT_DECRYPT  → CharData_Decode(ctx, g_CharData)  — XOR-decode g_CharData
//   CalculateAll  → CharData_RecalcStats(charData)
//   STRUCT_ENCRYPT  → CharData_Encode(ctx, g_CharData)  — XOR-encode g_CharData
//   FUN_004332e0  → PacketHandler_0x37(puVar8)
//   FUN_004337f0  → PacketHandler_0x3d(puVar8)
//   FUN_00434660  → PacketHandler_0x42(puVar8)
//   FUN_00434780  → PacketHandler_0x51(puVar8)
//   FUN_004348b0  → PacketHandler_0x52(puVar8)
//   FUN_00434950  → PacketHandler_0x53(puVar8)
//   HashTable_Insert_Short  → HashTable_RefDecrement(ctx, key)
//   FUN_00435280  → PacketHandler_0x56(puVar8)
//   FUN_00434dc0  → Trade_GetItemData(entityId, data, extraData)
//   FUN_00435110  → PacketHandler_0x5b(puVar8)
//   GuildWar_UpdateEntityRelation  → Entity_UpdateMisc(entity)
//   FUN_004353e0  → PacketHandler_0x60(puVar8)
//   FUN_00435390  → PacketHandler_0x61(puVar8)
//   FUN_004354f0  → PacketHandler_0x62(puVar8)
//   FUN_00435aa0  → PacketHandler_0x63(puVar8)
//   Party_PacketHandler  → PacketHandler_0x71()
//   FUN_00433a80  → PacketHandler_0x73(puVar8, iVar20)
//   FUN_00434170  → PacketHandler_0x81(puVar8)
//   FUN_00434400  → PacketHandler_0x82()
//   FUN_00434450  → SecondPassword_HandleResponse(puVar8)
//   FUN_004366c0  → PacketHandler_0x86(puVar8)
//   FUN_004367d0  → PacketHandler_0x87()
//   FUN_00436820  → PacketHandler_0x90(puVar8)
//   FUN_00436cb0  → PacketHandler_0x91(puVar8)
//   StartMatchCountDown  → CharSelect_SetSlotCount(count)
//   FUN_00436a80  → PacketHandler_0x93(puVar8)
//   FUN_004372c0  → PacketHandler_0x94(puVar8)
//   FUN_00437380  → PacketHandler_0x95(puVar8)
//   FUN_004373a0  → PacketHandler_0x96(puVar8)
//   FUN_004373d0  → PacketHandler_0x99(puVar8)
//   FUN_00436ac0  → PacketHandler_0x9a(puVar8)
//   FUN_00436e40  → PacketHandler_0x9b(puVar8)
//   FUN_0042e5c0  → PacketHandler_0x9c(puVar8, iVar20)
//   FUN_00437400  → PacketHandler_0x9d(puVar8)
//   FUN_00437450  → PacketHandler_0xa0(puVar8)
//   FUN_00437480  → PacketHandler_0xa1(puVar8)
//   FUN_004374b0  → PacketHandler_0xa2(puVar8)
//   FUN_004374e0  → PacketHandler_0xa3(puVar8)
//   FUN_00424010  → Login_RecvAccountList(puVar8)   — F1/00
//   FUN_004247d0  → Login_RecvServerList(puVar8, iVar20) — F1/02
//   FUN_00424240  → CharList_RecvList(puVar8)       — F3/00
//   FUN_00424390  → CharList_RecvDetail(puVar8)     — F3/01
//   FUN_00425840  → CharList_RecvExtra(puVar8, iVar20) — F3/03
//   FUN_004264d0  → CharList_RecvEnd()              — F3/04
//   FUN_00431180  → PacketHandler_F3_05()
//   FUN_00431480  → PacketHandler_F3_06(puVar8)
//   FUN_00431dc0  → PacketHandler_F3_08(puVar8)
//   FUN_00426cf0  → PacketHandler_F3_10(puVar8, iVar20)
//   FUN_004269f0  → PacketHandler_F3_11(puVar8)
//   ChangeCharacterExt  → Entity_SetExtraData(entity, data)
//   FUN_00436fb0  → PacketHandler_F3_30()
//   FUN_00436550  → PacketHandler_F3_40(puVar8)
//   FUN_00423e10  → Net_RecvRedirect(puVar8)        — F4/02
//   CreateSocket  → Net_Connect(ip, port)
//   CSimpleModulus_Decode  → Packet_Decrypt(ctx, outBuf, data, len)  — C3/C4 cipher
//   CSimpleModulus_Encode  → Packet_Encode / CRC_Compute
//   FUN_00412a70  → FUN_00412a70()  — se llama al login OK (F1/01/01)
//   CErrorReport_Write  → Log_SetString(buf, str)
//   PlayBuffer  → UI_SetScene(id, 0, 0)
//   UIChatLogWindow_AddText  → Widget_Draw(element, textureData, flag)
//   SetErrorMessage  → ShowErrorDialog(id)
//   Item_ReturnPickedItem  → Packet_Unknown_Log()
//   PACKET_DECRYPT → HashTable_GetOrInsert
//   PACKET_ENCRYPT  → HashTable_Decrement
//   HashTable_Insert  → HashTable_Insert
//   Packet_DecryptByte  → HashTable_Remove
//   HashTable_GetNode  → HashTable_Get
//   Packet_EncryptByte  → HashTable_Free(entry, key)
//   HashTable_GetIndex → Item_ReturnPickedItem area (addr in binary)
//   FUN_0043de60  → Net_Throttle()
//   Net_Disconnect at 0043dc90
//   operator_new  → MSVC heap alloc
//
// ── FUNCIONES AUXILIARES DE RED / MOVIMIENTO (0x43bde0..0x43ff60) ───────────
//
//   Todas en el mismo rango de dirección que Net_ProcessPacket pero son
//   funciones utilitarias de entidad/movimiento/math que los handlers llaman.
//
// ── NET CONTEXT MANAGEMENT ───────────────────────────────────────────────────
//
//   0x0043daf0  NetCtx_Clear(int ctx)   __fastcall
//     Limpia el buffer del contexto de red:
//       memset(ctx+0x401c, 0, 0x96258*4)   — packet buffer
//       ctx+0x4014 = 0; ctx+0x4018 = 0     — head/tail ptrs
//     Return: ctx
//
//   0x0043db30  Net_WSAInit(int ctx)   __fastcall
//     WSAStartup(0x0202, &local_190)
//     Si error: log "Winsock_DLL_Initialize_error" + MessageBoxA("IError") → return 0
//     Si versión OK (2.2): ctx+8=0; ctx+4=wVersion; CWsctlc__LogPrintOn(); return 1
//
//   0x0043dbf0  Net_CreateSocket(void* this, HWND hWnd)   __thiscall
//     socket(AF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=0) → this+8
//     g_bGameServerConnected = 0  (connected flag)
//     Si INVALID_SOCKET: log error + MessageBoxA → return 0
//     *this = hWnd  (guarda HWND para WSAAsyncSelect)
//     return 1
//
// ── ENTITY ANGLE / MOVEMENT MATH ─────────────────────────────────────────────
//
//   Estas funciones están en el mismo rango de dirección pero son math de
//   movimiento de entidades. Se documentan acá porque son llamadas por los
//   handlers de movimiento en Net_ProcessPacket.
//
//   0x0043e050  Entity_GetDirCode(float x1,y1, float x2,y2) → ushort
//     dx = x2-x1; dy = y2-y1
//     Math_Fabs(dx) = abs o sqrt
//     Si |dx| < _DAT_00552868: retorna código de dirección vertical (N/S)
//     Else: calcula atan2 → código de dirección ushort (8 direcciones)
//
//   0x0043e120  Angle_ShortestPath(int cur, int target, int maxStep) → int
//     Calcula la diferencia más corta entre ángulos (wraparound a 0x168=360)
//     Si |delta| > 180: ajusta vía offset de 360
//     Retorna min(|delta|, maxStep) con signo
//
//   0x0043e1b0  Angle_Interpolate(float cur, float target, float step) → float10
//     Normaliza ambos ángulos (+ 360 si < 0)
//     Interpola suavemente target → cur respetando wraparound de 360
//
//   0x0043e370  Angle_Delta(float from, float to, char wrap) → float10
//     Normaliza ambos; retorna (to - from) con wraparound de ±180
//
//   0x0043e430  Math_Atan2_ToAngle(float x1,y1, float x2,y2) → int
//     fpatan((y2-y1)/(x2-x1)) → ángulo en grados enteros
//     Si x2 < x1: ajusta 180°. Resultado en [0..360)
//
//   0x0043e4a0  Entity_UpdateFacing(float* pos, float* entity, float* target, float step)
//     Llama Entity_GetDirCode(pos, target) → código dirección
//     Llama Angle_Interpolate(entity[2], dirCode, step) → entity[2] (ángulo)
//     dx=pos-target; distancia 3D; actualiza ángulo de elevación
//
//   0x0043e570  Entity_ApplyRotation(float* out, float* quat, float* vec)
//     Matrix_BuildFromEuler(quat, mat4x3) — quaternion → matriz rotación
//     Vector_Rotate(vec, mat4x3, &local) — matriz × vector
//     out[0..2] += local[0..2]  (aplica rotación al offset)
//
//   0x0043e5c0  Entity_SmoothAngle(int entity)
//     entity+0x161 (char flag): si 0 → lerp suave hacia target:
//       entity+0x168 += (entity+0x164 - entity+0x168) * _DAT_005524f4
//     Si flag != 0 → snap directo (o animación invertida)
//
//   0x0043e680  Entity_UpdatePath(int p1,p2,p3,p4)
//     Función compleja (~40 líneas) con muchos floats y ftol
//     Actualiza la trayectoria de movimiento de una entidad
//
//   0x0043e820  Entity_SetAnimation(int entity, uint animId)
//     Verifica animId < animData[entity.type * 0xbc + 0x26] o == 0x4c/0x4d
//     Si animId != anim actual:
//       entity+0x106 = entity+0x105  (prev anim)
//       entity+0x10c = entity+0x108  (prev anim timer)
//       entity+0x105 = animId; entity+0x108 = 0  (reset timer)
//
//   0x0043e890  Entity_FaceTarget(int entity, int target)
//     Si target != 0:
//       Entity_GetDirCode(entity.pos, target.pos)
//       Angle_Delta(entity.angle, dirCode, 1)
//       Actualiza entity.angle suavemente
//
//   0x0043e940  Entity_AnimTick(int entity)
//     Lee entity+0x105 (anim state). Si != 0x06: avanza frame de animación
//     Máquina de estados de animación (idle/walk/attack/die/...)
//
//   0x0043ea20  Angle_ToDir(float angle, char mode) → undefined4
//     Convierte ángulo float a código de dirección / byte de dirección
//     Considera parámetro mode para inversión o modo especial
//
// ── PACKET QUEUE / ACTION QUEUE ──────────────────────────────────────────────
//
//   0x0043f2d0  PacketBuf_Free(void)
//     Libera buffers en DAT_05826df4 (índices 0xff y 0x102)
//     operator_delete para cada buffer no-NULL
//
//   0x0043f3e0  PacketQueue_Enqueue(uint id, float param2, uint p3, uint p4, void* data, float p6)
//     Wrapper: llama PATH_FindPath(DAT_05826df4, id, param2, p3, p4, 1, 2, p6)
//     local_4 = 2 (prioridad/tipo)
//
//   0x0043f500  ActionQueue_Insert(void* this, int id, float t, int p3, int p4, int p5, int p6, float p7)
//     __thiscall; inserta una acción en la cola de acciones del servidor
//     Gestiona slots, timestamps, prioridades
//     Usado para movimiento suavizado y predicción del cliente
//
//   0x0043fd30  Queue_SetRange(void* this, int param_1)   __thiscall
//     Clamp param_1 a [0, this+8)
//     Actualiza this+0x400 (puntero de escritura) y this+0x404 (puntero de lectura)
//     → control de rango circular del buffer
//
//   0x0043fd70  AnimTimer_Update(void)
//     Si FpsTimerInitialized == 0: init (FpsWindowStartTimeMs=timeGetTime(), FpsTimerInitialized=1)
//     Si no: actualiza DAT_05826e10 (delta del timer de frame) vía timeGetTime()
//     → timer global de animación, usado por Entity_AnimTick
//
//   0x0043fea0  LinkedList_Add(void* this, undefined4 data, int param_2)   __thiscall
//     Si this+8 == 0: this+4 += 1; operator_new(0x14); enlaza nodo
//     Linked list de nodos 0x14 bytes (data + next ptr)
//
//   0x0043ff60  LinkedList_Remove(undefined4* param_1)   __fastcall
//     Traversal de la lista enlazada; desenlaza y libera nodo
//     Usado por PacketQueue_Enqueue para gestión de memoria
//
// ── LOGIN / SEND BUILDERS ─────────────────────────────────────────────────────
//
//   0x0043c250  Login_BuildEncPacket(byte p1, byte p2, byte p3, byte p4)   __cdecl
//     Construye paquete 0xC1/0xF1 con buffer local_d3c[32] (32-byte XOR key)
//     Idéntico al bloque XOR de Net_ProcessPacket (caso 0x03/0x36)
//     Usado para re-auth desde Scene_Dispatch
//
//   0x0043ce50  CharSelect_BuildPacket(byte sub, undefined4 data)   __cdecl
//     Construye paquete de char-select con buffer local_434[32]
//     Envía selección de personaje con XOR encrypt
//
// ── ENTITY FLAGS (referenciados en opcodes) ───────────────────────────────────
//
//   0x0043bde0  Entity_SetFlag(byte flag, entity*)
//     Activa el flag especificado en entity->flags
//
//   0x0043c070  Entity_UpdateFlags(uint flags, entity*)
//     Actualiza múltiples flags de la entidad de una vez
//
//   0x0043c250  → ver Login_BuildEncPacket arriba
//
//   0x0043d3e0  HashTable_Insert2(void* this, undefined4)   __thiscall
//     Inserta en HashTable con lógica de colisión (abStack_14[4] key)
//
//   0x0043d670  HashTable_Find2(void* this, undefined4*)   __thiscall
//     Busca en HashTable, retorna undefined4 (ptr o índice)

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

void NetLog(const char* fmt, ...)
{
    if (!fmt) return;     // null guard — el invalid_parameter handler de
                          // ucrtbased recursaba sobre format=NULL.
    char buf[256];
    va_list ap; va_start(ap, fmt);
    int n = _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    if (n < 0) buf[0] = 0;
}

// ============================================================================
// Dispatcher principal
// ============================================================================
void Net_ProcessPacket(void)
{
    while (true) {
        BYTE* Msg = (BYTE*)CWsctlc_GetReadMsg((int)(uintptr_t)SocketClient);
        if (!Msg) return;

        int HeadCode, Size;
        BYTE hdr = Msg[0];
        bool bEncrypted = false;

        // COPIA DEL PAQUETE ANTES DE PROCESARLO.
        //
        // `CWsctlc_GetReadMsg` (GetReadMsg) devuelve un puntero DENTRO del buffer de
        // recepcion del socket, que es compartido.  Handlers que tardan --
        // sobre todo los de viewport, porque `CreateMonster` carga el BMD del
        // monstruo -- dejan que se bombee la cola de mensajes en el medio, entra
        // un `Net_Recv` y el buffer se sobreescribe MIENTRAS el handler todavia
        // esta recorriendo sus entradas.  Por eso todos los paquetes se copian (los
        // C3/C4 ademas se desencriptan a `scratch`).
        BYTE __pktCopy[0x2100];
        {
            const int __wire = (hdr == 0xC1 || hdr == 0xC3)
                             ? (int)Msg[1]
                             : (((int)Msg[1] << 8) | (int)Msg[2]);
            if (__wire > 0 && __wire <= (int)sizeof(__pktCopy)) {
                memcpy(__pktCopy, Msg, (size_t)__wire);
                Msg = __pktCopy;
            }
        }

        // ── C3/C4 in-place decrypt ────────────────────────────────────────
        // Paquetes encriptados server→cliente. Los bytes del cuerpo (después del
        // header de tamaño de cable) están codificados con CSimpleModulus. Los decodificamos in situ
        // en el mismo buffer con un header C1/C2 para que el resto del dispatcher
        // doesn't care about encryption.
        //
        //   C3 [encLen] [enc body...]   →   C1 [plainLen] [plain body...]
        //   C4 [hi][lo] [enc body...]   →   C2 [hi][lo]   [plain body...]
        //
        // CSimpleModulus_Decode(dst, src, srcLen, ?) escribe 8 bytes por cada 11 bytes encriptados.
        BYTE  scratch[0x800];
        if (hdr == 0xC3 || hdr == 0xC4) {
            bEncrypted = true;
            int hdrSz = (hdr == 0xC3) ? 2 : 3;
            int wireLen = (hdr == 0xC3) ? Msg[1] : ((Msg[1] << 8) | Msg[2]);
            int encLen  = wireLen - hdrSz;
            // DESBORDE DE PILA (2026-10-02, issue #75): `encLen` sale del CABLE
            // (hasta 65532 en un C4) y CSimpleModulus_Decode escribe 8 bytes por
            // cada 11 de entrada dentro de `scratch[0x800]`.  La validacion de
            // tamaño estaba MAS ABAJO, o sea despues de que la escritura ya ocurrio.
            // Con el server mandando su primer paquete encriptado eso pisaba la
            // cookie /GS y MSVC mataba el proceso con __fastfail: sin excepcion, sin
            // handler, sin cartel y sin linea de log.
            //
            // Decode con dst==0 devuelve solo el tamaño, asi que se valida primero.
            if (encLen <= 0) {
                NetLog("NET: C%c encLen invalido (%d) — descarto",
                       (hdr == 0xC3) ? '3' : '4', encLen);
                continue;
            }
            int needLen = CSimpleModulus_Decode(0, (int)(Msg + hdrSz), encLen, 0);
            if (needLen <= 0 || needLen > (int)sizeof(scratch)) {
                NetLog("NET: C%c decode no entra: encLen=%d needLen=%d > scratch=%d — descarto",
                       (hdr == 0xC3) ? '3' : '4', encLen, needLen, (int)sizeof(scratch));
                continue;
            }
            int outLen  = CSimpleModulus_Decode((int)scratch, (int)(Msg + hdrSz), encLen, 0);
            if (outLen <= 0) {
                NetLog("NET: C%c decode FAILED encLen=%d outLen=%d",
                       (hdr == 0xC3) ? '3' : '4', encLen, outLen);
                continue;
            }
            // Re-enmarcado como C1/C2 plano. outLen excluye los bytes del header — pero el
            // primer byte desencriptado es el SerialSend, que el dispatcher C1 original
            // NO ve (lee el opcode en Msg[2]). Así que mantenemos
            // the layout: [C1][len][serial][opcode][subop][payload].
            // El serial en +1 es el byte de tamaño para el dispatcher (Msg[1]).
            // Per IDA: C3 infla reemplazando el byte de serial por el tamaño, pero nuestro
            // dispatcher sólo usa Msg[1] para el largo y Msg[2..] para el opcode.
            // Descartamos el 1er byte (serial) y usamos el layout plano.
            int plainLen = 1 + outLen - 1 + 1;  // C1 + (outLen - serial) + total
            // Simpler: build plain = [C1][outLen][plain_body_after_serial...]
            int bodyLen = outLen - 1;            // skip serial byte
            if (bodyLen <= 0 || bodyLen + 2 > (int)sizeof(scratch)) {
                NetLog("NET: C3 decode bad bodyLen=%d", bodyLen);
                continue;
            }
            // Corre el payload 1 byte para hacer lugar al byte plainLen
            BYTE plain[0x800];
            plain[0] = 0xC1;
            // `plain[1]` es UN byte, asi que para un paquete de mas de 255 el tamaño se
            // trunca. Por eso el tamaño se lleva APARTE del buffer: `Size` es un int, se
            // toma del valor real y NO se relee de `Msg[1]`. El byte de `plain[1]` queda
            // truncado, pero no lo consume nadie.
            //
            // No se re-enmarca como C2 (que es lo que haria el original para un paquete
            // largo): eso correria +1 todos los offsets del cuerpo de cada handler que
            // hoy asume el layout C1.
            plain[1] = (BYTE)(bodyLen + 2);
            memcpy(plain + 2, scratch + 1, bodyLen);
            memcpy(Msg, plain, bodyLen + 2);
            hdr      = 0xC1;
            HeadCode = Msg[2];
            Size     = bodyLen + 2;   // real, no `Msg[1]` (que puede estar truncado)
            // El byte truncado se loguea al lado a proposito: cuando difiere de
            // `Size`, el paquete supera los 255 bytes.
            if (Size > 0xFF) {
                NetLog("NET: C3->C1 decoded encLen=%d -> Size=%d op=%02X  [byte truncado=%d — ANTES se usaba ESTE]",
                       encLen, Size, HeadCode, (int)Msg[1]);
            } else {
                NetLog("NET: C3->C1 decoded encLen=%d -> Size=%d op=%02X",
                       encLen, Size, HeadCode);
            }
        } else if (hdr == 0xC1) {
            HeadCode = Msg[2];
            Size     = Msg[1];
        } else if (hdr == 0xC2) {
            HeadCode = Msg[3];
            Size     = (Msg[1] << 8) | Msg[2];
        } else {
            NetLog("NET: unknown hdr=%02X — discard", hdr);
            continue;
        }

        BYTE sub = Msg[(hdr == 0xC1) ? 3 : 4];
        NetLog("NET: hdr=%02X op=%02X sub=%02X size=%d",
               hdr, HeadCode, sub, Size);

        // ── dispatch on opcode ───────────────────────────────────────────
        EquipWipe_Tick(HeadCode, sub);   // diagnostico del wipe de equipo

        switch (HeadCode) {
            case 0xF1: NetRecv_F1(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xF3: NetRecv_F3(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xF4: NetRecv_F4(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x0E: NetRecv_0E(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x01: NetRecv_01(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x02: NetRecv_02(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x53: NetRecv_53(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x54: NetRecv_54(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x55: NetRecv_55(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x56: NetRecv_56(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x10: NetRecv_10(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x03: NetRecv_03(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x11: NetRecv_11(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x12: NetRecv_12(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x45: NetRecv_45(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x13: NetRecv_13(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x14: NetRecv_14(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x15: NetRecv_15(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x18: NetRecv_18(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x07: NetRecv_07(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x19: NetRecv_19(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x1F: NetRecv_1F(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x1E: NetRecv_1E(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x1B: NetRecv_1B(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x16: NetRecv_16(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x17: NetRecv_17(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x25: NetRecv_25(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x24: NetRecv_24(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x2C: NetRecv_2C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x30: NetRecv_30(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x31: NetRecv_31(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x32: NetRecv_32(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x33: NetRecv_33(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x34: NetRecv_34(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x36: NetRecv_36(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x37: NetRecv_37(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x38: NetRecv_38(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x39: NetRecv_39(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x3A: NetRecv_3A(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x3B: NetRecv_3B(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x3C: NetRecv_3C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x3D: NetRecv_3D(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x44: NetRecv_44(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x40: NetRecv_40(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x41: NetRecv_41(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x42: NetRecv_42(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x43: NetRecv_43(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x46: NetRecv_46(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x50: NetRecv_50(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x51: NetRecv_51(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x52: NetRecv_52(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x71: NetRecv_71(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x81: NetRecv_81(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x82: NetRecv_82(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x83: NetRecv_83(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x86: NetRecv_86(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x87: NetRecv_87(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x88: NetRecv_88(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x73: NetRecv_73(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x8E: NetRecv_8E(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x8F: NetRecv_8F(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x90: NetRecv_90(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x91: NetRecv_91(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x92: NetRecv_92(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x93: NetRecv_93(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x94: NetRecv_94(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x95: NetRecv_95(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x96: NetRecv_96(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x97: NetRecv_97(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x9D: NetRecv_9D(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x99: NetRecv_99(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x9A: NetRecv_9A(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x9B: NetRecv_9B(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x9C: NetRecv_9C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x1A: NetRecv_1A(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x1C: NetRecv_1C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x23: NetRecv_23(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x22: NetRecv_22(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x26: NetRecv_26(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x27: NetRecv_27(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x28: NetRecv_28(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x29: NetRecv_29(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x2A: NetRecv_2A(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x2F: NetRecv_2F(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x5A: NetRecv_5A(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x5C: NetRecv_5C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x5D: NetRecv_5D(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x60: NetRecv_60(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x61: NetRecv_61(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x62: NetRecv_62(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x63: NetRecv_63(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x64: NetRecv_64(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x5B: NetRecv_5B(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x65: NetRecv_65(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x0B: NetRecv_0B(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x0C: NetRecv_0C(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x0D: NetRecv_0D(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x0F: NetRecv_0F(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x00: NetRecv_00(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x20: NetRecv_20(Msg, Size, hdr, sub, bEncrypted); break;
            case 0x21: NetRecv_21(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xA0: NetRecv_A0(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xA1: NetRecv_A1(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xA2: NetRecv_A2(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xA3: NetRecv_A3(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xDD: NetRecv_DD(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xDE: NetRecv_DE(Msg, Size, hdr, sub, bEncrypted); break;
            case 0xDF: NetRecv_DF(Msg, Size, hdr, sub, bEncrypted); break;

            default:
                NetLog("NET:  → op=%02X unhandled (in-game)", HeadCode);
                break;
        }

        (void)Size;
        (void)bEncrypted;
    }
}
