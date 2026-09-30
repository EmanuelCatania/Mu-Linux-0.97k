# Mu Online 0.97k — source code reconstruction

[![Project website](https://img.shields.io/badge/project%20website-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/es/)

[🇪🇸 Español](README.md) | 🇺🇸 English | [🇧🇷 Português](README.pt-BR.md)

A C++ port of the **Mu Online 0.97k** client (`main.exe`, MD5
`eb95ac0785e40a7ad60c9ddb5d8bef34`), reverse-engineered from the original
binary.

The goal is for the compiled client to **behave the same as the original
binary**: same login flow, same rendering, same packets. This is not a rewrite
nor a "client inspired by" — every function is a port of its counterpart in the
binary, and deliberate deviations are documented in a comment next to the code.

**The code and its comments are in Spanish.** Symbols that could already be
identified are named by responsibility; their canonical definition keeps an
`IDA: FUN_xxxxxxxx` or `IDA: DAT_xxxxxxxx` comment to preserve traceability back
to the binary.

---

## Status

**Works end-to-end**: it starts, connects, logs in, picks a character, enters
the world and is playable. Terrain, characters, inventory, equipment, chat,
party, guild, shop, vault, basic combat and effects are all operational.

It is not a finished client: some subsystems still have known gaps, and port
bugs keep surfacing as new code paths get exercised. The most partial area today
is NPC and monster movement.

### By subsystem

| Subsystem | Status |
|---|---|
| Startup, window, OpenGL | Complete |
| Login + server select (includes the ConnectServer flow) | Complete, verified against a real server |
| Character select | Working |
| Network / protocol (85+ opcodes) | Complete for everything exercised; new opcodes show up as new features get used |
| Terrain, lighting, water | Complete |
| Character and equipment rendering | Working |
| Effects, joints, particles, weather | Working |
| Inventory, equipment, vault, shop, trade | Working |
| Chat, party, guild | Working |
| Sound (DirectSound) | Working |
| Music (BGM) | the original launches `MuPlayer.exe` |
| Text and language | UI in Spanish. The port opens a fixed `Text.bmd`, which here is the `_Spn` variant; the `_Eng`/`_Por` variants do ship in `Data/Local/`, but the language selector comes from the DLL and is not ported |
| Combat | `Attack`, `Action` and `MoveCharacterVisual` audited 1:1 against IDA, together with their executor chains |
| NPC / monster movement | Partial |

### Architecture and technical debt

The ported code is laid out by domain (`Render/`, `Terrain/`, `UI/`, `Item/`,
`Entity/`, `Combat/`, `Net/`, `Scene/`, and so on); there is no longer a general
`stubs_*.cpp` dumping ground waiting to be split up. The current tree holds 251
`.cpp` files and 55 headers under `src/`.

`stubs_IDA_ports.cpp` is the intentional exception: it holds 26,431 lines of raw
IDA decompiles, each one guarded by `IDA_PORT_*`. It is preserved as the
canonical reference for the decompile and as a compatibility or
selective-activation mechanism while a port is being validated. **Do not
refactor, rename, or spread this file across modules.** Its aliases and ABI
bridges are not naming debt either: they exist so that selective ports keep
their original contract.

The `FUN_*` and `DAT_*` names that still appear outside that file are not, on
their own, renaming debt. Some describe infrastructure, the CRT, GameGuard,
binary layouts, pools, or compatibility; others need research or a future port
before they can safely be given a semantic name.

---

## What you need besides this repo

Almost nothing: the **game assets are already included** in `bin/Client/Data/`
(~209 MB — `.bmd` models, `.ozj`/`.ozt` textures, maps, sounds and music),
along with `MuPlayer.exe`, which is what the client launches to play the BGM.
Clone, build, run.

The only thing **not** included is the **original `main.exe`**, which you only
need if you want to decompile it yourself to verify a port against the binary.
It comes with any 0.97k client distribution; check the MD5
(`eb95ac0785e40a7ad60c9ddb5d8bef34`) before comparing addresses, because there
are many patched variants floating around and they do not match.

You also need a **server**. The port is validated against
[MuEmu - Linux](https://github.com/EmanuelCatania/Mu-Linux-0.97k) (season
0.97k), which is the authoritative source for packet layouts. The Windows
version [MuEmu - Kayito](https://github.com/nicomuratona/MuEmu-0.97k-kayito)
(season 0.97k) also works.

---

## Building

Requires **Visual Studio 2022** with the C++ desktop toolset.

1. Open `mu97k.sln`.
2. Select the **Debug** configuration / **Win32** platform.
3. Build.

**The platform has to be Win32 (x86).** The whole port assumes 32-bit pointers:
the original binary's addresses, the struct layouts and the memory pools. It
does not compile on x64, and if it did it would not work.

Output: `bin/Client/main.exe`. The project links straight into `bin/Client/`,
which is where the assets and `server.cfg` live, so there is no intermediate
copy and no risk of ending up running a stale binary.

Linked libraries (all from the Windows SDK, except libjpeg which is bundled):
`opengl32.lib`, `glu32.lib`, `winmm.lib`, `ws2_32.lib`.

### Running

1. Copy `server.cfg.example` to `bin/Client/server.cfg` and edit it (it is the
   only file not shipped in the repo).
2. Run `bin/Client/main.exe`.

#### `server.cfg`

Two kinds of line: the addresses (`<IP> <port>`) and the server identity ones
(`key=value`). Lines starting with `#` or `;` are comments.

```
127.0.0.1 44405        <- ConnectServer (server list + load bar)
127.0.0.1 55901        <- GameServer (fallback)

CustomerName=MuLinux
ServerSerial=TbYehR2hFUPBKgZj
ClientVersion=0.97.11
```

**Addresses.** With two lines the ConnectServer flow is used: the client asks
for the real list (`F4/02`), the server answers with names and occupancy, and
picking one makes `F4/03` redirect to the GameServer. With **a single line** it
connects straight to the GameServer — the classic behavior, and in that case the
server select screen shows a static placeholder entry.

> The server select screen **always** appears, even when pointing straight at
> the GameServer: it is a screen from the original flow, not an indication that
> you are reaching the ConnectServer.

**Server identity.** The three values have to match the GameServer's
(`MuServer/GameServer/DATA/GameServerInfo - StartUp.dat`). If any one of them
does not match, the client **connects but never gets in**, with no useful
message:

| Key | Where it comes from | What happens if it does not match |
|---|---|---|
| `CustomerName` | `CustomerName=` in the `.dat` | The client connects, decrypts garbage and hangs on *"connecting to GameServer"* forever |
| `ServerSerial` | `ServerSerial=` in the `.dat` | Same as above, **plus** the login returns *"wrong version"* |
| `ClientVersion` | `ServerVersion=` in the same `.dat` | Login rejected with *"wrong version"* |

`CustomerName` and `ServerSerial` feed the encryption key, which the GameServer
derives from the two combined
(`GameServer/HackCheck.cpp::InitHackCheck`); that is why changing the client's
name breaks the connection even when everything else is right. `ServerSerial`
does double duty: it goes into that derivation and the server also compares it
byte by byte at login.

If they are omitted, this fork's defaults are used (the ones in the block
above). `ClientVersion` accepts both `0.97.11` and `09711`.

**For diagnostics**, `bin/Client/debug.log` records the derived key at startup:

```
MuEmu: InitKeys CustomerName='MuLinux' Serial='TbYehR2hFUPBKgZj' -> EncDecKey1=0xC2 EncDecKey2=0x01 (xor=0xC2 add=0xC2)
server.cfg: ClientVersion='09711'
```

If the client hangs while connecting, that line is the first thing to look at:
compare it against the server's `CustomerName`.

**Client options.** The 0.97k reads these from the Windows registry, which is
where the official launcher leaves them. There is no launcher here, so they are
also accepted in `server.cfg` and, when present, they win over the registry —
the point is being able to ship the client already configured. They are accepted
as `0`/`1` or `on`/`off`, and whatever got applied ends up in `debug.log`; an
invalid value is ignored and logged as `IGNORADO`.

| Key | Binary default | Notes |
|---|---|---|
| `MusicOnOff` | `0` (off) | `server.cfg.example` ships it as `1`. If you comment it out you will not hear any BGM, and that is **not a bug**: it is the original default. The client does not decode the mp3, it launches `MuPlayer.exe` (included in `bin/Client/`). |
| `SoundOnOff` | `1` | Sound effects (DirectSound). |
| `Resolution` | `0` (640x480) | Index `0..4` or `WIDTHxHEIGHT`, but **only the binary's five**: 640x480, 800x600, 1024x768, 1280x1024, 1600x1200. Anything else is ignored and the client starts at 640x480. |
| `WindowMode` | — (deviation) | `1` = windowed, `0` = fullscreen. Ported from the injection DLL: the 0.97k only runs fullscreen and looks for a 16-bit color video mode that does not exist on Windows 10/11, so the mode switch fails silently and the window ends up borderless in a corner. |
| `Borderless` | — (deviation) | `1` = no title bar and no border. Only applies in windowed mode. |

To add a resolution that is not one of those five you have to touch two places
in `src/Config/Config_Load.cpp`: the `Resolution` parser, which maps
`WIDTHxHEIGHT` to the index, and the `switch` in section 5, which is what writes
`WindowWidth`/`WindowHeight`. That is enough for rendering to scale — the layout
scales (`g_fScreenRate_x/y`) are derived from those two variables — but nothing
in the client is tested outside the five original ones, so a wide resolution may
expose problems in panels and hit-tests.

---

## Layout

```
mu97k-src/
├── mu97k.sln            VS2022 solution
├── mu97k.vcxproj        project (Win32)
├── lib/libjpeg/         libjpeg 6b (decodes the .ozj textures)
└── src/
    ├── WinMain.cpp      entry point + WndProc + message loop
    ├── globals.{h,cpp}  global state; identified DATs keep their IDA traceability
    ├── functions.h      shared declarations and the IDA provenance of renamed symbols
    ├── structs.h        struct layouts verified against IDA
    ├── ghidra_compat.h  macros the Ghidra decompile assumes exist
    │                    (qmemcpy, LODWORD, SLOBYTE, ...)
    │
    ├── Combat/  Config/  Core/    Entity/  Game/     GameGuard/ Input/ Item/
    ├── Local/   Math/    Model/   Monster/ Net/       Party/     Path/  Physics/
    ├── Render/  Scene/   Sound/   Terrain/ Trade/     UI/        Util/
    │
    └── stubs_IDA_ports.cpp
                         raw IDA decompiles behind IDA_PORT_*;
                         preserved infrastructure, do not refactor
```

Modules group by responsibility. The address in the binary is still an important
clue for verifying a function or resolving a symbol, but it does not determine
where the ported code lives.

---

## How to work on this

### The main rule: faithful to the binary

The order of authority for settling any question:

1. **IDA / the original binary.** It is the truth. If the decompile says
   something strange, odds are the decompile is right and our intuition is not.
2. **The MuEmu server**, for anything about packet layouts.
3. **The injection DLL**, as a secondary behavioral reference.
4. **The Mu Online 5.2 source**, only as semantic and naming support when the
   current context allows it. Implementation is not copied and 5.2 behavior is
   not incorporated: its UI, definitions and features can differ from 0.97k.

Anything not in one of those sources is not invented. If a deviation is needed
(because a path in the original is unreachable, or depends on something not
ported yet), it gets implemented **and documented in a comment right there**,
explaining what the original does and why we departed from it.

The only thing deliberately skipped is the anti-tamper noise: the interleaved
hash-table operations, the unreachable blocks and the XOR scrambling of the
protected build. None of that is game logic.

### Known pitfalls

These cost entire debugging sessions. Every one of them came back more than
once.

**1. Duplicate symbols.** The same name defined twice: an old stub and the real
port. C++ can accept it as an overload if the signatures differ, and then each
caller resolves to a different copy. Typical symptom: a value gets corrupted and
no writer explains it. Before auditing any function, confirm you are reading
**the copy that actually compiles** — not one inside `#if 0`, nor one behind an
undefined `IDA_PORT_*` macro. Corollary: a diagnostic probe placed in dead code
returns zero results, and that silence looks like evidence that there is no bug.

**2. Locals that Ghidra split apart.** The decompile emits as separate variables
what was a contiguous block in the original frame, and the code walks them as if
they still were (`&local_XX` of a scalar passed as a `vec3`). The compiler does
not guarantee that layout. Symptom: the first component comes out fine and the
rest is garbage (values around 1e9). The fix is rebuilding the frame as one
contiguous array and mapping the names by offset.

**3. Integer fields read as float.** Ghidra types the slot as `float*` and then
*every* access comes out as a float, including the fields that are integers or
pointers. `(float)(uintptr_t)ptr` numerically converts what should have been
reinterpreted bit-wise. Symptom: it is not a crash, it is functionality that
simply never happens — comparisons that never come out true, pointers left at
zero, counters stuck. Hint: values around 1e9 that, read as bits, give small
sensible floats. Correct and broken accesses often coexist in the same file;
that mix is the giveaway.

**4. Struct padding in packets.** The server sends C structs with their
alignment padding. Reading fields at the "logical" offset instead of the real one
returns convincing garbage. It has already bitten in the stats, the guild list
and the damage numbers.

**5. Offset labels lie.** Several fields of the entity struct were mislabeled for
months (`+0x1BC` is not movement flags: it is the character class; `+0x34E` is
not "dead": it is SafeZone). Before trusting an offset's name, find who
**writes** it in the binary.

**6. Similarly named functions with opposite effects.** The recurring case is
the OpenGL state family: `EnableAlphaTest` (0x511680), `EnableAlphaBlend`
(0x511710, additive) and `DisableTexture` (0x511590, which turns texturing off).
Mixing them up paints white squares over half the frame, because GL state is
sticky and contaminates everything drawn afterwards.

**7. Invented bounds and clears.** Guards the binary does not have which, instead
of clamping, **discard the whole input**. It showed up on both sides: in the
network handlers (`count > 30` threw away all 41 monsters in a viewport; the
symptom read as a spawn bug, with a cascade of `key not found` in the log) and in
the input tick (an `else` that, on blocking the debounce, did
`MouseLButtonPush = 0; MouseLButton = 0;`, i.e. it lost the click: you had to
click several times to walk). Rule of thumb: any `= 0` or `clear` the port adds
on the *"not possible yet"* path is suspect — the binary almost always leaves the
state pending for the next tick.

### Protocol deviations

They are documented in the code, but they are worth knowing if you point the
client at a different server:

- **`C3:1E` (duration skill) is sent with 11 bytes, not 9.** Vanilla 0.97k does
  not include `index[]`, but MuEmu's `CGDurationSkillAttackRecv` always reads it
  (`SkillManager.cpp:2047`): with 9 bytes the server takes those two bytes from
  outside the packet and the skill hits a different entity. The injection DLL
  does the same (`CPatchs::SendRequestMagicContinue`).
- **Triple Shot sends the `angle` byte.** The server builds the cone from
  `angle`, not from `dir` (`SkillManager.cpp:1185`).
- **F3/12 on entering the world.** Without that ACK the server leaves `RegenOk`
  at 1 and rejects every later `/move`, on top of not sending the map's
  entities.

---

## License

MIT — see [LICENSE](LICENSE).

The license covers **the code**: everything under `src/`.

It does **not** cover the assets in `bin/Client/Data/` nor `MuPlayer.exe`, which
are copyright of WebZen Inc. and are in the repo only because it is private and
for internal use among collaborators.

`lib/libjpeg/jpeg-6b` belongs to the Independent JPEG Group, under its own
permissive license (`lib/libjpeg/jpeg-6b/README`, "LEGAL ISSUES" section).
