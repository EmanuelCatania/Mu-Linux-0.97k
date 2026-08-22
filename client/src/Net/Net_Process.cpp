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
//   Decryption: FUN_0053cca0(&DAT_05826c58, ...) — RC4-like stream cipher for C3/C4
//
// ── BUFFER POOL ───────────────────────────────────────────────────────────────
//
//   Net_GetFreeBuffer @ 0x0043E010
//     Pool base: DAT_055ca160 (300 entries, stride 0x2008)
//     Status flag: entry+0x401C (0 = free)
//     Returns: entry+0x4024 (packet data start), or NULL
//
//   puVar8   = current packet pointer
//   puVar9   = packet length (ushort)
//   param_1  = opcode (main switch key)
//
//   DAT_07e11dcc += byte[opcode] each packet (running byte counter)
//
// ── MAIN OPCODE SWITCH (param_1 = opcode byte) ────────────────────────────────
//
//   case 0x00:  Net_SendPacket(puVar8)         — re-queue/echo packet
//
//   case 0x01:  entity = FUN_0045ac80(byte[3]*256 + byte[2])
//               FUN_00481ba0(entity+0x1c1, puVar8+5, entity, 0, -1)
//               → entity name/class update (entity stride 0x394 at DAT_07abf5d0)
//
//   case 0x02:  World-enter / spawn position:
//               Copies position payload into locals (0xf dwords)
//               FUN_004801c0()           — world state init
//               if DAT_07e11d80: FUN_00404bc0(0x26, 0, 0)
//               FUN_00480620(posData, nameData, 0)
//
//   case 0x03:  XOR handshake:
//               32-byte key = {0xe7,0x6d,0x3a,0x89,...} (same global key)
//               FUN_00412de0(byte[2])    — process auth challenge byte
//               Builds response + send() with full WSAEWOULDBLOCK retry path
//
//   case 0x07:  Entity flag update:
//               entity = FUN_0045ac80(byte[3]*256 + byte[7])
//               if byte[3]==1 && !(entity->flags & byte[2]): FUN_0043bde0(byte[2], entity)
//               else: FUN_0043c070(flags, entity)
//
//   case 0x0b:  Packet buffer slot management (max 9 slots, DAT_07e11db4):
//               if DAT_07e11db4 > 9: shift buffer array down
//               Guarda puVar8 en DAT_07e016c8[slot], copia los datos a DAT_07e109cc[slot*0x100]
//               FUN_00500a80()           — process buffered data
//
//   case 0x0c:  if byte[3]==0: FUN_00480620(DAT_05826cb4, DAT_07d4d1fc, 2)
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
//   case 0x14:  Loop FUN_0045ac20(entityId) por la cantidad en byte[3] — lista de destrucción del viewport
//   case 0x15:  FUN_0042acc0(puVar8)
//   case 0x16:  FUN_0042db60(puVar8, iVar20)
//   case 0x17:  FUN_0042f030(puVar8)
//   case 0x18:  FUN_0042b4f0(puVar8)
//   case 0x19:  FUN_0042bca0(puVar8, puVar9, iVar20)
//   case 0x1a:  FUN_0042d780(puVar8)
//
//   case 0x1b:  Entity state switch on byte[3]:
//               entity = FUN_0045ac80(entityId)
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
//   case 0x32:  FUN_004cc660(&DAT_07ea8410, 8, 8, byte[3], puVar8+2, 0)
//               FUN_00404bc0(0x1d, 0, 0)    — inventory update + UI refresh
//
//   case 0x33:  if byte[3] != 0:
//                 DAT_07e91388 = 0
//                 FUN_00423040(&DAT_055c9bc8, DAT_07cf1ffc)  — decode g_CharData
//                 DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 FUN_0043d1d0(&DAT_055c9bc8, puVar23)       — re-encode g_CharData
//                 FUN_00404bc0(0x1d, 0, 0)
//
//   case 0x34:  if packet[2] != 0:
//                 FUN_00423040; DAT_07cf1ffc[0x152] = *(puVar8+2)
//                 FUN_0047e3c0(puVar23)
//                 FUN_0043d1d0; FUN_00404bc0(0x25, 0, 0)
//
//   case 0x36:  Server-triggered re-login:
//               Stores PIN buffer: DAT_07ea9834/38/3c ← puVar8[3/7/0xb]
//               DAT_07ea983e = 0
//               FUN_005142d0(0x79)  — ShowErrorDialog(0x79)
//               Después (si DAT_07e91388 >= 1) corta, si no: cae al 0x37
//               NOTA: el bloque XOR de envío de acá (líneas 655-840) es el camino de
//               respuesta al NACK de re-login del server — misma clave de 32 bytes, mismo loop de reintento.
//
//   case 0x37:  FUN_004332e0(puVar8)
//
//   case 0x38:  FUN_004cce00(byte[3], &DAT_07ea5298, 8)
//               FUN_00404bc0(0x1d, 0, 0)   — secondary inventory (bag/warehouse?)
//
//   case 0x39:  FUN_004cc660(&DAT_07ea5298, 8, 4, byte[3], puVar8+2, 1)
//               FUN_00404bc0(0x1d, 0, 0)
//
//   case 0x3a:  DAT_07eaa0f4 = -(byte[3]!=0) & DAT_05826c9c   — toggle effect bit
//
//   case 0x3b:  DAT_07eaa0f0 = *(puVar8+2)   — 4-byte misc update
//
//   case 0x3c:  PIN/SecondPassword UI control:
//               0 → DAT_07eaa0fc = 0
//               1 → DAT_07eaa0fc = 1; UI_SetScene(0x19)
//               2 → DAT_07eaa0fd = 0; UI_SetScene(0x19)
//               * → UI_SetScene(0x19)
//
//   case 0x3d:  FUN_004337f0(puVar8)
//
//   case 0x40:  DAT_07eaa0e4 = byte[3]*0x100 + byte[2]
//               FUN_005142d0(0x78)   — ShowErrorDialog(0x78)
//
//   case 0x41:  Shop slot display — inner switch byte[3] (0-5):
//               pcVar21/pcVar26 → Widget_Draw(pcVar21, pcVar26, 2)
//               FUN_005142d0(0)
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
//               FUN_005142d0(0x77)   — ShowErrorDialog(0x77)
//
//   case 0x51:  FUN_00434780(puVar8)
//   case 0x52:  FUN_004348b0(puVar8)
//   case 0x53:  FUN_00434950(puVar8)
//
//   case 0x54:  Second password / PIN full reset:
//               DAT_07eaa114-117 = 0
//               FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa118); DAT_07eaa118=0
//               FUN_00404040; DAT_07eaa119=0; DAT_00559f5f=0; DAT_07eaa14c=0
//               FUN_0043d8a0(&DAT_055c9bc8, &DAT_07eaa11b); DAT_07eaa11b=0
//               FUN_00404040; DAT_07eaa124=1; DAT_07eaa144=0
//
//   case 0x55:  Character list change (delete/create result):
//               DAT_07eaa124=1; DAT_07eaa144=1; DAT_07e11d70=1
//               DAT_00559c84=0; FUN_0047ec60(0)
//               _DAT_00559c94=8; DAT_00559c88=0
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
//               entity = FUN_0045ac80(byte[3]*256 + byte[2])
//               iVar20 = FUN_00434dc0(0xffffffff, puVar8+5, puVar8+0xd)
//               entity[0x1da] = (short)iVar20
//               FUN_00423ce0(entity)
//
//   case 0x5d:  entity = FUN_0045ac80(byte[3]*256 + byte[2])
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
//   case 0x71:  FUN_00433900()
//   case 0x73:  FUN_00433a80(puVar8, iVar20)
//   case 0x81:  FUN_00434170(puVar8)
//   case 0x82:  FUN_00434400()
//   case 0x83:  FUN_00434450(puVar8)   — second password response (0x83 server ack)
//   case 0x86:  FUN_004366c0(puVar8)
//   case 0x87:  FUN_004367d0()
//   case 0x90:  FUN_00436820(puVar8)
//   case 0x91:  FUN_00436cb0(puVar8)
//   case 0x92:  FUN_0047ec00(byte[3] + 1)
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
//     0x01 → DAT_05826cb0=0x14, DAT_05826cf8=2, FUN_00412a70(), state=3
//                                        (login OK, request char list)
//     0x02 → DAT_05826cb0=0x16, state=3  (wrong password)
//     0x03 → DAT_05826cb0=0x17, state=3  (account banned)
//     0x04 → DAT_05826cb0=0x18, state=3
//     0x05 → DAT_05826cb0=0x19, state=3
//     0x06 → DAT_05826cb0=0x1a, state=3; FUN_00405540("Version_dismatch")
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
//     byte[2]==1 → XOR decrypt 30 bytes with DAT_00559678[i%3]
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
//          XOR decode puVar8+2 (3-byte key DAT_00559678[i%3])
//          FUN_00423040 → decode g_CharData; g_CharData[0x1c] -= decoded_exp
//          FUN_0043d1d0 → re-encode g_CharData
//   F3/08: FUN_00431dc0(puVar8)
//   F3/10: FUN_00426cf0(puVar8, iVar20)
//   F3/11: FUN_004269f0(puVar8)
//   F3/13: entity=FUN_0045ac80(byte[2]*256+byte[5]); FUN_0045c8c0(entity, puVar8+7)
//   F3/14: DAT_07e91388=0; FUN_004cc660(&DAT_07ea8410,8,8,byte[2],puVar8+5,0)
//          FUN_00404bc0(0x31, 0, 0)
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
//          Parse IP from puVar8+2; Net_Disconnect(DAT_055ca160)
//          FUN_00423920(ip, port) — connect to new server
//          if result != 0: DAT_05826cf0 = 1
//          crt_sprintf + Widget_Draw — show "connecting" UI
//   F4/05: DAT_05826cb0=1; DAT_083a7c14=1   — back to Connecting state
//
// ── DEFAULT ───────────────────────────────────────────────────────────────────
//
//   Unrecognized opcode → FUN_004cd3b0() (log/discard)
//
// ── C2 / ENCRYPTED PACKET PATH ───────────────────────────────────────────────
//
//   if (byte[0] == 0xC2):  puVar9 = byte[1]*256+byte[2]; opcode = byte[3]
//   if (byte[0] == 0xC3):  FUN_0053cca0 → decrypt 1-byte-len; goto LAB_00439505
//   if (byte[0] == 0xC4):  FUN_0053cca0 → decrypt 2-byte-len; goto LAB_00439505
//   Los dos caminos desencriptados vuelven al mismo LAB_00439505 → switch de opcodes
//
//   Packet sequence tracking (anti-replay / dedup):
//     DAT_05826cec = rolling sequence counter
//     HashTable_GetIndex / operator_new(2) / HashTable_Insert / HashTable_Remove
//     → trackea los IDs de secuencia de paquetes en vuelo, manda NACK si no coinciden
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_0043E010  → Net_GetFreeBuffer(pool)
//   FUN_0045ac80  → Entity_GetIndex(entityId)  — returns 0-based entity slot
//   FUN_0045ac20  → Entity_Spawn(entityId)     — create or update entity slot
//   FUN_00481ba0  → Entity_UpdateNameData(name, data, entity, 0, -1)
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
//   FUN_0042bca0  → PacketHandler_0x19(puVar8, puVar9, iVar20)
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
//   FUN_004cc660  → ItemTable_UpdateSlot(table, stride, size, slot, data, flag)
//   FUN_00423040  → CharData_Decode(ctx, g_CharData)  — XOR-decode g_CharData
//   FUN_0047e3c0  → CharData_RecalcStats(charData)
//   FUN_0043d1d0  → CharData_Encode(ctx, g_CharData)  — XOR-encode g_CharData
//   FUN_004332e0  → PacketHandler_0x37(puVar8)
//   FUN_004337f0  → PacketHandler_0x3d(puVar8)
//   FUN_00434660  → PacketHandler_0x42(puVar8)
//   FUN_00434780  → PacketHandler_0x51(puVar8)
//   FUN_004348b0  → PacketHandler_0x52(puVar8)
//   FUN_00434950  → PacketHandler_0x53(puVar8)
//   FUN_0043d8a0  → HashTable_RefDecrement(ctx, key)
//   FUN_00435280  → PacketHandler_0x56(puVar8)
//   FUN_00434dc0  → Trade_GetItemData(entityId, data, extraData)
//   FUN_00435110  → PacketHandler_0x5b(puVar8)
//   FUN_00423ce0  → Entity_UpdateMisc(entity)
//   FUN_004353e0  → PacketHandler_0x60(puVar8)
//   FUN_00435390  → PacketHandler_0x61(puVar8)
//   FUN_004354f0  → PacketHandler_0x62(puVar8)
//   FUN_00435aa0  → PacketHandler_0x63(puVar8)
//   FUN_00433900  → PacketHandler_0x71()
//   FUN_00433a80  → PacketHandler_0x73(puVar8, iVar20)
//   FUN_00434170  → PacketHandler_0x81(puVar8)
//   FUN_00434400  → PacketHandler_0x82()
//   FUN_00434450  → SecondPassword_HandleResponse(puVar8)
//   FUN_004366c0  → PacketHandler_0x86(puVar8)
//   FUN_004367d0  → PacketHandler_0x87()
//   FUN_00436820  → PacketHandler_0x90(puVar8)
//   FUN_00436cb0  → PacketHandler_0x91(puVar8)
//   FUN_0047ec00  → CharSelect_SetSlotCount(count)
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
//   FUN_0045c8c0  → Entity_SetExtraData(entity, data)
//   FUN_00436fb0  → PacketHandler_F3_30()
//   FUN_00436550  → PacketHandler_F3_40(puVar8)
//   FUN_00423e10  → Net_RecvRedirect(puVar8)        — F4/02
//   FUN_00423920  → Net_Connect(ip, port)
//   FUN_0053cca0  → Packet_Decrypt(ctx, outBuf, data, len)  — C3/C4 cipher
//   FUN_0053cc30  → Packet_Encode / CRC_Compute
//   FUN_00412a70  → FUN_00412a70()  — se llama al login OK (F1/01/01)
//   FUN_00405540  → Log_SetString(buf, str)
//   FUN_00404bc0  → UI_SetScene(id, 0, 0)
//   FUN_00480620  → Widget_Draw(element, textureData, flag)
//   FUN_005142d0  → ShowErrorDialog(id)
//   FUN_004cd3b0  → Packet_Unknown_Log()
//   FUN_00422df0  → HashTable_GetOrInsert
//   FUN_00404040  → HashTable_Decrement
//   FUN_00403f80  → HashTable_Insert
//   FUN_00404330  → HashTable_Remove
//   FUN_00404280  → HashTable_Get
//   FUN_00423710  → HashTable_Free(entry, key)
//   HashTable_GetIndex → FUN_004cd3b0 area (addr in binary)
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
//     Si versión OK (2.2): ctx+8=0; ctx+4=wVersion; FUN_00403a30(); return 1
//
//   0x0043dbf0  Net_CreateSocket(void* this, HWND hWnd)   __thiscall
//     socket(AF_INET=2, SOCK_STREAM=1, IPPROTO_TCP=0) → this+8
//     DAT_05826cf0 = 0  (connected flag)
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
//     FUN_005129f0(dx) = abs o sqrt
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
//     FUN_004f9db0(quat, mat4x3) — quaternion → matriz rotación
//     FUN_004fa0b0(vec, mat4x3, &local) — matriz × vector
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
//     Wrapper: llama FUN_0043f500(DAT_05826df4, id, param2, p3, p4, 1, 2, p6)
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
//     Si DAT_05826e0c == 0: init (DAT_05826e04=timeGetTime(), DAT_05826e0c=1)
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
#include "Net/Net.h"
#include "Net/HWID.h"
#include "Net/MuEmu.h"

// C1:36 PMSG_TRADE_REQUEST_SEND carries the requester name but no viewport
// clave. MuEmu asocia el peer del lado del server; preservamos el nombre para el
// faithful response layout.
char s_tradeRequestName[11] = {};

// 0x00474310. Queda acá porque el paquete 0x1C es el fin autoritativo de un
// Teleport local, después de que el paquete de skill 0x19 arrancó su animación.
extern "C" void __cdecl CreateTeleportEnd(unsigned int entity);

extern "C" void __cdecl CreatePoint(float Position[3], int Value,
                                    float Color[3], float scale);

// 2026-05-04: Hero equipment stash (definidos en Render_PlayerEquipment.cpp).
// F3/03 los popula; HeroEquipWatchdog los re-aplica per-frame.
extern "C" {
    extern short g_HeroEquipStash_LH, g_HeroEquipStash_RH;
    extern short g_HeroEquipStash_Wing, g_HeroEquipStash_Pendant;
    extern short g_HeroEquipStash_Body[6];
    extern unsigned char g_HeroEquipStash_LHLvl, g_HeroEquipStash_RHLvl;
    extern unsigned char g_HeroEquipStash_WingLvl, g_HeroEquipStash_PendantLvl;
    extern unsigned char g_HeroEquipStash_BodyLvl[6];
    extern unsigned char g_HeroEquipStash_BodyOpt1[6];
    extern unsigned char g_HeroEquipStash_BodyOpt2[6];
    extern unsigned char g_HeroEquipStash_BodyOpt3[6];
    extern unsigned char g_HeroEquipStash_LHOpt, g_HeroEquipStash_RHOpt;
    extern unsigned char g_HeroEquipStash_WingOpt, g_HeroEquipStash_PendantOpt;
    extern unsigned char g_HeroEquipStash_LHOpt2, g_HeroEquipStash_RHOpt2;
    extern unsigned char g_HeroEquipStash_WingOpt2, g_HeroEquipStash_PendantOpt2;
    extern unsigned char g_HeroEquipStash_LHOpt3, g_HeroEquipStash_RHOpt3;
    extern unsigned char g_HeroEquipStash_WingOpt3, g_HeroEquipStash_PendantOpt3;
    extern int g_HeroEquipStash_Valid;
}

// ============================================================================
// Net_ProcessPacket @ 0x004389A0 — server→client opcode dispatcher
// ============================================================================
//
// Estructura portada desde IDA (ProtocolCore, 1824 líneas, 489 basic blocks):
//   while ((Msg = CWsctlc::GetReadMsg(&SocketClient)) != NULL) {
//       bEncrypted = 0;
//       if (Msg[0] == 0xC1)  { HeadCode = Msg[2]; Size = Msg[1]; }
//       else if (Msg[0]==0xC2) { HeadCode = Msg[3]; Size = Msg[1]*256+Msg[2]; }
//       else if (Msg[0]==0xC3 o 0xC4) { /* desencripta in situ */ ... }
//       dispatch on HeadCode...
//   }
//
// Esta primera iteración implementa el scaffolding completo + los opcodes del
// flujo de login (F1/00, F1/01, F4/02, F4/03, F4/05). Los handlers opcode
// complejos (combate, movimiento, char list, etc.) son stubs que registran
// llegada pero no avanzan estado — se añaden cuando el server los dispara.
//
// Paquetes cifrados (C3/C4) todavía no se desencriptan aquí: en el flujo
// inicial ConnectServer → cliente solo envía/recibe C1/C2.
// ============================================================================

extern int __fastcall FUN_0043e010(int poolBase);   // GetReadMsg (stubs.cpp) — returns ptr as int

// Forward decls for inventory packet handlers (Item/Item_Inventory.cpp).
extern "C" void __cdecl Recv_Inventory     (const BYTE* Msg);   // F3/10
extern "C" void __cdecl Recv_InventoryOpen (const BYTE* Msg);   // 0x55
extern "C" void __cdecl Recv_InventoryClose(const BYTE* Msg);   // 0x54
extern "C" void HeroEquipWatchdog(int c);
extern "C" void __cdecl SeedQuickPotionTypesFromInventory(void);
extern "C" void SetGuildNoticeText(const char* text);

// 2026-05-08: inventory/warehouse pool symbols (defined in HUD_Pass3.cpp).
extern "C" BYTE OffsetInventoryItems[];
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda (120 slots)
extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;
int __cdecl Entity_FindById(int entity_id);   // stubs.cpp
extern "C" void __cdecl UI_Main(int slot_idx, short* inv_base,
                                 unsigned int gridW);  // Item_ClickHandler.cpp
extern "C" int  pPickedItem;
extern "C" int  Level;
extern "C" BYTE byte_7E9136B;
extern void __cdecl FUN_004cc660(BYTE* Inv, int W, int H, int Index,
                                 BYTE* Item, int First);
extern "C" int __cdecl ConvertItemType(BYTE* Item);

// ── ShopInsertItem (PORT FIEL de IDA sub_4CC0E0, 2026-07-25) ─────────────────
// Inserta un item de tienda en el pool Inventory[32 + slot], llenando su
// footprint Width×Height (de ItemAttribute[type]).  El item de tienda son 4
// bytes: [typeLo][levelByte][durability][flags].  Key se setea solo en la celda
// primaria (el render usa Key>0 como gate).  El ItemConvert completo (que llena
// DamageMin/Defense/etc para el tooltip) se omite: el tooltip los recalcula
// on-hover desde ItemAttribute[type].  Pool shop = Inventory[32..151] (grid 8×15).
static void ShopInsertItem(int slot, const BYTE* Item)
{
    int type = ConvertItemType((BYTE*)Item);
    if (type == 255 || type < 0 || type >= 512) return;
    // 2026-07-27 FIX (tienda abre vacía — causa raíz): DAT_07d78068
    // (ItemAttribute base) se corrompe a ~1 (confirmado por el diag:
    // "SHOPINS slot=0 type=5 attrBase=00000001"). El guard de abajo abortaba
    // TODOS los inserts → el pool quedaba limpio → tienda vacía. Restauramos
    // desde el backup (mismo watchdog que Item_GetAttribute / DropItemEx) en
    // vez de descartar la lista.
    {
        unsigned int p = (unsigned int)(uintptr_t)DAT_07d78068;
        if ((p < 0x100000u || p >= 0x80000000u)
            && g_ItemAttribute_Backup >= 0x100000u
            && g_ItemAttribute_Backup < 0x80000000u)
        {
            DAT_07d78068 = (int)g_ItemAttribute_Backup;
        }
    }
    BYTE* attrBase = (BYTE*)(uintptr_t)DAT_07d78068;
    if ((uintptr_t)attrBase < 0x100000u || (uintptr_t)attrBase >= 0x80000000u) return;
    BYTE* attr = attrBase + type * 0x40;
    int W = attr[32]; int H = attr[33];   // ITEM_ATTRIBUTE.Width / .Height
    if (W < 1) W = 1;
    if (H < 1) H = 1;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int idx = slot + r * 8 + c;
            if (idx < 0 || idx >= 120) continue;    // bound del pool 8×15
            // CRÍTICO: el pool de tienda es un overlay que arranca en
            // &Inventory[idx].WalkSpeed (offset +24), NO en el base del ITEM.
            // El render (sub_4E38B0, HUD_Pass3:382) recibe &Inventory[32].WalkSpeed
            // y lee Type@+0, Level@+4, Durability@+26, Option1@+27, Key@+0x38
            // relativo a ese puntero. sub_4CC0E0 escribe con el mismo convenio.
            BYTE* cell = ShopItems + idx * 0x44;
            *(short*)(cell + 0)    = (short)type;                  // Type
            *(int*)(cell + 4)      = (int)Item[1];                 // Level (raw byte)
            cell[26]               = Item[2];                      // Durability
            cell[27]               = Item[3];                      // Option1
            *(DWORD*)(cell + 0x38) = (r == 0 && c == 0) ? 1u : 0u; // Key (gate render)
            // CRÍTICO (2026-07-27): x/y = posición-origen del item en el grid
            // (slot%8, slot/8), escrito en TODAS las celdas del footprint (igual
            // que sub_4CC0E0 ->x=a1%8 ->y=a1/8). El hover (Item_ClickHandler:634)
            // normaliza celdas de footprint al origen vía `inv_base + 34*(8*y+x)`.
            // Sin esto x/y=0 → TODA celda normaliza a slot 0 → el tooltip siempre
            // mostraba el primer item sin importar cuál hovereabas.
            cell[62]               = (BYTE)(slot % 8);             // x
            cell[63]               = (BYTE)(slot / 8);             // y
        }
    }
}

static BYTE* ItemMove_GetPool(DWORD pool)
{
    return (BYTE*)(uintptr_t)pool;
}

static int ItemMove_GetGridH(BYTE* pool)
{
    if (pool == OffsetWarehouseItems) return 15;
    if (pool == OffsetTradeItems || pool == OffsetMixItems) return 4;
    return 8;
}

static int ItemMove_ToGridSlot(BYTE* pool, int slot)
{
    return slot;
}

static BYTE* ItemMove_GetEquipSlotPtr(int slot)
{
    static const int kEquipOffsets[12] = {
        536, 604, 672, 740, 808, 876, 944, 1012, 1080, 1148, 1216, 1284
    };

    if (slot < 0 || slot >= 12 || DAT_07cf1ffc == 0) {
        return nullptr;
    }
    return (BYTE*)(uintptr_t)DAT_07cf1ffc + kEquipOffsets[slot];
}

static bool ItemMove_IsInventoryEquipSlot(BYTE* pool, int slot)
{
    return pool == OffsetInventoryItems && slot >= 0 && slot < 12;
}

static void ItemMove_RestoreSlot(BYTE* pool, int slot, const BYTE* item68)
{
    if (!item68) return;

    if (ItemMove_IsInventoryEquipSlot(pool, slot)) {
        BYTE* dst = ItemMove_GetEquipSlotPtr(slot);
        if (dst) {
            memcpy(dst, item68, sizeof(ITEM));
        }
        return;
    }

    int slotIndex = (pool == OffsetInventoryItems) ? slot : ItemMove_ToGridSlot(pool, slot);
    int slotMax = (pool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(pool));
    if (slotIndex >= 0 && slotIndex < slotMax) {
        int first = (pool == OffsetInventoryItems) ? 0 : 1;
        // 2026-07-27 FIX "el item se transforma en otro al moverlo":
        // item68 es el ITEM struct de 68 bytes copiado del slot al hacer pickup,
        // NO formato wire. FUN_004cc660/InsertInventoryItem espera 4-5 bytes wire
        // [typeLo][optByte][dur][hi][ext]; pasarle el struct crudo reinterpretaba
        // Type-high/Level-int/etc como opciones → el item restaurado quedaba con
        // type/opciones equivocadas. Se disparaba en CADA move denegado (server
        // devuelve result=FF cuando el inventario está lleno / slot destino
        // ocupado) → el item de origen se corrompía. Reconstruimos el wire desde
        // los offsets conocidos de la struct (Type@0, Level@4, Durability@26,
        // Unkown@60=byteHi, byColorState@61=ext).
        BYTE wire[6] = { 0, 0, 0, 0, 0, 0 };
        wire[0] = item68[0];    // Type low byte
        wire[1] = item68[4];    // raw optByte (Level int, low byte)
        wire[2] = item68[26];   // Durability
        wire[3] = item68[60];   // Unkown (= byteHi, incluye bit8 de type + exc)
        wire[4] = item68[61];   // byColorState (extByte)
        FUN_004cc660(pool, 8, ItemMove_GetGridH(pool), slotIndex, wire, first);
    }
}

static void ItemMove_ApplyServerSlot(BYTE* pool, int slot, const BYTE* itemWire12, const BYTE* item68)
{
    if (!itemWire12 || !item68) return;

    if (ItemMove_IsInventoryEquipSlot(pool, slot)) {
        BYTE* dst = ItemMove_GetEquipSlotPtr(slot);
        if (dst) {
            memcpy(dst, item68, sizeof(ITEM));
            memcpy(dst, itemWire12, 12);
        }
        return;
    }

    int slotIndex = (pool == OffsetInventoryItems) ? slot : ItemMove_ToGridSlot(pool, slot);
    int slotMax = (pool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(pool));
    if (slotIndex >= 0 && slotIndex < slotMax) {
        BYTE item12[12];
        memcpy(item12, item68, 12);
        memcpy(item12, itemWire12, 12);
        int first = (pool == OffsetInventoryItems) ? 0 : 1;
        FUN_004cc660(pool, 8, ItemMove_GetGridH(pool), slotIndex, item12, first);
    }
}

static BYTE* ItemMove_GetActiveSecondaryPool()
{
    if (DAT_07eaa11b) return OffsetTradeItems;
    if (DAT_07eaa119) return OffsetWarehouseItems;
    if (DAT_07eaa11a) return OffsetMixItems;
    return OffsetTradeItems;
}

static void ItemMove_ClearPoolPreview(BYTE* pool, int slots)
{
    if (!pool) return;
    for (int i = 0; i < slots; ++i) {
        pool[i * 0x44 + 0x40] = 0;
    }
}

static void ItemMove_UpdateInventoryDurability(int slot, BYTE durability)
{
    if (slot < 12 || slot >= 76) {
        return;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    ITEM* cell = &inv[slot - 12];
    if (cell->Type == -1) {
        return;
    }

    int originX = cell->x;
    int originY = cell->y;
    if (originX < 0 || originX >= 8 || originY < 0 || originY >= 8) {
        return;
    }

    ITEM* origin = &inv[originY * 8 + originX];
    if (origin->Type == -1) {
        return;
    }

    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    int width = 1;
    int height = 1;
    if (attr && origin->Type >= 0 && origin->Type <= 0xFFF) {
        width = attr[origin->Type].Width;
        height = attr[origin->Type].Height;
        if (width <= 0 || width > 8) width = 1;
        if (height <= 0 || height > 8) height = 1;
    }

    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int x = originX + dx;
            int y = originY + dy;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) continue;
            ITEM* it = &inv[y * 8 + x];
            if (it->Type == -1) continue;
            it->Durability = durability;
        }
    }
}

static bool ItemMove_LooksLikeStackMerge(BYTE* targetPool, int targetSlot, const BYTE* sourceItem68)
{
    if (!targetPool || !sourceItem68) {
        return false;
    }

    if (targetPool != OffsetInventoryItems || targetSlot < 12 || targetSlot >= 76) {
        return false;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    ITEM* target = &inv[targetSlot - 12];
    short sourceType = *(short*)sourceItem68;
    if (sourceType < 0 || target->Type < 0) {
        return false;
    }

    if (target->Type != sourceType) {
        return false;
    }

    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!attr || sourceType > 0xFFF) {
        return false;
    }

    if (attr[sourceType].Width != 1 || attr[sourceType].Height != 1) {
        return false;
    }

    BYTE sourceOpt = sourceItem68[4];
    BYTE targetOpt = (BYTE)target->Level;
    if (sourceOpt != targetOpt) {
        return false;
    }

    if (target->Durability <= 0 || target->Durability >= 255) {
        return false;
    }

    return true;
}

static void ItemMove_ClearPickedState()
{
    DAT_07e91388 = 0;
    DAT_07eaa165 = 0;
    EnableUse = 0;
    DAT_07ea5b18 = 0xFFFFFFFFu;
    DAT_07e11e78 = 0xFFFFFFFFu;
    DAT_07ea9844 = 0;
    DAT_07ea9800 = 0;
    DAT_07eaa160 = 0;
    DAT_083a4124 = 0;
    DAT_083a42eb = 0;
    g_ItemMoveSourcePool = 0;
    g_ItemMoveTargetPool = 0;

    memset(DAT_07e91350, 0, sizeof(DAT_07e91350));
    *(short*)DAT_07e91350 = (short)0xFFFF;
    pPickedItem = -1;
    Level = 0;
    byte_7E9136B = 0;

    auto ClearItemRefGrid = [](BYTE* grid, size_t size) {
        for (size_t off = 0; off < size; off += 0x44) {
            memset(grid + off, 0, 0x44);
            *(short*)(grid + off) = (short)0xFFFF;
        }
    };

    ClearItemRefGrid(DAT_07ea8448, sizeof(DAT_07ea8448));
    ClearItemRefGrid(DAT_07ea5b68, sizeof(DAT_07ea5b68));
    ClearItemRefGrid(DAT_07ea9880, sizeof(DAT_07ea9880));
    ClearItemRefGrid(DAT_07ea7bc0, sizeof(DAT_07ea7bc0));
    ClearItemRefGrid(DAT_07e11fb0, sizeof(DAT_07e11fb0));

    ItemMove_ClearPoolPreview(OffsetInventoryItems, 64);
    ItemMove_ClearPoolPreview(OffsetTradeItems, 32);
    ItemMove_ClearPoolPreview(OffsetMixItems, 32);
    ItemMove_ClearPoolPreview(OffsetWarehouseItems, 120);
}

// Globals de guild (definidos en Render/HUD_Pass6.cpp con linkage extern "C").
extern "C" int  g_nGuildMemberCount;
extern "C" int  GuildTotalScore;
// src/UI/ChatListBox.cpp — espejo de los dispatches de ReceiveGuildList.
extern "C" char byte_7E91790[];   // tabla de miembros, stride 13
extern "C" void GuildList_Clear(void);
extern "C" void GuildList_AddMember(const char* name, char connected, char partyNumber);
#define byte_7E919BC  DAT_07e919bc   // tabla global stride 80 (ver globals.h)
extern "C" void GuildCreator_OpenFromServer(void);
extern "C" void GuildCreator_CloseFromResult(void);

// 00434DC0 owns a 1000 × 80 guild-mark table (07E919BC..07EA51EC), separate
// del buffer chico de lista de miembros que usa el panel G. Mantenerlos separados
// es esencial: C2:52 refresca miembros mientras C2:5A/5C refrescan las marcas del viewport.
static const int kGuildMarkRecordCount = 1000;
static int  s_GuildRecordKey[kGuildMarkRecordCount] = { 0 };
static BYTE s_GuildMarkRecord[kGuildMarkRecordCount][80] = {};

static int Guild_UpsertRecord(int key, const BYTE* name8, const BYTE* mark32)
{
    int row = -1;
    for (int i = 0; i < kGuildMarkRecordCount; ++i) {
        if (s_GuildMarkRecord[i][0] != 0 &&
            memcmp(s_GuildMarkRecord[i], name8, 8) == 0) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        for (int i = 0; i < kGuildMarkRecordCount; ++i) {
            if (s_GuildMarkRecord[i][0] == 0) { row = i; break; }
        }
    }
    if (row < 0) row = 0; // same bounded-table fallback as 00434DC0.

    BYTE* dst = s_GuildMarkRecord[row];
    s_GuildRecordKey[row] = key;
    memset(dst, 0, 80);
    memcpy(dst, name8, 8);
    if (mark32) {
        for (int i = 0; i < 64; ++i)
            dst[9 + i] = (i & 1) ? (mark32[i / 2] & 0x0F)
                                  : (mark32[i / 2] >> 4);
    }
    return row;
}

static int Guild_FindRecordByKey(int key)
{
    for (int i = 0; i < kGuildMarkRecordCount; ++i)
        if (s_GuildMarkRecord[i][0] != 0 && s_GuildRecordKey[i] == key)
            return i;
    return -1;
}

extern "C" const char* Guild_GetMarkName(int row)
{
    return (row >= 0 && row < kGuildMarkRecordCount)
        ? (const char*)s_GuildMarkRecord[row] : "";
}

// sub_434DC0 guarda las celdas 8x8 decodificadas de la marca de guild en record+9. Dejamos
// el renderer detrás de este accesor para que su índice de marca siga siendo el mismo índice
// de registro que escriben los paquetes de viewport 5B/5C en Character+474.
extern "C" const BYTE* Guild_GetMarkPixels(int row)
{
    return (row >= 0 && row < kGuildMarkRecordCount)
        ? &s_GuildMarkRecord[row][9] : nullptr;
}

// Declaración adelantada (la declaración real de DbgLogPublic está más abajo, cerca de la
// línea 676 — la dejamos acá para que los handlers de F3/E0/E1 compilen antes de su bloque extern "C").
extern "C" void DbgLogPublic(const char* msg);

// ── F3/E0 PMSG_NEW_CHARACTER_INFO_RECV ───────────────────────────────────────
// Port FIEL del DLL injection (Mu-linux-97K/Source/Client/Main/Protocol.cpp:849
// CProtocol::GCNewCharacterInfoRecv). El binario 0.97k vanilla NO maneja este
// packet — fue agregado en versiones posteriores. Sin esto el HUD queda vacío
// (Level/HP/MP/EXP no se populan).
//
// Layout (76 bytes): header(4) + 18 DWORDs (Level..ViewGrandReset).
// Escritos a CharacterAttribute (offsets per DLL): 0x0E Level, 0x14..0x1A
// stats, 0x54 LevelUpPoint. CurHP/MaxHP/CurMP/MaxMP los popula F3/E1.
//
// GET_MAX_WORD_VALUE clamps DWORD → WORD (max 0xFFFF).
static inline WORD ClampToWord(DWORD v) { return v >= 0xFFFF ? (WORD)0xFFFF : (WORD)v; }

static void Recv_NewCharacterInfo(const BYTE* Msg)
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

// ── F3/E1 PMSG_NEW_CHARACTER_CALC_RECV ───────────────────────────────────────
// Port FIEL del DLL injection (Protocol.cpp:898 GCNewCharacterCalcRecv).
// Recibe stats calculados (HP/MP actuales tras buffs/items, defense, attack).
// Layout: header(4) + ~17 DWORDs (ViewCurHP..MagicDamageRate).
static void Recv_NewCharacterCalc(const BYTE* Msg)
{
    BYTE* CA = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!CA) return;

    const BYTE* p = Msg + 4;
    DWORD ViewCurHP            = *(const DWORD*)(p + 0);
    DWORD ViewMaxHP            = *(const DWORD*)(p + 4);
    DWORD ViewCurMP            = *(const DWORD*)(p + 8);
    DWORD ViewMaxMP            = *(const DWORD*)(p + 12);
    DWORD ViewCurBP            = *(const DWORD*)(p + 16);
    DWORD ViewMaxBP            = *(const DWORD*)(p + 20);
    DWORD ViewPhysiSpeed       = *(const DWORD*)(p + 24);
    DWORD ViewMagicSpeed       = *(const DWORD*)(p + 28);
    // bytes 32-39: MagicDamageMin/Max (skip — set later)
    DWORD ViewAttackSuccessRate= *(const DWORD*)(p + 40);
    DWORD ViewDefense          = *(const DWORD*)(p + 48);
    DWORD ViewDefenseSuccess   = *(const DWORD*)(p + 52);

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
}

// Debug log (defined in WinMain.cpp).
extern "C" void DbgLogPublic(const char* msg);

static void NetLog(const char* fmt, ...)
{
    if (!fmt) return;     // null guard — el invalid_parameter handler de
                          // ucrtbased recursaba sobre format=NULL.
    char buf[256];
    va_list ap; va_start(ap, fmt);
    int n = _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    if (n < 0) buf[0] = 0;
    DbgLogPublic(buf);
}

// Insert/ClearBuffPhysicalEffect @ 0043BDE0/0043C070.  The 0x07 effect-state
// paquete es el camino autoritativo de los buffs persistentes; en particular Mana Shield
// (0x100) lo renderizan cinco instancias del joint 266 atadas al owner, no sólo
// setting the entity status bitmap.
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType);
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType);
extern "C" void __cdecl DeleteCharacter(int Key);  // 0045AC20

static void ApplyPersistentSkillEffect97k(BYTE* entity, WORD effect, BYTE state)
{
    if (!entity || effect == 0) return;
    DWORD& physicalEffects = *(DWORD*)(entity + 120);

    if (state == 1) {
        if ((effect & 0x10) != 0) {
            DeleteEffect(1150, (DWORD)(uintptr_t)entity, 1);
            FUN_00460dc0(1150, (float*)(entity + 16), (float*)(entity + 28),
                         (float*)(entity + 232), (float*)1, (float*)entity,
                         (float*)-1, nullptr, 0);
        }
        if ((effect & 0x20) != 0 && (physicalEffects & 0x20) == 0) {
            DeleteEffect(190, (DWORD)(uintptr_t)entity, 1);
            float angle[3] = { *(float*)(entity + 28), *(float*)(entity + 32),
                               *(float*)(entity + 36) };
            FUN_00460dc0(190, (float*)(entity + 16), angle, (float*)(entity + 232),
                         (float*)1, (float*)entity, (float*)-1, nullptr, 0);
            angle[2] += 180.0f;
            FUN_00460dc0(190, (float*)(entity + 16), angle, (float*)(entity + 232),
                         (float*)2, (float*)entity, (float*)-1, nullptr, 0);
        }
        if ((effect & 0x40) != 0 && (physicalEffects & 0x40) == 0) {
            DeleteEffect(1274, (DWORD)(uintptr_t)entity, 0);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            FUN_00460dc0(1274, (float*)(entity + 16), (float*)(entity + 28),
                         light, nullptr, (float*)entity, (float*)-1, nullptr, 0);
            PlayBuffer(104, (DWORD)(uintptr_t)entity, 0);
        }
        if ((effect & 0x80) != 0 && (physicalEffects & 0x80) == 0) {
            DeleteEffect(1274, (DWORD)(uintptr_t)entity, 3);
            float light[3] = { 1.0f, 1.0f, 1.0f };
            FUN_00460dc0(1274, (float*)(entity + 16), (float*)(entity + 28),
                         light, (float*)3, (float*)entity, (float*)-1, nullptr, 0);
        }
        if ((effect & 0x100) != 0 && (physicalEffects & 0x100) == 0 &&
            *(WORD*)(entity + 2) != 325) {
            PlayBuffer(103, 0, 0);
            DeleteJoint(266, (DWORD)(uintptr_t)entity, 0);
            for (int i = 0; i < 5; ++i) {
                FUN_0046d840(266, (float*)(entity + 16), (float*)(entity + 16),
                              (float*)(entity + 28), 0,
                              (int)(uintptr_t)entity, 50.0f, -1, 0);
            }
        }
        physicalEffects |= effect;
        return;
    }

    switch (effect) {
    case 0x08:  DeleteJoint(266, (DWORD)(uintptr_t)entity, 4); break;
    case 0x10:  DeleteEffect(1150, (DWORD)(uintptr_t)entity, 1); break;
    case 0x40:  DeleteEffect(1274, (DWORD)(uintptr_t)entity, 0); break;
    case 0x80:  DeleteEffect(1274, (DWORD)(uintptr_t)entity, 3); break;
    case 0x100: DeleteJoint(266, (DWORD)(uintptr_t)entity, 0); break;
    default: break;
    }
    physicalEffects &= ~((DWORD)effect);
}

// CreateMagicShiny @ 004741E0. ReceiveMagicPosition lo llama en la
// primera mano del caster antes de la animación del hechizo. El pase de render de entidades
// keeps the model and bone matrices at +276, so no synthetic screen-space
// acá no hace falta posicionarlo.
static void CreateMagicShiny97k(BYTE* entity, int hand)
{
    if (!entity || hand < 0 || hand > 1 || DAT_05828d58 == 0) return;
    const DWORD bones = *(DWORD*)(entity + 276);
    if (bones == 0) return;

    const WORD type = *(WORD*)(entity + 2);
    const BYTE bone = entity[628 + hand * 24];
    float offset[3] = { 0.0f, 0.0f, 0.0f };
    float position[3];
    float light[3] = { 1.0f, 0.5f, 0.2f };
    void* const model = (void*)(uintptr_t)(DAT_05828d58 + type * 0xBC);

    FUN_004409a0(model, (float*)(uintptr_t)(bones + bone * 0x30),
                 offset, position, 1);
    // CreateSprite(1231, Position, 1.0, Light, Hand, 0.0, Character).
    // El quinto argumento es el owner y el último es el subtipo; esto
    // preserva el orden exacto de parámetros de 004741E0_CreateMagicShiny.c.
    FUN_004795c0(1231, position, 1.0f, light, hand, 0.0f, (int)(uintptr_t)entity);
    FUN_004795c0(1231, position, 1.0f, light, hand + 2, 0.0f, (int)(uintptr_t)entity);
}

// ── Globals del state machine de login (mapping IDA → nuestro codebase) ────
// IDA                            | nuestro
// -------------------------------|----------------------------
// CurrentProtocolState           | DAT_05826cb0 (server response code)
// dword_83A7C14                  | DAT_083a7c14 (login sub-state)
// HeroKey                        | g_HeroKey (nuevo)

static unsigned short g_HeroKey = 0;

// ---------------------------------------------------------------------------
// F1/00 — ReceiveJoinServer  (@ 0x00424010)
// Server saluda tras conectar. Sub-byte Msg[4]: 1=OK, otro=fail.
// ---------------------------------------------------------------------------
static void Recv_JoinServer(const BYTE* Msg)
{
    if (Msg[4] == 1) {
        g_HeroKey      = (unsigned short)(Msg[6] | (Msg[5] << 8));
        // FIX 2026-07-24: DAT_05826cac (HeroKey que usa ClearCharacters vía
        // OpenWorld) NUNCA se seteaba → quedaba en 0.  Con eso, al entrar al
        // mundo ClearCharacters(0) conservaba las entidades con Key==0 (incluida
        // la del Hero stale del slot 0 que quedaba de antes del join) → fantasma
        // renderizado + hover pegado.  Ahora lleva el HeroKey real.
        DAT_05826cac   = g_HeroKey;
        DAT_05826cb0   = 2;          // CurrentProtocolState = 2
        DAT_083a7c14   = 2;          // login sub-state = CredentialInput
        PlayBuffer(27, 0, 0);
        NetLog("NET:    JoinServer OK: HeroKey=%d state→2/2", g_HeroKey);
        // ── 2026-04-25: solo F1/05 HWID re-activado ────────────────────────
        // Tras agregar el LoginKey chain XOR al F1/01 build, el server pasó
        // de mudo a responder con code=05 (HardwareID rechazado / blacklist).
        // El server MuEmu requiere F1/05 SetHwid antes del F1/01 — sin él,
        // CheckHardwareID en Blacklist.cpp considera el HWID vacío como
        // blacklisted y devuelve code 05.
        //
        // Compatibilidad MuEmu: el deploy actual corta la sesión temprano si
        // tras F1/00 no recibe también F1/04 antes de F1/05/F1/01. Esto no
        // existe en el flujo Webzen original; queda aislado acá.
        //   Lang_Send(1);
        HWID_Send();
    } else {
        NetLog("NET:    JoinServer FAIL code=%d → SetErrorMessage(113)", Msg[4]);
        SetErrorMessage(113);        // "Connecting error"
    }
    // Version sanity: bytes 7..11 deben ser Version[i]-i-1. No bloqueante.
}

// ---------------------------------------------------------------------------
// F1/01 — Login result  (inline en ProtocolCore, cases 0..0x11/0xC0..0xD2)
// Sub-byte Msg[4] → mapea a CurrentProtocolState = 20..38.
// ---------------------------------------------------------------------------
static void Recv_LoginResult(const BYTE* Msg)
{
    DWORD state;
    switch (Msg[4]) {
        case 0x00: state = 21; break;
        case 0x01: state = 20; break;   // LogIn success → LogIn=2, CheckHack
        case 0x02: state = 22; break;
        case 0x03: state = 23; break;
        case 0x04: state = 24; break;
        case 0x05: state = 25; break;
        case 0x06: state = 26; break;   // version mismatch
        case 0x08: state = 28; break;
        case 0x09: state = 37; break;
        case 0x0A: state = 29; break;
        case 0x0B: state = 30; break;
        case 0x0C: state = 31; break;
        case 0x0D: state = 32; break;
        case 0x11: state = 38; break;
        case 0xC0: case 0xD0: state = 34; break;
        case 0xC1: case 0xD1: state = 35; break;
        case 0xC2: case 0xD2: state = 36; break;
        default:   state = 27; break;
    }
    DAT_05826cb0 = state;
    DAT_083a7c14 = 3;
    // 2026-07-25 (#1): log del resultado de login crudo por intento, para
    // diagnosticar el "primer enter = dato mal, segundo enter entra".  Si el
    // primer intento trae un code de fallo (0x02 pass, 0x0C/0x0D, etc.) y el
    // segundo (mismas credenciales) trae 0x01 OK, el problema es el PRIMER
    // paquete (serial/encriptación) o un estado stale; si ambos códigos son
    // iguales, es del server.  Msg[4]=code server, state=nuestro DAT_05826cb0.
    NetLog("NET:  → F1/01 LOGIN-RESULT code=0x%02X -> state=%u (t=%lu)",
           Msg[4], state, (unsigned long)GetTickCount());

    // 2026-07-25 (#1): auto-reintento del PRIMER login fallido con code 0x02.
    // El diagnóstico probó que el 1er F1/01 se rechaza (0x02 = pass incorrecta)
    // con datos IDÉNTICOS (userLen/passLen/crc iguales) al 2do intento que SÍ
    // entra (0x01) — quirk del primer paquete C3 contra el server MuEmu, no es
    // la contraseña.  Reintentamos UNA sola vez simulando Enter (DAT_055ca038),
    // que hace que Game_SceneUpdate re-envíe las MISMAS credenciales.  Si la
    // pass fuera realmente incorrecta, el 2do intento también da 0x02 y ahí sí
    // se muestra el error.  Sólo 0x02 — no reintentamos banned/already-online/
    // version-mismatch para no enmascararlos.
    {
        static int s_loginAutoRetried = 0;
        if (Msg[4] == 0x02 && !s_loginAutoRetried) {
            s_loginAutoRetried = 1;
            DAT_055ca038 = 1;   // simula Enter → re-envía el login (mismas creds)
            NetLog("NET:    F1/01 AUTO-RETRY (first 0x02 = likely first-packet quirk)");
        } else {
            s_loginAutoRetried = 0;   // reset en éxito o en 2do fallo
        }
    }
}

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
// 2026-05-05: cache F3/00 packet para replay desde JoinChar (MuEmu no
// re-envía char-list cuando recibe F1/02/01 — cierra socket directamente).
extern "C" {
    BYTE g_CharListCache[256] = {0};
    int  g_CharListCacheLen   = 0;
}

static void Recv_CharList(const BYTE* Msg)
{
    const int CHAR_STRIDE  = 0x394;
    const int CHAR_SLOT_AT = 0x2D2;       // entity+0x2D2 = "selected" flag
    const int MAX_PREVIEW  = 5;            // slots renderizados en char-select

    // 2026-05-05: cache para replay en JoinChar
    {
        int len = (int)Msg[1];
        if (len > 0 && len <= 255) {
            memcpy(g_CharListCache, Msg, len);
            g_CharListCacheLen = len;
        }
    }

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

        unsigned char* c = FUN_0045f930((int)slot, klass, 0, x, y, rotate);
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
        FUN_0045c8c0((int)slot, (BYTE*)&rec[16]);

        // CreateHero llamó SetPlayerStop con las alas todavía en -1, así que
        // dejó action=1 (idle pegado al piso). Re-llamamos ahora que las alas
        // están equipadas para que action pase a 9 (float-idle) y los chars
        // con alas aparezcan flotando como en el cliente original.
        FUN_004430c0((int)(uintptr_t)c);
    }

    // CurrentProtocolState = 51 → Scene_CharSelect lo usa como gate de render
    DAT_05826cb0 = 51;
}

// 2026-05-05: wrapper público para que UI_InGameMenu lo llame al volver
// desde JoinChar (replay de la char-list desde el cache).
extern "C" void Recv_CharListReplay(const BYTE* Msg)
{
    Recv_CharList(Msg);
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
static void Recv_CreateChar(const BYTE* Msg)
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
        unsigned char* c = FUN_0045f930((int)slot, klass, skin, x, y, 0.0f);
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
static void Recv_DeleteChar(const BYTE* Msg)
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
static void Recv_JoinMapServer(const BYTE* Msg, int bEncrypted)
{
    // BUG-FIX 2026-04-28: el F3/03 que envía el server MuEmu (Protocol.cpp
    // GDCharacterInfoSend → DataServer → DGCharacterInfoRecv → cliente) llega
    // como packet plano C1 (no C3-encriptado vía SimpleModulus). Nuestro
    // dispatcher en Net_ProcessPacket inicializa bEncrypted=false y nunca lo
    // setea. Antes hacía early-return si !bEncrypted asumiendo que era el
    // path "GameGuard re-auth"; pero en MuEmu el path bEncrypted=false ES el
    // path real → skip → hero nunca se posiciona → pantalla negra.
    //
    // El IDA original 0.97K diferenciaba ambos paths para soportar
    // re-handshakes de GameGuard. Nuestro server MuEmu no usa GG, así que
    // ejecutamos siempre el world-load path.
    (void)bEncrypted;

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

        // 2026-07-27 FIX zen al login: PMSG_CHARACTER_INFO_SEND (F3/03) trae
        // Money (DWORD) en offset 40 — la struct NO es pack(1): tras MaxBP
        // (WORD@36) hay 2 bytes de padding porque Money (DWORD) se alinea a 4 →
        // offset 40. Verificado por hex del paquete: RB[40..43]=D4 17 F1 1B =
        // 0x1BF117D4 (leyendo en 38 daba 0x17D40000 = valor corrido). El zen se
        // muestra desde CharacterMachine+1352 (= DAT_07cf1ffc+1352).
        if (DAT_07cf1ffc != 0) {
            DWORD money = *(const DWORD*)(RB + 40);
            *(DWORD*)((BYTE*)(uintptr_t)DAT_07cf1ffc + 1352) = money;
            NetLog("NET:    F3/03 Money=%u -> CharacterMachine+1352", money);
        }

        // 2026-07-27 FIX (alas/cuerpo rojos "PK"): PMSG_CHARACTER_INFO_SEND trae
        // PKLevel (BYTE) justo después de Money → offset 44. El render aplica el
        // tinte rojo (1.0,0.1,0.1) cuando entity+0x2EA >= 6
        // (Entity_UpdateRender:333). Entity_Spawn inicializa ese campo en 3 para
        // los mobs, pero el HÉROE no pasa por ese path → quedaba con basura
        // (el diag mostró 2ea=255 → rojo permanente). Ahora guardamos el PKLevel
        // real que manda el server.
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

    // BUG-FIX 2026-04-28: leer class + body-part slots del char-select entity
    // ANTES de OpenWorld (que llama ClearCharacters y borra los entities).
    // Body parts (helm/armor/pant/glove/boot) son lo que efectivamente renderiza
    // el cuerpo del hero — sin esto FUN_00456770 entra pero no dibuja nada.
    // (3) World setup: mapa, terreno, tiles.
    DAT_0055a7ac = world;

    // 2026-05-07: WIPE entity pool de slots stale del CharSelect ANTES de
    // OpenWorld + hero spawn. Sin esto, los slots de chars del CharSelect
    // quedan activos con sus nombres en +0x1C1 y aparecen como "NPCs" cuando
    // el hover detect los recoge.
    //
    // Wipea TODOS los slots — el nuevo hero se crea via CreateCharacterPointer
    // a unas líneas más abajo (paso 4-5) en un slot random, sobrescribiendo
    // sea cual sea. Los mobs/players del world via 0x12/0x13 viewport packets
    // llegan DESPUÉS del OpenWorld y populan el pool limpio.
    //
    // Nota: el wipe es FULL (slot[0]=0 + 0x84=0 + 0x160=0 + 0x1C1=0). Si
    // omitía slots por preservar el viejo hero, ese slot quedaba con nombre
    // y kind viejos → seguía apareciendo como entidad fantasma in-world.
    // User reportó "leo nombres del select character" + "entra con walking
    // animation sin mobs" 2026-05-07.
    if (DAT_07abf5d0) {
        // Full memset del pool para garantizar todos los bytes limpios.
        // 400 slots × 0x394 = ~366 KB. CreateCharacterPointer + viewport spawn
        // packets repopulan los campos relevantes después.
        memset((void*)(uintptr_t)DAT_07abf5d0, 0, 400 * 0x394);
        // Reset hero pointer too — el siguiente CreateCharacterPointer setea uno nuevo.
        DAT_07abf5d8 = nullptr;
    }

    // BUG-FIX 2026-04-29: FUN_0050e5a0 (OpenWorld) bloquea ~2 segundos cargando
    // BMDs. Durante ese tiempo el server manda ~3KB de packets post-JoinMapServer,
    // pero como nuestro message pump está bloqueado no hacemos recv → server's
    // IoSideBuffer overflows o WSASend falla con WSAENOBUFS → CloseClient.
    // Pumpear la message queue ANTES de empezar el BMD load (drena lo que
    // ya llegó), y al final del load (drena lo nuevo) ayuda a que el server
    // no cierre por backpressure.
    {
        MSG msg;
        // Se mantiene estructuralmente sólo para diagnóstico. 00425840 no despacha
        // los paquetes de viewport encolados antes de crear al héroe, más abajo.
        for (int i = 0; false && i < 32 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    FUN_0050e5a0();              // OpenWorld(World) — BMD load (~2s)

    // Pump messages POST-load para drenar lo que llegó durante el bloqueo.
    {
        MSG msg;
        for (int i = 0; false && i < 64 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); ++i) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // (4-5) Random hero slot + spawn at packet position.
    DAT_05826ca0 = (DWORD)(_rand() % 400);    // HeroIndex
    unsigned int heroIndex = DAT_05826ca0;
    unsigned char* heroPtr = (unsigned char*)(uintptr_t)DAT_07abf5d0 + (heroIndex * 0x394);
    float Rotation = ((float)direction - 1.0f) * 45.0f;
    FUN_0045adc0(heroPtr, 390, PosX, PosY, Rotation);

    // BUG-FIX 2026-04-28: FUN_0045adc0 setea cached_wp (+0x388/0x38c) pero NO
    // target_grid (+0x306/0x307). El per-frame walker FUN_0043ea20 en
    // Player_InputTick lee target_grid → como inicialmente está en 0,0, el
    // hero camina automáticamente a la esquina del mapa.
    // Forzar target == position para que el walker quede idle hasta el primer click.
    heroPtr[0x306] = PosX;
    heroPtr[0x307] = PosY;
    heroPtr[0x305] = 0;             // move-pending OFF
    heroPtr[0x354] = 0;             // path_current_wp = 0
    heroPtr[0x355] = 0;             // path_substep = 0
    heroPtr[0x356] = 0;             // path_wp_count = 0
    // 2026-05-07: post-wipe init de anim_state — sin esto, el wipe deja 0x105=0
    // y anim 0 puede no ser "idle" para algunos models. anim_state=1 es el
    // idle1 standard.
    heroPtr[0x105] = 1;             // anim_state = idle
    heroPtr[0x106] = 1;             // anim_state_prev
    *(float*)(heroPtr + 0x108) = 0.0f;  // anim_frame

    // (6) Hero binding + class/skill bonus bytes from packet.
    *(unsigned short*)(heroPtr + 0x1DC) = g_HeroKey;
    BYTE* charAttr = (BYTE*)CharacterAttribute;
    heroPtr[444] = charAttr[11];

    // 2026-06-16: el ReceiveJoinMapServer de IDA NO espeja el cuerpo/equipo desde
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
    // Effect_Spawn signature: see src/Render/Particle_Spawn.cpp / FUN_00475220.
    // Wire-up TODO: nuestro Effect_Spawn aún no está parameterizado igual; la
    // entrada al mundo funciona sin el efecto visual.

    // (10) Avanzar state machine.
    DAT_05826cb0 = 61;           // CurrentProtocolState → enter-world fade
    // LockInputStatus = 0; CheckIME_Status(1, 0) — TODO

    // BUG-FIX 2026-04-29: enviar F3/12 CharacterMoveViewportEnable + 0E LiveClient
    // inmediatamente. El server MuEmu (Protocol.cpp:1439 CGCharacterMoveViewportEnableRecv)
    // pone RegenOk=2 al recibir esto. Sin un ACK del cliente post-JoinMapServer
    // el server cree que el cliente está congelado y cierra el socket en
    // ~50-100ms (visto en logs). El IDA original manda F3/12 al final de su
    // ReceiveJoinMapServer; nuestro port no lo hacía.
    // 2026-04-29: post-F3/03 sends removidos. El keepalive 1Hz en Game_MainLoop
    // manda 0x0E. F3/12 fue la causa probable del kick previo (server-side
    // procesa MainCheck antes de F3/12, y nuestro F3/12 desincronizaba algo).
    //
    // 2026-05-04: send INMEDIATO de keepalive 0x0E al recibir F3/03 — el
    // server espera saber que el cliente sigue vivo apenas recibe el world
    // entry. El keepalive 1Hz en Game_MainLoop puede tardar hasta 1s en
    // arrancar (state==5 transición + frame interval), tiempo que el server
    // a veces no perdona.
    // 2026-05-05: el binario original 0.97k (FUN_00425840 Recv_JoinMapServer)
    // en el path bEncrypted=TRUE (= C3 packet, como manda MuEmu) NO envía
    // NINGÚN packet post-F3/03. Solo procesa los datos del char y carga el
    // mundo. El send de F3/12 ViewportEnable solo ocurre en el path
    // bEncrypted=FALSE (raro, no aplica con MuEmu C3).
    //
    // Sends previos (0x0E keepalive + F3/12 ViewportEnable) eran ADD-ON
    // nuestros que NO existen en el original. El keepalive 1Hz en
    // Game_MainLoop ya cubre el liveness check post-F3/03.
    //
    // RegenOk (=2 normalmente seteado por F3/12) — en MuEmu el server-tick
    // lo procesa y eventualmente lo lleva a 0 (OBJECT_PLAYING) sin necesidad
    // del F3/12 desde cliente.

    // (11) Stop dungeon BGM 110 si no estamos en mapa-evento (11..16).
    if (world < 11 || world > 16) {
        // FUN_00404c60(110, 1) — StopBuffer; existe en Sound.cpp
        // FUN_00404c60(110);  // (firma simplificada en nuestro port)
    }

    NetLog("NET:    F3/03 JoinMapServer world=%d pos=(%d,%d) dir=%d rot=%.1f heroIdx=%d",
           world, PosX, PosY, direction, Rotation, (int)heroIndex);
}

// ---------------------------------------------------------------------------
// F1/02 — ReceiveLogOut  (@ 0x004247D0)
// Respuesta del server al packet C1/05/F1/02/<sub> que envía UI_InGameMenu
// (botones Salir / Ir-a-otro-server / Ir-a-otro-char).  Sub-byte Msg[4]:
//   0 = Exit:   WM_DESTROY (cierra el proceso).
//   1 = JoinChar (volver a char-select sin reconectar): mantiene socket,
//       g_GameState=4, descarga in-game scene, pide F3/00 char list.
//   2 = JoinSrv (volver a server-select): cierra socket, g_GameState=2,
//       reset login init flags, vuelve a Scene_Login_ServerSelect.
//
// IDA hace además un ack-resend del packet cuando bEncrypted=false (echo),
// pero para nuestro flujo MuEmu-encrypted basta con la transición de estado.
// ---------------------------------------------------------------------------
static void Recv_LogOut(const BYTE* Msg)
{
    BYTE sub = Msg[4];
    NetLog("NET:    F1/02 LogOut sub=%d gs=%d", sub, (int)DAT_005615c0);

    if (sub == 0) {
        // Exit: WM_DESTROY → WndProc cleanup → PostQuitMessage
        SendMessageA(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    if (sub == 1) {
        // JoinChar — back to char-select, keep connection open
        if (DAT_005615c0 == 5) {
            StopMusic();
            AllStopSound();
            FUN_004cd3b0();              // CharPreview_Refresh
            ReleaseMainData();
        }
        DAT_005615c0 = 4;                // g_GameState = CharSelect
        DAT_083a7c14 = 0;                // sub-state reset (will be set to 0x14/0x15 by EnterWorldTick init)
        DAT_083a7c18 = 0;
        DAT_05826cb0 = 50;               // CurrentProtocolState
        // 2026-05-05: RESET scene-init guards. EnterWorldTick (state=4) and
        // CharSelectTick (state=5) tienen una guarda DAT_083a7c4b/4c que
        // sólo permite re-inicializar la escena la PRIMERA vez. Sin reset,
        // post-JoinChar el substate DAT_083a7c14 quedaba en 0x1c (post-OK-
        // click) heredado del primer login → la siguiente tick disparaba
        // F3/03 select-char inmediato → server respondía con JoinMapServer
        // → cliente "recargaba el mapa" en vez de mostrar char-select.
        DAT_083a7c4b = 0;                // EnterWorld scene init
        DAT_083a7c4c = 0;                // CharSelect per-tick init
        DAT_083a7c4d = 0;                // Main scene warning
        DAT_083a4299 = 0;                // double-click flag
        DAT_083a4124 = 0;                // single-click flag
        DAT_005616ac = -1;               // selected slot
        DAT_005616b0 = -1;               // creation slot
        // Manda el pedido de lista de personajes F3/00 vía Net_SendSmallPacket para que
        // salga como C3 (encriptado) con el contador de serial correcto + chain-XOR. Los
        // envíos C1 planos, sin serial, el server los descartaba en silencio en
        // CheckSerial (SocketManager.cpp:328-330) — lee DecBuff[1] como
        // DecSerial y rechaza el paquete si se rompe la monotonía.
        BYTE pkt[4] = { 0xC1, 0x04, 0xF3, 0x00 };
        NetLog("NET:    F3/00 char-list request (post-JoinChar)");
        Net_SendSmallPacket(pkt, 4);

        // ── BUG-FIX 2026-08-17: faltaba la cola de ReceiveLogOut ──────────────
        // IDA 0x4247D0 LABEL_117: DESPUÉS del send, la rama sub==1 hace
        // `CurrentProtocolState = 0` e `InitGame()`, igual que la rama sub==2.
        // Sin el InitGame quedaba `World` (DAT_0055a7ac) con el mapa anterior.
        // Eso importa porque nuestro RequestTerrainHeight (Terrain_Utils.cpp:49)
        // gatea con `World < 0` en vez del `g_GameState != 5` del original — una
        // desviación deliberada por el orden del JoinMapServer. Con World=7
        // (Atlans) heredado y su heightmap todavía cargado, CreateCharacterPointer
        // le daba a cada personaje del char-select la altura del terreno de
        // Atlans en vez de 0 → aparecían flotando más arriba. `World = -1` de
        // InitGame es justamente lo que hace que el guard relajado se comporte
        // como el original acá.
        DAT_05826cb0 = 0;                // CurrentProtocolState
        InitGame();
        return;
    }

    if (sub == 2) {
        // JoinSrv — back to login/server-select, close socket
        if (DAT_005615c0 == 5) {
            StopMusic();
            AllStopSound();
            FUN_004cd3b0();
            ReleaseMainData();
        }
        FUN_0043dc90((int)(uintptr_t)DAT_055ca160);  // Net_Disconnect (close socket)
        FUN_005102c0();                  // ReleaseCharacterSceneData
        DAT_005615c0 = 2;                // g_GameState = Login
        DAT_083a7c14 = 0;                // sub-state = ServerSelect
        DAT_083a7c18 = 0;
        DAT_05826cb0 = 0;                // CurrentProtocolState
        DAT_083a7c48 = 0;                // ConnectionCheckEnable
        DAT_083a7c49 = 0;                // InitLogIn
        DAT_083a7c4b = 0;                // InitCharacterScene → reload assets
        DAT_083a7c4c = 0;                // InitMainScene
        DAT_083a7c4d = 0;                // EnableMainRender / warning flag
        InitGame();
        return;
    }
}

// ---------------------------------------------------------------------------
// Envío PLANO (sin encriptar, sin serial byte) al socket actual. El
// ConnectServer NO usa la encriptación MuEmu del GameServer, así que sus
// requests (C1 04 F4 02 / C1 06 F4 03) van crudos. Mismo patrón de
// WSAEWOULDBLOCK-queue que usa CServerSelWin en el binario original.
// ---------------------------------------------------------------------------
void CS_SendPlain(const BYTE* data, int len)
{
    if (DAT_055ca168 == 0xffffffff) return;
    unsigned int remain = (unsigned int)len;
    int off = 0;
    while ((int)remain > 0) {
        int r = ::send(DAT_055ca168, (const char*)data + off, (int)remain, 0);
        if (r == -1) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                if (DAT_055cc16c + (int)remain < 0x2001) {
                    memcpy((char*)DAT_055ca16c + DAT_055cc16c, data + off, remain);
                    DAT_055cc16c += (int)remain;
                } else {
                    Net_Disconnect((int)(uintptr_t)DAT_055ca160);
                }
            } else {
                Net_Disconnect((int)(uintptr_t)DAT_055ca160);
            }
            return;
        }
        if (r == 0) return;
        remain -= (unsigned int)r;
        off += r;
    }
}

// ---------------------------------------------------------------------------
// F4/04 — CustomServerList (ConnectServer CCCustomServerListSend)
// Pobla la tabla local de nombres DAT_07d52c34 (stride 300, indexada por
// ServerCode/20) que ReceiveServerList (F4/02) lee para el nombre de cada
// grupo. Formato (C2 header):
//   [C2][sizeHB][sizeLB][F4][04][countHB][countLB] + count × {WORD ServerCode; char Name[32]}
//   → entry stride 34 bytes.
// ---------------------------------------------------------------------------
static void Recv_CustomServerList(const BYTE* Msg)
{
    int count = (Msg[5] << 8) | Msg[6];   // SET_NUMBERHB/LB → HB en [5], LB en [6]
    const BYTE* p = Msg + 7;
    for (int i = 0; i < count && i < 24; i++) {
        unsigned short serverCode = (unsigned short)(p[0] | (p[1] << 8));
        const char* name = (const char*)(p + 2);
        int group = serverCode / 20;
        if (group >= 0 && group < 24)
            lstrcpynA(DAT_07d52c34 + 300 * group, name, 32);  // deja hueco null-term
        p += 34;
    }
    NetLog("NET:  → F4/04 CustomServerList count=%d (names populated)", count);
}

// ---------------------------------------------------------------------------
// F4/02 — ReceiveServerList  (@ 0x00423E10)  — port FIEL desde IDA.
// Lista de game servers publicada por el ConnectServer. Cada entrada trae
// {WORD ServerCode; BYTE UserTotal; pad} (stride 4). UserTotal (0..100) es el
// load que Scene_Login_ServerSelect Pass 4 dibuja como cuadros llenos.
// El nombre de cada grupo se copia desde DAT_07d52c34 (poblada por F4/04).
// Layout del display array (DAT_083a45d8, stride 542 por slot):
//   +0x14 num_channels · +0x2c channel_id (stride 26) · +0x2e load (stride 26)
// Slot destino = 23 - (ServerCode/20)  (o 24 para el grupo evento 12).
// ---------------------------------------------------------------------------
static void Recv_ServerList(const BYTE* Msg)
{
    unsigned char count = Msg[5];
    DAT_083a7c40 = count;

    // Clear num_channels (+0x14) de todos los slots (25). Repuebla desde el
    // paquete; esto también elimina la entrada estática "MuServer" del slot 0.
    for (int s = 0; s < 25; s++)
        *(unsigned char*)(DAT_083a45d8 + 542 * s + 0x14) = 0;

    if (count == 0)
        return;

    const BYTE* entry = Msg + 6;
    for (int i = 0; i < count; i++, entry += 4) {
        unsigned short serverCode = (unsigned short)(entry[0] | (entry[1] << 8));
        unsigned char  load       = entry[2];
        int group = serverCode / 20;
        int slot;
        if (group == 12) {
            slot = 24;
            // grupo evento: nombre desde GlobalText[559] (si existe)
            if (GlobalText[559]) lstrcpynA(DAT_083a78a8, (const char*)GlobalText[559], 20);
        } else {
            slot = 23 - group;
            if (slot < 0 || slot > 24) continue;
            // nombre desde la tabla local poblada por F4/04
            lstrcpynA(DAT_083a45d8 + 542 * slot, DAT_07d52c34 + 300 * group, 20);
        }
        char* slotBase = DAT_083a45d8 + 542 * slot;
        unsigned char chan = *(unsigned char*)(slotBase + 0x14);   // num_channels actual
        *(unsigned short*)(slotBase + 0x2c + 26 * chan) = serverCode;  // channel_id
        *(unsigned char*) (slotBase + 0x2e + 26 * chan) = load;        // load (UserTotal)
        *(unsigned char*) (slotBase + 0x14) = chan + 1;               // ++num_channels
    }
    NetLog("NET:  → F4/02 ServerList parsed count=%d (loads applied)", count);
}

// ---------------------------------------------------------------------------
// F4/03 — Server redirect: cerrar socket, reconnect a (IpAddr, port)
// ---------------------------------------------------------------------------
static void Recv_Redirect(const BYTE* Msg)
{
    char IpAddr[16] = {0};
    // Msg+4..Msg+19 = IP string (16 bytes), Msg+20..21 = port
    for (int i = 0; i < 15 && Msg[4 + i]; ++i) IpAddr[i] = (char)Msg[4 + i];
    unsigned short port = (unsigned short)(Msg[20] | (Msg[21] << 8));

    NetLog("NET:    Redirect → Disconnect + Connect(%s:%d)", IpAddr, port);
    // Salimos del modo ConnectServer: el socket nuevo habla con el GameServer,
    // que responderá con JoinServer (F1/00) y arranca el login normal.
    // Reactivamos la capa MuEmu (byte-XOR) que el GameServer sí usa.
    g_ConnectServerMode      = 0;
    g_ConnectServerRequested = 0;
    MuEmu::SetActive(true);
    FUN_0043dc90((int)(uintptr_t)DAT_055ca160);   // Net_Disconnect
    FUN_00423920(IpAddr, port);                   // Net_Connect
    DAT_05826cf0 = 1;                              // g_bGameServerConnected
}

// ---------------------------------------------------------------------------
// F4/05 — Vuelta a estado Connecting
// ---------------------------------------------------------------------------
static void Recv_BackToConnecting(void)
{
    DAT_05826cb0 = 1;
    DAT_083a7c14 = 1;
}

// ============================================================================
// Dispatcher principal
// ============================================================================
void Net_ProcessPacket(void)
{
    while (true) {
        BYTE* Msg = (BYTE*)FUN_0043e010((int)(uintptr_t)DAT_055ca160);
        if (!Msg) return;

        int HeadCode, Size;
        BYTE hdr = Msg[0];
        bool bEncrypted = false;

        // ── C3/C4 in-place decrypt ────────────────────────────────────────
        // Paquetes encriptados server→cliente. Los bytes del cuerpo (después del
        // header de tamaño de cable) están codificados con CSimpleModulus. Los decodificamos in situ
        // en el mismo buffer con un header C1/C2 para que el resto del dispatcher
        // doesn't care about encryption.
        //
        //   C3 [encLen] [enc body...]   →   C1 [plainLen] [plain body...]
        //   C4 [hi][lo] [enc body...]   →   C2 [hi][lo]   [plain body...]
        //
        // FUN_0053cca0(dst, src, srcLen, ?) escribe 8 bytes por cada 11 bytes encriptados.
        BYTE  scratch[0x800];
        if (hdr == 0xC3 || hdr == 0xC4) {
            int hdrSz = (hdr == 0xC3) ? 2 : 3;
            int wireLen = (hdr == 0xC3) ? Msg[1] : ((Msg[1] << 8) | Msg[2]);
            int encLen  = wireLen - hdrSz;
            int outLen  = FUN_0053cca0((int)scratch, (int)(Msg + hdrSz), encLen, 0);
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
            plain[1] = (BYTE)(bodyLen + 2);
            memcpy(plain + 2, scratch + 1, bodyLen);
            memcpy(Msg, plain, bodyLen + 2);
            hdr      = 0xC1;
            HeadCode = Msg[2];
            Size     = Msg[1];
            NetLog("NET: C3->C1 decoded encLen=%d → plainLen=%d op=%02X",
                   encLen, bodyLen + 2, HeadCode);
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
        switch (HeadCode) {
            case 0xF1: {
                switch (sub) {
                    case 0x00:
                        NetLog("NET:  → F1/00 JoinServer result=%d hero=%d",
                               Msg[4], Msg[5] | (Msg[6] << 8));
                        Recv_JoinServer(Msg);
                        break;
                    case 0x01:
                        NetLog("NET:  → F1/01 LoginResult code=%02X", Msg[4]);
                        Recv_LoginResult(Msg);
                        break;
                    case 0x02:
                        NetLog("NET:  → F1/02 LogOut sub=%02X", Msg[4]);
                        Recv_LogOut(Msg);
                        break;
                    default:
                        NetLog("NET:  → F1/%02X unhandled", sub);
                        break;
                }
                break;
            }

            case 0xF3: {
                switch (sub) {
                    case 0x00:
                        NetLog("NET:  → F3/00 CharList count=%d", Msg[4]);
                        Recv_CharList(Msg);
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
                    case 0x06: {
                        // ── F3/06 PMSG_LEVEL_UP_POINT_SEND ───────────────────
                        // 2026-08-08 FIX ("al subir un punto los números se
                        // vuelven locos"): el port leía un layout INVENTADO
                        // (`WORD` en Msg+5 y Msg+7). El struct real del server
                        // (Protocol.h:467, GAMESERVER_EXTRA=1 en stdafx.h:8) es,
                        // con el padding de MSVC:
                        //    +0..3  PSBMSG_HEAD  (C1, size, F3, 06)
                        //    +4     BYTE result  (= 16 + type, 0 = rechazado)
                        //    +5     padding                     ← el port leía acá
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
                            NetLog("NET:  → F3/06 AddPoint OK (no-extra) slot=%d maxLifeMana=%u",
                                   slot, maxLifeMana);
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
                        // 2026-05-06: port FIEL desde server source
                        // Mu-linux-97K/Source/MuServer/GameServer/SkillManager.cpp:2256
                        // GCSkillListSend. Wire format:
                        //   [C1][size][F3][11][count] [slot][skill][level]×count
                        //
                        // Per PMSG_SKILL_LIST (server SkillManager.h:161):
                        //   slot:  position in skill array (0..MAX_SKILL_LIST-1)
                        //   skill: skill ID
                        //   level: (m_level << 3) | (m_index & 7)  ← packed
                        //
                        // ANTES: el handler decía "HotbarUpdate" (mal-port) y
                        // solo logueaba. User reportó "no veo skills en la UI"
                        // — los skills nunca llegaban a CharacterAttribute.
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
                        if ((DWORD)DAT_005616ac >= 4) DAT_005616ac = 0;
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
                        Recv_NewCharacterCalc(Msg);
                        break;
                    }
                    case 0xE2: case 0xE3: case 0xE4: case 0xE5: {
                        // F3/E3 (126B) quest, F3/E4 (854B) skills, F3/E5 (1111B) master tree.
                        // Pendientes — dump-only por ahora (estructuras Protocol.h aún no porteadas).
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
                        // Antes esto solo logueaba como "SkillList" y encima leía Msg+6
                        // (el payload arranca en Msg+4) → el bloque de opciones NUNCA se
                        // aplicaba: el chat window quedaba en el default hardcodeado del
                        // ctor en vez del guardado por personaje.
                        const BYTE* p = Msg + ((hdr == 0xC1) ? 4 : 5);
                        NetLog("NET:  → F3/30 Option size=%d", Size);
                        if (Size < 19) { NetLog("NET:    F3/30 too short — skip"); break; }

                        // 1) Mapa de skill-keys: 64 bytes por héroe en CharacterAttribute+215.
                        //    Para cada slot i del hotbar, busca su skill en la lista del
                        //    personaje (+87, 64 entradas) y marca keyMap[slot_skill] = i.
                        if (CharacterAttribute) {
                            int hero = (int)DAT_005616ac;
                            if (hero >= 0 && hero <= 4) {
                                BYTE* attr   = (BYTE*)CharacterAttribute;
                                BYTE* keyMap = attr + 215 + (hero << 6);
                                memset(keyMap, 0xFF, 0x40);
                                for (int i = 0; i < 10; ++i) {
                                    BYTE sk = p[i];
                                    if (sk == 255) continue;
                                    for (int j = 0; j < 64; ++j) {
                                        if (sk == attr[j + 87]) { keyMap[j] = (BYTE)i; break; }
                                    }
                                }
                            }
                        }

                        // 2) Opciones de juego.
                        DAT_07e11e18 = ((p[10] & 1) == 1);          // m_bAutoAttack
                        DAT_07e11e26 = (BYTE)((p[10] & 4) == 4);    // m_bWhisperSound
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
                            DAT_005590ac = 1;
                            if (DAT_055c9ff0) {          // vtable +56 = setVisibleCnt (slot 14)
                                DWORD* cobj = (DWORD*)DAT_055c9ff0;
                                void** cvt  = (void**)*cobj;
                                if (cvt) {
                                    typedef int (__fastcall *FnSetCnt)(DWORD*, int, int);
                                    ((FnSetCnt)cvt[14])(cobj, 0, chatLines);
                                }
                            }
                        } else {
                            DAT_005590ac = 0;            // sin recuadro (modo clásico)
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
                            if (DAT_005590ac) { cobj[11] = 186;          cobj[12] = 420; }
                            else              { cobj[11] = (DWORD)(-10); cobj[12] = 81;  }
                        }

                        NetLog("NET:    F3/30 opt=%02X Q=%d W=%d E=%d chatWin=%02X lines=%d transp=%d listbox=%d",
                               p[10], DAT_00559c60, DAT_00559c64, DAT_00559c68,
                               cw, chatLines, transp, (int)DAT_005590ac);
                        break;
                    }

                    default: {
                        // 2026-07-25 (#3): dump del payload de CUALQUIER F3 sub
                        // desconocido, para tener la estructura cuando toque
                        // portarlo.  F3/E6 (~326B, trae nombres tipo "Devil
                        // Square") = lista de eventos/GameServer info; F3/E7+ etc.
                        // El server MuEmu los manda en loop; hoy los ignoramos
                        // (inofensivo), pero acá queda el hex para identificarlos.
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
                break;
            }

            case 0xF4: {
                switch (sub) {
                    case 0x02:
                        NetLog("NET:  → F4/02 ServerList count=%d", Msg[5]);
                        Recv_ServerList(Msg);
                        break;
                    case 0x04:
                        NetLog("NET:  → F4/04 CustomServerList");
                        Recv_CustomServerList(Msg);
                        break;
                    case 0x03:
                        NetLog("NET:  → F4/03 Redirect ip=%.15s port=%d",
                               (const char*)(Msg + 4), Msg[20] | (Msg[21] << 8));
                        Recv_Redirect(Msg);
                        break;
                    case 0x05:
                        NetLog("NET:  → F4/05 BackToConnecting");
                        Recv_BackToConnecting();
                        break;
                    default:
                        NetLog("NET:  → F4/%02X unhandled", sub);
                        break;
                }
                break;
            }

            // ── IN-GAME OPCODES ─────────────────────────────────────────────
            // BUG-FIX 2026-04-28: el default case ignoraba TODOS los packets
            // in-game. Sin estos handlers, server manda 0x10 (move-confirm),
            // 0x11 (position-set), 0x14 (entity spawn) etc, cliente los descarta
            // → hero estático, NPCs invisibles, movimiento roto.

            case 0x0E:  // LiveClient ACK (server confirma keepalive)
                NetLog("NET:  → 0x0E LiveClient ACK");
                break;

            // ── CHAT ─────────────────────────────────────────────────────────
            // 2026-07-19: estos tres opcodes NO estaban en el dispatch → todo el
            // chat entrante se descartaba en silencio. Layout autoritativo del
            // server MuEmu (Protocol.h) + confirmado 1:1 con IDA ReceiveChat
            // (0x427630), que lee name en +3 y el mensaje en +13.
            //   C1:00  PMSG_CHAT_SEND         name[10]@+3  message[60]@+13
            //   C1:01  PMSG_CHAT_TARGET_SEND  index[2]@+3  message[60]@+5
            //   C1:02  PMSG_CHAT_WHISPER_SEND name[10]@+3  message[60]@+13
            // 2026-07-27 FIX (el guardia no responde): PMSG_CHAT_TARGET_SEND.
            // El server manda el mensaje de un NPC "hablado" por acá
            // (GCChatTargetSend — NpcTalk::NpcGuard lo usa para el guardia), pero
            // NO había handler in-game → se descartaba en silencio y clickear al
            // guardia sólo hacía el sonido de UI. Layout: index[2]@+3 (BE),
            // message[60]@+5. Se muestra como burbuja de chat sobre la entidad.
            case 0x01: {
                if (Size < 6) break;
                int entIdx = Entity_FindById((Msg[3] << 8) | Msg[4]);
                char cmsg[61] = {0};
                int clen = Size - 5;
                if (clen > 60) clen = 60;
                memcpy(cmsg, Msg + 5, clen);
                NetLog("NET:  → 0x01 ChatTarget ent=%d msg='%s'", entIdx, cmsg);
                if (entIdx >= 0 && entIdx < 400 && cmsg[0]) {
                    BYTE* ent = (BYTE*)(uintptr_t)DAT_07abf5d0 + (uintptr_t)entIdx * 0x394;
                    if (ent[0] != 0) {
                        // CreateChat(nombre, texto, entidad, 0, -1) — igual que el
                        // path de NPC hover (FUN_004cb6f0).
                        FUN_00481ba0((char*)(ent + 0x1C1), cmsg, (DWORD)(uintptr_t)ent, 0, -1);
                    }
                }
                break;
            }

            case 0x02: {
                // Whisper — mismo layout que chat normal; canal 3 (whisper) en el log.
                NetLog("NET:  → 0x02 ChatWhisper size=%d", Size);
                // FIX 2026-07-19: mismo bug que el case 0x00 — el guard exigía el
                // tamaño MÁXIMO (73) de una struct de longitud VARIABLE
                // (`14 + strlen`). Por esto los `/post` dorados nunca aparecían:
                // CommandManager::GCPostMessageGold los manda por este opcode y un
                // post corto llega con Size ~30.
                // PORT FIEL de IDA `ReceiveWhisper` @ 0x4278F0 (identificada por
                // disasm: la función sin nombre entre ReceiveChat y ReceiveNotice):
                //     a0 ac 1d e1 07  mov al, byte_7E11DAC   ; m_bBlockWhisper
                //     84 c0 / 0f 85   test+jnz → descarta el mensaje
                //     8d 4a 03        lea ecx, [edx+3]       ; name  = buf+3
                //     8d 72 0d        lea esi, [edx+0Dh]     ; msg   = buf+13
                //     6a 26 ...       PlayBuffer(0x26, 0, 0) ; sonido de whisper
                //     6a 00 ...       UIChatLogWindow_AddText(name, msg, 0)
                //
                // FIX 2026-07-19: el canal era **3** (invención mía al agregar el
                // handler, nunca validada). IDA usa **0** → negro sobre fondo
                // celeste (0x9632C8FF), el look clásico de whisper. Por eso los
                // `/post` dorados salían con estilo de chat normal.
                if (Size >= 14 && DAT_07e11dac == 0) {   // m_bBlockWhisper (toggle F3)
                    char wname[11] = {0};
                    char wmsg[61]  = {0};
                    memcpy(wname, Msg + 3, 10);
                    // Copiar solo los bytes que el paquete realmente trae.
                    int wlen = Size - 13;
                    if (wlen > 60) wlen = 60;
                    memcpy(wmsg, Msg + 13, wlen);
                    PlayBuffer(0x26, 0, 0);
                    UIChatLogWindow_AddText(wname, wmsg, 0);
                }
                break;
            }

            case 0x53: {
                // ReceiveGuildLeave @ 00434950.
                if (Size < 4) break;
                static const int textIndex[] = { 511, 512, 513, 514, 515 };
                const BYTE result = Msg[3];
                if (result < _countof(textIndex))
                    UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
                if (result == 1 || result == 4) {
                    if (result == 4 && Hero) {
                        const short row = *(short*)((BYTE*)Hero + 474);
                        if (row >= 0 && row < kGuildMarkRecordCount) {
                            s_GuildRecordKey[row] = -1;
                            s_GuildMarkRecord[row][0] = 0;
                        }
                        *(short*)((BYTE*)Hero + 474) = -1;
                    }
                    g_nGuildMemberCount = -1;
                    GuildOpened = 0;
                }
                NetLog("NET: GuildLeave result=%u", (unsigned)result);
                break;
            }

            case 0x54: {
                // 0x54 — server requests "close inventory" (port 0.52 sub_407BA0).
                NetLog("NET:  → 0x54 InventoryClose");
                Recv_InventoryClose(Msg);
                break;
            }

            case 0x55: {
                // MuEmu CGGuildMasterOpenRecv authorizes the native guild
                // creador con el frame de tres bytes C1:03:55.
                if (Size == 3) {
                    GuildCreator_OpenFromServer();
                    NetLog("NET: GuildCreatorOpen");
                    break;
                }
                // 0x55 — server requests "open inventory" (port 0.52 sub_407BD0).
                NetLog("NET:  → 0x55 InventoryOpen");
                Recv_InventoryOpen(Msg);
                break;
            }

            case 0x56: {
                // ReceiveCreateGuildResult @ 00435280.
                if (Size < 4) break;
                static const int textIndex[] = { 516, -1, 517, 518, 940, 941, 942 };
                const BYTE result = Msg[3];
                if (result == 1) {
                    GuildCreator_CloseFromResult();
                    PlayBuffer(28, 0, 0);
                    DAT_07e11d28 = 0;
                    DAT_00559bec = 6;
                } else if (result < _countof(textIndex) && textIndex[result] >= 0) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
                }
                NetLog("NET: GuildCreateResult result=%u", (unsigned)result);
                break;
            }

            case 0x10: {
                // PacketHandler_0x10: move-confirmed
                // [C1][len][0x10][id_hi][id_lo][gx][gy][speed_flags]
                if (Size < 8) break;
                WORD entityId = ((Msg[3] & 0x7F) << 8) | Msg[4];
                BYTE gx = Msg[5], gy = Msg[6];
                NetLog("NET:  → 0x10 Move id=%d to (%d,%d)", entityId, gx, gy);
                // BUG-FIX 2026-04-29: actualizar entidad sin importar si es hero u otra.
                BYTE* h = nullptr;
                if (entityId == g_HeroKey && DAT_07abf5d8) {
                    h = (BYTE*)DAT_07abf5d8;
                } else {
                    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                    for (int s = 0; s < 400; ++s) {
                        BYTE* slot = basePtr + s * 0x394;
                        if (slot[0] && *(WORD*)(slot + 0x1dc) == entityId) {
                            h = slot; break;
                        }
                    }
                }
                if (h && h[0x2fd] == 0) {
                    // ReceiveMoveCharacter @ 00427B90: speed is always updated.
                    h[0x2fc] = (BYTE)(Msg[7] >> 4);

                    if (h == (BYTE*)DAT_07abf5d8) {
                        // IDA no pisa acá la ruta local del héroe ni su posición
                        // en el mundo. Sólo refresca este cache de respaldo
                        // while no locally interpolated path is active.
                        if (h[748] == 0) {
                            *(int*)(h + 0x388) = gx;
                            *(int*)(h + 0x38c) = gy;
                        }
                    } else {
                        // ReceiveMoveCharacter 00427B90: remote entities store
                        // el destino del server en +774/+775, calcula la
                        // ruta desde las coordenadas de grilla cacheadas (+904/+908), y después
                        // deja que el tick normal de movimiento consuma esa ruta.
                        h[774] = gx;
                        h[775] = gy;
                        if (h[848] == 0) {
                            const int srcX = *(int*)(h + 0x388);
                            const int srcY = *(int*)(h + 0x38c);
                            if (FUN_0043f3e0(srcX, srcY, gx, gy, h + 852, 0.0f)) {
                                h[748] = 1;
                                // 00427B90 invokes SetPlayerWalk immediately
                                // para el tipo 322. Al resto de las entidades remotas las
                                // cambia MoveMonsterClient antes de MovePath.
                                if (*(WORD*)(h + 2) == 322) {
                                    FUN_00443930((int)(uintptr_t)h);
                                    h[773] = 1;
                                }
                            } else {
                                h[748] = 0;
                                SetPlayerStop(h);
                            }
                        }
                    }
                }
                break;
            }

            case 0x03: {
                // 2026-05-05 BUG-FIX CRÍTICO (causa raíz del FD_CLOSE post-F3/03):
                // Server log muestra:
                //   [HackPacketCheck][...][...] Packet encryption error
                //   (Index: 3, Value: -1, Encrypt: [0][1])
                //
                // Server HackPacketCheck.txt: opcode 3 (MainCheck) Encrypt=1.
                // Nuestro client mandaba el ACK como C1 plain con MuEmu byte-XOR
                // → server veía encrypt=0 → mismatch [0][1] → CloseClient.
                // Esto explica por qué SIEMPRE kick post-F3/03: el server manda
                // 0x03 MainCheck challenge en el batch post-CharacterInfo, y
                // nuestro ACK plain dispara hack-detection.
                //
                // Net_SendSmallPacket wrap correcto: chain-XOR + serial counter
                // + SimpleModulus + envelope C3.
                NetLog("NET:  → op=0x03 MainCheck challenge size=%d, sending ACK (C3)", Size);
                if (DAT_055ca168 != 0xffffffff && Size >= 3) {
                    BYTE ack[16];
                    int ackSize = Size > (int)sizeof(ack) ? (int)sizeof(ack) : Size;
                    ack[0] = 0xC1;
                    ack[1] = (BYTE)ackSize;
                    ack[2] = 0x03;
                    // Devuelve la clave como eco (el server no valida cuando MainChecksum=0)
                    if (Size > 3) {
                        memcpy(ack + 3, Msg + 3, ackSize - 3);
                    }
                    Net_SendSmallPacket(ack, ackSize);
                }
                break;
            }

            case 0x11: {
                // PacketHandler_0x11: position-set / teleport
                // [C1][len][0x11][id_hi][id_lo][gx][gy]
                if (Size < 7) break;
                WORD entityId = ((Msg[3] & 0x7F) << 8) | Msg[4];
                BYTE gx = Msg[5], gy = Msg[6];
                NetLog("NET:  → 0x11 Position id=%d to (%d,%d)", entityId, gx, gy);
                BYTE* h = nullptr;
                if (entityId == g_HeroKey && DAT_07abf5d8) {
                    h = (BYTE*)DAT_07abf5d8;
                } else {
                    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                    for (int s = 0; s < 400; ++s) {
                        BYTE* slot = basePtr + s * 0x394;
                        if (slot[0] && *(WORD*)(slot + 0x1dc) == entityId) {
                            h = slot; break;
                        }
                    }
                }
                if (h) {
                    // ReceiveMovePosition 00427F40 sólo refresca el estado de
                    // grilla y encola la actualización de posición. Las coordenadas
                    // de mundo las avanza el tick normal de la entidad.
                    *(int*)(h + 0x388) = gx;
                    *(int*)(h + 0x38c) = gy;
                    h[774] = gx;
                    h[775] = gy;
                    h[0x305] = 1;
                }
                break;
            }

            case 0x12: {
                // 2026-04-30 (v3): re-enabled with C1/C2 framing fix.
                // Server-emu sends ViewportPlayer as C2 (multi-byte length),
                    // pero nuestro handler anterior leía Msg[3] esperando enmarcado C1
                    // — y ése es el byte de OPCODE para C2. Resultado: count=18 era
                // actually 0x12 (the opcode itself!), causing bad-stride skip.
                //
                // Layout per framing:
                //   C1: [C1][len][op=12][count][entries...]   data@Msg+3
                //   C2: [C2][len_hi][len_lo][op=12][count][entries...] data@Msg+4
                int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                NetLog("NET:  → 0x12 ViewportPlayer count=%d size=%d hdr=%02X",
                       Msg[3 + hdrOff], Size, Msg[0]);
                if (!DAT_07abf5d8) {
                    NetLog("NET:    0x12 SKIP - hero not yet allocated");
                    break;
                }
                int count = Msg[3 + hdrOff];
                if (count <= 0 || count > 30) {
                    NetLog("NET:    0x12 SKIP - count=%d out of range", count);
                    break;
                }
                int entryStart = 4 + hdrOff;
                int entryStride = 32;
                if ((entryStart + count * entryStride) > Size) {
                    // Probamos strides más chicos que usan algunos emuladores de server
                    int payloadLen = Size - entryStart;
                    int autoStride = payloadLen / count;
                    if (autoStride >= 18 && autoStride <= 36) {
                        entryStride = autoStride;
                        NetLog("NET:    0x12 auto-stride=%d (payload=%d, count=%d)",
                               autoStride, payloadLen, count);
                    } else {
                        NetLog("NET:    0x12 SKIP - bad stride (count=%d, payload=%d)",
                               count, payloadLen);
                        break;
                    }
                }
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int i = 0; i < count && (entryStart + i*entryStride + 30) <= Size; ++i) {
                    const BYTE* e = Msg + entryStart + i*entryStride;
                    // 2026-05-05: layout decompilado del binario MuEmu
                    // server (CViewport::GCViewportPlayerSend, asm a1590).
                    // Stride 32 bytes:
                    //   e[0..1]=Key BE, e[2]=PosX, e[3]=PosY,
                    //   e[4..b]=Equipment(8B), e[c..d]=PkLevel,
                    //   e[e]=CtlCode, e[f]=padding,
                    //   e[10..11]=ViewSkillState (LE), e[12..1b]=Name(10B),
                    //   e[1c]=TargetX, e[1d]=TargetY, e[1e]=Path|Dir.
                    // Antes leíamos name desde e[0x10] (off-by-2,
                    // 2 bytes basura + 8 chars de nombre real).
                    WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];   // strip CREATE bit
                    BYTE x = e[2], y = e[3];
                    char name[11] = {0};
                    memcpy(name, e + 0x12, 10);
                    BYTE tx = e[0x1C], ty = e[0x1D];
                    BYTE dirpk = e[0x1E];
                    BYTE dir = (dirpk >> 4) & 0x0F;
                    const WORD viewSkillState = (WORD)(e[0x10] | (e[0x11] << 8));
                    // Un refresco de viewport por gate de mapa de MuEmu puede devolver
                    // como eco al personaje local. El original mantiene al Hero como único objeto
                    // para HeroKey; pasarlo por CreateCharacter crea
                    // una segunda copia del jugador, renderizada por separado.
                    if (entityId == g_HeroKey) {
                        BYTE* hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
                        if (hero) {
                            *(int*)(hero + 0x388) = x;
                            *(int*)(hero + 0x38c) = y;
                            hero[0x306] = tx;
                            hero[0x307] = ty;
                        }
                        NetLog("NET:    0x12 own HeroKey=%u synchronized, no viewport clone",
                               (unsigned)entityId);
                        continue;
                    }
                    // CreateCharacter (0045BFA0) first reuses a matching key,
                    // y después cae a un slot inactivo. Mantenemos ese orden:
                    // packets may refresh an already visible player.
                    int spawnSlot = -1;
                    for (int s = 0; s < 400; ++s) {
                        BYTE* slot = basePtr + s * 0x394;
                        if (slot == (BYTE*)DAT_07abf5d8) continue;
                        if (slot[0] != 0 && *(WORD*)(slot + 476) == entityId) {
                            spawnSlot = s;
                            break;
                        }
                        if (slot[0] == 0 && spawnSlot < 0) spawnSlot = s;
                    }
                    if (spawnSlot >= 0) {
                        BYTE* slot = basePtr + spawnSlot * 0x394;
                        float rot = ((float)dir - 1.0f) * 45.0f;
                        FUN_0045adc0(slot, 390, x, y, rot);
                        *(WORD*)(slot + 0x1dc) = entityId;
                        // Combat_PacketDispatch (00429690) applies the packed
                        // la clase/estado del jugador y después ChangeCharacterExt con
                        // CharSet[1..10]. Sin esto la entidad existe pero
                        // has no faithful class/equipment visual setup.
                        const BYTE charSet0 = e[4];
                        slot[0x1bc] = (BYTE)(((charSet0 >> 4) | (charSet0 & 0x10)) >> 1);
                        slot[445] = 0;
                        slot[0x2ea] = dirpk & 0x0F;
                        switch (charSet0 & 0x0F) {
                        case 1:
                            CreateTeleportEnd((unsigned int)(uintptr_t)slot);
                            break;
                        case 2:
                            FUN_0043e820((int)(uintptr_t)slot,
                                         (slot[0x1bc] & 7) == 2 ? 135 : 133);
                            break;
                        case 3:
                            FUN_0043e820((int)(uintptr_t)slot,
                                         (slot[0x1bc] & 7) == 2 ? 140 : 139);
                            break;
                        case 4:
                            FUN_0043e820((int)(uintptr_t)slot,
                                         (slot[0x1bc] & 7) == 2 ? 138 : 137);
                            break;
                        default:
                            break;
                        }
                        FUN_0045c8c0(spawnSlot, (BYTE*)e + 5);
                        memcpy(slot + 449, name, 10);  // Character.Name (+0x1C1)
                        slot[0x84] = 1;     // type: player (1)
                        slot[0x160] = 1;    // visible flag — sin esto Entity_RenderAll_3D skip
                        slot[0xdc] = 1;     // also "is rendered" sub-flag
                        slot[0x305] = 0;
                        slot[0x306] = tx;
                        slot[0x307] = ty;
                        slot[0x356] = 0;    // path_wp_count
                        *(int*)(slot + 0x388) = x;
                        *(int*)(slot + 0x38c) = y;
                        slot[0] = 1;        // active flag
                        if ((e[0] & 0x80) != 0) {
                            // CreateFlag branch in Combat_PacketDispatch:
                            // la posición llegue de forma autoritativa, y recién ahí emitir el
                            // standard player teleport-in effect (1265).
                            FUN_00460dc0(1265, (float*)(slot + 16), (float*)(slot + 28),
                                         (float*)(slot + 232), nullptr, (float*)slot,
                                         (float*)-1, nullptr, 0);
                            *(DWORD*)(slot + 360) = 0;
                        }
                        // El viewport trae efectos que ya estaban activos
                        // antes de que esta entidad se hiciera visible. Le pasamos el
                        // bitfield through InsertBuffPhysicalEffect semantics
                        // para que Mana Shield y el resto de los buffs persistentes
                        // appear without waiting for another 0x07 transition.
                        ApplyPersistentSkillEffect97k(slot, viewSkillState, 1);
                        NetLog("NET:    0x12 spawn player slot=%d id=%d name=%s pos=(%d,%d)",
                               spawnSlot, entityId, name, x, y);
                    }
                }
                break;
            }

            case 0x45: {
                // ViewportChange lo emite MuEmu cuando una entidad ya visible
                // player changes skin.  Unlike the old client packet, MuEmu's
                // el PMSG_VIEWPORT_CHANGE nativo son los 32 bytes con alineación por defecto
                // record from Viewport.h:
                //   id(2), x, y, skin, pad, ViewSkillState(LE), name(10),
                //   tx, ty, dir|pk, charset(11).
                // ReceiveCreateTransformViewport (00429C50) creates the
                // la forma de monstruo transformada para esta clave; no es apenas
                // un flag de skin sobre la entidad de jugador existente.
                const int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                if (Size <= 3 + hdrOff) break;

                const int count = Msg[3 + hdrOff];
                const int entryStart = 4 + hdrOff;
                constexpr int entryStride = 32;
                if (count <= 0 || count > 30 || entryStart + count * entryStride > Size) {
                    NetLog("NET:    0x45 SKIP count=%d size=%d", count, Size);
                    break;
                }

                int transformed = 0;
                for (int i = 0; i < count; ++i) {
                    const BYTE* const e = Msg + entryStart + i * entryStride;
                    const WORD entityId = (WORD)(((e[0] & 0x7F) << 8) | e[1]);
                    const BYTE skin = e[4];
                    const WORD viewSkillState = (WORD)(e[6] | (e[7] << 8));
                    if (skin == 0) continue;

                    BYTE* const entity = (BYTE*)FUN_0045ccf0(skin, e[2], e[3], entityId, 0);
                    if (!entity) continue;

                    // Campos de estado exactos de 00429C50, adaptados sólo para el
                    // MuEmu record offsets above.
                    if (entity[747] == 7) *(float*)(entity + 12) = 0.8f;
                    if (DAT_07abf5d8) entity[444] = ((BYTE*)DAT_07abf5d8)[444];
                    entity[746] = e[20] & 0x0F;
                    entity[132] = 1;
                    entity[847] = 1;
                    entity[0x160] = 1; // required by this renderer's active pass
                    entity[0xdc] = 1;
                    entity[0x305] = 0;
                    entity[0x306] = e[18];
                    entity[0x307] = e[19];
                    *(float*)(entity + 36) = ((float)(e[20] >> 4) - 1.0f) * 45.0f;
                    *(int*)(entity + 0x388) = e[2];
                    *(int*)(entity + 0x38c) = e[3];
                    memcpy(entity + 449, e + 8, 10);
                    entity[459] = 0;
                    ApplyPersistentSkillEffect97k(entity, viewSkillState, 1);

                    if ((e[0] & 0x80) != 0) {
                        // Rama CreateFlag: efecto de transformación 233 más su
                        // partícula 1191, exactamente como el handler original.
                        FUN_00460dc0(233, (float*)(entity + 16), (float*)(entity + 28),
                                     (float*)(entity + 232), nullptr, (float*)entity,
                                     (float*)-1, nullptr, 0);
                        FUN_004795c0(1191, (float*)(entity + 16), 1.0f,
                                     (float*)(entity + 232), (int)(uintptr_t)entity,
                                     0.0f, 0);
                        *(DWORD*)(entity + 360) = 0;
                    }
                    ++transformed;
                }
                NetLog("NET: -> 0x45 ViewportChange count=%d transformed=%d", count, transformed);
                break;
            }

            case 0x13: {
                // 2026-04-30 (v3): el mismo fix de C1/C2 que en 0x12.
                int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                NetLog("NET:  → 0x13 ViewportMonster count=%d size=%d hdr=%02X",
                       Msg[3 + hdrOff], Size, Msg[0]);
                if (!DAT_07abf5d8) {
                    NetLog("NET:    0x13 SKIP - hero not allocated");
                    break;
                }
                int count = Msg[3 + hdrOff];
                if (count <= 0 || count > 30) {
                    NetLog("NET:    0x13 SKIP - count=%d out of range", count);
                    break;
                }
                int entryStart = 4 + hdrOff;
                int entryStride = (Size - entryStart) / (count > 0 ? count : 1);
                if (entryStride < 10 || entryStride > 16) entryStride = 10;
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int i = 0; i < count && (entryStart + i*entryStride + 11) <= Size; ++i) {
                    const BYTE* e = Msg + entryStart + i*entryStride;
                    // 2026-05-05: layout decompilado del binario MuEmu
                    // Linux server (CViewport::GCViewportMonsterSend, asm
                    // a17c0). Confirmado disasm:
                    //   e[0]=KeyH|CREATE, e[1]=KeyL, e[2]=Class,
                    //   e[3]=padding, e[4..5]=ViewSkillState (LE),
                    //   e[6]=PosX, e[7]=PosY, e[8]=TargetX, e[9]=TargetY,
                    //   e[10]=Path|Dir, e[11]=padding.
                    // Antes leíamos x=e[5] y=e[6] (off-by-1) → monsters
                    // spawneaban en pos=(0,N) con N=TargetX en lugar de
                    // PosX/PosY reales.
                    WORD entityId = ((WORD)(e[0] & 0x7F) << 8) | e[1];
                    BYTE type = e[2];
                    if (type == 0) {
                        NetLog("NET:    0x13 skip (type=0 unsupported)");
                        continue;
                    }
                    BYTE x = e[6], y = e[7];
                    BYTE tx = e[8], ty = e[9];
                    BYTE dirpk = e[10];
                    BYTE dir = (dirpk >> 4) & 0x0F;
                    const WORD viewSkillState = (WORD)(e[4] | (e[5] << 8));
                    // 2026-05-04: usar CreateMonster (FUN_0045ccf0) en vez de
                    // FUN_0045adc0 (CreateCharacterPointer). El primero ADEMÁS
                    // carga el BMD model via OpenMonsterModel/OpenNpc, que es
                    // lo que faltaba — antes los slots se creaban "vacíos"
                    // sin modelo → no rendían en pantalla.
                    extern char* __cdecl FUN_0045ccf0(unsigned int Type, int PosX,
                                                     int PosY, int Key, int);
                    float rot = ((float)dir - 1.0f) * 45.0f;
                    char* slotChar = FUN_0045ccf0((unsigned int)type, x, y,
                                                  (int)entityId, 0);
                    if (slotChar) {
                        BYTE* slot = (BYTE*)slotChar;
                        // El facing va por separado — CreateMonster no lo setea.
                        *(float*)(slot + 0x24) = rot;
                        *(WORD*)(slot + 0x1dc) = entityId;
                        // FIX 2026-07-25: NO sobrescribir +0x84 (kind).  CreateMonster
                        // (FUN_0045ccf0) ya lo setea correcto por Type: 2=monster,
                        // 4=NPC (type>200), 8=ground-item.  El override a 2 forzaba a
                        // TODOS los NPCs (blacksmith, mage, etc) a kind=2 → Target_Render
                        // los mostraba como banner de monstruo (RenderCenteredText arriba)
                        // en vez del chat flotante de NPC (CreateChat).
                        slot[0x160] = 1;    // visible flag
                        slot[0xdc] = 1;     // is_rendered sub-flag
                        slot[0x305] = 0;
                        slot[0x306] = tx;
                        slot[0x307] = ty;
                        slot[0x356] = 0;
                        *(int*)(slot + 0x388) = x;
                        *(int*)(slot + 0x38c) = y;
                        slot[0] = 1;
                        // 2026-05-06: init +0x168 (screen distance) a 1.0f para
                        // que mob sea targetable INMEDIATAMENTE (antes del primer
                        // frame de render). Sin esto, FUN_004afdc0 (hover detect)
                        // filtra mob por `_DAT_00552580 < ent[+0x168]` (= 0
                        // por default) → mob no es hovered hasta que sea
                        // rendered (1+ frame later).
                        *(float*)(slot + 0x168) = 1.0f;
                        // 2026-05-05: init move speed (+0x2FA = +762 word).
                        // CreateMonster solo setea esto para case 11. Resto de
                        // los tipos quedan en 0 → FUN_00454b00 retorna 0 →
                        // sin movimiento. O queda en basura → mobs acelerados.
                        // 4 = "slow walk" baseline para mobs/NPCs (vs player ~12).
                        if (*(unsigned short*)(slot + 0x2FA) == 0) {
                            *(unsigned short*)(slot + 0x2FA) = 4;
                        }
                        ApplyPersistentSkillEffect97k(slot, viewSkillState, 1);
                        int spawnSlot = (int)((slot - basePtr) / 0x394);
                        NetLog("NET:    0x13 spawn monster slot=%d id=%d type=%d pos=(%d,%d) rot=%.0f",
                               spawnSlot, entityId, type, x, y, rot);
                    } else {
                        NetLog("NET:    0x13 SKIP - CreateMonster returned NULL (type=%d)",
                               type);
                    }
                }
                break;
            }

            case 0x14: {
                // PMSG_VIEWPORT_DESTROY_SEND (MuEmu):
                //   [C1][size][14][count][index_hi][index_lo]...
                // Fiel a ProtocolCore 004389A0:606-618: cada índice de
                // 15 bits se manda a DeleteCharacter (0045AC20), que es dueña
                // the slot/butterfly/cloth teardown.
                const int count = Msg[3];
                const int available = (Size >= 4) ? (Size - 4) / 2 : 0;
                const int entries = (count < available) ? count : available;
                for (int i = 0; i < entries; ++i) {
                    const int offset = 4 + i * 2;
                    const int entityId = ((Msg[offset] << 8) | Msg[offset + 1]) & 0x7FFF;
                    DeleteCharacter(entityId);
                }
                NetLog("NET:  -> 0x14 ViewportDestroy count=%d processed=%d size=%d",
                       count, entries, Size);
                break;
            }

            case 0x15: {
                // 2026-05-06 BUG-FIX MAYÚSCULO: opcode 0x15 server→cliente NO es
                // "ViewportDestroy" (eso era un mal-port de Ghidra). Es
                // GCDamageSend → PMSG_DAMAGE_SEND per
                // Mu-linux-97K/Source/MuServer/GameServer/Protocol.cpp:1595-1626.
                //
                // Layout (basic, 7 bytes; con GAMESERVER_EXTRA es 15):
                //   [C1][07][0x15]
                //   [3] = (target_idx_hi & 0x7F) | (kill_flag << 7)
                //   [4] = target_idx_lo
                //   [5] = (damage_hi & 0x0F) | (damage_type & 0xF0)
                //   [6] = damage_lo
                //
                // El kill_flag (bit 7 de Msg[3]) es CRÍTICO: cuando vale 1, este
                // hit MATÓ al target. El cliente debe entonces:
                //   - Setear dead_flag (+0x34e = 1) para que no sea targetable
                //   - Disparar animación de muerte
                //   - Mostrar "+EXP / -damage" en HUD (placeholder por ahora)
                //
                // ANTES: case 0x15 leía Msg[3] como `count` y trataba el packet
                // como una lista de entity IDs a borrar. Esto significaba que
                // el bit 7 (kill_flag) hacía `count = 128+` → OOB read sobre el
                // packet, y los mobs muertos seguían targetables (user reportó
                // "le pegue hasta animación de muerte pero podía seguir
                // pegando", screenshot 2026-05-06).
                // 2026-05-06: port FIEL desde IDA mu97k-src-IDA/raw/
                // 0042ACC0_ReceiveAttackDamage.c. Parses damage + color flags,
                // le baja HP al héroe si el objetivo es el héroe, y llama a CreatePoint
                // (FUN_004792c0) para spawnear los números de daño flotantes en el espacio del mundo.
                if (Size < 7) {
                    NetLog("NET:  → 0x15 Damage size=%d (too small, skip)", Size);
                    break;
                }
                // Wire format: index[0] high bit = kill flag; bits 0-6 + index[1] = id.
                BYTE  killFlag  = (Msg[3] >> 7) & 0x01;
                WORD  targetId  = ((WORD)(Msg[3] & 0x7F) << 8) | Msg[4];
                // Damage: lower 12 bits across damage[0..1]; upper 4 bits of damage[0]
                // are damage-type flags.
                DWORD damage    = ((DWORD)(Msg[5] & 0x0F) << 8) | Msg[6];
                // 2026-08-15 BUG-FIX (el daño se mostraba truncado: un crítico
                // de 12392 salía como 104, y el daño normal saturaba en ~4000).
                //
                // El campo legacy `damage[2]` sólo lleva 12 BITS: el server hace
                //     damage[0] = (SET_NUMBERHB(dmg) & 0x0F) | (type & 0xF0);
                //     damage[1] =  SET_NUMBERLB(dmg);
                // o sea el nibble ALTO de damage[0] es el tipo y sólo quedan 4
                // bits para el byte alto del daño → máximo 0x0FFF = 4095, y por
                // encima de eso se pierden los bits 12+. Comprobado con el caso
                // real: 12392 = 0x3068 → HB=0x30, `& 0x0F` = 0, LB = 0x68 = 104.
                //
                // Con `GAMESERVER_EXTRA=1` (nuestro server: el paquete llega con
                // size=16) el valor REAL viaja sin truncar en `ViewDamageHP`.
                // Layout de PMSG_DAMAGE_SEND (Protocol.h:203), con el padding
                // que mete el DWORD:
                //     +0..2  header      +3,+4  index[2]
                //     +5,+6  damage[2]   +7     PADDING
                //     +8..11 ViewCurHP   +12..15 ViewDamageHP
                if (Size >= 16) {
                    const DWORD viewDamage = *(const DWORD*)(Msg + 12);
                    if (viewDamage != 0) damage = viewDamage;
                }
                BYTE  typeBits  = (Msg[5] >> 4) & 0x0F;
                bool  bIgnore    = (typeBits & 1) != 0;  // yellow (PvP defense ignore)
                bool  bReflect   = (typeBits & 2) != 0;  // purple (reflect dmg)
                bool  bExcellent = (typeBits & 4) != 0;  // cyan
                bool  bCritical  = (typeBits & 8) != 0;  // blue
                NetLog("NET:  → 0x15 Damage tgt=%d dmg=%d flags=I%d/R%d/E%d/C%d kill=%d",
                       targetId, damage, bIgnore, bReflect, bExcellent, bCritical, killFlag);

                // Find target entity slot by entity_id (+0x1dc)
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                BYTE* tgtSlot = nullptr;
                if (targetId == g_HeroKey && DAT_07abf5d8) {
                    tgtSlot = (BYTE*)DAT_07abf5d8;
                } else {
                    for (int s = 0; s < 400; ++s) {
                        BYTE* sp = basePtr + s * 0x394;
                        if (sp[0] && *(WORD*)(sp + 0x1dc) == targetId) {
                            tgtSlot = sp; break;
                        }
                    }
                }
                if (!tgtSlot) {
                    NetLog("NET:    0x15 SKIP - target id=%d not found", targetId);
                    break;
                }

                // ── Hero HP decrement ────────────────────────────────────────
                // CharacterAttribute del héroe en DAT_07cf1ff4; HP en el offset 28 (WORD).
                if (targetId == g_HeroKey && DAT_07cf1ff4) {
                    WORD* pHP = (WORD*)((BYTE*)(uintptr_t)DAT_07cf1ff4 + 28);
                    if (damage < *pHP) *pHP -= damage;
                    else *pHP = 0;
                }

                // ReceiveAttackDamage (IDA 0042ACC0) no transiciona una
                // entidad a muerta y nunca llama a SetPlayerDie. El bit alto
                // identifica el golpe terminal para presentar el daño, pero
                // la transición de estado llega en ReceiveDie (0x17). Llamar a
                // SetPlayerDie acá hacía que el héroe/objetivo local entrara en su
                // action before MoveCharacter's native death sequence.

                // ── Cachea el último daño en la entidad (+0x2F8 = 760) per IDA ──
                *(WORD*)(tgtSlot + 760) = damage;

                // ── Hit reaction (anim + grunt) — SetPlayerShock ─────────────
                // 2026-05-08: imported from companion-DLL `IgnoreRandomStuck`
                // patch (Patchs.cpp). The original 0.97k client rolls a 50/50
                // el chequeo aleatorio adentro de ReceiveAttackDamage y llama a
                // SetPlayerShock incondicionalmente cuando la tirada pasa — eso
                // produce el bug del "random stuck", donde el héroe se traba en la
                // anim de shock 130 en medio del combate. El DLL companion hookea el
                // call site para saltearlo cuando la entidad es el jugador (tipo 390).
                // Reproducimos el mismo comportamiento acá: los monstruos reciben
                // su anim de shock + el quejido; el jugador NO.
                if (damage > 0 && !killFlag &&
                    *(WORD*)(tgtSlot + 2) != 390)
                {
                    // 50/50 random roll matching IDA `rand() & 0x80000001`.
                    unsigned r = (unsigned)rand() & 0x80000001u;
                    bool fire = (r == 0) ||
                                (((int)r < 0) &&
                                 ((((char)r - 1) | (int)0xFFFFFFFE) == -1));
                    if (fire) {
                        extern void __cdecl FUN_00444b60(int c, int Hit);
                        FUN_00444b60((int)tgtSlot, (int)damage);
                    }
                }

                // ── Spawn damage popup ───────────────────────────────────────
                // CreatePoint(Position[3], Value, Color[3], scale)
                // Per IDA ReceiveAttackDamage:
                //   damage==0 → MISS: scale=15, color white(hero) or gray(other)
                //   IGNORE   → yellow (1,1,0)  scale=50
                //   EXCELLENT→ cyan   (0,1,0.6)scale=50
                //   CRITICAL → blue   (0,0.6,1)scale=50
                //   REFLECT  → purple (1,0,1)  scale=15
                //   else hero target  → red    (1,0,0)
                //   else other target → orange (1,0.6,0)
                float pos[3];
                pos[0] = *(float*)(tgtSlot + 0x10);
                pos[1] = *(float*)(tgtSlot + 0x14);
                pos[2] = *(float*)(tgtSlot + 0x18);
                float color[3];
                float scale;
                int   displayValue;
                if (damage == 0) {
                    displayValue = -1;  // MISS sentinel
                    scale = 15.0f;
                    if (targetId == g_HeroKey) {
                        color[0] = color[1] = color[2] = 1.0f;     // white miss
                    } else {
                        color[0] = color[1] = color[2] = 0.5f;     // gray miss
                    }
                } else {
                    displayValue = (int)damage;
                    scale = 15.0f;
                    if (bIgnore) {
                        scale = 50.0f;
                        color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f;  // yellow
                    } else if (bExcellent) {
                        scale = 50.0f;
                        color[0] = 0.0f; color[1] = 1.0f; color[2] = 0.6f;  // cyan
                    } else if (bCritical) {
                        scale = 50.0f;
                        color[0] = 0.0f; color[1] = 0.6f; color[2] = 1.0f;  // blue
                    } else if (bReflect) {
                        color[0] = 1.0f; color[1] = 0.0f; color[2] = 1.0f;  // purple
                    } else if (targetId == g_HeroKey) {
                        color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;  // red
                    } else {
                        color[0] = 1.0f; color[1] = 0.6f; color[2] = 0.0f;  // orange
                    }
                }
                CreatePoint(pos, displayValue, color, scale);
                break;
            }

            case 0x18: {
                // 2026-05-06: port FIEL desde IDA mu97k-src-IDA/raw/
                // 0042B4F0_ReceiveAction.c. Server PMSG_ACTION_SEND format:
                //   struct {
                //     PBMSG_HEAD header;  // [C1][size][0x18]
                //     BYTE index[2];      // [3..4] entity index BIG-endian
                //     BYTE dir;           // [5] direction (1..8) → angle = (dir-1)*45
                //     BYTE action;        // [6] action code (NOT raw anim_state)
                //   };
                //
                // BUG-FIX MAYÚSCULO: el handler viejo leía `action = Msg[5]`
                // (el dir byte) y lo escribía DIRECTAMENTE como anim_state. Eso
                // causaba que el hero después de cada attack del server
                // quedara con anims raros (porque dir=1..8 mapea a glyphs aleat).
                //
                // Action codes per IDA (con sufijo "(walk)" si bit 1 de c+444==2):
                //   18  → emote 92 (sound 81)
                //   100 → SetPlayerAttack (atk1) — dispatched por weapon
                //   101 → SetPlayerAttack (atk2)
                //   102/103 → SetPlayerStop (cancel)
                //   108..125 → emotes: pares walk/idle (anim 93..122)
                //   126..131 → special anims 123..127
                //   else → SetAction(c, raw action)
                if (Size < 7) break;
                // 2026-05-07 BUG-FIX: IDA ReceiveAction:14 NO maskea bit 7 de
                // Msg[3]. Es el byte alto del Key completo (16 bits). Antes
                // hacíamos `& 0x7F` pensando que era kill_flag (eso es 0x15,
                // NO 0x18). Resultado: cuando el server enviaba un 0x18 con
                // bit 7 del high byte set en el key, fallaba el match con
                // +0x1dc del slot real → caía a `slot=nullptr` y bail-out, OR
                // matcheaba un slot equivocado (otro entity cuyo +0x1dc por
                // casualidad coincidía con el key masked) → ANIMABA EL ENTITY
                // EQUIVOCADO. User reportó: "el hero ataca solo cuando otro
                // mob/player ataca cerca".
                WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
                BYTE dirByte  = Msg[5];                  // [5] = dir (1..8)
                BYTE action   = Msg[6];                  // [6] = action code
                NetLog("NET:  → 0x18 Action key=0x%04x dir=%d act=%d", entityKey, dirByte, action);

                // FindCharacterIndex equivalent: walk CharactersClient[400]
                // matching +0x1dc == entityKey. NO hero-key shortcut — el
                // hero es solo otra entity en el pool. Si no hay match, bail.
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                BYTE* slot = nullptr;
                int slotIdx = -1;
                for (int s = 0; s < 400; ++s) {
                    BYTE* sp = basePtr + s * 0x394;
                    if (sp[0] && *(WORD*)(sp + 0x1dc) == entityKey) {
                        slot = sp; slotIdx = s; break;
                    }
                }
                if (!slot) {
                    NetLog("NET:    0x18 SKIP key=0x%04x not found", entityKey);
                    break;
                }
                // ReceiveAction uses FindCharacterIndex without a Hero
                // excepción: una acción válida para el jugador local tiene que seguir
                // el mismo camino de animación que cualquier otra entidad.
                const bool isHero = (slot == (BYTE*)DAT_07abf5d8);
                NetLog("NET:    0x18 RESOLVED key=0x%04x -> slot=%d isHero=%d type=0x%04x",
                       entityKey, slotIdx, isHero ? 1 : 0,
                       (int)*(WORD*)(slot + 2));

                // Update facing per IDA ReceiveAction:21:
                //   *(float*)(c + 36) = (dir - 1) * 45.0
                *(float*)(slot + 0x24) = ((float)(int)dirByte - 1.0f) * 45.0f;
                slot[0x2EC] = 0;  // alive flag cleared (per IDA c+748=0)
                // ReceiveAction también aplica la grilla objetivo recibida antes
                // (+774/+775) a la posición de mundo, antes de la animación.
                *(float*)(slot + 16) = (float)slot[774] * 100.0f + 50.0f;
                *(float*)(slot + 20) = (float)slot[775] * 100.0f + 50.0f;

                // Helper: equivalente a SetAction — setea anim_state guardando el previo en cache
                auto SetAnim = [](BYTE* s, BYTE animId) {
                    s[0x106] = s[0x105];
                    s[0x105] = animId;
                    *(float*)(s + 0x108) = 0.0f;
                };
                bool walking = (slot[0x1bc] & 7) == 2;

                switch (action) {
                case 18:
                    SetAnim(slot, 92);
                    PlayBuffer(81, 0, 0);
                    break;
                case 100: case 101: {
                    // Ataque despachado — llama a SetPlayerAttack para animar por
                    // weapon equipped. FUN_00444410 is SetPlayerAttack.
                    extern void __cdecl FUN_00444410(int, int, int, int);
                    FUN_00444410((int)slot, 0, 0, 0);
                    slot[0x2F5] = 1;             // c+757=1 attack pending
                    *(int*)(slot + 0x108) = 0;   // reset frame
                    *(WORD*)(slot + 0x310) = 0xFFFF;  // c+784 = -1 (no skill target)
                    break;
                }
                case 102: case 103: {
                    extern void __cdecl FUN_004430c0(int);  // SetPlayerStop
                    FUN_004430c0((int)slot);
                    break;
                }
                case 108: SetAnim(slot, walking ? 135 : 133); break;
                case 109: SetAnim(slot, walking ? 140 : 139); break;
                case 110: SetAnim(slot, walking ? 138 : 137); break;
                case 111: SetAnim(slot, walking ? 94  : 93);  break;
                case 112: SetAnim(slot, walking ? 96  : 95);  break;
                case 113: SetAnim(slot, walking ? 98  : 97);  break;
                case 114: SetAnim(slot, walking ? 104 : 103); break;
                case 115: SetAnim(slot, walking ? 102 : 101); break;
                case 116: SetAnim(slot, walking ? 106 : 105); break;
                case 117: SetAnim(slot, walking ? 108 : 107); break;
                case 118: SetAnim(slot, walking ? 100 : 99);  break;
                case 119: SetAnim(slot, walking ? 110 : 109); break;
                case 120: SetAnim(slot, walking ? 112 : 111); break;
                case 121: SetAnim(slot, walking ? 114 : 113); break;
                case 122: SetAnim(slot, walking ? 116 : 115); break;
                case 123: SetAnim(slot, walking ? 118 : 117); break;
                case 124: SetAnim(slot, walking ? 120 : 119); break;
                case 125: SetAnim(slot, walking ? 122 : 121); break;
                case 126: SetAnim(slot, 123); break;
                case 127: SetAnim(slot, 124); break;
                case 128: SetAnim(slot, 128); break;
                case 129: SetAnim(slot, 125); break;
                case 130: SetAnim(slot, 126); break;
                case 131: SetAnim(slot, 127); break;
                default:
                    // Desconocido — setea la acción cruda como anim_state (coincide con IDA
                    // SetAction default branch).
                    SetAnim(slot, action);
                    break;
                }
                break;
            }

            case 0x07: {
                // PMSG_EFFECT_STATE_SEND (C1:07): [state][effect:LE16]
                // [index:BE16].  MuEmu sends this whenever a persistent
                // effect starts or ends (including Mana Shield = 0x100).
                if (Size < 8) break;
                const BYTE state = Msg[3];
                const WORD effect = (WORD)(Msg[4] | (Msg[5] << 8));
                const WORD entityKey = (WORD)((Msg[6] << 8) | Msg[7]);
                BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
                if (!entityBase) break;
                bool foundEntity = false;
                for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
                    BYTE* entity = entityBase + slotIndex * 0x394;
                    // FindCharacterIndex (0045AC80) requires an active slot.
                    if (!entity[0] || *(WORD*)(entity + 476) != entityKey) continue;

                    // ProtocolCore 004389A0 llama a Insert/Clear sólo cuando el
                    // requested mask changes the entity's physical-effects
                    // bitmap. Esto importa para los efectos con owner visual:
                    // un arranque duplicado no tiene que recrear sus partículas/joints.
                    const DWORD currentEffects = *(DWORD*)(entity + 120);
                    if (state == 1) {
                        if ((currentEffects & effect) != effect) {
                            ApplyPersistentSkillEffect97k(entity, effect, 1);
                        }
                    } else if ((currentEffects & effect) == effect) {
                        ApplyPersistentSkillEffect97k(entity, effect, 0);
                    }
                    foundEntity = true;
                    break;
                }
                NetLog("NET:    EffectState state=%u effect=0x%03X key=%u entity=%s",
                       (unsigned)state, (unsigned)effect, (unsigned)entityKey,
                       foundEntity ? "found" : "missing");
                break;
            }

            case 0x19: {
                // PacketHandler_0x19 Skill — server tells client about skill effects.
                // Format: [C1][size][0x19][skill_idx][src_id_hi][src_id_lo][tgt_id_hi][tgt_id_lo]
                // 2026-05-07: delega en PacketHandler_0x19 de Skills.cpp, que tiene
                // the full 30+ skill type dispatch (Poison/Ice/Lightning/Combo/etc.).
                // El inline mínimo setea el lock de objetivo + el flag skill_active como respaldo.
                if (Size < 8) break;
                NetLog("NET:  → 0x19 Skill idx=%d size=%d", Msg[3], Size);
                extern void PacketHandler_0x19(BYTE* pkt);
                PacketHandler_0x19((BYTE*)Msg);
                break;
            }

            case 0x1F: {
                // ReceiveCreateSummonViewport @ 0042A530.  MuEmu emits this
                // como C2:1F, seguido de un contador y registros alineados de 22 bytes:
                //   KeyH|CREATE, KeyL, Type, pad, ViewSkillState(WORD),
                //   X, Y, TargetX, TargetY, Dir|PK, pad, OwnerName[10].
                // El padding en los offsets 3 y 11 es parte del struct nativo del
                // server; tratarlo como packed corre la posición en uno.
                const int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                const int countOff = 3 + hdrOff;
                const int entryStart = 4 + hdrOff;
                const int entryStride = 22;
                if (Size <= countOff || !DAT_07abf5d8) break;

                const int count = Msg[countOff];
                if (count <= 0 || count > 30 ||
                    entryStart + count * entryStride > Size) {
                    NetLog("NET:    0x1F SKIP - malformed summon viewport count=%d size=%d",
                           count, Size);
                    break;
                }

                BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int i = 0; i < count; ++i) {
                    const BYTE* e = Msg + entryStart + i * entryStride;
                    const bool create = (e[0] & 0x80) != 0;
                    const WORD key = (WORD)(((e[0] & 0x7F) << 8) | e[1]);
                    const BYTE type = e[2];
                    const WORD viewSkillState = (WORD)(e[4] | (e[5] << 8));
                    const BYTE x = e[6], y = e[7];
                    const BYTE tx = e[8], ty = e[9];
                    const BYTE dirPk = e[10];
                    if (type == 0) continue;

                    BYTE* summon = (BYTE*)FUN_0045ccf0(type, x, y, key, 0);
                    if (!summon) {
                        NetLog("NET:    0x1F SKIP - CreateMonster NULL type=%d", type);
                        continue;
                    }

                    // Mantener esto sincronizado con el setup normal del viewport de
                    // monstruos 0x13, y después aplicar la etiqueta/estado propios del invocado.
                    *(float*)(summon + 0x24) = ((float)((dirPk >> 4) & 0x0F) - 1.0f) * 45.0f;
                    *(WORD*)(summon + 0x1dc) = key;
                    summon[0x160] = 1;
                    summon[0xdc] = 1;
                    summon[0x305] = 0;
                    summon[0x306] = tx;
                    summon[0x307] = ty;
                    summon[0x356] = 0;
                    *(int*)(summon + 0x388) = x;
                    *(int*)(summon + 0x38c) = y;
                    *(float*)(summon + 0x168) = 1.0f;
                    if (*(WORD*)(summon + 0x2fa) == 0) *(WORD*)(summon + 0x2fa) = 4;
                    summon[132] = 1;
                    summon[746] = dirPk & 0x0F;
                    if ((dirPk & 0x0F) >= 6) *(WORD*)(summon + 446) = 1;

                    // El cliente original reemplaza la etiqueta de la criatura con
                    // "<owner name>'s <old monster name>".  Preserve the
                    // important owner identity without relying on localized
                    // el storage de GlobalText mientras se portea el subsistema de texto.
                    char oldName[101] = {};
                    strncpy(oldName, (char*)(summon + 449), sizeof(oldName) - 1);
                    memcpy(summon + 449, e + 12, 10);
                    summon[459] = 0;
                    strncat((char*)(summon + 449), "'s ", 100 - strlen((char*)(summon + 449)));
                    strncat((char*)(summon + 449), oldName, 100 - strlen((char*)(summon + 449)));
                    summon[0] = 1;
                    ApplyPersistentSkillEffect97k(summon, viewSkillState, 1);

                    if (create) AppearMonster((DWORD)(uintptr_t)summon);
                    NetLog("NET:    0x1F summon id=%d type=%d owner=%.10s pos=(%d,%d)",
                           key, type, (const char*)(e + 12), x, y);
                }
                break;
            }

            case 0x1E: {
                // ReceiveMagicContinue @ 0042CD10. El broadcast del server es
                // [C3][size][1E][skill][casterHi][casterLo][x][y][dir].
                // El casteo local ya es dueño de su animación; las entidades remotas
                // eligen acá la acción específica del skill.
                if (Size < 9) break;
                const WORD casterKey = (WORD)((Msg[4] << 8) | Msg[5]);
                BYTE* caster = nullptr;
                BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
                    BYTE* candidate = entityBase + slotIndex * 0x394;
                    if (*(WORD*)(candidate + 476) == casterKey) {
                        caster = candidate;
                        break;
                    }
                }
                if (!caster) break;

                const BYTE skill = Msg[3];
                // ReceiveMagicContinue @ 0042CD10 stores the received skill
                // en c+770 antes de armar c+757. CharacterAnimation después
                // llama a AttackStage, que despacha sus efectos visuales desde
                // this exact byte (Evil Spirit included).
                caster[770] = skill;
                caster[776] = Msg[6];
                caster[777] = Msg[7];
                if (caster == (BYTE*)DAT_07abf5d8) {
                    caster[757] = 1;
                    *(WORD*)(caster + 784) = 0xFFFF;
                    caster[756] = 0;
                    break;
                }

                if (*(WORD*)(caster + 2) == 390) {
                    switch (skill) {
                    case 10: FUN_0043e820((int)caster, 90); break;
                    case 12: FUN_0043e820((int)caster, 88); break;
                    case 14: FUN_0043e820((int)caster, 89); break;
                    case 24:
                    case 52: FUN_00444410((int)caster, 0, 0, 0); break;
                    case 41:
                    case 55: FUN_0043e820((int)caster, 61); break;
                    case 42: FUN_0043e820((int)caster, 62); break;
                    case 43: FUN_0043e820((int)caster, 67); break;
                    case 47: FUN_0043e820((int)caster, 66); break;
                    case 56: FUN_0043e820((int)caster, 81); break;
                    default: FUN_00444a80((int)caster); break;
                    }
                } else {
                    FUN_00444410((int)caster, 0, 0, 0);
                }
                *(DWORD*)(caster + 264) = 0;
                caster[757] = 1;
                *(WORD*)(caster + 784) = 0xFFFF;
                caster[756] = 0;
                break;
            }

            case 0x1B: {
                // ProtocolCore @ 004389A0: PMSG_SKILL_CANCEL_SEND
                // [C1][6][1B][skill][casterHi][casterLo]. Clear the exact
                // flag de efecto físico asociado al skill cancelado.
                if (Size < 6) break;
                DWORD effectMask = 0;
                switch (Msg[3]) {
                case 1:    effectMask = 1; break;
                case 7:    effectMask = 2; break;
                case 0x10: effectMask = 0x100; break;
                case 0x1B: effectMask = 8; break;
                case 0x1C: effectMask = 4; break;
                case 0x30: effectMask = 0x10; break;
                case 0x33: effectMask = 0x20; break;
                case 0x37: effectMask = 0x40; break;
                default: break;
                }
                if (effectMask == 0) break;

                const WORD casterKey = (WORD)((Msg[4] << 8) | Msg[5]);
                BYTE* entityBase = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int slotIndex = 0; slotIndex < 400; ++slotIndex) {
                    BYTE* caster = entityBase + slotIndex * 0x394;
                    if (caster[0] && *(WORD*)(caster + 476) == casterKey) {
                        // La misma limpieza que ReceiveEffectState(state=0): apenas
                        // clearing +120 leaves Mana Shield's joint 266 (and
                        // other owner-bound visuals) alive after cancellation.
                        ApplyPersistentSkillEffect97k(caster, (WORD)effectMask, 0);
                        break;
                    }
                }
                break;
            }

            case 0x16: {
                // 2026-05-06: NOTA — opcode 0x16 NO lo envía el server para
                // monster die. El server usa:
                //   - 0x9C (C3 encrypted) con PMSG_REWARD_EXPERIENCE_SEND para
                //     EXP/damage al killer (GCMonsterDieSend en Protocol.cpp:1811)
                //   - 0x15 (C1) con PMSG_DAMAGE_SEND y kill_flag=1 a viewers
                //     (GCDamageSend en Protocol.cpp:1595)
                //
                // 2026-05-07: delega en PacketHandler_0x16 de Skills.cpp, que
                // handles the kill confirm + teleport begin/end + EXP gain.
                // (El fallback inline — sólo el flag de muerto — queda después.)
                if (Size < 7) break;
                NetLog("NET:  → 0x16 MonsterDie/Teleport size=%d", Size);
                extern void PacketHandler_0x16(BYTE* pkt);
                PacketHandler_0x16((BYTE*)Msg);

                // Respaldo: asegura que la anim de muerte quede seteada en el objetivo por id (la
                // versión de Skills.cpp setea dead_flag pero no despacha el
                // SetPlayerDie correcto, que necesitamos para la anim 131/6 según la clase).
                {
                    WORD mobId = ((Msg[3] & 0x7F) << 8) | Msg[4];
                    BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                    for (int s = 0; s < 400; ++s) {
                        BYTE* sp = basePtr + s * 0x394;
                        if (sp[0] && *(WORD*)(sp + 0x1dc) == mobId) {
                            sp[0x2FD] = 1;
                            sp[0x2EC] = 0;
                            // (2026-08-10: sin 0x34e — es SafeZone, no dead)
                            extern void __cdecl FUN_00444d90(int c_in);
                            FUN_00444d90((int)sp);
                            break;
                        }
                    }
                }
                break;
            }

            case 0x17: {
                // ReceiveDie @ 0042F030. El paquete es exactamente la clave de la
                // entidad en los bytes 3..4; no es una actualización de HP/escudo. IDA
                // marca la entidad como muerta, detiene su movimiento y despacha
                // the model-specific death animation.
                if (Size < 5) {
                    NetLog("NET:  → 0x17 Die size=%d (too small, skip)", Size);
                    break;
                }
                const WORD entityId = (WORD)((Msg[3] << 8) | Msg[4]);
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                BYTE* entity = nullptr;
                if (entityId == g_HeroKey && DAT_07abf5d8) {
                    entity = (BYTE*)DAT_07abf5d8;
                } else if (basePtr) {
                    for (int slot = 0; slot < 400; ++slot) {
                        BYTE* candidate = basePtr + slot * 0x394;
                        if (candidate[0] && *(WORD*)(candidate + 0x1DC) == entityId) {
                            entity = candidate;
                            break;
                        }
                    }
                }
                if (!entity) {
                    NetLog("NET:  → 0x17 Die id=%d (entity missing)", entityId);
                    break;
                }

                // ReceiveDie exacta (0042F030): sólo arranca el contador de muerte
                // y detiene el movimiento. MoveCharacter incrementa +765 y
                // despacha SetPlayerDie en el frame terminal nativo.
                entity[765] = 1;
                entity[748] = 0;
                const int entitySlot = basePtr ? (int)((entity - basePtr) / 0x394) : -1;
                NetLog("NET:  → 0x17 Die id=%d slot=%d", entityId, entitySlot);
                break;
            }

            case 0x24: {
                // 2026-05-09: Server response to PMSG_ITEM_MOVE_RECV (client
                // manda 0x24 para pedir un movimiento; el server responde con el mismo
                // opcode confirming or denying).
                //
                // Per server ItemManager.h:100 PMSG_ITEM_MOVE_SEND:
                //   PBMSG_HEAD header;   // C3:24 (3 bytes)
                //   BYTE result;         // 0xFF = denied, else success (= target slot)
                //   BYTE slot;           // target slot
                //   BYTE ItemInfo[4];    // wire-format item bytes
                //
                // After our C3 unwrap, Msg layout: [hdr=C1][size][24][result]
                //   [slot][ItemInfo×4]. So:
                //   Msg[3] = result
                //   Msg[4] = slot
                //   Msg[5..8] = ItemInfo
                NetLog("NET:  → 0x24 ItemMoveSend result=%02X slot=%d size=%d",
                       Msg[3], Msg[4], Size);
                BYTE* sourcePool = ItemMove_GetPool(g_ItemMoveSourcePool);
                BYTE* targetPool = ItemMove_GetPool(g_ItemMoveTargetPool);
                if (!sourcePool) sourcePool = OffsetInventoryItems;
                if (!targetPool) targetPool = OffsetInventoryItems;
                if (Msg[3] == 0xFF) {
                    bool stackMergeAck = ItemMove_LooksLikeStackMerge(targetPool, (int)Msg[4], (BYTE*)DAT_07e91350);
                    // Server denied. Restore picked item to source slot OR
                    // sólo limpia el flag de "cargando" (el slot de origen todavía tiene
                    // el item — lo limpiamos localmente al levantarlo, pero el server
                    // no lo movió). Lo reinsertamos en el origen.
                    if (!stackMergeAck) {
                        ItemMove_RestoreSlot(sourcePool, (int)DAT_07ea5b18, (BYTE*)DAT_07e91350);
                    }
                    ItemMove_ClearPickedState();
                    PlayBuffer(29, 0, 0);
                } else {
                    if (Size >= 9) {
                        // Server confirmed the move. Msg[4] = target slot,
                        // Msg[5..8] = ItemInfo (4 bytes, ItemByteConvert).
                        //
                        // 2026-07-27 FIX "item se transforma en otro al moverlo":
                        // el port anterior armaba el item mezclando 12 bytes del
                        // ITEM struct agarrado (DAT_07e91350) con 4 bytes wire del
                        // server. El ITEM struct NO está en formato wire — sus
                        // bytes 4-11 (Durability/Option1/x/y/Key…) se
                        // reinterpretaban como opciones/serial del item → el slot
                        // quedaba con type/opciones equivocadas = "otro item".
                        // Ahora construimos el item SOLO desde los 4 bytes wire
                        // del server (autoritativo), igual que el snapshot F3/10
                        // y el buy 0x32. InsertInventoryItem lee hasta Item[4];
                        // dejamos ext=0.
                        BYTE targetSlot = Msg[4];
                        BYTE itembytes[6] = { 0, 0, 0, 0, 0, 0 };
                        memcpy(itembytes, &Msg[5], 4);
                        {
                            int t = ConvertItemType(itembytes);
                            NetLog("NET:    0x24 place slot=%d wire=[%02X %02X %02X %02X] type=%d",
                                   (int)targetSlot, itembytes[0], itembytes[1],
                                   itembytes[2], itembytes[3], t);
                        }

                        if (ItemMove_IsInventoryEquipSlot(targetPool, (int)targetSlot)) {
                            // Equip slot: FUN_004cc660 rutea a WriteEquipmentSlot
                            // internamente cuando slotIdx < 12 y pool = main inv.
                            FUN_004cc660(OffsetInventoryItems, 8, 8,
                                         (int)targetSlot, itembytes, 1);
                        } else {
                            int slotIndex = (targetPool == OffsetInventoryItems) ? (int)targetSlot : ItemMove_ToGridSlot(targetPool, (int)targetSlot);
                            int slotMax = (targetPool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(targetPool));
                            if (slotIndex >= 0 && slotIndex < slotMax) {
                                int first = (targetPool == OffsetInventoryItems) ? 0 : 1;
                                FUN_004cc660(targetPool, 8, ItemMove_GetGridH(targetPool), slotIndex, itembytes, first);
                            }
                        }
                    }
                    // Algunas ramas del server confirman el movimiento con un paquete más corto
                    // pero igual esperan que el cliente suelte el estado de arrastre.
                    ItemMove_ClearPickedState();
                    PlayBuffer(29, 0, 0);
                }
                SeedQuickPotionTypesFromInventory();
                break;
            }

            case 0x30: {
                // ── ReceiveTalk (IDA 0x4301B0, server→client) ────────────────
                // 2026-07-25 (#2 shops): el server ordena abrir la ventana de un
                // NPC al hablarle. Msg[3] = tipo:
                //   2 = Warehouse (baúl)   3 = Chaos Machine (mezcla)
                //   4/6 = Event window     5 = Server division
                //   default = Shop (comprar/vender)
                // El anti-tamper hash-table que en IDA envuelve el set de
                // ShopOpened se omite per policy — el efecto neto es ShopOpened=1.
                // La rama cliente→server (bEncrypted=0) del IDA es el SEND de la
                // request de "hablar"; nosotros no la usamos (mandamos directo).
                NetLog("NET:  → 0x30 ReceiveTalk type=%d size=%d", Msg[3], Size);
                InventoryOpened = 1;
                switch (Msg[3]) {
                    case 2:  // Warehouse
                        // 2026-07-27 FIX: los paneles de NPC son mutuamente
                        // excluyentes. Si quedaba ShopOpened=1 de una tienda
                        // anterior, el baúl se titulaba "Comprar (B)" y los drops
                        // caían en la rama de VENDER (cartel "item caro") en vez
                        // de guardarse en el baúl.
                        ShopOpened = 0; ChaosMixOpened = 0; TradeOpened = 0;
                        WarehouseOpened = 1;
                        DAT_00559f5f = 0;     // byte_559F5F
                        DAT_07eaa14c = 0;     // dword_7EAA14C
                        break;
                    case 3:  // Chaos Machine (mix)
                        ShopOpened = 0; WarehouseOpened = 0; TradeOpened = 0;
                        ChaosMixOpened = 1;
                        DAT_07eaa140 = 0;     // MixState = 0
                        for (int i = 0; i < 4; i++)
                            DAT_0055a3e8[i] = Msg[4 + i];
                        SetErrorMessage(143);
                        break;
                    case 4:  // Event window (type 0)
                        EventType = 0;
                        CloseInventoryRelatedWindows();
                        EventWindowOpened = 1;
                        InventoryOpened = 1;
                        break;
                    case 5:  // Server division
                        CloseInventoryRelatedWindows();
                        InventoryOpened = 0;
                        g_bServerDivisionEnable = 1;
                        g_bServerDivisionAccept = 0;
                        break;
                    case 6:  // Event window (type 1)
                        EventType = 1;
                        CloseInventoryRelatedWindows();
                        EventWindowOpened = 1;
                        InventoryOpened = 1;
                        break;
                    default:  // Shop (buy/sell)
                        WarehouseOpened = 0; ChaosMixOpened = 0; TradeOpened = 0;
                        ShopOpened = 1;
                        *((BYTE*)&DAT_07eaa150 + 2) = 0;   // BYTE2(dword_7EAA150)=0
                        break;
                }
                CharacterOpened = 0;
                GuildOpened     = 0;
                PartyOpened     = 0;
                PlayBuffer(25, 0, 0);
                PlayBuffer(28, 0, 0);
                // IDA además reposiciona el cursor del OS (SetCursorPos) a la zona
                // de la ventana; lo omitimos (mover el cursor del sistema es
                // intrusivo y no afecta la lógica del juego).
                break;
            }

            case 0x31: {
                // 2026-05-19: bulk inventory list for storage/mix/shop.
                // Por ahora cableamos los paneles útiles reales que ya tenemos localmente:
                // storage and chaos mix.
                if (Size < 5) break;
                BYTE sub = Msg[3];
                BYTE count = Msg[4];
                int cursor = 5;
                NetLog("NET:  -> 0x31 InventoryList sub=%d count=%d size=%d hdr=%02X", sub, count, Size, hdr);
                // 2026-07-25 (#2 shops): dump crudo del 0x31 para capturar la
                // estructura de la lista de items de TIENDA (viene C2, offsets
                // distintos del layout C1 que asume este handler). Con esto
                // porteamos la rama shop → pool Inventory[] con los offsets reales.
                {
                    // Dump en chunks de 32 bytes: NetLog trunca a 256 y devuelve
                    // -1 → salía vacío. DbgLogPublic no trunca.
                    for (int off = 0; off < Size && off < 128; off += 32) {
                        char b[200]; int p = wsprintfA(b, "0x31 RAW[%02d] hdr=%02X: ", off, hdr);
                        for (int i = off; i < off + 32 && i < Size; ++i)
                            p += wsprintfA(b + p, "%02X ", Msg[i]);
                        DbgLogPublic(b);
                    }
                }

                // 2026-07-25 (#2 shops) PIEZA C: rama SHOP con los offsets C2
                // reales (IDA ReceiveTradeInventory 0x427560): type=Msg[4],
                // count=Msg[5], records desde Msg[6] stride 5 = [slot(1)][info(4)].
                // El handler viejo de abajo asume layout C1 (Msg[3]/Msg[4], stride
                // 13) — correcto para warehouse C1 pero NO para el shop C2.
                {
                    BYTE listType  = Msg[4];
                    BYTE listCount = Msg[5];
                    // 2026-07-27 FIX (tienda abre vacía a veces): el gate exigía
                    // `ShopOpened` ya en 1, pero el 0x30 (que lo setea) y el 0x31
                    // llegan casi juntos — si el 0x31 se procesaba antes de que
                    // ShopOpened estuviera seteado, esta rama se saltaba y caía al
                    // handler viejo (layout C1), que limpiaba el pool sin popular
                    // → tienda vacía intermitente (confirmado por el diag SHOPREND:
                    // ShopOpened=1 pos ok pero occ=0). El discriminante correcto es
                    // el FORMATO: header C2 = shop list (stride 5), C1 = warehouse.
                    // 2026-07-27 FIX (el baúl no carga items): el server manda la
                    // lista del BAÚL con el MISMO opcode 0x31 y el MISMO type=0
                    // que la tienda (Warehouse.cpp:276 vs Shop.cpp:251; sólo el
                    // ChaosBox usa type=3). Son indistinguibles por formato, así
                    // que el destino se decide por QUÉ VENTANA está abierta —
                    // como hacía el original. El gate anterior (header C2) mandaba
                    // la lista del baúl al pool de la tienda → baúl vacío.
                    // El 0x30 (que setea Warehouse/ShopOpened) siempre llega ANTES
                    // que el 0x31, así que el flag ya está puesto acá.
                    bool isWarehouseList = (Msg[0] == 0xC2) && (listType != 3) && WarehouseOpened;
                    bool isShopList      = (Msg[0] == 0xC2) && (listType != 3) && !WarehouseOpened;

                    if (isWarehouseList) {
                        // Baúl: grid 8×15 (120 slots) en OffsetWarehouseItems.
                        for (int i = 0; i < 120; ++i) {
                            BYTE* c = OffsetWarehouseItems + i * 0x44;
                            *(short*)c = (short)0xFFFF;
                            *(DWORD*)(c + 0x38) = 0;
                        }
                        const BYTE* rec = Msg + 6;
                        int placed = 0;
                        for (int i = 0; i < listCount && (6 + i * 5 + 5) <= Size; ++i, rec += 5) {
                            BYTE itembytes[6] = { 0, 0, 0, 0, 0, 0 };
                            memcpy(itembytes, rec + 1, 4);
                            FUN_004cc660(OffsetWarehouseItems, 8, 15, (int)rec[0], itembytes, 1);
                            placed++;
                        }
                        NetLog("NET:    0x31 WAREHOUSE populated %d/%d items", placed, listCount);
                        break;
                    }

                    if (isShopList) {
                        // Limpiar pool de tienda: overlay en &Inventory[i].WalkSpeed
                        // (offset +24). Type@+0=0xFFFF (vacío), Key@+0x38=0.
                        for (int i = 0; i < 120; ++i) {
                            BYTE* c = ShopItems + i * 0x44;
                            *(short*)c = (short)0xFFFF;
                            *(DWORD*)(c + 0x38) = 0;
                        }
                        const BYTE* rec = Msg + 6;
                        int placed = 0;
                        for (int i = 0; i < listCount && (6 + i * 5 + 5) <= Size; ++i, rec += 5) {
                            ShopInsertItem(rec[0], rec + 1);
                            placed++;
                        }
                        NetLog("NET:    0x31 SHOP populated %d/%d items", placed, listCount);
                        break;
                    }
                }

                auto ClearItemPool = [](BYTE* pool, int slots) {
                    for (int i = 0; i < slots; ++i) {
                        BYTE* cell = pool + i * 0x44;
                        *(short*)cell = (short)0xFFFF;
                        memset(cell + 4, 0, 0x40);
                    }
                };

                if (sub == 3 || sub == 5) {
                    ClearItemPool(OffsetMixItems, 32);
                    if (sub == 3 || sub == 5) {
                        DAT_07eaa140 = 0;
                    }
                    for (int i = 0; i < count && cursor + 13 <= Size; ++i) {
                        BYTE slot = Msg[cursor];
                        if (slot < 32) {
                            FUN_004cc660(OffsetMixItems, 8, 4, (int)slot, (BYTE*)Msg + cursor + 1, 1);
                        }
                        cursor += 13;
                    }
                    break;
                }

                if (DAT_07eaa119 != 0) {
                    ClearItemPool(OffsetWarehouseItems, 120);
                    for (int i = 0; i < count && cursor + 13 <= Size; ++i) {
                        BYTE slot = Msg[cursor];
                        if (slot < 120) {
                            FUN_004cc660(OffsetWarehouseItems, 8, 15, (int)slot, (BYTE*)Msg + cursor + 1, 1);
                        }
                        cursor += 13;
                    }
                }
                break;
            }

            case 0x32: {
                // 2026-07-27 FIX: este es el buy-response del shop
                // (PMSG_ITEM_BUY_SEND, ItemManager.cpp CGItemBuyRecv):
                //   [C1][08][32][result][i0][i1][i2][i3]  (Size=8)
                //   result = slot ABSOLUTO del inventario (>=12 = grid
                //            principal en result-12) donde cayó el item,
                //            o 0xFF si la compra falló (sin zen / sin espacio).
                //   i0..i3 = 4-byte ItemInfo (ItemByteConvert): index, level/
                //            opts, durability, hi/exc.
                // El port anterior exigía Size>=16 y usaba el path de item-move
                // (12 bytes) → NUNCA insertaba el item comprado (Size real = 8).
                // Ahora reusa InsertInventoryItem, el mismo path fiel que el
                // snapshot F3/10 (stride 5 = slot + 4 bytes) que sí funciona.
                // IDA ProtocolCore L826: FUN_004cc660(&Inv, 8, 8, byte[3],
                // body+2, 0).
                BYTE result = Msg[3];
                NetLog("NET:  → 0x32 BuyResult slot=%d size=%d", result, Size);
                if (result == 0xFF) {
                    // compra rechazada — el server ya avisó (GCNoticeSend);
                    // no hay item que insertar.
                    ItemMove_ClearPickedState();
                } else if (Size >= 8) {
                    // 4-byte ItemInfo → buffer de 6 (InsertInventoryItem lee
                    // Item[4] extByte; lo dejamos en 0 para no leer basura).
                    BYTE itembytes[6] = { 0, 0, 0, 0, 0, 0 };
                    memcpy(itembytes, (BYTE*)Msg + 4, 4);
                    FUN_004cc660(OffsetInventoryItems, 8, 8,
                                 (int)result, itembytes, 1);
                    ItemMove_ClearPickedState();
                    PlayBuffer(29, 0, 0);
                }
                SeedQuickPotionTypesFromInventory();
                DAT_05826d1c = 0;
                break;
            }

            case 0x33: {
                // 2026-05-08: slot de trade aceptado por el server. Limpia el
                // item flag (server confirmed the swap completed).
                // Per IDA ProtocolCore L834-845.
                NetLog("NET:  → 0x33 TradeAck sub=%d", Msg[3]);
                BYTE flag = Msg[3];
                if (flag != 0) {
                    if (flag == 0xFF || flag == 0xFE) {
                        ItemMove_ClearPickedState();
                        DAT_05826d1c = 0;
                        UIChatLogWindow_AddText(nullptr, GlobalText[733], 2);
                    } else {
                        ItemMove_ClearPickedState();
                        DAT_05826d1c = 0;
                        if (Size >= 8 && DAT_07cf1ffc != 0) {
                            *(DWORD*)((BYTE*)DAT_07cf1ffc + 1352) = *(DWORD*)(Msg + 4);
                        }
                        PlayBuffer(29, 0, 0);
                    }
                } else {
                    ItemMove_ClearPickedState();
                    DAT_05826d1c = 0;
                }
                break;
            }

            case 0x34: {
                NetLog("NET:  -> 0x34 Repair size=%d", Size);
                if (Size >= 7 && DAT_07cf1ffc != 0) {
                    DWORD gold = *(DWORD*)(Msg + 3);
                    if (gold != 0) {
                        *(DWORD*)((BYTE*)DAT_07cf1ffc + 1352) = gold;
                        PlayBuffer(0x25, 0, 0);
                    }
                }
                DAT_05826d1c = 0;
                break;
            }

            case 0x36: {
                // PMSG_TRADE_REQUEST_SEND (MuEmu Trade.h): C1:36,name[10].
                // La respuesta la manda el diálogo 128 como C1:37.
                if (Size < 13) break;
                memcpy(s_tradeRequestName, Msg + 3, 10);
                s_tradeRequestName[10] = '\0';
                SetErrorMessage(128);
                NetLog("NET: -> 0x36 TradeRequest name=%s", s_tradeRequestName);
                break;
            }

            case 0x37: {
                NetLog("NET:  -> 0x37 TradeResult sub=%d size=%d", Msg[3], Size);
                BYTE sub = Msg[3];
                if (sub == 0) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[492], 2);
                } else if (sub == 2) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[493], 2);
                } else if (sub == 1) {
                    DAT_07eaa11b = 1;
                    DAT_05826d30 = 1;
                    DAT_07eaa0e8 = 0;
                    DAT_07eaa0fd = 0;
                    DAT_00559684 = 0xFFFFFFFF;
                    ItemMove_ClearPickedState();
                    if (Size >= 14) {
                        memset(lpString_05826bfc, 0, 0x50);
                        memcpy(lpString_05826bfc, Msg + 4, 10);
                    }
                }
                break;
            }

            case 0x38: {
                // 2026-05-08: UI_Main slot-clear notification. Per IDA L1126-1128.
                // Server tells client to clear inventory slot pkt[3].
                NetLog("NET:  → 0x38 SlotClear slot=%d", Msg[3]);
                // IDA 004389A0: C1:38 limpia `Inventory`, la grilla superior
                // de sólo lectura que representa la oferta del otro comerciante.
                UI_Main((int)Msg[3], (short*)Inventory, 8u);
                PlayBuffer(29, 0, 0);
                break;
            }

            case 0x39: {
                // PMSG_TRADE_ITEM_ADD_SEND (MuEmu Trade.h): slot + 4-byte
                // ItemInfo. El guard anterior de >=16 venía de otro
                // packet family and discarded every valid MuEmu trade item.
                NetLog("NET:  → 0x39 TradeWarehouseSlot slot=%d size=%d",
                       Msg[3], Size);
                if (Size >= 8) {
                    // IDA 004389A0: C1:39 inserta en la misma grilla remota.
                    FUN_004cc660(Inventory, 8, 4,
                                 (int)Msg[3], (BYTE*)Msg + 4, 1);
                    PlayBuffer(29, 0, 0);
                }
                break;
            }

            case 0x3A: {
                // 2026-05-19: documentado en las notas locales de IDA de este cliente.
                DAT_07eaa0f4 = (Msg[3] != 0) ? DAT_05826c9c : 0;
                break;
            }

            case 0x3B: {
                // PMSG_TRADE_MONEY_SEND: monto que ofrece la contraparte.
                if (Size >= 7) {
                    DAT_07eaa0f0 = *(DWORD*)(Msg + 3);
                }
                break;
            }

            case 0x3C: {
                // PMSG_TRADE_OK_BUTTON_SEND. Estos son los dos confirm
                // lamps, not second-password UI state.
                const BYTE sub = Msg[3];
                if (sub == 0) {
                    DAT_07eaa0fc = 0;
                } else if (sub == 1) {
                    DAT_07eaa0fc = 1;
                    FUN_00404bc0(0x19, 0, 0);
                } else {
                    if (sub == 2) {
                        DAT_07eaa0fd = 0;
                    }
                    FUN_00404bc0(0x19, 0, 0);
                }
                break;
            }

            case 0x3D: {
                NetLog("NET:  -> 0x3D TradeExit state=%d size=%d", Msg[3], Size);
                BYTE state = Msg[3];

                if (state == 0) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[492], 2);
                    DAT_07eaa0e8 = 0;

                    for (int slot = 0; slot < 32; ++slot) {
                        BYTE* item = Inventory + slot * 0x44;
                        DWORD key = *(DWORD*)(item + 4);
                        if (*(short*)item != (short)0xFFFF && key != 0) {
                            UI_Main(slot, (short*)Inventory, 8u);
                        }
                    }
                } else if (state == 2) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[495], 2);
                } else if (state == 3) {
                    UIChatLogWindow_AddText(nullptr, GlobalText[496], 2);
                    SetErrorMessage(0);
                }
                // AUDITORIA 2026-07-20: aca habia un `else if (state == 4)` con
                // GlobalText[2108] — indice FUERA del Text.bmd del 0.97k (1000
                // filas).  ReceiveTradeExit (IDA 0x4337F0) solo maneja los
                // estados 0, 2 y 3; el 4 es un graft de version posterior.

                DAT_07eaa11b = 0;
                DAT_05826d30 = 0;
                DAT_07e91388 = 0;
                DAT_07eaa165 = 0;
                DAT_07eaa0fd = 0;
                EnableUse = 0;
                g_ItemMoveSourcePool = 0;
                g_ItemMoveTargetPool = 0;
                FUN_00423db0();
                CloseInventoryRelatedWindows();

                if (DAT_083a7c24 == 116) {
                    SetErrorMessage(0);
                    FUN_0047ec60(0);
                    _InputTextMaxArr[0] = 42;
                    DAT_00559c88 = 2;
                    InputEnable = 0;
                }
                break;
            }

            case 0x44: {
                // Party HP bars (ProtocolCore @ 004389A0): entries start at
                // pkt[4], high nibble=party slot and low nibble=HP step.
                NetLog("NET:  → 0x44 PartyHPBars size=%d", Size);
                extern void PacketHandler_0x44(BYTE* pkt, int size);
                PacketHandler_0x44((BYTE*)Msg, Size);
                break;
            }

            case 0x40: {
                // ProtocolCore @ 004389A0: party invitation.  The inviter
                // la clave es big-endian y RenderErrorMessage(0x78) resuelve
                // esta clave al nombre del personaje para el prompt.
                if (Size < 5) break;
                DAT_07eaa0e4 = (DWORD)((Msg[3] << 8) | Msg[4]);
                SetErrorMessage(120);
                NetLog("NET: -> 0x40 PartyRequest inviter=%u",
                       (unsigned)DAT_07eaa0e4);
                break;
            }

            case 0x41: {
                // ProtocolCore @ 004389A0 cierra el prompt de pedido de party con
                // cada resultado del server. La elección del string de chat es puramente
                // presentational; party membership itself is authoritative
                // recién después de la lista 0x42 siguiente.
                if (Size < 4) break;
                SetErrorMessage(0);
                NetLog("NET: -> 0x41 PartyResult result=%u", (unsigned)Msg[3]);
                break;
            }

            case 0x42: {
                // ReceivePartyList @ 00434660. Esto no es estado opcional de
                // UI: Combat::Attack usa PartyNumber y la tabla Party original
                // de 36 bytes para autorizar Mana Shield y Teleport
                // Ally before emitting their MuEmu packets.
                extern void ReceivePartyList97k(BYTE* pkt, int size);
                ReceivePartyList97k((BYTE*)Msg, Size);
                NetLog("NET: -> 0x42 PartyList count=%u", Size >= 5 ? (unsigned)Msg[4] : 0);
                break;
            }

            case 0x43: {
                // ProtocolCore @ 004389A0: party dissolved / local member
                // removido. El original limpia el contador de inmediato.
                PartyNumber = 0;
                NetLog("NET: -> 0x43 PartyClear");
                break;
            }

            case 0x46: {
                // 2026-05-07: Terrain tile update (Terrain_TileUpdate in Party.cpp).
                // Sub-type at pkt[3]: 0x00 = rect update, 0x01 = single tile.
                NetLog("NET:  → 0x46 TerrainTileUpdate size=%d", Size);
                extern void Terrain_TileUpdate(BYTE* pkt);
                Terrain_TileUpdate((BYTE*)Msg);
                break;
            }

            case 0x50: {
                // Guild invitation. ProtocolCore stores the inviter viewport
                // clave y después abre el diálogo 119; la respuesta es C1:51, no el
                // C1:41 de party. Mantener este estado aislado de los pedidos de party.
                if (Size < 5) break;
                DAT_07eaa0d8 = (DWORD)((Msg[3] << 8) | Msg[4]);
                SetErrorMessage(119);
                NetLog("NET: -> 0x50 GuildRequest inviter=%u",
                       (unsigned)DAT_07eaa0d8);
                break;
            }

            case 0x51: {
                // ReceiveGuildResult @ 00434780: outcome text + dismiss.
                if (Size < 4) break;
                static const int textIndex[] = { 503, 504, 505, 506,
                                                 507, 508, 509, 510 };
                const BYTE result = Msg[3];
                if (result < _countof(textIndex))
                    UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[result]], 2);
                SetErrorMessage(0);
                NetLog("NET: -> 0x51 GuildResult result=%u", (unsigned)result);
                break;
            }

            case 0x52: {
                // ReceiveGuildList @ 004348B0. MuEmu Guild.h defines the
                // native long frame as:
                // [C2][sizeHi][sizeLo][52][result][count][TotalScore:DWORD]
                // [score] followed by count * { name[10], number, connected }.
                // La rama 0x65 existente es de otro protocolo y
                // no se puede usar acá porque los offsets de sus campos difieren.
                // 2026-08-15 BUG-FIX (el panel abría con el nombre del guild pero
                // la lista de miembros salía vacía): los offsets estaban corridos
                // 5 bytes por el PADDING de la struct del server. MuEmu Guild.h:
                //     struct PMSG_GUILD_LIST_SEND {
                //         PWMSG_HEAD header;   // C2:52   +0..3
                //         BYTE  result;        //         +4
                //         BYTE  count;         //         +5
                //                              //         +6,+7  PADDING
                //         DWORD TotalScore;    //         +8..11   (alineado a 4)
                //         BYTE  score;         //         +12
                //     };                       // sizeof = 16
                // Los miembros (`PMSG_GUILD_LIST`, 12 bytes: name[10], number,
                // connected) arrancan en +16, no en +11. Verificado contra el
                // wire real: el nombre "mago" caía en Msg[16].
                // Mismo patrón que el F3/06 de los stats (ver CLAUDE.md).
                const int kHeaderSize = 16;
                if (Size < kHeaderSize) break;
                const BYTE count = Msg[5];
                const int maxMembers = 16; // backing table: 16 rows × 80 bytes
                const int memberCount = (count < maxMembers) ? count : maxMembers;
                if (Size < kHeaderSize + (int)count * 12) break;

                g_nGuildMemberCount = memberCount;
                GuildTotalScore = *(const int*)(Msg + 8);
                if (GuildTotalScore < 0) GuildTotalScore = 0;
                memset(byte_7E919BC, 0, maxMembers * 80);
                for (int i = 0; i < memberCount; ++i) {
                    const BYTE* src = Msg + kHeaderSize + i * 12;
                    char* dst = &byte_7E919BC[i * 80];
                    memcpy(dst, src, 12);
                    dst[10] = 0; // name[10] is fixed-width on the wire.
                }

                // 2026-08-15: alimentar el WIDGET de lista (dword_55C9FF4), que
                // es de donde `RenderGuildList` saca las filas.  Antes sólo se
                // llenaba `byte_7E919BC` (que el render usa nada más que para el
                // nombre del guild en el título), así que el listado salía vacío.
                // Fiel a IDA ReceiveGuildList @0x4348B0: vtable[10] para limpiar
                // y vtable[28] por cada miembro, con el registro de 13 bytes
                //   +0..9 name · +10 NUL · +11 connected · +12 party (o -1).
                GuildList_Clear();
                memset(byte_7E91790, 0, 0x22C);
                for (int i = 0; i < memberCount; ++i) {
                    const BYTE* src = Msg + kHeaderSize + i * 12;
                    // El original arma cada registro EN `byte_7E91790` (stride
                    // 13) y desde ahi se lo pasa a vtable[28]; el estado 126 de
                    // UI_InGameMenu lo re-lee para el paquete de expulsar, asi
                    // que hay que dejarlo escrito, no solo en un local.
                    char* rec = &byte_7E91790[i * 13];
                    memcpy(rec, src, 10);
                    rec[10] = 0;
                    // Wire: [name:10][number][connected].  IDA lee el byte
                    // "connected" (src[11]) para el flag y el "number" (src[10])
                    // para el party: si tiene el bit alto puesto, party =
                    // number & 0x7F; si no, -1 (= 0xFF, "sin party").
                    char connected = (char)src[11];
                    char number    = (char)src[10];
                    char party     = (number >= 0) ? (char)-1 : (char)(number & 0x7F);
                    rec[11] = connected;
                    rec[12] = party;
                    GuildList_AddMember(rec, connected, party);
                }
                NetLog("NET: -> 0x52 GuildList result=%u members=%d score=%d",
                       (unsigned)Msg[4], memberCount, GuildTotalScore);
                break;
            }

            case 0x71: {
                // 2026-05-07: Party keepalive — server pings, client ACKs [C1][03][71].
                NetLog("NET:  → 0x71 PartyKeepalive");
                BYTE ack[3] = { 0xC1, 0x03, 0x71 };
                if (DAT_055ca168 != 0xFFFFFFFF) {
                    ::send(DAT_055ca168, (const char*)ack, 3, 0);
                }
                break;
            }

            case 0x81: {
                NetLog("NET:  -> 0x81 StorageGold size=%d", Size);
                if (Size >= 12 && DAT_07cf1ffc != 0) {
                    BYTE result = Msg[3];
                    if (result != 0) {
                        DWORD storageGold = *(DWORD*)(Msg + 4);
                        DWORD gold = *(DWORD*)(Msg + 8);
                        BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
                        *(DWORD*)(charMachine + 1356) = storageGold;
                        *(DWORD*)(charMachine + 1352) = gold;
                    }
                }
                break;
            }

            case 0x82: {
                NetLog("NET:  -> 0x82 StorageExit");
                DAT_07eaa119 = 0;
                DAT_00559f5f = 0;
                DAT_07eaa14c = 0;
                break;
            }

            case 0x83: {
                NetLog("NET:  -> 0x83 StorageStatus size=%d", Size);
                if (Size >= 4) {
                    BYTE status = Msg[3];
                    switch (status) {
                    case 0:
                        DAT_00559f5f = 0;
                        DAT_07eaa148 = 0;
                        break;
                    case 1:
                        DAT_00559f5f = 1;
                        DAT_07eaa148 = 0;
                        break;
                    case 12:
                        DAT_00559f5f = 1;
                        DAT_07eaa148 = 1;
                        break;
                    default:
                        break;
                    }
                }
                break;
            }

            case 0x86: {
                NetLog("NET:  -> 0x86 MixResult idx=%d size=%d", Size >= 4 ? Msg[3] : -1, Size);
                if (DAT_07eaa11a != 0 && DAT_07eaa140 == 0) {
                    DAT_07eaa140 = 1;
                }
                break;
            }

            case 0x87: {
                NetLog("NET:  -> 0x87 MixExit");
                DAT_07eaa11a = 0;
                DAT_07eaa140 = 0;
                break;
            }

            case 0x73: {
                // 2026-05-07: Party char-sync or BGM notification.
                // pkt_len == 0 → resend F1/01 char-sync (party join ACK)
                // pkt_len != 0 → BGM track name in pkt+4 (4-byte string ptr)
                NetLog("NET:  → 0x73 PartyCharSync size=%d", Size);
                extern void Party_CharSync(BYTE* pkt, int pkt_len);
                Party_CharSync((BYTE*)Msg, Size);
                break;
            }

            case 0x90: {  // Guild create result
                NetLog("NET:  → 0x90 Guild_CreateOk");
                extern void Guild_CreateOk(BYTE* pkt);
                Guild_CreateOk((BYTE*)Msg);
                break;
            }
            case 0x91: {  // Guild add member result
                NetLog("NET:  → 0x91 Guild_AddMemberResult");
                extern void Guild_AddMemberResult(BYTE* pkt);
                Guild_AddMemberResult((BYTE*)Msg);
                break;
            }
            case 0x93: {  // Guild member list
                NetLog("NET:  → 0x93 Guild_MemberList");
                extern void Guild_MemberList(BYTE* pkt);
                Guild_MemberList((BYTE*)Msg);
                break;
            }
            case 0x94: {  // Guild char-select result
                NetLog("NET:  → 0x94 Guild_CharSelectResult");
                extern void Guild_CharSelectResult(BYTE* pkt);
                Guild_CharSelectResult((BYTE*)Msg);
                break;
            }
            case 0x95: {  // Guild update pos
                NetLog("NET:  → 0x95 Guild_UpdatePos");
                extern void Guild_UpdatePos(BYTE* pkt);
                Guild_UpdatePos((BYTE*)Msg);
                break;
            }
            case 0x96: {  // Guild set target pos
                NetLog("NET:  → 0x96 Guild_SetTargetPos");
                extern void Guild_SetTargetPos(BYTE* pkt);
                Guild_SetTargetPos((BYTE*)Msg);
                break;
            }
            case 0x99: {  // Guild join toggle
                NetLog("NET:  → 0x99 Guild_JoinToggle");
                extern void Guild_JoinToggle(BYTE* pkt);
                Guild_JoinToggle((BYTE*)Msg);
                break;
            }

            case 0x9C: {
                // ReceiveDieExpLarge @ 0x42E5C0 — el paquete que MuEmu manda de
                // verdad al morir un mob: GCMonsterDieSend (Protocol.cpp:1811)
                // usa PMSG_REWARD_EXPERIENCE_SEND con header.setE(0x9C).
                // (El 0x16 / PMSG_MONSTER_DIE_SEND del Protocol.h no se usa.)
                //
                // Layout con el padding del struct — el cliente lo lee EXACTO:
                //   +3,+4  BYTE  index[2]        (bit 0x8000 = el muerto era player)
                //   +5     padding
                //   +6..9  WORD  experience[2]   (HW en +6, LW en +8)
                //   +10,11 BYTE  damage[2]
                //   +12..  ViewDamageHP / ViewExperience / ViewNextExperience
                //
                // IDA L102-123:
                //   v28 = Rb[4] + (Rb[3] << 8);
                //   v83 = *(WORD*)(Rb+8) + (*(WORD*)(Rb+6) << 16);   // exp 32-bit
                //   v30 = Rb[11] + (Rb[10] << 8);                    // damage
                //   Index = FindCharacterIndex(v28 & 0x7FFF);
                //   if (v28 & 0x8000) { SetPlayerDie(c); CreatePoint(...); }
                //   else { Hero+756 = 2; Hero+758 = v30; Hero+784 = Index;
                //          CreatePoint(...); }
                //   c+765 = 1; c+748 = 0;
                //   CharacterAttribute+16 += v83;
                //   if (v83 > 0) { sprintf(GlobalText[486], v83); chat log }
                //
                // `Hero+756 = 2` es lo que dispara las DOS esferas de EXP que
                // van del mob al jugador: MoveCharacter (0x449900 L2450) hace
                //   if (c+756 == 2) { CreateJoint(1258, ..., subtype 0);
                //                     CreateJoint(1258, ..., subtype 1); }
                // y limpia +756/+758 al final del bloque. Ese consumidor ya
                // estaba portado (SecondPassword.cpp:3980) — faltaba el writer.
                if (Size < 12) { NetLog("NET:  → 0x9C DieExp size=%d (corto)", Size); break; }
                {
                    const int   key   = Msg[4] + (Msg[3] << 8);
                    const DWORD exp   = (DWORD)(*(WORD*)(Msg + 8))
                                      + ((DWORD)(*(WORD*)(Msg + 6)) << 16);
                    const int   dmg   = Msg[11] + (Msg[10] << 8);
                    const int   index = FUN_0045ac80(key & 0x7FFF);

                    NetLog("NET:  → 0x9C DieExp key=%04X idx=%d exp=%u dmg=%d",
                           key & 0x7FFF, index, exp, dmg);

                    float color[3] = { 1.0f, 0.6f, 0.0f };   // naranja
                    if (index >= 0 && index < 400 && DAT_07abf5d0) {
                        BYTE* c = (BYTE*)(uintptr_t)DAT_07abf5d0 + 916 * index;
                        if (key & 0xFFFF8000) {
                            // Murio un PLAYER (PvP): anim de muerte, sin EXP.
                            extern void __cdecl FUN_00444d90(int c_in);
                            FUN_00444d90((int)(intptr_t)c);
                        } else if (DAT_07abf5d8) {
                            BYTE* hero = (BYTE*)DAT_07abf5d8;
                            *(BYTE*) (hero + 756) = 2;        // gate de las esferas
                            *(WORD*) (hero + 758) = (WORD)dmg;
                            *(WORD*) (hero + 784) = (WORD)index;
                        }
                        CreatePoint((float*)(c + 16), dmg, color, 15.0f);
                        *(BYTE*)(c + 765) = 1;   // dead_flag (+0x2FD)
                        *(BYTE*)(c + 748) = 0;
                    }

                    if (CharacterAttribute)
                        *(DWORD*)((BYTE*)(uintptr_t)CharacterAttribute + 16) += exp;

                    if ((int)exp > 0) {
                        char Buffer[100];
                        sprintf(Buffer, GlobalText[486], exp);
                        UIChatLogWindow_AddText(nullptr, Buffer, 1);
                    }
                }
                break;
            }

            case 0x1A: {
                // 2026-05-07: ReceiveMagicPosition @ 0x0042D780 (port FIEL).
                // Server broadcasts a magic-area-attack:
                //   Msg[3..4] = caster entity ID (BE)
                //   Msg[5..6] = ID del skill mágico (BE) (se usa para el efecto visual)
                //   Msg[7]    = direction byte (unused in viz)
                //   Msg[8]    = target count
                //   Msg[9+i*2..10+i*2] = target IDs (BE)
                // Por cada objetivo: anim de shock + popup de daño.
                if (Size < 9) break;
                WORD casterId = ((WORD)Msg[3] << 8) | Msg[4];
                BYTE count = Msg[8];
                NetLog("NET:  → 0x1A MagicPosition caster=%d targets=%d", casterId, count);

                // Find caster, set magic-cast animation
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                BYTE* caster = nullptr;
                for (int s = 0; s < 400; ++s) {
                    BYTE* sp = basePtr + s * 0x394;
                    if (sp[0] && *(WORD*)(sp + 0x1dc) == casterId) {
                        caster = sp; break;
                    }
                }
                if (caster) {
                    CreateMagicShiny97k(caster, 0);
                    caster[0x106] = caster[0x105];
                    caster[0x105] = 90;            // cast anim (action 90)
                    *(float*)(caster + 0x108) = 0.0f;
                    caster[770] = Msg[5];           // ReceiveMagicPosition
                    caster[0x2F5] = 1;             // attack pending
                    PlayBuffer(88, 0, 0);          // magic cast sound
                }

                // Iterate targets — apply shock + damage popup
                if (Size >= 9 + (int)count * 2) {
                    for (int i = 0; i < count; ++i) {
                        WORD tgtId = ((WORD)Msg[9 + i*2] << 8) | Msg[10 + i*2];
                        BYTE* tgt = nullptr;
                        if (tgtId == g_HeroKey && DAT_07abf5d8) {
                            tgt = (BYTE*)DAT_07abf5d8;
                        } else {
                            for (int s = 0; s < 400; ++s) {
                                BYTE* sp = basePtr + s * 0x394;
                                if (sp[0] && *(WORD*)(sp + 0x1dc) == tgtId) {
                                    tgt = sp; break;
                                }
                            }
                        }
                        if (!tgt) continue;
                        // Daño cacheado en +760 (0x2F8) por el 0x15 — se usa como visual
                        WORD dmg = *(WORD*)(tgt + 760);
                        // ReceiveMagicPosition tira el mismo 50/50 de reacción
                        // que usaba el original antes de dibujar el daño en área.
                        const unsigned r = (unsigned)rand() & 0x80000001u;
                        const bool shock = (r == 0) ||
                                           ((int)r < 0 &&
                                            ((((char)r - 1) | (int)0xFFFFFFFE) == -1));
                        if (shock) FUN_00444b60((int)(uintptr_t)tgt, dmg);
                        if (dmg) {
                            float pos[3];
                            pos[0] = *(float*)(tgt + 0x10);
                            pos[1] = *(float*)(tgt + 0x14);
                            pos[2] = *(float*)(tgt + 0x18);
                            float color[3] = { 1.0f, 0.6f, 0.0f };
                            CreatePoint(pos, (int)dmg, color, 15.0f);
                        }
                        // Hero damaged: decrement HP
                        if (tgtId == g_HeroKey && DAT_07cf1ff4) {
                            WORD* pHP = (WORD*)((BYTE*)(uintptr_t)DAT_07cf1ff4 + 28);
                            if (dmg && dmg <= *pHP) *pHP -= dmg;
                            else if (dmg) *pHP = 0;
                        }
                    }
                }
                break;
            }

            case 0x1c: {
                // Teleport response compatibility: use MuEmu's active server
                // format, not an assumed original-server packet variation.
                // MuEmu GameServer/Move.h::PMSG_TELEPORT_SEND is:
                // C3:size:1C gate,map,x,y,dir (8 bytes). For Teleport skill 6
                // Move.cpp manda gate=0 después de su broadcast normal de skill 0x19.
                // IDA ReceiveTeleport supplies the visual semantics below.
                if (Size < 8 || !Hero) {
                    NetLog("NET: 0x1C Teleport malformed size=%d", Size);
                    break;
                }

                const BYTE gate = Msg[3], map = Msg[4], gridX = Msg[5];
                const BYTE gridY = Msg[6], direction = Msg[7];
                NetLog("NET: 0x1C Teleport gate=%u map=%u xy=(%u,%u) dir=%u",
                       (unsigned)gate, (unsigned)map, (unsigned)gridX,
                       (unsigned)gridY, (unsigned)direction);

                BYTE* hero = (BYTE*)(uintptr_t)Hero;
                const float worldX = ((float)gridX + 0.5f) * 100.0f;
                const float worldY = ((float)gridY + 0.5f) * 100.0f;
                *(float*)(hero + 16) = worldX;
                *(float*)(hero + 20) = worldY;

                // ReceiveTeleport @ 00428210 calcula la altura del piso en
                // el tile autorizado por el server. Su rama de montado/zona segura
                // preserva el offset vertical original antes de terminar el
                // teleport animation.
                float worldZ = FUN_004f7500(worldX, worldY);
                if (World != -1 && *(short*)(hero + 696) == 819 && !hero[846])
                    worldZ += (World == 8 || World == 10) ? 90.0f : 30.0f;
                *(float*)(hero + 24) = worldZ;
                *(float*)(hero + 788) = worldX;
                *(float*)(hero + 792) = worldY;
                *(DWORD*)(hero + 0x388) = gridX;
                *(DWORD*)(hero + 0x38c) = gridY;
                *(DWORD*)(hero + 904) = gridX;
                *(DWORD*)(hero + 908) = gridY;
                hero[0x306] = gridX;
                hero[0x307] = gridY;
                *(float*)(hero + 36) = ((float)(direction & 0x0F) - 1.0f) * 45.0f;

                if (gate != 0) {
                    // ReceiveTeleport's gate branch clears the old viewport
                    // antes de aceptar el par mapa/posición del server. Esto
                    // es deliberadamente distinto del Teleport de mago: los gates
                    // no llaman a CreateTeleportEnd; recargan el mundo
                    // cuando hace falta y esperan los paquetes de viewport nuevos.
                    ClearItems();
                    ClearCharacters((int)DAT_05826cac);

                    if (map != (BYTE)World) {
                        World = map;
                        FUN_0050e5a0();

                        // OpenWorld replaces terrain data, so IDA evaluates
                        // la altura de aterrizaje una segunda vez contra el mapa nuevo.
                        worldZ = FUN_004f7500(worldX, worldY);
                        if (World != -1 && *(short*)(hero + 696) == 819 && !hero[846])
                            worldZ += (World == 8 || World == 10) ? 90.0f : 30.0f;
                        *(float*)(hero + 24) = worldZ;
                    }

                    // ── ACK de fin de carga: C1 04 F3 12 ──────────────────
                    // 2026-08-15 BUG-FIX (el `/move` sólo funcionaba una vez y
                    // el mapa nuevo quedaba sin NPCs ni mobs).
                    //
                    // IDA `ReceiveTeleport` @0x428210, dentro del branch de gate
                    // y DESPUÉS de OpenWorld, arma y envía un paquete
                    // (`v118[2]=0xC1 v118[3]=1 v118[4]=0xF3` + chain-XOR) y
                    // recién entonces setea `LoadingWorld = 30`. Nuestro port
                    // hacía el ClearItems/ClearCharacters/OpenWorld pero nunca
                    // enviaba el ACK.
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
                        Net_SendSmallPacket(ackPkt, 4);
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
                    DAT_05826d04 = 0;
                    DAT_07e11d28 = 0;                 // MouseUpdateTime
                    DAT_00559bec = 6;                 // MouseUpdateTimeMax
                    InventoryOpened = 0;
                    ShopOpened = 0;
                    WarehouseOpened = 0;
                    DAT_00559f5f = 0;
                    DAT_07eaa14c = 0;
                    TradeOpened = 0;
                    EventWindowOpened = 0;
                    FUN_00460dc0(1265, (float*)(hero + 16), (float*)(hero + 28),
                                 (float*)(hero + 232), nullptr, (float*)hero,
                                 (float*)-1, nullptr, 0);
                    *(DWORD*)(hero + 0x168) = 0;
                    DAT_083a3ff0 = 0;                  // EnableEvent
                    DAT_00559c48 = -1;                 // SelectedItem
                    DAT_00559c4c = -1;                 // SelectedNpc
                    DAT_00559c50 = -1;                 // SelectedCharacter
                    DAT_00559c54 = -1;                 // SelectedOperate
                    DAT_00559c58 = -1;                 // Attacking
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
                    DAT_05826d04 = 0;
                }

                // Este store es común a las dos ramas en el original.
                hero[748] = 0;
                FUN_004430c0((int)(uintptr_t)hero);
                break;
            }

            case 0x23: {
                // 2026-05-07: ReceiveDropItem @ 0x0042F690 (port FIEL).
                // Server response after hero drops/moves an item.
                //   Msg[3] == 0  → drop FAILED → reset inventory drag UI
                //   Msg[3] != 0  → drop OK
                //     If Msg[4] >= 12 → equipment slot update (UI_Main path)
                //     Else            → inventory slot drop (CharacterMachine update)
                //   En los dos casos de éxito: limpia DAT_07e91388 (arrastre activo)
                //   and reset SendDropItem = -1
                if (Size < 4) break;
                BYTE result = Msg[3];
                NetLog("NET:  → 0x23 DropItem result=%d slot=%d", result, Size >= 5 ? Msg[4] : -1);
                // 2026-07-27 FIX: el item se sacó del slot de origen al hacer
                // pickup (UI_Main limpia el footprint). Por eso:
                //  - result==0 (drop rechazado por el server): hay que
                //    RESTAURAR el item a su slot de origen o desaparece de la
                //    vista (el server no lo removió). El port anterior sólo
                //    limpiaba el cursor → item perdido visualmente.
                //  - result!=0 (drop OK): el server removió el item; sólo hay
                //    que soltar el cursor. El port anterior escribía a globales
                //    equivocados (DAT_07ea9328 / CharacterMachine+552) con lógica
                //    slot>=12 invertida → corrupción; el slot ya estaba vacío
                //    desde el pickup así que esos writes eran innecesarios.
                if (result == 0) {
                    ItemMove_RestoreSlot(OffsetInventoryItems, (int)DAT_07ea5b18,
                                         (BYTE*)DAT_07e91350);
                } else {
                    PlayBuffer(29, 0, 0);
                }
                ItemMove_ClearPickedState();
                DAT_07e91388 = 0;
                DAT_07e11990 = -1;  // SendDropItem = -1
                break;
            }

            case 0x22: {
                // 2026-05-07: ReceiveGetItem @ 0x0042F360 (port FIEL).
                // Respuesta del server cuando el héroe levanta un item del piso vía el envío 0x22.
                //   Msg[3] == 0xFF → pickup failed (no inventory space, etc.)
                //   Msg[3] == 0xFE → levanta zen; el monto es el DWORD BE en Msg[4..7]
                //                    stored at CharacterMachine + 0x548
                //   si no          → item en el slot Msg[3], datos en Msg[4..]
                if (Size < 4) break;
                BYTE slot = Msg[3];
                NetLog("NET:  → 0x22 GetItem slot=0x%02x", slot);
                // IDA arma un puntero `Item` en cada rama y DESPUÉS decide el
                // sonido una sola vez con ConvertItemType(Item) — ver el bloque
                // compartido al final del case.
                const BYTE* Item = nullptr;
                if (slot == 0xFF) {
                    // Pickup failed
                } else if (slot == 0xFE) {
                    if (Size >= 8 && DAT_07cf1ffc != 0) {
                        DWORD gold = ((DWORD)Msg[4] << 24) | ((DWORD)Msg[5] << 16)
                                   | ((DWORD)Msg[6] << 8)  |  (DWORD)Msg[7];
                        BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
                        *(DWORD*)(charMachine + 0x548) = gold;
                    }
                    // IDA ReceiveGetItem L38-40: en la rama del zen `Item` queda
                    // apuntando a CharacterMachine, no a los bytes del paquete.
                    // Es una rareza del binario, pero es lo que hace: el
                    // ConvertItemType de abajo termina leyendo los primeros bytes
                    // de la struct del personaje, casi nunca da un tipo de joya y
                    // por eso el zen suena con pGetItem.wav (29).
                    Item = (const BYTE*)(uintptr_t)DAT_07cf1ffc;
                } else {
                    // 2026-07-27 FIX (item levantado no aparecía en inventario):
                    // PMSG_ITEM_GET_SEND es [C3][08][22][result][i0..i3] = Size 8
                    // (ItemInfo son 4 bytes, MAX_ITEM_INFO). El gate `Size >= 16`
                    // (formato de 12 bytes) NUNCA se cumplía → el item se
                    // levantaba en el server pero jamás se insertaba en el grid.
                    // Mismo bug que tenía el buy 0x32. InsertInventoryItem lee
                    // hasta Item[4]; dejamos ext=0.
                    if (Size >= 8 && slot < 76) {
                        BYTE itembytes[6] = { 0, 0, 0, 0, 0, 0 };
                        memcpy(itembytes, (BYTE*)Msg + 4, 4);
                        FUN_004cc660(OffsetInventoryItems, 8, 8, (int)slot, itembytes, 1);
                    }
                    if (Size >= 8) Item = (const BYTE*)Msg + 4;   // ConvertItemType lee hasta Item[3]
                }

                // Sonido de pickup — IDA L61-71, compartido por las dos ramas:
                //   ConvertItemType(Item) in {461,462,464,399,470} → 49 (eGem.wav)
                //   resto                                          → 29 (pGetItem.wav)
                // 2026-08-21: la rama del zen tenía PlayBuffer(49) hardcodeado
                // ("jewel pickup sound"), que es invención del port — al levantar
                // zen sonaba la joya en vez del pickup normal.
                if (slot != 0xFF && Item != nullptr) {
                    // ConvertItemType (0x0047B110): Item[0] + (Item[3] & 0x80) * 2
                    int type = (int)Item[0] + ((Item[3] & 0x80) ? 256 : 0);
                    bool jewel = (type == 461 || type == 462 || type == 464 ||
                                  type == 399 || type == 470);
                    PlayBuffer(jewel ? 49 : 29, (DWORD)(uintptr_t)Hero, 0);
                }
                DAT_07e11998 = -1;  // SendGetItem
                break;
            }

            case 0x26: {
                // 2026-05-07: ReceiveLife @ 0x00431780 (port FIEL).
                // Server pushes HP / MaxHP updates and item-durability decrements
                // por este opcode. El sub-byte en Msg[3] elige:
                //   0xFD     → EnableUse = 0 (item slot lock)
                //   0xFE     → MaxLife (CharacterAttribute+32) = WORD BE
                //   0xFF     → Life    (CharacterAttribute+28) = WORD BE
                //   else     → Inventory slot N decrement: ItemAttribute[N].Durability--
                if (Size < 6) break;
                BYTE sub = Msg[3];
                NetLog("NET:  → 0x26 Life sub=0x%02x", sub);
                if (DAT_07cf1ff4 == 0) break;
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
                break;
            }

            case 0x27: {
                // 2026-05-07: ReceiveMana @ 0x00431A90 (port FIEL).
                // Server pushes Mana / BP / MaxMana / MaxBP updates.
                //   0xFE → MaxMana(offset 34) + MaxBP(offset 38), cada uno WORD BE
                //   0xFF → Mana(offset 30)    + BP(offset 36),    cada uno WORD BE
                if (Size < 8) break;
                BYTE sub = Msg[3];
                NetLog("NET:  → 0x27 Mana sub=0x%02x", sub);
                if (DAT_07cf1ff4 == 0) break;
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
                break;
            }

            case 0x28: {
                // 2026-06-02: borrado de slot del lado del server. Lo usa mucho
                // stackable moves (source slot emptied after merge).
                if (Size < 5) break;
                BYTE slot = Msg[3];
                NetLog("NET:  -> 0x28 ItemDelete slot=%d flag=%d", slot, Msg[4]);
                if (slot < 76) {
                    UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
                }
                SeedQuickPotionTypesFromInventory();
                PlayBuffer(29, 0, 0);
                break;
            }

            case 0x2A: {
                // 2026-06-02: actualización de durabilidad/cantidad del lado del server. La usa
                // stackable potions/jewels after partial merge.
                if (Size < 6) break;
                BYTE slot = Msg[3];
                BYTE durability = Msg[4];
                BYTE flag = Msg[5];
                NetLog("NET:  -> 0x2A ItemDur slot=%d dur=%d flag=%d", slot, durability, flag);

                if (slot < 12) {
                    BYTE* equip = ItemMove_GetEquipSlotPtr((int)slot);
                    if (equip && *(short*)equip != (short)0xFFFF) {
                        equip[26] = durability;
                        if (durability == 0) {
                            UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
                        }
                    }
                } else if (slot < 76) {
                    if (durability == 0) {
                        UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
                    } else {
                        ItemMove_UpdateInventoryDurability((int)slot, durability);
                    }
                }

                if (flag != 0) {
                    EnableUse = 0;
                }
                SeedQuickPotionTypesFromInventory();
                break;
            }

            case 0x2F: {
                // 2026-05-07: ReceiveDurability @ 0x00431EA0 (port FIEL).
                // Per-equipment-slot durability update.
                //   Msg[3] = inventory slot index (0..11)
                //   Msg[4] = new durability value
                //   Msg[5] = if non-zero, EnableUse = 0 (item just consumed/broke)
                // El original escribe en CharacterMachine + 68*slot + 562, que es
                // CharacterMachine.EquipmentSlots[slot].Durability.
                if (Size < 6) break;
                BYTE slot = Msg[3];
                BYTE durVal = Msg[4];
                BYTE consumed = Msg[5];
                NetLog("NET:  → 0x2F Durability slot=%d dur=%d consumed=%d", slot, durVal, consumed);
                if (DAT_07cf1ffc != 0 && slot < 12) {
                    BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
                    *(charMachine + 68 * slot + 562) = durVal;
                }
                if (consumed) {
                    EnableUse = 0;
                }
                break;
            }

            case 0x5A: {
                if (Msg[0] == 0xC2 && Size >= 5) {
                    const BYTE count = Msg[4];
                    if (Size < 5 + (int)count * 42) break;
                    for (int i = 0; i < count; ++i) {
                        const BYTE* entry = Msg + 5 + i * 42;
                        Guild_UpsertRecord((entry[0] << 8) | entry[1],
                                           entry + 2, entry + 10);
                    }
                    NetLog("NET: GuildViewport batch count=%u", (unsigned)count);
                    break;
                }
                // 2026-05-07: Trade/Shop entity association (Shop_EntitySlots).
                // PacketHandler_0x5a in Trade.cpp.
                NetLog("NET:  → 0x5A Shop/Trade slots");
                extern void PacketHandler_0x5a(BYTE* pkt);
                PacketHandler_0x5a((BYTE*)Msg);
                break;
            }
            case 0x5C: {
                if (Size >= 45) {
                    const WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
                    const int row = Guild_UpsertRecord(-1, Msg + 5, Msg + 13);
                    const int entitySlot = FUN_0045ac80(entityKey);
                    BYTE* base = (BYTE*)(uintptr_t)DAT_07abf5d0;
                    if (base && entitySlot >= 0 && entitySlot < 400)
                        *(short*)(base + entitySlot * 916 + 474) = (short)row;
                    NetLog("NET: GuildViewport entity=%u row=%d",
                           (unsigned)entityKey, row);
                    break;
                }
                // 2026-05-07: Trade clear (PacketHandler_0x5c in Trade.cpp).
                NetLog("NET:  → 0x5C Trade clear");
                extern void PacketHandler_0x5c(BYTE* pkt);
                PacketHandler_0x5c((BYTE*)Msg);
                break;
            }
            case 0x5D: {
                if (Size == 5) {
                    const WORD entityKey = (WORD)((Msg[3] << 8) | Msg[4]);
                    const int entitySlot = FUN_0045ac80(entityKey);
                    BYTE* base = (BYTE*)(uintptr_t)DAT_07abf5d0;
                    if (base && entitySlot >= 0 && entitySlot < 400)
                        *(short*)(base + entitySlot * 916 + 474) = -1;
                    NetLog("NET: GuildViewportDelete entity=%u", (unsigned)entityKey);
                    break;
                }
                // 2026-05-07: Trade/Shop item slot clear.
                NetLog("NET:  → 0x5D Trade slot clear");
                extern void PacketHandler_0x5d(BYTE* pkt);
                PacketHandler_0x5d((BYTE*)Msg);
                break;
            }
            case 0x60: {
                // 2026-05-07: Trade request result.
                NetLog("NET:  → 0x60 Trade_RequestResult");
                extern void Trade_RequestResult(BYTE* pkt);
                Trade_RequestResult((BYTE*)Msg);
                break;
            }
            case 0x61: {
                // 2026-05-07: Incoming trade/duel request.
                NetLog("NET:  → 0x61 Trade_IncomingReq");
                extern void Trade_IncomingReq(BYTE* pkt);
                Trade_IncomingReq((BYTE*)Msg);
                break;
            }
            case 0x62: {
                // 2026-05-07: Trade window opened.
                NetLog("NET:  → 0x62 Trade_Open");
                extern void Trade_Open(BYTE* pkt);
                Trade_Open((BYTE*)Msg);
                break;
            }
            case 0x63: {
                // 2026-05-07: Trade item update / result.
                NetLog("NET:  → 0x63 Trade_ItemUpdate");
                extern void Trade_ItemUpdate(BYTE* pkt);
                Trade_ItemUpdate((BYTE*)Msg);
                break;
            }

            case 0x5B: {
                // EntityGuildList — el server reporta el índice de guild de cada entidad
                // del viewport. Per IDA sub_435110 @ 0x00435110.
                //   [C1][size][0x5B][count][stride 4: id_hi id_lo guild_hi guild_lo]
                //
                // Por cada entidad:
                //   guildId = busca Msg[+offset+0..1] (word BE) en la
                //             name table (g_szGuildName[] / byte_7E919BC).
                //   entity+474 = índice encontrado (o -1 si no está en la tabla).
                //   if (entity == Hero o su guild coincide con la del Hero): setea el flag de aliado.
                //
                // No tenemos la tabla de búsqueda de nombres de guild (la setea el 0x65); por
                // ahora copiamos el guild_idx directo del paquete (el server puede
                // already pre-resolve indexes).  Either way, Hero+474 gets
                // poblada → aparece la insignia de "en guild" en el panel C/G.
                if (Size < 5) break;
                BYTE count = Msg[4];
                NetLog("NET:  → 0x5B EntityGuildList count=%d", count);
                if (Size < 5 + count * 4) break;
                BYTE* basePtr = (BYTE*)(uintptr_t)DAT_07abf5d0;
                for (int i = 0; i < count; ++i) {
                    const BYTE* e = Msg + 5 + i * 4;
                    WORD entityId = (WORD)((e[0] << 8) | e[1]) & 0x7FFF;
                    const int guildKey = (e[2] << 8) | e[3];
                    // Find entity slot by id
                    BYTE* slot = nullptr;
                    if (entityId == g_HeroKey && DAT_07abf5d8) {
                        slot = (BYTE*)DAT_07abf5d8;
                    } else if (basePtr) {
                        for (int s = 0; s < 400; ++s) {
                            BYTE* sp = basePtr + s * 0x394;
                            if (sp[0] && (*(WORD*)(sp + 0x1dc) & 0x7FFF) == entityId) {
                                slot = sp; break;
                            }
                        }
                    }
                    if (slot) {
                        const int guildRow = Guild_FindRecordByKey(guildKey);
                        // 00435110 limpia el flag de relación antes de calcular
                        // ally/war state, so an old guild association cannot
                        // survive a viewport update.
                        slot[745] = 0;
                        // Una asociación 5B cuya clave no está en la 5A
                        // registry must clear a previous guild association.
                        *(short*)(slot + 474) = (short)guildRow;
                        if (guildRow < 0) continue;
                        // Flag de aliado si es del mismo guild que el héroe
                        if (DAT_07abf5d8 && slot != (BYTE*)DAT_07abf5d8) {
                            short heroGuild = *(short*)((BYTE*)DAT_07abf5d8 + 474);
                            if (heroGuild != -1 && guildRow == heroGuild) {
                                slot[745] = 1;  // ally
                            }
                        }
                    }
                }
                break;
            }

            case 0x65: {
                // ReceiveGuildList — Per IDA ReceiveGuildList @ 0x004348B0.
                //   [C1][size][0x65][_][_][count][_]...
                //   Msg[5] = member count
                //   Msg[8..11] = guild total score (DWORD)
                //   Msg[16..] = guild name (8 bytes?)
                //   Msg[27..] = members (12 bytes per entry: name(10), level/role(2))
                //
                // Actualiza la lista global de miembros del guild + el score. La
                // member row layout matches IDA — copies into byte_7E919BC
                // (tabla de datos de miembro con stride de 13 bytes). Hero+474 lo setea
                // aparte el 0x5B (arriba) cuando el server lo pre-resuelve,
                // O acá por coincidencia de nombre cuando cargan los nombres de guild.
                if (Size < 6) break;
                // Clamp igual que el 0x52: el loop de copia se corta en 16 pero
                // `g_nGuildMemberCount` es lo que consume el render, así que
                // dejarlo sin acotar (Msg[5] llega hasta 255) hacía que el panel
                // recorriera slots nunca escritos.
                g_nGuildMemberCount = (Msg[5] < 16) ? Msg[5] : 16;
                if (Size >= 12) {
                    int score = *(const int*)(Msg + 8);
                    GuildTotalScore = score < 0 ? 0 : score;
                }
                NetLog("NET:  → 0x65 GuildList members=%d score=%d",
                       g_nGuildMemberCount, GuildTotalScore);
                // Copia cada fila de miembro en byte_7E919BC[i*80] (per el layout de IDA)
                // Entrada de miembro en Msg+27+i*12, destino en byte_7E919BC[i*80].
                // Layout: [name 10B][level/role 2B] = 12 bytes per packet,
                // expandido a 80 bytes por slot en la memoria del cliente.
                if (g_nGuildMemberCount > 0 && Size >= 27 + g_nGuildMemberCount * 12) {
                    for (int i = 0; i < g_nGuildMemberCount && i < 16; ++i) {
                        const BYTE* src = Msg + 27 + i * 12;
                        char* dst = &byte_7E919BC[i * 80];
                        memcpy(dst, src, 12);
                        dst[12] = 0;  // null-terminate name
                    }
                }
                break;
            }

            case 0x0D: {
                // 2026-05-06 BUG-FIX MAYÚSCULO (port FIEL desde IDA
                // mu97k-src-IDA/raw/00427A00_ReceiveNotice.c):
                //
                // PMSG_NOTICE_SEND server layout (server source
                // Mu-linux-97K/Source/MuServer/GameServer/Notice.cpp):
                //   struct { PBMSG_HEAD header; BYTE type; char message[256]; }
                //   = [C1][size][0x0D][type][message null-terminated]
                //
                // type 0 → CreateNotice(msg, 0) — CENTERED blue/cyan banner con
                //          blink (FUN_0047fae0). Eventos del servidor (Blood
                //          Castle, Happy Hour, server close, etc).
                // type 1 → UIChatLogWindow_AddText — BLUE chat log local
                //          (mensajes personales: login welcome, errors, etc).
                // type 2 → guild notice (sprintf "Guild: %s" + CreateNotice
                //          gold). Deferred — guild stack no portado.
                //
                // ANTES: parser leía message desde Msg+7 (offset incorrecto, mal
                // port basado en formato 5.2 con extra fields). Y ruteaba TODO
                // a chat log → eventos aparecían en azul abajo-izquierda en vez
                // de centrados en pantalla. User reportó este bug 2026-05-06.
                if (Size < 5) {
                    NetLog("NET:  → 0x0D Notice size=%d (too small)", Size);
                    break;
                }
                BYTE type = Msg[3];
                // El mensaje arranca en el offset 4 (después de C1, size, 0x0D, type).
                // El server termina el buffer con NUL, pero copiamos con
                // length cap as a safety net.
                char text[256] = {0};
                int textLen = Size - 4;
                if (textLen <= 0) {
                    NetLog("NET:  → 0x0D Notice type=%d empty", type);
                    break;
                }
                if (textLen > (int)sizeof(text) - 1) textLen = sizeof(text) - 1;
                memcpy(text, Msg + 4, textLen);
                text[textLen] = 0;
                NetLog("NET:  → 0x0D Notice type=%d msg='%s'", type, text);

                if (type == 0) {
                    // Centered banner (blink cyan). Color flag stored at
                    // slot[0x104]; el renderer FUN_0047fce0 lo lee para cambiar
                    // between blink-cyan (flag=0) and gold (flag=1).
                    extern void __cdecl FUN_0047fae0(char*, unsigned char);
                    FUN_0047fae0(text, 0);
                } else if (type == 1) {
                    // Personal blue chat log entry.
                    extern void UIChatLogWindow_AddText(const char* strID,
                                                       const char* msg,
                                                       int color);
                    // IDA pasa una cadena vacía como strID, no NULL.
                    UIChatLogWindow_AddText("", text, 1);
                } else if (type == 2) {
                    // Guild notice — gold centered banner. Original IDA
                    // formats with GlobalText[483] ("Guild Notice: %s").
                    extern void __cdecl FUN_0047fae0(char*, unsigned char);
                    char guildText[320] = {0};
                    SetGuildNoticeText(text);
                    if (GlobalText[483] && GlobalText[483][0] != 0) {
                        wsprintfA(guildText, GlobalText[483], text);
                        FUN_0047fae0(guildText, 1);
                    } else {
                        FUN_0047fae0(text, 1);
                    }
                }
                break;
            }

            case 0x0F: {
                // 2026-05-07: port FIEL desde IDA ProtocolCore:554
                //   case 0xF:
                //     Weather = ReceiveBuffer[3];
                //     if (Weather >> 4) {
                //       if (Weather >> 4 == 1) RainTarget = 6 * (Weather & 0xF);
                //     } else RainTarget = 0;
                //
                // Server controls weather state per map. High nibble = weather
                // type (0=clear, 1=rain). Low nibble = intensity (0..15).
                if (Size >= 4) {
                    BYTE w = Msg[3];
                    BYTE wType = (w >> 4) & 0x0F;
                    BYTE wIntensity = w & 0x0F;
                    NetLog("NET:  → 0x0F Weather type=%d intensity=%d", wType, wIntensity);
                    if (wType == 1) {
                        // Lluvia — setea el global RainTarget si está definido
                        // Per IDA: RainTarget = 6 * intensity (= 0..90)
                        // Nuestro build: buscar un global similar. Por ahora no
                        // tenemos RainTarget específicamente, pero Weather.cpp
                        // usa el área DAT_07eaa178 para el estado del clima.
                        // Por ahora logueamos y salteamos; el clima va a andar vía MoveWeather.
                    }
                }
                break;
            }

            case 0x00: {
                // PMSG_CHAT_SEND server→client — [C1][size][0x00][name 10B][msg 60B]
                // 2026-07-19: delega al port FIEL `ReceiveChat` (IDA 0x427630).
                // El handler anterior aproximaba mal: pasaba nullptr como nombre y
                // pre-formateaba "name: msg" (IDA los pasa SEPARADOS — el renderLine
                // del ChatListBox compone "name: text"), usaba canal 0 en vez de 3
                // (color equivocado), y no hacía el dispatch de prefijos
                // ('~'=party/4, '@'=guild/5, '#'=solo burbuja) ni la burbuja
                // sobre el personaje (AssignChat).
                NetLog("NET:  → 0x00 Chat size=%d", Size);
                // GUARDA 2026-07-19: el server manda `C1 04 00 xx` (4 bytes) como
                // ping/handshake. ReceiveChat lee name@+3 y mensaje hasta +72, así que
                // con 4 bytes sobre-lee. El IDA solo ACKea esos en estado login
                // (g_GameState==2) y cae al parseo de chat en el resto → mismo
                // sobre-lectura. Procesamos como chat solo si el paquete tiene el
                // tamaño de PMSG_CHAT_SEND (3 hdr + 10 name + 60 msg = 73).
                extern void __cdecl ReceiveChat(BYTE* ReceiveBuffer);
                // FIX 2026-07-19: el guard era `Size >= 73` (tamaño MÁXIMO de la
                // struct). PMSG_CHAT_SEND es de longitud VARIABLE — el server hace
                //   header.set(0x00, sizeof(pMsg) - (sizeof(pMsg.message) - (size+1)))
                //   = 14 + strlen(mensaje)
                // así que solo un mensaje de 59 chars llegaba a 73. Todo mensaje más
                // corto se descartaba en silencio; en particular los `/post` azul (`~`)
                // y verde (`@`) de CommandManager::GCPostMessageBlue/Green.
                // Mínimo real = 3 (hdr) + 10 (name) + 1 (al menos un char) = 14.
                // El server null-termina el mensaje, así que la lectura de 60 bytes
                // que hace ReceiveChat (fiel a IDA) se corta sola en el NUL.
                if (Size >= 14 || DAT_005615c0 == 2) {
                    ReceiveChat(Msg);
                } else {
                    NetLog("NET:    0x00 too short (%d) — ping/handshake, no chat parse", Size);
                }
                break;
            }

            // ── 0x20 ViewportItem (ground items) ────────────────────────────
            // Port FIEL del IDA ReceiveCreateItemViewport @ 0x0042F240.
            // Per-entry stride 8 bytes (o 9 si Jewel of Chaos = type 0x1CF).
            // Spawnea cada item en DAT_07e12840 pool; el render lo hace
            // FUN_005038e0 (Entity_Render) que itera el pool por slots activos.
            case 0x20: {
                int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                int count = Msg[3 + hdrOff];
                NetLog("NET:  → 0x20 ViewportItem count=%d size=%d", count, Size);
                int cursor = 4 + hdrOff;
                // 2026-07-27: escribir sobre el ITEM-BASE (DAT_07e127f8), no
                // sobre DAT_07e12840 (= item-base+72). Los offsets de abajo son
                // ip-relativos de CreateItem (ip+4 type, ip+72 active, ip+88 pos);
                // con la base correcta el active queda en ip+72 = DAT_07e12840+0,
                // que es donde Entity_Render lo lee.
                BYTE* itemPool = (BYTE*)&DAT_07e12840[0];
                extern int  __cdecl FUN_00502ba0(int);   // ItemObjectAttribute
                extern void __cdecl FUN_005030c0(int);   // ItemAngle
                // 2026-07-27: este server (MuEmu) manda PMSG_VIEWPORT_ITEM =
                // index[2]+x+y+ItemInfo[MAX_ITEM_INFO+1] = 2+1+1+5 = 9 bytes por
                // item SIEMPRE (Viewport.h). El port usaba stride 8 (0.97k) salvo
                // type 463 → desalineaba todos los items después del 1ro. Bound y
                // stride ahora son 9.
                for (int i = 0; i < count && cursor + 9 <= Size; ++i) {
                    const BYTE* e = Msg + cursor;
                    WORD raw = (e[0] << 8) | e[1];
                    WORD key = raw & 0x7FFF;
                    bool createFlag = (raw & 0x8000) != 0;
                    BYTE gx = e[2], gy = e[3];
                    const BYTE* itemInfo = e + 4;
                    // ConvertItemType: type = info[0] + (info[3] & 0x80) * 2
                    int itemType = (int)itemInfo[0] + ((itemInfo[3] & 0x80) ? 256 : 0);

                    if (key >= 1000) key = 0;  // safety clamp per IDA

                    BYTE* ip = itemPool + (size_t)key * 0x204;

                    // Per IDA CreateItem layout:
                    //   ip+4   short type (raw, sin +400)
                    //   ip+8   int level/option (Item[1] o packed for type 463)
                    //   ip+30  byte exc_option (Item[2])
                    //   ip+31  byte ?           (Item[3])
                    //   ip+72  byte active_flag = 1
                    //   ip+74  short model_index = type+400 (con overrides)
                    //   ip+76  int unknown = 1
                    //   ip+88  float pos[3] (X, Y, Z)
                    //   ip+352..364 float scale (-12.5..-12.5..-12.5..25.0..25.0..25.0)
                    *(WORD*)(ip + 4) = (WORD)itemType;
                    if (itemType == 463) {
                        // Zen (GET_ITEM(14,15)): la CANTIDAD viaja en 24 bits
                        // repartidos en info[1] (bits 16-23), info[2] (8-15) e
                        // info[4] (0-7) — ver Viewport.cpp:926 del server.
                        // Queda en ip+8, que es de donde la leen Entity_Render
                        // (para el tamaño del montón de monedas) y RenderItemName
                        // (para el texto "Zen <cantidad>").
                        int v5 = (int)itemInfo[4] + (((int)itemInfo[2] + ((int)itemInfo[1] << 8)) << 8);
                        *(int*)(ip + 8) = v5;
                        ip[30] = 0;
                        ip[31] = 0;
                        if (createFlag) PlayBuffer(31, 0, 0);   // pDropMoney.wav
                    } else {
                        *(int*)(ip + 8) = (int)itemInfo[1];
                        ip[30] = itemInfo[2];
                        ip[31] = itemInfo[3];
                        if (createFlag) {
                            // IDA CreateItem L38-46: joyas y pergaminos suenan
                            // distinto que el resto.
                            if (itemType == 461 || itemType == 462 || itemType == 464 ||
                                itemType == 399 || itemType == 470)
                                PlayBuffer(49, (DWORD)(uintptr_t)(ip + 72), 0);
                            else
                                PlayBuffer(30, (DWORD)(uintptr_t)(ip + 72), 0);
                        }
                    }
                    ip[72] = 1;                                    // active flag
                    *(WORD*)(ip + 74) = (WORD)(itemType + 400);    // model index
                    *(int*)(ip + 76) = 1;

                    // Model overrides (CreateItem switch L58-126): arrows/fruit/etc
                    // cuyo modelo NO es type+400.
                    {
                        int lvl = *(int*)(ip + 8) >> 3;
                        if (itemType == 459) {                     // arrows
                            switch (lvl) {
                                case 1: *(WORD*)(ip + 74) = 951; break;
                                case 2: *(WORD*)(ip + 74) = 952; break;
                                case 3: *(WORD*)(ip + 74) = 953; break;
                                case 5: *(WORD*)(ip + 74) = 955; break;
                                case 6: *(WORD*)(ip + 74) = 956; break;
                                case 8: case 9: case 10: case 11: case 12:
                                        *(WORD*)(ip + 74) = 957; break;
                            }
                        } else if (itemType == 469) {
                            if (lvl == 1) *(WORD*)(ip + 74) = 958;
                        } else if (itemType == 435) {              // fruit
                            if (lvl == 0)      { *(WORD*)(ip + 74) = 570; *(int*)(ip + 8) = 0; }
                            else if (lvl == 1) { *(WORD*)(ip + 74) = 419; *(int*)(ip + 8) = 0; }
                            else if (lvl == 2) { *(WORD*)(ip + 74) = 546; *(int*)(ip + 8) = 0; }
                        } else if (itemType == 457 && lvl == 1) {
                            *(WORD*)(ip + 74) = 954;
                        }
                    }

                    // ItemObjectAttribute (CreateItem LABEL_33): setea atributos
                    // de render del objeto (ip+72). Sin esto el modelo puede
                    // quedar sin scale/flags → invisible.
                    FUN_00502ba0((int)(ip + 72));

                    // BoundingBox del objeto (IDA CreateItem L129-134):
                    // min = (-30,-30,-30), max = (30,30,30).
                    // 2026-08-21: el port tenía -12.5 / 25.0 (mal decodificados
                    // desde los literales -1041235968 / 1106247680).
                    *(int*)(ip + 352) = (int)0xC1F00000;  // -30.0f
                    *(int*)(ip + 356) = (int)0xC1F00000;
                    *(int*)(ip + 360) = (int)0xC1F00000;
                    *(int*)(ip + 364) = (int)0x41F00000;  //  30.0f
                    *(int*)(ip + 368) = (int)0x41F00000;
                    *(int*)(ip + 372) = (int)0x41F00000;

                    // World position: ((grid + 0.5) * 100.0)
                    *(float*)(ip + 88) = ((float)gx + 0.5f) * 100.0f;
                    *(float*)(ip + 92) = ((float)gy + 0.5f) * 100.0f;
                    // Z: terrain height (RequestTerrainHeight 0x004F7500).
                    extern float __cdecl FUN_004f7500(float, float);
                    float terrainH = FUN_004f7500(*(float*)(ip + 88), *(float*)(ip + 92));

                    // Caída del item recién dropeado (IDA CreateItem L139-172):
                    // nace por encima del suelo con velocidad Z en ip+288 y
                    // MoveItems (0x503760) lo hace caer y rebotar.  Sin esto el
                    // item aparecía clavado en el suelo, sin animación de drop.
                    // Omitido: los CreateEffect(250)/CreateEffect(248) de las
                    // flechas/bolts (modelos 955/956), que necesitan los args
                    // vec3 de CreateEffect.
                    if (createFlag) {
                        WORD model = *(WORD*)(ip + 74);
                        if (model == 955 || model == 956) {
                            *(int*)(ip + 288)   = (int)0x42480000;   // 50.0f
                            *(float*)(ip + 96)  = terrainH + 3.0f;
                        } else {
                            *(int*)(ip + 288)   = (int)0x41A00000;   // 20.0f
                            *(float*)(ip + 96)  = terrainH + 180.0f;
                        }
                    } else {
                        *(float*)(ip + 96) = terrainH;
                    }

                    // ItemAngle (CreateItem final): setea rotación del item en
                    // el suelo según el terreno.
                    FUN_005030c0((int)(ip + 72));

                    NetLog("NET:    0x20 item[%d] key=%u type=%d mine=%d pos=(%d,%d)",
                           i, key, itemType, createFlag, gx, gy);

                    // Stride fijo 9 (ItemInfo[5] en este server).
                    cursor += 9;
                }
                break;
            }

            // ── 0x21 ViewportItemDestroy (item picked up / disappeared) ─────
            case 0x21: {
                int hdrOff = (Msg[0] == 0xC1) ? 0 : 1;
                int count = Msg[3 + hdrOff];
                NetLog("NET:  → 0x21 ViewportItemDestroy count=%d", count);
                // 2026-05-05: limpiar slots en DAT_07e12840 pool. Per-entry
                // 2 bytes (key WORD, big-endian).
                int entryStart = 4 + hdrOff;
                BYTE* itemPool = (BYTE*)&DAT_07e12840;
                for (int i = 0; i < count && entryStart + i*2 + 1 < Size; ++i) {
                    WORD raw = (Msg[entryStart + i*2] << 8) | Msg[entryStart + i*2 + 1];
                    WORD key = raw & 0x7FFF;
                    if (key < 1000) {
                        itemPool[(size_t)key * 0x204 + 72] = 0;  // active flag = 0
                    }
                }
                break;
            }


            // ── 0xA0..0xA3 — sistema de quests ──────────────────────────────
            // 2026-08-21: los cuatro opcodes no estaban en el dispatcher (sólo
            // el comentario del mapa de opcodes al principio del archivo), asi
            // que el estado de quest nunca llegaba aunque los cuerpos ya
            // estuvieran portados.  ProtocolCore (0x4389A0 L1376-1387) los
            // manda a ReceiveQuestHistory / State / Result / Prize.
            case 0xA0: {   // ReceiveQuestHistory @ 0x00437450
                if (Size < 4 || g_csQuest == 0) break;
                NetLog("NET:  → 0xA0 QuestHistory num=%d", Msg[3]);
                // IDA lee `*(BYTE*)(Hero + 444)` directo, pero en nuestro
                // build este paquete llega ANTES de que exista la entidad del
                // heroe (en el mismo tick se ve "0x12 SKIP - hero not yet
                // allocated"), asi que Hero es 0.  Con Class = -1
                // setQuestLists NO escribe This+4, que queda en el 0xFF que le
                // pone el ctor — y FindQuestContext lo usa como indice de clase
                // para leer `pQuest + 44 + clase`, o sea se iba 255 bytes fuera
                // de la entrada, no encontraba contexto y caia en el LABEL_5
                // que decrementa el indice de quest (0 -> 255).  Ese era el
                // qIdx=255 del panel.
                // Fallback: CharacterAttribute+11, que el char-select ya dejo
                // seteado y es de donde Recv_JoinMapServer copia hero+444.
                int heroClass = -1;
                if (DAT_07abf5d8 != 0)
                    heroClass = *(BYTE*)((BYTE*)(uintptr_t)DAT_07abf5d8 + 444);
                else if (CharacterAttribute != nullptr)
                    heroClass = *(BYTE*)((BYTE*)CharacterAttribute + 11);
                // El destino (CSQuest + 0x1C848) son 0x32 bytes: IDA hace
                // memset de 48 + el WORD de +0x1C878.  El server manda
                // QuestInfo[50] con count = MAX_QUEST_LIST/4 = 48 (cada byte
                // lleva 4 quests de 2 bits, de ahi el `index >> 2` de
                // setQuestList).
                int num = Msg[3];
                if (num > 0x32) num = 0x32;
                if (Size < 4 + num) num = Size - 4;
                if (num > 0)
                    CSQuest__setQuestLists((int)g_csQuest, 0, (BYTE*)Msg + 4, num, heroClass);
                break;
            }

            case 0xA1: {   // ReceiveQuestState @ 0x00437480
                if (Size < 5 || g_csQuest == 0) break;
                NetLog("NET:  → 0xA1 QuestState index=%d result=%d", Msg[3], Msg[4]);
                CSQuest__setQuestList((int)g_csQuest, 0, Msg[3], Msg[4]);
                CSQuest__ShowQuestNpcWindow((void*)(uintptr_t)g_csQuest, 0, -1);
                break;
            }

            case 0xA2: {   // ReceiveQuestResult @ 0x004374B0
                if (Size < 6 || g_csQuest == 0) break;
                NetLog("NET:  → 0xA2 QuestResult index=%d err=%d state=%d",
                       Msg[3], Msg[4], Msg[5]);
                if (Msg[4] == 0) {          // IDA: sólo si el byte 4 es 0
                    CSQuest__setQuestList((int)g_csQuest, 0, Msg[3], Msg[5]);
                    CSQuest__ShowQuestNpcWindow((void*)(uintptr_t)g_csQuest, 0, -1);
                }
                break;
            }

            // ReceiveQuestPrize @ 0x004374E0 — premio de quest.
            // El decompile son 2579 bytes pero ~70% es ruido de hash-table
            // (anti-tamper) alrededor de CharacterMachine; omitido por policy.
            // Lógica real, con Msg[5] como sub-tipo:
            //   0xC8  puntos de stat  (CharacterAttribute+84 += Msg[6])
            //   0xC9  cambio de clase (entidad+444 y CharacterAttribute+11)
            //   0xCA  clase + puntos
            //   0xCB  clase, sin puntos
            case 0xA3: {
                if (Size < 7) break;
                int key = ((Msg[4] + (Msg[3] << 8)) & 0x7FFF);
                int idx = FUN_0045ac80(key);
                NetLog("NET:  → 0xA3 QuestPrize sub=0x%02X key=%d idx=%d val=%d",
                       Msg[5], key, idx, Msg[6]);
                if (idx < 0 || idx >= 400) break;
                if (DAT_07abf5d0 == 0) break;

                BYTE *c    = (BYTE*)(uintptr_t)DAT_07abf5d0 + (size_t)idx * 916;
                BYTE *hero = (BYTE*)(uintptr_t)DAT_07abf5d8;
                BYTE *attr = (BYTE*)(uintptr_t)DAT_07cf1ff4;
                // Clase codificada igual que en la lista de personajes:
                //   ((b >> 4) | (b & 0x10)) >> 1
                BYTE klass = (BYTE)((((Msg[6] >> 4) | (Msg[6] & 0x10))) >> 1);

                if (Msg[5] == 0xC8) {
                    if (c == hero && attr) {
                        // Con GAMESERVER_EXTRA el server manda el LevelUpPoint
                        // ya resuelto en ViewPoint (+8, sizeof = 12).  Igual que
                        // con el F3/06 de stats y el 0x15 de damage, ese valor
                        // es el autoritativo; el `+=` es el camino del 0.97k.
                        if (Size >= 12) *(WORD*)(attr + 84) = (WORD)*(const DWORD*)(Msg + 8);
                        else            *(WORD*)(attr + 84) += Msg[6];
                    }
                    // 15 chispas + destello, sin rebuild del personaje
                    for (int n = 0; n < 15; ++n)
                        FUN_0046d840(1249, (float*)(c + 16), (float*)(c + 16),
                                     (float*)(c + 28), 0, (int)(uintptr_t)c,
                                     40.0f, 2, 0);
                    FUN_00460dc0(1264, (float*)(c + 16), (float*)(c + 28),
                                 (float*)(c + 232), nullptr, (float*)c,
                                 (float*)-1, nullptr, 0);
                    PlayBuffer(71, 0, 0);
                    break;
                }

                if (Msg[5] == 0xC9 || Msg[5] == 0xCA || Msg[5] == 0xCB) {
                    *(BYTE*)(c + 444) = klass;
                    if (c == hero && attr) {
                        *(BYTE*)(attr + 11) = klass;
                        // Sólo el sub-tipo 0xCA suma puntos, y usa EL MISMO
                        // byte que la clase.  Verificado a nivel instrucción
                        // (0x437B9x: `mov [eax+0Bh], cl` seguido de
                        // `movzx cx, [edx+6]` / `add [eax+54h], cx`), no sólo
                        // en el decompile.  Raro, pero es lo que hace el
                        // binario; el server es autoritativo igual y reenvía
                        // el LevelUpPoint real.
                        if (Msg[5] == 0xCA) {
                            if (Size >= 12) *(WORD*)(attr + 84) = (WORD)*(const DWORD*)(Msg + 8);
                            else            *(WORD*)(attr + 84) += Msg[6];
                        }
                    }
                    float up[3] = { *(float*)(c + 16), *(float*)(c + 20),
                                    *(float*)(c + 24) + 200.0f };
                    for (int n = 0; n < 15; ++n) {
                        FUN_0046d840(1249, (float*)(c + 16), (float*)(c + 16),
                                     (float*)(c + 28), 0, (int)(uintptr_t)c,
                                     40.0f, 2, 0);
                        FUN_0046d840(1249, up, up,
                                     (float*)(c + 28), 10, (int)(uintptr_t)c,
                                     40.0f, 2, 0);
                    }
                    FUN_00460dc0(1264, (float*)(c + 16), (float*)(c + 28),
                                 (float*)(c + 232), nullptr, (float*)c,
                                 (float*)-1, nullptr, 0);
                    FUN_0045c720((int)(uintptr_t)c);   // rebuild de body-parts
                    FUN_0043e820((int)(uintptr_t)c, 124);
                    PlayBuffer(72, 0, 0);
                }
                break;
            }

            // ── Character config opcodes (DLL Protocol.cpp:308-327) ─────────
            // 2026-05-04: las structs PMSG_CHARACTER_*_RECV usan PBMSG_HEAD (3 bytes)
            // + member alineado al tipo. WORD se alinea a 2 → +1 PAD entre header
            // y data. DWORD se alinea a 4 → +1 PAD igual. Por eso los offsets son
            // 4 para WORD/DWORD (no 3) y la size on-wire es 6/4/8 (no 5/4/7).
            // Confirmado en debug.log: hdr=C1 op=DD size=6, op=DE size=4, op=DF size=8.
            case 0xDD: {
                // [C1][06][DD][PAD][WORD level]  → offset 4
                if (Size >= 6) {
                    WORD maxDelLvl = *(const WORD*)(Msg + 4);
                    NetLog("NET:  → 0xDD CharDeleteMaxLevel=%u", maxDelLvl);
                    g_CharDeleteMaxLevel = maxDelLvl;
                }
                break;
            }

            case 0xDE: {
                // [C1][04][DE][BYTE result]  → offset 3 (no padding for BYTE)
                if (Size >= 4) {
                    BYTE flag = Msg[3];
                    NetLog("NET:  → 0xDE CharCreationEnable=%u", flag);
                    g_CharCreationEnable = flag;
                }
                break;
            }

            case 0xDF: {
                // [C1][08][DF][PAD][DWORD MaxLevel]  → offset 4
                if (Size >= 8) {
                    DWORD maxLvl = *(const DWORD*)(Msg + 4);
                    NetLog("NET:  → 0xDF MaxCharacterLevel=%u", maxLvl);
                    g_MaxCharacterLevel = maxLvl;
                }
                break;
            }

            default:
                NetLog("NET:  → op=%02X unhandled (in-game)", HeadCode);
                break;
        }

        (void)Size;
        (void)bEncrypted;
    }
}
