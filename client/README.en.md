# Mu Online 0.97k — source code reconstruction

[![Project website](https://img.shields.io/badge/project%20website-mu--linux.com-0ea5e9?logo=googlechrome&logoColor=white)](https://mu-linux.com/es/)

[🇪🇸 Español](README.md) | 🇺🇸 English | [🇧🇷 Português](README.pt-BR.md)

C++ port of the **Mu Online 0.97k** client (`main.exe`, MD5
`eb95ac0785e40a7ad60c9ddb5d8bef34`), reconstructed by reverse engineering the
original binary.

The compiled client **behaves like the original binary**: same login flow, same
rendering, same packets. It is not a rewrite or a client "inspired by": every
function is a port of its counterpart in the binary, and deliberate deviations
are documented in a comment next to the code.

On top of that base, the features of the `Main.dll` injection DLL were
integrated (Phase 2), Linux support was added (Phase 3), and the client moved
to 64 bits with real timing, a modern renderer and Unicode text (Phase 4).

---

## Status

**The reconstruction is complete.** There are no functions of the binary left
to port. A latent bug may still show up when an uncommon path gets exercised,
and it gets fixed by comparing against IDA, but the reverse-engineering base is
done (tag `0.97.00`).

The client starts, connects, logs in, picks a character, enters the world and
plays: terrain, characters, NPCs and monsters, inventory, equipment, shop,
vault, trade, chat, party, guild, combat, skills, effects, events, sound and
music.

| Platform | Renderer | Status |
|---|---|---|
| Windows x64 | OpenGL 3.3 and WebGPU (DirectX 12, DirectX 11, Vulkan) | Complete; this is the published build |
| Linux x64 | OpenGL 3.3 and WebGPU (Vulkan) | Complete; this is the published build |
| Windows x86, Linux x86 | OpenGL 3.3 | Historical base: it builds, but it is not tested and gets no WebGPU |

### What's new in 0.97.40 — end of Phase 4

- **64 bits.** The client is built and published as x64. The 32-bit layouts of
  the original binary (entities, models, pools, pointers stored in integers)
  were migrated to their native sizes. x86 stays as the historical base.
- **Real timing.** The simulation runs at 25 steps per second, like the
  original, and drawing is separate: more FPS don't speed the game up. FPS
  limit and VSync are configurable.
- **Modern renderer.** OpenGL 3.3 core with shaders instead of the 0.97k's fixed
  pipeline, and a second WebGPU backend (Dawn) on DirectX 12, DirectX 11 or
  Vulkan. See [Rendering](#rendering).
- **New graphics options**, all off by default: texture quality, sharpening,
  antialiasing (FXAA and 4x MSAA), cast shadows, depth fog, draw distance with
  the 3D camera, and an optional HD texture pack.
- **Unicode text.** The client types, draws and sends UTF-8: character and
  guild names in any script, color emoji, chat up to 200 bytes and fallback
  fonts for Chinese, Japanese, Korean, Arabic, Hebrew, Thai and Devanagari. See
  [Text, languages and Unicode](#text-languages-and-unicode).
- **Text sent by the server.** Item, monster and skill names, the tooltip lines
  of custom items and the client's own texts arrive from the server translated
  into the player's language.
- **Interface.** Paged event panel (H key), buff bar with time left and who cast
  it, chat with a history of the last messages, emoji picker, wide screens up to
  3440x1440 with the interface anchored to the edges, fog with per-map
  distances defined by the server.
- **Linux x64** with WebGPU on Vulkan, on X11 and Wayland.

**Compatibility: client 0.97.40 ↔ [server 0.97.40](https://github.com/EmanuelCatania/Mu-Linux-0.97k).**
The protocol changed (40-byte names and 200-byte chat), so client and server
have to be updated together: an older client no longer gets into a 0.97.40
server, and vice versa.

---

## What you need

- **The assets are already included** in `bin/Client/Data/` (~209 MB: `.bmd`
  models, `.ozj`/`.ozt` textures, maps, sounds, music and the fallback fonts).
  Clone, build, and it runs.
- **A graphics card with OpenGL 3.3**, with its driver installed. WebGPU also
  needs DirectX 12, DirectX 11 or Vulkan; if the device can't be created, the
  client keeps going with OpenGL.
- **A 0.97.40 server.** The port is validated against
  [MuEmu - Linux](https://github.com/EmanuelCatania/Mu-Linux-0.97k) (season
  0.97k), which is the authoritative source for the packet format. Each client
  Release says which server version it works with. 0.97k servers without the
  0.97.40 changes (for example,
  [MuEmu - Kayito](https://github.com/nicomuratona/MuEmu-0.97k-kayito)) use
  10-byte names and are not compatible with this client.

---

## Building

The client uses **SDL3** for window, input, sound and networking, and
FreeType, HarfBuzz and SheenBidi for text. Everything is downloaded at
configure time, at pinned versions, so the first configure needs internet.

The published builds are **x64**. The `Win32` backend (the window and the GDI
text of the original binary) and the 32-bit builds stay as a historical base:
they build, but have no CI and are not tested.

### Windows x64

Requires **Visual Studio 2022** with the C++ desktop toolset (it ships CMake)
and [vcpkg](https://github.com/microsoft/vcpkg) for Dawn, the WebGPU backend.
The `VCPKG_ROOT` environment variable has to point at a vcpkg copy (the one
that comes with Visual Studio or your own `git clone` both work):

```
cmake --preset windows-sdl3-x64-release
cmake --build --preset windows-sdl3-x64-release
```

The first configure takes a while: vcpkg builds Dawn and then keeps it in its
cache. Everything links statically, with the static C++ runtime (`/MT`), so the
exe ships no DLLs of its own and doesn't need the Visual C++ redistributable.
The Dawn version is pinned by the baseline in `vcpkg-configuration.json`; to
move to the newest: `vcpkg x-update-baseline`. The custom triplet lives in
`cmake/triplets/`.

Output: `bin/Client/main.exe` (plus `main.map` to resolve crash stacks). The
project links straight into `bin/Client/`, where the assets and `Config.ini`
live, so there is no intermediate copy and no risk of ending up running a stale
binary.

### Linux x64

You need Git, CMake 3.25+, Ninja, Python 3, pkg-config, GCC/G++ and vcpkg (with
`VCPKG_ROOT` pointing at it), plus the desktop development libraries:

- **Debian/Ubuntu:** `build-essential cmake ninja-build git python3 pkg-config
  libx11-dev libx11-xcb-dev libxext-dev libxrandr-dev libxinerama-dev
  libxcursor-dev libxi-dev libgl1-mesa-dev libegl-dev libasound2-dev
  libpulse-dev libwayland-dev libxkbcommon-dev libdecor-0-dev wayland-protocols
  libvulkan-dev`. Ubuntu 22.04 ships CMake 3.22: install CMake 3.25+ before
  configuring. On other distributions, the equivalent packages.

```sh
cmake --preset linux-x64-release
cmake --build --preset linux-x64-release
```

The executable ends up in `build/linux-x64-release/bin/main` (not in
`bin/Client/`, so it doesn't mix with the Windows one). To run it against the
repo's assets, point `MU_ASSET_ROOT` at `bin/Client` (see
[Running on Linux](#running-on-linux)).

### Historical builds (x86 and Win32)

They are still available for comparing against the original binary, which is
32-bit. They use OpenGL only:

- **Windows x86:** `cmake --preset windows-sdl3-x86-release` and
  `cmake --build --preset windows-sdl3-x86-release`. It also lands in
  `bin/Client/main.exe`, so it overwrites the x64 one.
- **Linux x86:** presets `linux-x86-release` and `linux-x86-debug`, with the
  32-bit libraries and `PKG_CONFIG_LIBDIR` pointing at them (for example,
  `/usr/lib32/pkgconfig:/usr/share/pkgconfig` on Arch).
- **Win32 backend:** `cmake -S . -B build -A Win32 -T v143 -DMU_PLATFORM_BACKEND=Win32`
  or `mu97k.sln` in Visual Studio. Its text is GDI and only covers
  Windows-1252.

### For contributors

- While the `.sln` and CMake coexist, a new `.cpp` is added to the `.vcxproj`
  **and** to `CMakeLists.txt`. CI fails if the two lists don't match; you can
  check locally with `python .github/scripts/check_cmake_sources.py`.
- Dependencies included in the repo: `lib/stb` (`.ozj`/`.ozt` textures and
  screenshots), `lib/miniaudio` (music), `lib/utf8proc` (Unicode
  normalization) and `lib/libunibreak` (where a line may break). SDL3,
  FreeType, HarfBuzz and SheenBidi are downloaded at configure time; Dawn, with
  vcpkg.
- CI builds Windows x64 with WebGPU, Linux x64 with WebGPU and the historical
  SDL3 bases, runs the unit tests and builds the Linux packages.

---

## Configuring and playing

### Pointing it at your server

The server address and identity are **compiled into the client**, as in MU 5.2:
there is no config file to ship. They live in `src/Config/ServerConfig.h` and
point to the project's reference server; to use yours, edit that file and
rebuild.

The address can be an IP or a host name (the client resolves it through DNS,
like the original). A name is better: no public IP is hardcoded and the server
can move without a rebuild. If the domain is on Cloudflare, the record must be
**unproxied** ("DNS only"): the proxy only lets web traffic through and cuts the
game connections.

```cpp
constexpr char           ConnectServerIP[]   = "mu.server-pups.space";
constexpr unsigned short ConnectServerPort   = 44405;   // 0 = no ConnectServer
constexpr char           GameServerIP[]      = "mu.server-pups.space";
constexpr unsigned short GameServerPort      = 55901;
constexpr char           CustomerName[]      = "MuLinux";
constexpr char           ServerSerial[]      = "TbYehR2hFUPBKgZj";
constexpr char           ClientVersion[]     = "0.97.40";
```

**Addresses.** With a non-zero `ConnectServerPort` the ConnectServer flow is
used: the client requests the real list (`F4/02`), the server answers with names
and load, and picking one sends `F4/03`, which redirects to the GameServer; if
the ConnectServer does not answer, it connects to the GameServer. With `0` it
connects straight to the GameServer and the server-select shows a fixed entry.

> The server-select **always** shows up, even when pointing straight at the
> GameServer: it is a screen of the original flow, not a sign that you are
> reaching the ConnectServer.

**Server identity.** All three values must match the GameServer's
(`MuServer/GameServer/DATA/GameServerInfo - StartUp.dat`). If any of them does
not match, the client **connects but never gets in**, with no useful message:

| Value | Where it comes from | What happens if it doesn't match |
|---|---|---|
| `CustomerName` | `CustomerName=` in the `.dat` | The client connects, decrypts garbage and hangs on *"connecting to the GameServer"* forever |
| `ServerSerial` | `ServerSerial=` in the `.dat` | Same as above, **and** the login returns *"wrong version"* |
| `ClientVersion` | `ServerVersion=` in the `.dat` | Login rejected with *"wrong version"* |

`CustomerName` and `ServerSerial` feed the encryption key, which the GameServer
derives from both combined (`GameServer/HackCheck.cpp::InitHackCheck`).
`ServerSerial` does double duty: it goes into that derivation and the server
also compares it byte by byte at login. `ClientVersion` accepts `0.97.40` or
`09740`.

**To troubleshoot**, `debug.log` records the address in use and the derived key
at startup:

```
ServerConfig: ConnectServer=mu.server-pups.space:44405 GameServer=mu.server-pups.space:55901 version='09740'
MuEmu: InitKeys CustomerName='MuLinux' Serial='TbYehR2hFUPBKgZj' -> EncDecKey1=0xC2 EncDecKey2=0x01 (xor=0xC2 add=0xC2)
```

If the client hangs while connecting, that line is the first thing to check:
compare it with the server's `CustomerName`.

### Player options

Preferences are read from **`Config.ini`**, with the same sections and keys the
`Main.dll` injection DLL used, so the `Config.ini` you already have works. On
Windows it is `bin/Client/Config.ini`; on Linux, `~/.local/share/mu97k/Config.ini`.
Most of them can also be changed from the in-game options menu, which saves them
to the same file.

On Windows, if a key is missing from `Config.ini` the registry wins (where the
original 0.97k kept what the official launcher configured) and, if it is not
there either, the binary's default. What was applied is logged to `debug.log`
(`Config.ini: ...` line).

| Key | Default | Notes |
|---|---|---|
| `[Window] WindowMode` | — (deviation) | `1` = windowed, `0` = fullscreen. The 0.97k only runs fullscreen and looks for a 16-bit video mode that does not exist on Windows 10/11. |
| `[Window] Borderless` | — (deviation) | `1` = no title bar or border. Windowed mode only. |
| `[Window] Resolution` | `0` (640x480) | DLL indices: `0` 640x480, `1` 800x600, `2` 1024x768, `3` 1280x1024, `4` 1280x720, `5` 1366x768, `6` 1600x900, `7` 1920x1080, and `8` 3440x1440 (deviation). Note: `4` is not the registry's `4` (there it is 1600x1200). On wide screens the interface and notices are anchored to the edges. |
| `[Window] RenderBackend` | `opengl` | Rendering backend; see [Rendering](#rendering), which also lists the graphics options (`TextureQuality`, `Sharpness`, `Antialiasing`, `Shadows`, `DepthFog`, `DrawDistance`, `HDTextures`). |
| `[Window] RenderDecoupled` | `1` | Decouples rendering; `0` restores the 40 ms cap. Simulation stays at 25 Hz. |
| `[Window] FpsLimit` | `60` | FPS cap while rendering is decoupled; `0` = unlimited. Options → Screen offers 25/30/60/120/144/unlimited. |
| `[Window] VSync` | `0` | Vertical synchronization. Decoupling, VSync and FPS changes apply immediately and are saved from Options → Screen. VSync also caps unlimited rendering at the display refresh rate. F12 toggles the FPS counter on the left inside the game, including fullscreen; it starts hidden. |
| `[Sound] EnableSound` | `1` | Sound effects. |
| `[Sound] EnableMusic` | `0` (off) | The repo's `Config.ini` ships it as `1`. The login theme is `Data\Music\MuTheme.mp3`, which the pack does not include (as in the original). |
| `[Sound] SoundLevel`, `MusicLevel` | — (deviation) | Volume from `0` (muted) to `9` (original volume); like the DLL, each level is 6.25 dB. |
| `[User] Username` | — | Prefills the login username field. |
| `[Language] LangSelection` | — (`Text.bmd`, which here is the Spanish variant) | Interface language: `Eng`, `Spn` or `Por`. Picks among the `Text_*.bmd` and `Dialog_*.bmd` in `Data/Local/`, and is the language the client asks the server for item, monster and skill names; see [Text, languages and Unicode](#text-languages-and-unicode). |
| `[MiniMap] Alpha`, `ZoomLevel` | `10`, `0` | Minimap transparency (`3` to `10`) and zoom (`0` to `6`). |
| `[Antilag] Delete…` | `0` | Turn off parts of the rendering for slow machines: `DeleteShadows`, `DeleteObjects`, `DeleteFloor`, `DeleteSkills`, `DeleteStaticEffects`, `DeleteDynamicEffects`, `DeleteWings`, `DeleteHealthBar`, `DeleteInterface`, `DeleteWeather`, `DeleteGlow`. |
| `[Font] FontName`, `FontHeight`, `FontBold`, `FontItalic`, `FontCharset`, `FontWidth`, `FontUnderline`, `FontQuality`, `FontStrikeOut` | Arial, height by resolution | Like the DLL: fixed height (capped at 25) and the big font at twice the size. The repo's `Config.ini` ships Verdana 13. If the whole `[Font]` section is removed, the client goes back to the original font. |
| `[Font] TextShadow`, `TextOutline`, `ColorEmoji` | `0`, `0`, `1` | Text shadow and outline, and color emoji (deviations). Also in Options → Font. |

### Shortcuts that are not in the original

| Key | What it does |
|---|---|
| **F10** / **F11** | Turns on the 3D camera / puts it back to normal |
| **F12** | Shows or hides the FPS |
| **H** | Event panel |
| **Ctrl+V** | Pastes into the text field |

### Running on Windows

Run `bin/Client/main.exe`.

### Running on Linux

Each Release ships the Linux x64 package (in CI, the
`mu97k-linux-x64-release` artifact): the `.tar.gz`, its SHA256 and the library
inventory. It contains `main`, `Data`, licenses, README and `launch.sh`; it does
not carry the developer's `Config.ini`.

The executable is **ELF64 x86-64**, built on Ubuntu 24.04 (it uses no glibc
symbols newer than 2.39). SDL3, FreeType, HarfBuzz, SheenBidi, miniaudio and
Dawn are linked statically; OpenGL, Vulkan, glibc and the desktop libraries
(X11, Wayland, ALSA/PulseAudio) come from the system. On a desktop with the
graphics driver installed they are usually already there; if one is missing,
`ldd ./main` shows it. WebGPU needs the Vulkan loader (`libvulkan1` on
Debian/Ubuntu, `vulkan-icd-loader` on Arch) and the card's Vulkan driver.

Extract the `.tar.gz` on a Linux partition (on NTFS the execute permissions are
lost) and run `./launch.sh` from a terminal, without sudo. Configuration,
screenshots and `debug.log` are stored in
`${XDG_DATA_HOME:-$HOME/.local/share}/mu97k`; `MU_CLIENT_ROOT` picks another
directory. The first start creates a windowed 1024×768 configuration and never
overwrites existing preferences.

To read the assets from another folder without copying them (for example, an
NTFS disk or the repo's `bin/Client`), point `MU_ASSET_ROOT` at the folder
**that contains `Data`**:

```sh
MU_ASSET_ROOT='/run/media/user/HDD/path/Client' ./launch.sh
```

Without `SDL_VIDEODRIVER`, a Wayland session tries Wayland and then
X11/XWayland; the chosen backend and renderer are printed to stderr and
`debug.log` at startup. To force one: `SDL_VIDEODRIVER=x11 ./launch.sh` or
`SDL_VIDEODRIVER=wayland ./launch.sh`. WebGPU works on both.

Other Linux details:

- `libdecor` provides client-side window decorations. Without it, a compositor
  that doesn't decorate server-side may show the window without a frame.
- On HiDPI the game's logical resolution is kept and the compositor scales the
  image, which may look less sharp.
- Color emoji come from Noto Color Emoji, if it is installed (`noto-fonts-emoji`
  on Arch, `fonts-noto-color-emoji` on Debian/Ubuntu).
- Screenshots are taken with **Print Screen**. On KDE, Spectacle grabs it before
  it reaches the game.

### x86 build on Linux: Wayland and NVIDIA GPUs

This only affects the historical 32-bit build. With NVIDIA it has two
consequences on native Wayland (found on Arch, KDE Plasma, RTX 3080 Ti, driver
615):

- **Without the 32-bit EGL Wayland platform, Wayland uses software rendering.**
  NVIDIA only ships `egl-wayland` for x64. Without `libnvidia-egl-wayland.so.1`
  in `/usr/lib32`, the Wayland GL context falls back to Mesa `llvmpipe`. The
  client detects it at startup (`Renderer por software en wayland`) and switches
  to X11/XWayland on its own, where GLX does use the GPU.
- **With that platform installed, native Wayland works but runs much slower**:
  NVIDIA's 32-bit EGL driver resolves thread-local variables on every GL call.
  With NVIDIA, run the x86 build on X11 (`SDL_VIDEODRIVER=x11 ./launch.sh`).

---

## Rendering

The 0.97k draws with OpenGL 1.x's fixed pipeline, vertex by vertex. The client
keeps what you see (the same models, textures, blends and effects), but sends
it to the GPU through buffers and shaders. There are two backends, picked in
**Options → Screen → Advanced → Graphics** (`[Window] RenderBackend` in
`Config.ini`) and applied on restart:

| Backend | Key | What it uses |
|---|---|---|
| OpenGL | `opengl` (default) | OpenGL 3.3 core context. Available in every build. |
| WebGPU | `auto` | [Dawn](https://dawn.googlesource.com/dawn) chooses: on Windows it prefers Vulkan, and otherwise DirectX 12 or 11; on Linux, Vulkan. |
| DirectX 12 / DirectX 11 / Vulkan | `d3d12` / `d3d11` / `vulkan` | WebGPU forced onto that API. DirectX only on Windows. |

WebGPU only exists in the x64 builds. If the device can't be created, the
client keeps going with OpenGL. With WebGPU the models stay resident on the GPU
and skinning (deforming the mesh with the bones) runs on the GPU.

The active backend and the card are written to `debug.log` at startup (for
example `WebGPU: Vulkan | NVIDIA GeForce RTX 3080 Ti`).

> **NVIDIA and DirectX:** NVIDIA's driver ships a profile for executables named
> `main.exe` that caps DirectX 11 and 12 at 60 FPS. Vulkan doesn't have it; that
> is why `auto` prefers it on Windows.

### Graphics options

They are all deviations from the original and start off: with the defaults it
looks like the 0.97k. They are changed from the same submenu, **Options → Screen →
Advanced → Graphics**, and saved under `[Window]`.

| Key | Values | What it does |
|---|---|---|
| `TextureQuality` | `0` original, `1` mipmaps, `2` mipmaps + bicubic | Distant textures without shimmering. Switching to mipmaps applies on restart. |
| `Sharpness` | `0` to `3` | Edge sharpening on the 3D image; `3` is AMD CAS's maximum. |
| `Antialiasing` | `0` off, `1` FXAA, `2` 4x MSAA, `3` 4x MSAA + FXAA | MSAA applies on restart. |
| `Shadows` | `0` the original's flat shadow, `1` soft, `2` strong | Cast shadows with a shadow map. |
| `DepthFog` | `0` / `1` | Depth fog with the 3D camera. |
| `DrawDistance` | `0` to `3` (×1, ×1.5, ×2, ×3) | Draw range with the 3D camera. Far away only the terrain is drawn. |
| `HDTextures` | `0` / `1` | Uses the HD texture pack in `Data/HD/` if present. It is not in the repo; applies on restart. |

The **3D camera** (**F10**, **F11** puts it back to normal) is the injection
DLL's. Draw distance and depth fog only apply with it: the normal camera sees
the same as the original. Each map's fog is defined by the server
(`MapFog.txt`); with the 3D camera and `DepthFog=1` the depth fog replaces it.

---

## Text, languages and Unicode

The 0.97k stores and draws text in Windows-1252: Latin alphabet only, and
character names are 10 bytes. Since 0.97.40 **client and server use UTF-8 end to
end**.

### What you can type

- **Character and guild names** in any script: up to 10 characters for a
  character and 8 for a guild (each counts the way the player sees it, even if
  it takes several bytes). Letters, marks and digits are accepted; the server
  applies the same rule and stores them normalized (NFC). Accounts and passwords
  stay ASCII.
- **Chat, whispers, guild notices and `/post`** up to 200 bytes.
- **Input:** the system keyboard (with an IME for Chinese, Japanese or Korean)
  and **Ctrl+V** to paste.
- **Emoji** in color, with the system font (Segoe UI Emoji on Windows, Noto
  Color Emoji on Linux). There is a picker next to the chat field, and the codes
  `:D`, `<3` and `:fire:` are replaced with their emoji. With
  `[Font] ColorEmoji=0` they come out black and white.

### How it is drawn

Text goes through HarfBuzz (glyph shaping, ligatures), SheenBidi (right-to-left
scripts such as Arabic and Hebrew) and libunibreak (where a line may break in
chat, bubbles and dialogs). Whatever the chosen font lacks is looked up,
character by character, in the fallback fonts in `bin/Client/Data/Font/`
(Chinese, Japanese, Korean, Arabic, Hebrew, Thai, Devanagari and emoji) and then
in the system's. More `.ttf` or `.otf` files can be added to that folder; the
details are in [`Respaldo-README.md`](bin/Client/Data/Font/Respaldo-README.md).

`[Font]` in `Config.ini` picks the main font, its size, the smoothing
(`FontQuality`), a shadow (`TextShadow=1`) and an outline (`TextOutline=1`); it
can also be changed from Options → Font.

### Languages and texts sent by the server

`[Language] LangSelection` (`Eng`, `Spn` or `Por`) chooses two things:

1. **The client's texts**, as in the original: the `Text_*.bmd` and
   `Dialog_*.bmd` in `Data/Local/`.
2. **The texts it asks the server for.** On connecting, the client tells the
   server its language, and the server sends it, along with the content
   catalog, the translations for that language:

| Server file | What it translates |
|---|---|
| `Data/Text/<language>/Item.txt` | item names |
| `Data/Text/<language>/Monster.txt` | monster and NPC names |
| `Data/Text/<language>/Skill.txt` | skill names |
| `Data/Text/<language>/Tooltip.txt` | tooltip lines of custom items |
| `Data/Text/<language>/Client.txt` | the client's own texts (options, event panel, buff bar, window title) |

Anything not translated comes from the client's `.bmd` files (or, for
`Client.txt`, from the English built into it). This way a server can translate
its custom items, or fix a translation, without shipping a new client. In
`Client.txt`, each line's `%d`/`%s` must be the same as the English ones and in
the same order: otherwise the client discards the line and uses its own.

The files are edited by hand (UTF-8 without BOM) or from the translation editor
in the server's web panel, which validates the same things as the client. The
format is explained in each file's header.

---

## How to add custom items

With the injected `Main.dll`, a custom item had to be added on both sides: the
server defined it in its `Item.txt`, and the client needed a regenerated
`item.bmd` plus the `Encoder` `.txt` files (`CustomItem.txt`, `CustomGlow.txt`,
etc.) packed into `ClientInfo.bmd`. If client and server disagreed, the item
rendered wrong or the server rejected it.

Now **the server is the only source**. On login it sends the client a catalog
with every definition (items, monsters, effects, pets). The client **only needs
the model files** (`.bmd` and textures) under `bin/Client/Data`. No `item.bmd`
to regenerate and no code to touch.

The DLL's `Encoder` `.txt` files are still read from the server's
`Data/Custom/Encoder`, so a customs folder built for the DLL works without
converting anything.

### Indexes: the classic range and the extended one

In 0.97k every item section (swords, axes, …, jewels) has **32 indexes** (0 to
31). On top of that, every section accepts indexes **32 to 511**: those are the
*added* items. On the wire they use 13 bits (an item is 7 bytes instead of 5),
so they never collide with a vanilla item.

An added item has to say **which vanilla item it behaves like** (the *Behavior*
column): that drives the logic 0.97k hardcodes per item type (whether it is a
bow and uses arrows, whether it is a wing, a jewel, which excellent options it
can roll). Everything else —name, stats, size, model, glow, effects— comes from
its own row.

### Example 1: the Knight Blade, the DLL way

The item sits inside the classic range (`0,20`); model and glow are defined in
the `Encoder`:

```
// Data/Custom/Encoder/CustomItem.txt
00,020		22		"Sword21"		// Knight Blade

// Data/Custom/Encoder/CustomGlow.txt
00,020		191	165	127		// Knight Blade
```

Client assets: `bin/Client/Data/Item/Custom/20/` (`sword21.bmd` and its
textures).

### Example 2: the Crimson Knight Blade, outside the classic range

The same model as an added item (`0,32`), all in one row of
`Data/Item/Item.txt`. After the usual columns come: behavior, model folder and
name, and the glow color (RGB):

```
32	0	22	1	4	1	1	0	"Crimson Knight Blade"	...	00,020	"Item\Custom\20\"	"Sword21"	255	40	40
```

It behaves like the Knight Blade (`00,020`), uses the `Sword21` model and glows
red. Test command: `/make 0 32`.

### Example 3: the Great Dragon set

Five pieces at index `21` of sections 7 to 11, with the model defined in the
`Encoder`'s `CustomItem.txt` (as with the DLL):

```
07,021		0		"HelmMale22"		// Great Dragon Helm
08,021		0		"ArmorMale22"		// Great Dragon Armor
09,021		0		"PantMale22"		// Great Dragon Pant
10,021		0		"GloveMale22"		// Great Dragon Glove
11,021		0		"BootMale22"		// Great Dragon Boot
```

Client assets: `bin/Client/Data/Player/Custom/21/`. The same pieces can also be
defined without the `Encoder`, with the model columns in `Item.txt`
(`"Player\Custom\21\" "HelmMale22"`).

### Example 4: custom effects and inventory pose

`Data/Custom/Items/<section>_<index>.json` gives an item (custom or vanilla) its
inventory pose and effects the client draws on the model's bones. For example,
wings with the sparkles of the 5.2 Wings of Illusion:

```json
{
  "item": "12,032",
  "effects": [
    { "on": "equipped", "type": "sprite", "texture": "Effect/Flare.jpg",
      "bones": [5, 6, 7, 8, 18, 19], "color": [0.5, 0.0, 0.0], "scale": 0.6,
      "pulse": { "speed": 0.002, "scale": 0.2, "color": 0.4 } },
    { "on": "equipped", "type": "particle", "particle": 1230,
      "bones": [13, 31], "chance": 2, "color": [0.8, 0.8, 0.3], "scale": 0.5 }
  ]
}
```

The server repo has a real case: `Data/Custom/Items/03_000.json` fixes the
position of the (vanilla) Light Spear in its inventory cell, using the 5.2
correction.

### Example 5: a custom pet (Pet Rudolph)

The 5.2 Rudolph as a pet that circles the player and picks up nearby zen. It
uses a high index (`13,400`) on purpose, to show off the extended range. Four
files, all included in the repos:

| Where | File | What it defines |
|---|---|---|
| client | `bin/Client/Data/Item/Custom/Rudolph/` | the `xmas_deer.bmd` model and its textures |
| server | `Data/Item/Item.txt` | the item row: `Slot` 8 (helper), behavior `*` (it does not mimic any vanilla pet) and the model |
| server | `Data/Custom/Items/13_400.json` | the inventory pose |
| server | `Data/Custom/Pets/13_400.json` | how it moves and what it does |

```json
{
  "item": "13,400",
  "blendMesh": 0,
  "movement": { "type": "orbit", "radius": 50, "period": 4000, "height": 20 },
  "abilities": [ { "type": "pickup", "what": "zen", "range": 3, "interval": 1000, "delay": 1500 } ]
}
```

The client draws the movement; the abilities (picking up zen) are run by the
server, which is the one that decides. Test command: `/make 13 400`.

### Example 6: a custom monster (Karane)

The DLL way, with `Data/Custom/Encoder/CustomMonster.txt` (index, type
`0`=NPC `1`=monster, golden, scale, folder and model):

```
152		1		1		2.0		"Monster\\Karane\\"		"Karane"		// Karane
```

Or directly in `Data/Monster/Monster.txt`, with the same columns at the end of
the monster's row:

```
152	0	"Karane"	...	0	0	1	1	2.0	"Monster\Karane\"	"Karane"
```

Client assets: `bin/Client/Data/Monster/Karane/`.

### Reference

Optional columns at the end of each `Item.txt` row (`*` = no value):

| Column | Example | What it does |
|---|---|---|
| Behavior | `00,020` | vanilla item it behaves like |
| Model folder and name | `"Item\Custom\20\" "Sword21"` | model in the inventory, on the ground and on the character |
| Glow | `255 40 40` | per-level glow color |
| Worn model folder and name | `"Item\Custom\FenrirMount\" "fenril_black"` | only if the item looks different when worn (a mount) |
| Gate | `22` | for scrolls: always takes you to that gate |

`Data/Custom/Items` JSON files also accept `"tooltip"`: up to 6 extra lines
below the item name, as plain text (`"Zen picker"`) or with a color
(`{ "text": "Zen picker", "color": "gold" }`; colors: `white`, `blue`, `red`,
`gold`, `green`, `darkred`, `purple`, `darkblue`, `darkgold`).

For custom wings, `Data/Item/CustomWing.txt` adds the defense and damage
constants. The `Data/Custom/Items` and `Data/Custom/Pets` JSON files are
validated when the server starts: a file with errors is dropped whole and the
reason goes to `GameServer/LOG`.

---

## Layout

```
mu97k-src/
├── CMakeLists.txt       client build (SDL3 on Windows and Linux, x64 and x86)
├── CMakePresets.json    Windows and Linux presets
├── vcpkg.json           Dawn (WebGPU), only for the x64 builds
├── mu97k.sln            VS2022 solution (historical Win32 backend)
├── mu97k.vcxproj        project (historical Win32)
├── cmake/               CMake modules, toolchains and the vcpkg triplet
├── tests/               unit tests run by CI
├── lib/stb/             decodes the .ozj/.ozt textures and saves screenshots
├── lib/miniaudio/       plays the music
├── lib/utf8proc/        Unicode normalization
├── lib/libunibreak/     line breaking (UAX #14)
└── src/
    ├── WinMain.cpp      entry point + message loop
    ├── Platform/        platform interfaces and their backends (SDL3/, Posix/, Win32/)
    ├── Render/Backend/  rendering backends (OpenGL 3.3 and WebGPU)
    ├── Text/            UTF-8 text: measuring, shaping, line breaking and emoji
    ├── globals.{h,cpp}  global state; identified DATs keep their IDA traceability
    ├── functions.h      shared declarations and the IDA origin of renamed symbols
    ├── structs.h        struct layouts verified against IDA
    ├── ghidra_compat.h  macros the decompiler takes for granted (qmemcpy, LODWORD, ...)
    │
    ├── Combat/  Config/  Core/    Entity/  Game/     GameGuard/ Input/ Item/
    ├── Local/   Math/    Model/   Monster/ Net/       Party/     Path/  Physics/
    └── Render/  Scene/   Sound/   Terrain/ Text/      Trade/     UI/    Util/
```

Modules are grouped by responsibility. The address in the binary is still an
important clue to verify a function or resolve a symbol, but it does not decide
where the ported code lives.

---

## Roadmap

Each `XX.XX.00` version matches a Webzen version: `0.97.00` is 0.97k, `0.99.00`
will be 0.99, and so on with each Season. The tags in between are the path to
the next version.

### The 0.97 line

Tag `0.97.00` closes the faithful reconstruction: the client behaves like the
original binary. On top of that base, the 0.97 line is worked in engineering
phases; each closed phase is a version.

```mermaid
flowchart TD
    F0["0.97.00 · Faithful reconstruction (base)"] --> F1
    F1["0.97.10 · Phase 1: engineering base<br/>CMake, CI, code and comment cleanup, protocol mirror"] --> F2
    F2["0.97.20 · Phase 2: native integration of Main.dll + modularization<br/>customs built into the base logic; Webzen switches and defines → tables and enums"] --> F3
    F3["0.97.30 · Phase 3: platform abstraction + Linux"] --> F4
    F4["0.97.40 · Phase 4: real timing + modern renderer + Unicode<br/>simulation decoupled from rendering, high fps without speeding up the game, OpenGL 3.3, WebGPU, x64 and UTF-8 text"] --> F5
    F5["0.97.50 · Phase 5: content shared by client + server, server authority and security<br/>0.97.5x: launcher, updater and other improvements"] --> F6
    F6["0.97.60 → 0.97.8x · new content and features"] --> F9
    F9["0.97.9x · 97+99: 0.99 items and maps on top of 0.97"] --> V99
    V99["0.99.00 · full 0.99 version, with the Dark Lord"]
```

Each phase's number is the version planned when it closes; the scope of each one
may be adjusted along the way.

### After 0.97

Engineering phases belong to the 0.97 line. From `0.99.00` on, the tags in
between add what each Webzen version brings, following its patches, until the
next one is complete: Season 1, then Season 2 (which arrives with the Summoner),
and so on.

### Versioning

Versions are `XX.XX.YZ`, always two digits at the end.

| Tag | What it is |
|---|---|
| `0.97.00` | faithful-reconstruction base (equivalent to 0.97k) |
| `0.97.01`, `0.97.02`… | fixes on top of the base |
| `0.97.10` | Phase 1 closes |
| `0.97.11`, `0.97.12`… | fixes and improvements on top of Phase 1 |
| `0.97.20` … `0.97.50` | the following phases close, and so on |
| `0.97.9x` | 97+99: 0.99 content on top of 0.97 |
| `0.99.00` | full 0.99 version |

In the 0.97 line, **Y** is the phase and **Z** the fix or improvement. In the
following lines, the tags in between mark the progress toward the next Webzen
version.

The client and the [server](https://github.com/EmanuelCatania/Mu-Linux-0.97k) use
the same numbering, but the server only gets a new tag when it changes. Each
client Release states which server tag it works with (for example, client
`0.97.32` works with server `0.97.30`).

### Branches

```mermaid
gitGraph
    commit id: "base" tag: "0.97.00"
    branch fase/1
    checkout fase/1
    commit id: "PR topic A"
    commit id: "PR topic B"
    checkout main
    branch hotfix
    checkout hotfix
    commit id: "fix"
    checkout main
    merge hotfix tag: "0.97.01"
    checkout fase/1
    merge main id: "brings the fix"
    checkout main
    merge fase/1 tag: "0.97.10"
    branch fase/2
    checkout fase/2
    commit id: "PR topic C"
```

- `main` only receives phase closings and fixes; each merge gets its tag and its
  Release.
- Each phase is worked on in `fase/N`. Topic branches (`fix/…`, `feat/…`,
  `chore/…`) branch off `fase/N` and come back through a PR.
- A fix on a published version branches off the affected tag, is merged into
  `main` with its new tag and also into the phase in progress.

---

## License

MIT — see [LICENSE](LICENSE).

The license covers **the code**: everything under `src/`.

It does **not** cover the assets in `bin/Client/Data/`, which are copyright of
WebZen Inc.

Third-party libraries keep their own licenses: stb and miniaudio (public domain /
MIT-0), utf8proc (MIT), libunibreak (zlib), SDL3 (zlib), FreeType (FreeType
License), HarfBuzz (MIT), SheenBidi (Apache 2.0) and Dawn (BSD 3-Clause). The
fonts in `bin/Client/Data/Font/` carry theirs next to them (SIL OFL 1.1 or
Apache 2.0). The Linux package includes the licenses in `licenses/`.

---

## Appendix: the reconstruction

This part is for anyone who wants to verify a port against the binary or follow
the history of the reverse engineering. You do not need it to build or play.

### The original binary

The original `main.exe` is not in the repo: you only need it to decompile it and
compare a port against it. It comes with any 0.97k client distribution. Check the
MD5 (`eb95ac0785e40a7ad60c9ddb5d8bef34`) before comparing addresses: there are
many patched variants around and they don't match.

### The main rule: faithful to the binary

The order of authority to settle any doubt:

1. **IDA / the original binary.** It is the truth. If the decompile says
   something odd, the decompile is probably right and our intuition is not.
2. **The MuEmu server**, for anything related to the packet format.
3. **The injection DLL**, as a second behavior reference.
4. **The Mu Online 5.2 source**, only as semantic and naming support when the
   current context allows it. No implementation is copied and no 5.2 behavior is
   brought in: the UI, definitions and features may differ from 0.97k.

What is not in any of those sources is not made up. If a deviation is needed
(because a path of the original is unreachable, or depends on something not yet
ported), it is implemented **and documented in a comment right there**,
explaining what the original does and why we depart from it.

The only thing skipped on purpose is the anti-tamper noise: the interleaved
hash-table operations, the unreachable blocks and the XOR scrambling of the
protected build. They are not game logic.

### Names and comments in the code

**The code and comments are in Spanish.** Symbols that could be identified are
named by responsibility; their canonical definition keeps a
`// IDA: <name> (<address>)` comment to preserve traceability to the binary.

The `FUN_*` and `DAT_*` still present in the code are not, by themselves,
renaming debt. Some describe infrastructure, CRT, GameGuard, binary layouts,
pools or compatibility; others need research before they can get a safe semantic
name. Nor are the aliases and ABI bridges left in `functions.h`/`globals.h`
debt: they keep the original contract of the ports that use them.

Comments describe **the current code**: the IDA reference, the layout of a
struct or packet, the deviations from the binary that are in effect, and the
warnings that keep something from breaking. The story of how a fix was reached
(dates, probes, discarded attempts, what the old port did) goes in the commit
message. The development comments that were in the code and the IDA decompiles
that were never activated (formerly in `src/stubs_IDA_ports.cpp`) are archived in
`docs/` of tag `0.97.21`; the full original text is still in tag `0.97.00`.

### Known pitfalls

These cost whole debugging sessions. All of them came back more than once.

**1. Duplicate symbols.** The same name defined twice: an old stub and the real
port. C++ may accept it as an overload if the signatures differ, and then each
caller resolves to a different copy. Typical symptom: a value gets corrupted and
no writer explains it. Before auditing any function, make sure you are reading
**the copy that gets compiled** — not one inside `#if 0`. Corollary: a diagnostic
probe placed in dead code yields zero results, and that silence looks like
evidence that there is no bug.

**2. Locals that Ghidra split.** The decompile emits as separate variables what
was a contiguous block in the original frame, and the code walks them as if they
were (`&local_XX` of a scalar passed as a `vec3`). The compiler does not
guarantee that layout. Symptom: the first component comes out right and the rest
is garbage (values around 1e9). It is fixed by rebuilding the frame as one
contiguous array and mapping the names by offset.

**3. Integer fields read as float.** Ghidra types the slot as `float*` and then
*every* access comes out as float, including the fields that are integers or
pointers. `(float)(uintptr_t)ptr` converts numerically what had to be
reinterpreted bit by bit. Symptom: not a crash, but functionality that simply
does not happen — comparisons that are never true, null pointers, stuck
counters. Hint: values around 1e9 that, read as bits, give small and reasonable
floats. Correct and broken accesses often coexist in the same file; that mix is
the sign.

**4. Struct padding in packets.** The server sends C structs with their
alignment padding. Reading fields at a "logical" offset instead of the real one
returns convincing garbage. It already bit the stats, the guild list and the
damage numbers.

**5. Offset labels lie.** Several fields of the entity struct were mislabeled
for months (`+0x1BC` is not movement flags: it is the character class; `+0x34E`
is not "dead": it is SafeZone). Before trusting an offset's name, look for who
**writes** it in the binary.

**6. Similar function names with opposite effects.** The recurring case is the
OpenGL state family: `EnableAlphaTest` (0x511680), `EnableAlphaBlend`
(0x511710, additive) and `DisableTexture` (0x511590, which turns texturing off).
Mixing them up paints white squares over half the frame, because GL state sticks
and contaminates everything drawn afterwards.

**7. Made-up bounds and clears.** Guards the binary doesn't have that, instead of
trimming, **discard the whole input**. It showed up on both sides: in the network
handlers (`count > 30` threw away the 41 monsters of a viewport; the symptom
read as a spawn bug, with a cascade of `key not found` in the log) and in the
input tick (an `else` that, when the debounce blocked, did
`MouseLButtonPush = 0; MouseLButton = 0;`, i.e. it lost the click: you had to
click several times to walk). Rule: any `= 0` or `clear` the port adds on the
*"not yet"* path is suspicious — the binary almost always leaves the state
pending for the next tick.

### Protocol deviations

They are documented in the code, but worth knowing if you point it at another
server:

- **`C3:1E` (duration skill) is sent with 11 bytes, not 9.** The vanilla 0.97k
  does not include `index[]`, but MuEmu's `CGDurationSkillAttackRecv` always
  reads it (`SkillManager.cpp:2047`): with 9 bytes the server takes those two
  bytes from outside the packet and the skill hits another entity. The injection
  DLL does the same (`CPatchs::SendRequestMagicContinue`).
- **Triple Shot sends the `angle` byte.** The server builds the cone from
  `angle`, not from `dir` (`SkillManager.cpp:1185`).
- **UTF-8 text (0.97.40).** 40-byte character names, 32-byte guild names and
  200-byte chat, instead of the 0.97k's 10 bytes in Windows-1252. Packets that
  go over 255 bytes with those lengths (character list, party, Devil Square
  ranking) travel with a C2 header. A server without these changes is not
  compatible.
- **F3/12 when entering the world.** Without that ACK the server leaves `RegenOk`
  at 1 and rejects every later `/move`, besides not sending the map's entities.
