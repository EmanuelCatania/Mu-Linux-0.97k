# Historial de comentarios: `src/Render/` (parte 1: archivos A–H)

Comentarios de desarrollo movidos desde los archivos de `src/Render/` cuyo nombre
empieza con A–H (sin `src/Render/Texture/`) según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Render/Ambient_Particles.cpp`

### Línea 106 en `AmbientParticles_Update` — antes de `const uintptr_t arrBase   = (uintptr_t)DAT_083a2f78;`

```cpp
    // BUG-FIX 2026-04-28: las dos comparaciones contra 0x83a34ac y 0x83a40cf
    // eran direcciones absolutas del binario original. Ahora computamos
    // bounds relativos al array real DAT_083a2f78 (10 × 0x1bc).
    // - Active range (slots 0..2) = primeros 3 × 0x1bc = 0x534 bytes desde base
    // - Total range (slots 0..9) = 10 × 0x1bc = 0x1158 bytes desde base
```

### Línea 258 en `AmbientParticles_Update` — antes de `if (modelIdx < 0 || modelIdx > 0x4A8) goto LAB_00502b38;`

```cpp
            // BUG-FIX 2026-04-28: el modelIdx puede venir garbage (ej. de un
            // slot reciclado de char-select). Models[] válido en 0..~0x4A8.
```

### Línea 303 en `AmbientParticles_Update` — antes de `float __h = RequestTerrainHeight(*pfVar1, *(float*)(puVar9 - 0x35));`

```cpp
                    // BUG-FIX 2026-04-28: pass explicit (xf, yf) — pos at pfVar1[0/1]
```

## `src/Render/BMD_Anim.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Moved from stubs.cpp lines 14074-14176 (B3 refactor 2026-05-07).
```

### Línea 48 en `BMD__PlayAnimation` — antes de `if (actionsBase < 0x100000 || actionsBase > 0x7FFFFFFF) return true;`

```cpp
    // BUG-FIX 2026-04-28: actionsBase puede ser garbage si el slot fue
    // parcialmente inicializado (ej. particle pool con stale data). Sanity-
    // check del rango: ptr válido en el address space del proceso (heap).
```

### Línea 88 — antes de `extern "C" bool __cdecl CharacterAnimation(int c, int o)`

```cpp
// ── CharacterAnimation @ 0x00448600 (port of IDA decomp, anti-tamper stripped) ─
// Per-character animation tick: reads model action speed, applies multipliers,
// then calls BMD__PlayAnimation (BMD_AnimTick) which advances entity[+0x108] (frame).
// Without this, character entities stay frozen in their initial frame.
//
// IDA original (sub_448600): hash-table reference-count of `c+770` on entry/exit
// — pure obfuscation per CLAUDE.md, omitted here. Real work is the speed calc
// and the sub_440AA0 call.
```

## `src/Render/BMD_Collision.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// BMD_Collision.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

## `src/Render/BMD_DrawMesh.cpp`

### Línea 172 en `BMD__RenderMesh` — antes de `long long worldTime = (long long)DAT_05826e08;`

```cpp
    // WorldTime wave value (for animated texcoords)
    // ── BUG-FIX 2026-07-15: `__ftol()` leía el tope de la pila x87 (ST0), NO
    // WorldTime — el C no garantiza que WorldTime esté cargado ahí → fase BASURA.
    // IDA sub_440D50: `v42 = (__int64)WorldTime % 10000 * 0.0001`. Con la fase
    // basura, el scroll de texcoords del glow +N (chrome, textura 1170) mapeaba
    // coords random → el glow salía sólido/dorado que NO seguía la forma de la
    // malla (la espada del BK se veía como un recuadro/flama dorada) y con fase
    // distinta por-parte cada frame → las piezas del set +11 brillaban
    // desincronizadas. WorldTime = DAT_05826e08 (la misma que usan las
    // animaciones sin de char-select).
```

### Línea 285 en `BMD__RenderMesh` — antes de `pfVar10 = ((float *)&DAT_05828d5c) + 1;   // = &DAT_05828d60 (V0 slot)`

```cpp
                // BUG-FIX CRÍTICO: Ghidra decompiló mal la base.
                // Disasm @ 0x00441194:  MOV ECX,0x5828d60   (NO 0x5828d58)
                // Layout real del array: d5c=U0, d60=V0, d64=U1, d68=V1, ...
                // Con el base mal puesto en d58, pfVar10[-1] escribía en
                // DAT_05828d54 y *pfVar10 en DAT_05828d58.
                // **DAT_05828d58 es el puntero base a la tabla de modelos**
                // (extern DWORD DAT_05828d58 → g_Models). El write lo pisaba con
                // un float aleatorio cada frame que renderizaba chrome → todo
                // acceso posterior a Models[type*0xbc] punteaba memoria basura
                // → heap corruption + render colapsado a wireframe.
```

### Línea 296 en `BMD__RenderMesh` — antes de `pfVar8  = (float *)((char*)&DAT_06f433c0 + meshIndex * 180000);  // source normals`

```cpp
                // BUG-FIX: DAT_06f433c0 está declarado como `float`, así que
                // &DAT_06f433c0 + meshIdx*180000 hace aritmética float* (=
                // +meshIdx*720000 bytes). El stride real per-mesh son 180000
                // BYTES (verificado en disasm @ 0x004411a8: ADD EAX,0x6f433c0
                // tras EBP*180000). Castear a char* para que el +N sea byte arith.
```

### Línea 350 en `BMD__RenderMesh` — antes de `blendMesh <= -2 || (int)*(short *)(pcVar1 + 2) == blendMesh`

```cpp
            // BUG-FIX CRÍTICO 2 (banner MU Logo03 backdrop):
            // El binario original compara como INT, no float. IDA sub_440D50:
            //   else if ( a5 <= -2 || *(__int16 *)(v40 + 2) == a5 )
            // `a5` llega como `*(_DWORD *)(obj+100)` → bits raw del entero
            // (MUGAME escribe DWORD 1 en obj+100 vía `v6[25]=1`). Nuestro port
            // pasa eso por un `float` param → 0x00000001 se interpreta como
            // denormal 1.4e-45f, nunca matchea `1.0f`. Reinterpretamos los bits
            // de blendMesh a int para matchear la comparación integer del orig.
            //
            // Sentinel "no blend": caller pasa 0xffffffff (como float = NaN).
            //   - Como int: (int)0xffffffff = -1 → cond1 `<= -2` = FALSE ✓
            //   - Como int: -1 != mesh.Texture (siempre ≥ 0) → cond2 FALSE ✓
            // Sentinel alt: caller pasa float -1.0f → bits 0xBF800000 (int ~ -1.1e9)
            //   - cond1 `<= -2` = TRUE → dispara blend, IDA también (`-1.1e9 <= -2`).
            //   - En IDA real original: a5=-1 integer → cond1 FALSE, cond2 mesh_tex==-1 FALSE.
            //   - Acá divergía pre-fix. Aceptamos la divergencia pues el sentinel
            //     real que los callers usan es 0xffffffff (ver Entity_DrawByType.cpp).
            // MUGAME type 0xa2: obj+100 = DWORD 1 → como int == mesh.Texture=1 (backdrop)
            //   → cond2 TRUE → EnableAlphaBlend (aditivo) sobre backdrop naranja.
```

### Línea 412 en `BMD__RenderMesh` — antes de `// SAFETY NET para caso lightEnable=0 sin StreamMesh + RENDER_MODE_TEXTURE:`

```cpp
    // Triangle render loop — estructura verbatim del binario original
    // (Ghidra @ 0x00440D50): glBegin(GL_TRIANGLES) UNA sola vez antes del
    // face loop, glEnd() UNA vez después. Las BMD 97k están pre-trianguladas
    // (nv==3 para todas las faces, confirmado via diag _dbgTris/_dbgQuads).
    // El "fix" previo a GL_TRIANGLE_FAN por face divergía del binario real.
```

### Línea 418 en `BMD__RenderMesh` — antes de `if (bmd_obj == (void*)(DAT_05828d58 + 816 * 0xbc) && meshIndex == 1) {`

```cpp
    // SAFETY NET para caso lightEnable=0 sin StreamMesh + RENDER_MODE_TEXTURE:
    // si el triangle loop no setea glColor per-vertex, el color previo de
    // GL puede ser (0,0,0) → mesh invisible. Forzamos el bodyLight flat
    // (con boost) justo antes de glBegin como última línea de defensa.
    //
    // NOTA banner Mu (type 0xA2): la rama blend-mesh (línea ~440 arriba) ya
    // setea glColor3f(blendLight * OBJECT.Light[0..2]) correctamente y deja
    // param_2_b0 = '\0'. Si acá volviéramos a pisar glColor con boostedBL
    // (que viene de `bodyLight` combinado de terrain + tint), romperíamos
    // el ramp del MUGAME. Por eso NO se toca glColor en este safety-net:
    // confiamos en el glColor que cada rama del switch ya dejó puesto.
    // El "rectangulo negro" que intentaba prevenir este bloque se resolvió
    // correctamente vía el fix integer del blend-mesh + FUN_004fdc00 ramp.
    // Helper1 (modelo 816) — el "hada"/Guardian Angel. Su BMD tiene 2 meshes:
    //   mesh 0 = fairy.jpg   (el cuerpo, 46 triangulos)
    //   mesh 1 = fairy2.jpg  (el glow, 4 triangulos = 2 quads)
    // Ninguna de las dos lleva el marcador `_R` en el nombre (verificado leyendo
    // Helper01.bmd, que es version 10 y NO esta encriptado), asi que
    // `TextureScriptParsing::parsingTScript` (0x40C190 — reconoce R/H/S/N tras
    // un `_`) no las marca como bright y el mesh del glow queda RENDER_TEXTURE
    // opaco: el fondo negro del JPG se dibuja como un recuadro negro.
    //
    // 2026-08-24: antes esto se gateaba por ESCENA (`SceneFlag == 2 || == 4`),
    // dejando in-world afuera a proposito "porque ahi el 816 puede renderizarse
    // como pet-item de inventario (opaco)". Consecuencia: con el Guardian Angel
    // equipado, en el mundo se veia el recuadro negro (reportado sobre el pet de
    // otro jugador). El gate correcto no es la escena sino la MESH: solo el glow
    // (mesh 1) necesita el aditivo; el cuerpo (mesh 0) debe seguir opaco. Asi
    // vale igual en el mundo y en el inventario, donde el glow tambien es glow.
    //
    // DESVIACION documentada: no encontre en IDA el mecanismo por el que el
    // original decide este blend — no sale del asset (el BMD v10 no tiene campo
    // de RenderType; se deriva del nombre de textura) ni de `RenderLinkObject`
    // (0x455430), que no toca BlendMesh. Queda como forzado explicito, igual que
    // el gate por escena que reemplaza.
```

### Línea 498 en `BMD__RenderMesh` — antes de `glTexCoord2f((&DAT_05828d5c)[*psVar14 * 2 + 0],`

```cpp
                        // BUG-FIX: Ghidra emitió `(&DAT_05828d5c + 4)[idx*2]` que
                        // es aritmética float* (+4 = +16 bytes), leyendo
                        // V de d6c+idx*8 en vez del correcto d60+idx*8.
                        // Disasm @ 0x004413bd-c4:
                        //   MOV ECX,[EAX*0x8 + 0x5828d60]   ; V
                        //   MOV EDX,[EAX*0x8 + 0x5828d5c]   ; U
                        // El array es {U,V,U,V,...} contiguo desde d5c → V está
                        // en idx*2+1, no idx*2+4.
```

### Línea 525 en `BMD__RenderMesh` — antes de `GL_EnableDepthWrites();  // EnableDepthMask`

```cpp
    // BUG-FIX: restaurar depth-mask al salir. El path chrome (flag&0x40) llama
    // DisableDepthMask() arriba pero nunca lo restauraba dentro de la función,
    // causando que los siguientes meshes del mismo frame dibujaran sin depth-
    // write → efecto "ghost" (se ven las caras traseras a través de las
    // frontales, y cada mesh sucesivo blendea sobre el anterior). El binario
    // original hace EnableDepthMask en los setters de estado entre meshes,
    // pero en nuestra versión el cache DAT_083a42e8 quedaba desincronizado.
```

## `src/Render/BMD_FilterData.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// BMD_FilterData.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.
```

## `src/Render/BMD_LegacyDraw.cpp`

### Línea 1 — antes de `// stubs_helpers.cpp`

```cpp
// BMD_LegacyDraw.cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.
```

### Línea 5 — antes de `#include "stdafx.h"`

```cpp
// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.
```

## `src/Render/BMD_LegacyEffects.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// BMD_LegacyEffects.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.
```

### Línea 15 en `BMD__RenderMeshEffect` — antes de `char *this_ = (char*)model;`

```cpp
    // 2026-09-04 -- BUG-FIX: el port recorria TODAS las mallas del modelo.
    // IDA (sub_441BE0) trabaja sobre UNA sola, la de indice `a2`:
    //     result = this[10] + 40 * a2;        // this + 0x28 = array de mallas
    //     if ( *(__int16 *)(result + 10) > 0 ) ...
    // y el mismo `a2` es el que elige el bloque de 15000 vertices del pool
    // BoneVertex.  Los dos call sites pasan a2 = 0.  Con el bucle sobre todas
    // las mallas se spawneaban varias veces mas efectos de los que corresponde.
```

## `src/Render/BMD_LegacyLoading.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

### Línea 55 — antes de `// Note: la signature original es variadic ('...' para extra anim paths) pero`

```cpp
// IDA: OpenModel (0x00505E90)
// OpenModel(Type, Dir, ModelFileName, ...).
// Port FIEL del IDA (raw 0x505E90):
//   1. FileName = Dir + ModelFileName
//   2. Itera variadic args (extra animation .smd paths) hasta NULL o "end"
//   3. Si v11>0: OpenSMDModel(Type, FileName, v11, unk_83A4100) +
//      OpenSMDAnimation(Type, FileNameN, lockFlag) por cada N
//   4. Si v11==0: OpenSMDModel(Type, FileName, 1, unk_83A4100) +
//      OpenSMDAnimation(Type, FileName, 0)
//
// NOTA 2026-05-01: los archivos Data2/Item/<class>/<file>.smd NO existen en el
// filesystem distribuido (solo Data/Item/<file>.bmd está). Las llamadas a
// fopen dentro de OpenSMDModel/OpenSMDAnimation retornarán NULL → early return →
// no-op silencioso. El path BMD (AccessModel) cubre la carga real de items.
// (OpenSMDModel, OpenSMDAnimation — declared via functions.h.
//  DAT_083a4100 — declared in globals.h.)
```

### Línea 85 en `OpenModel` — antes de `char* slot = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * Type);`

```cpp
    // BUG-FIX 2026-05-04: el cliente 0.97k distribuido NO tiene Data2/Object*/
    // (solo Data/Object*/ con archivos .bmd). Las SMDs no cargan → Lorencia
    // queda sin casas/decoraciones porque su path de OpenWorldModels usa SMDs
    // exclusivamente. Como fallback, si después del SMD load el slot sigue
    // vacío (mesh count = 0), intentamos varias transformaciones del nombre
    // .smd → .bmd para encontrar el archivo real (case-insensitive en Win32).
```

### Línea 222 en `TextParser_GetToken` — antes de `char* TokenStringBuf = (char*)&TextParserTokenString[0];`

```cpp
    // 2026-08-22 FIX: escribia en ParserTokenString, que es el buffer del OTRO
    // tokenizer (Parse_NextToken / OpenWorldModels).  TokenString es 0x07CF1EF0.
```

### Línea 228 en `TextParser_GetToken` — antes de `FILE* fp = DAT_07d7806c;`

```cpp
    // CRITICAL 2026-05-03: data parsers (Item_Data, Monster_Data, Skill_Data,
    // Filter_Data, NPC_Data, Gate_Data) all open their file via DAT_07d7806c
    // (= IDA's SMDFile_0). The other "SMDFile" symbol at DAT_0055c0a0 is a
    // separate misnamed global from an early port pass — unused here.
```

### Línea 327 en `AccessModel` — antes de `{`

```cpp
    // BUG-FIX 2026-04-29: pump message queue cada N llamadas para evitar que
    // OpenWorld (que llama esta func ~hundreds de veces) bloquee el message
    // pump por 2+ segundos. El server MuEmu nos kickea por backpressure si
    // no consumimos los packets que envía después del JoinMapServer.
```

### Línea 351 en `AccessModel` — antes de `if (numBonesInSlot > 0)`

```cpp
        // HQ path original: si el SMD ya cargó bones, BMD__Save agrega la anim BMD.
        // PORT FALLBACK: como nuestro SMD loader (OpenModel) es stub y nunca
        // popula bones, caemos al loader completo BMD__Open para al menos traer
        // la geometría BMD y ver algo del background 3D.
```

### Línea 374 — antes de `int __cdecl FindTextureByName(char *Name, DWORD *dwTexture);`

```cpp
// Forward-declare FindTextureByName (real implementation at ~line 12786 below).
```

### Línea 400 en `OpenTexture` — antes de `char* slot = (char*)(DAT_05828d58 + Model * 0xBC);`

```cpp
    // ── BUG fix (crash 0xC0000005 @ 0x61746168 "ataH"): el Model slot ES la
    //    estructura BMD completa (stride 0xBC), NO un puntero a datos. Los
    //    primeros 32 bytes del slot son el Name (string), no un data ptr.
    //    Los contadores y tablas están inline:
    //       slot +0x24 short  numMeshes
    //       slot +0x34 char*  texNameTable (char[n][0x20])
    //       slot +0x38 short* indexTexture (short[n])
    //    Verificado en Ghidra BMD__Open (BMD::Open): this[0x24]=numMeshes,
    //    this[0x34]=texName[] y this[0x38]=indexTex[] se asignan directamente.
```

### Línea 452 en `OpenTexture` — antes de `char local_40[128];`

```cpp
            // Build full path: SubFolder + Name  (into local_40)
            // 2026-05-05: Si Name ya contiene un path (ej. "Data\Npc\foo.OZT"
            // como guardan algunos BMDs de NPC), NO concatenar SubFolder —
            // sino que sale "Data\Npc\Data\Npc\foo.OZT" → fopen FAIL.
            // Detectamos path absoluto: arranca con "Data\" o "Data/" o
            // contiene '\\' o '/' antes del primer '.'.
```

### Línea 533 — antes de `extern "C" bool __cdecl OpenSMDFile(const char* FileName, int Type, char Flip);`

```cpp
// Forward decls for the SMD parsing chain (stubs below — files no existen
// en filesystem, retornan false; mantienen estructura del binario).
```

## `src/Render/BMD_Load.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// BMD_Load.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12147-12807 (661 lines).
//
// BMD (Mu Online 3D model format) loaders:
//   BMD__Open (BMD::Open)              — load compressed BMD file into model slot
//   BMD__FindTriangleForEdge (BMD_BuildAdjacentFaceTable)
//   BMD__FindNearTriangle (BMD_InitAdjFaceTable)
//   BMD_CreateBoundingBox (BMD_ComputeBounds)
//   BMD__Init (BMD_ResetAnimState)
//   BMD__Save (BMD_SaveToFile)
```

### Línea 203 en `BMD__Open` — antes de `if (!isPlayer && s_parsed < 200) {`

```cpp
        // 2026-05-04: temporarily upped limit from 30 to 200 to debug Lorencia
        // BMD load (slots 0..0xa0). Will revert when validated.
```

### Línea 272 en `BMD__Open` — antes de `{`

```cpp
        // BUG-FIX: el port previo alocaba un script en cero SIN parsear → todas las
        // meshes aditivas (glows, auras, alas) de todos los modelos renderizaban
        // opacas → recuadros negros. Ahora se parsea el nombre como el binario.
```

## `src/Render/BMD_SetupRender.cpp`

### Línea 109 en `BMD_SetupRenderByType` — antes de `glColor3f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),`

```cpp
        // BUG-FIX: glColor3f espera float; *(undefined4*) lee bits y los pasa como int → C castea int→float = basura.
```

### Línea 172 en `BMD_SetupRenderByType` — antes de `glColor4f(*(float *)((int)param_1 + 0x48), *(float *)((int)param_1 + 0x4c),`

```cpp
            // BUG-FIX: leer como float, no como undefined4 (int). Alpha 0x3f4ccccd=0.8f, 0x3f000000=0.5f.
```

### Línea 211 en `BMD_SetupRenderByType` — antes de `glColor3f(0.3f, 0.3f, 0.3f);`

```cpp
        // BUG-FIX: 0x3e99999a = bits de 0.3f (≈ gris oscuro)
```

### Línea 219 en `BMD_SetupRenderByType` — antes de `glColor3f(1.0f, 1.0f, 1.0f);`

```cpp
        // BUG-FIX: 0x3f800000 = bits de 1.0f (blanco)
```

### Línea 276 — antes de `void __cdecl BMD__RotationPosition(void *model, float *bone_mat, float *pos_in, float *pos`

```cpp
// BMD__RotationPosition @ 0x00440a30 — BoneTransformOffset (sub_440A30 en IDA)
// Transforms pos_in through bone rotation, scales by model scale (this[+0x68]),
// stores result in pos_out, AND COPIES the bone matrix into the global root
// matrix DAT_06989c9c so that BMD_Animation, cuando procesa el
// root bone del ala/arma con parentIdx=-1 y param_7=='\x01', use esta matriz
// del bone padre del player como su "parent transform" → el modelo linked
// queda renderizado en la posición del hueso del player en lugar del origen.
//
// IDA original (sub_440A30 @ 0x00440A30):
//   void __thiscall sub_440A30(float *this, float in2[3][4], float in1[3], float *a4) {
//       float out[3];
//       VectorRotate(in1, in2, out);                  // pure rotation
//       *a4   = this[26] * out[0];                    // scale by this[+0x68] (=this[26])
//       a4[1] = this[26] * out[1];
//       a4[2] = this[26] * out[2];
//       memcpy(matrix, in2, 0x30);                    // copy 12 floats
//   }
//
// BUGFIX 2026-04-26: el Ghidra port había stripped tanto el escalado como
// (crítico) la copia de matriz. Sin la copia, las alas/armas renderizaban
// en (entity_pos + rotated_offset_de_15) ≈ pies del char en lugar del back.
```

### Línea 320 en `Model_BoneParticle` — antes de `float in[3]  = { 0.0f, 0.0f, 0.0f };`

```cpp
  // 2026-09-04 FIX -- dos errores, y esta funcion la usan nueve sitios (el
  // brillo de arcos y bastones de RenderLinkObject, el equipo del jugador y
  // AttackEffect), asi que el radio es amplio.  IDA 0x004553C0:
  //
  //   memset(v7, 0, sizeof(v7));
  //   TransformPosition(This, (float (*)[4])BoneMatrix[3 * a3], v7, Position, 1);
  //   return CreateSprite(Type, Position, Scale, Light, Owner, 0.0, 0);
  //
  // 1) La matriz salia de `model + bone_idx*0x30`, o sea del principio del
  //    struct del BMD, cuando el original indexa el buffer global de huesos
  //    (`BoneMatrix` = 0x06970A9C, confirmado por xrefs).  `BoneMatrix[3*a3]`
  //    con filas de float[4] son 48 bytes por hueso = bone_idx * 0x30.
  // 2) Los argumentos 3 y 4 de TransformPosition son ENTRADA y SALIDA.  El port
  //    pasaba `world_pos` (sin inicializar) como entrada, recogia el resultado
  //    en `world_col` y despues emitia el sprite con `world_pos` -- o sea con
  //    la entrada basura, nunca con la posicion transformada.
  //
  // Se notaba sobre todo en el Celestial Bow (Type 545), que es el unico case
  // que llama diez veces a esta funcion (huesos 13-18 y 5-8).
```

## `src/Render/Camera.cpp`

### Línea 21 en `Camera_SetupFrustum` — antes de `float out3[3];`

```cpp
    // Vec3_Transform output buffer.
    // BUG-FIX 2026-06-29: antes eran 3 locals SEPARADAS (`float local_78,
    // local_74, local_70;`).  VectorIRotate (Vector_InverseRotate) escribe un float[3]
    // desde &local_78 asumiendo contigüidad (en el binario original están en
    // ebp-0x78/-0x74/-0x70, contiguas).  MSVC no garantiza ese layout con vars
    // sueltas → out[1]/out[2] caían en stack equivocado y wy/wz leían basura
    // constante → TODOS los corners del frustum con el mismo Y (frustum colapsado
    // a una línea → bound no contenía al héroe → terreno negro).  Mismo bug que
    // CreateTerrainNormal.  Fix: array contiguo real.
```

### Línea 149 en `Camera_SetupFrustum` — antes de `Triangle_ComputeNormal(c0, c1, c2, plane0);`

```cpp
    // IDA Camera_SetupFrustum L169-173 — los 4 planos laterales salen del ápice
    // y el quinto es la BASE de la pirámide (el que corta por distancia):
    //     FaceNormalize(V[0], V[1], V[2], normal[0]);
    //     FaceNormalize(V[0], V[2], V[3], normal[1]);
    //     FaceNormalize(V[0], V[3], V[4], normal[2]);
    //     FaceNormalize(V[0], V[4], V[1], normal[3]);
    //     FaceNormalize(V[3], V[2], V[1], normal[4]);   <-- base, NO el ápice
    //
    // 2026-08-21: el quinto se armaba con (V[2], V[1], V[0]) y su D se
    // referenciaba a V[2] en vez de V[1], así que el plano de corte quedaba
    // pasando por la cámara en vez de por la base.  Efecto: todo lo que se
    // alejaba un poco caía del lado de afuera y se marcaba como no visible —
    // por eso desaparecían los nombres (y el modelo) de los items del suelo
    // sin estar realmente lejos.  Afecta a TODO lo que pasa por sub_4F9590.
```

### Línea 217 — antes de `void __cdecl Camera_SetMatrix(float *cam_pos)`

```cpp
// ── Compatibility helper; no standalone IDA function ─────────────────────────
// DEAD CODE 2026-05-04: esta función NO se llama. Es una decompilación errónea
// que asume corners en DAT_07eab1bc..1e8 (que ya están en world coords post
// Camera_SetupFrustum). La verdadera FUN_004F8EB0 (CreateFrustrum2D) vive en
// stubs.cpp:9127 — usa 4 corners hardcoded escalados por GetScreenWidth(),
// rotados Z=45°, trasladados por cam_pos. Mantenida por compatibilidad
// histórica del header pero no debe llamarse.
```

### Línea 249 en `Camera_SetMatrix` — antes de `float* outX = (float*)&FrustrumX;`

```cpp
    // BUG-FIX 2026-05-01: el código previo SOLO escribía corner[0]. Pero
    // TestFrustrum2D (Frustum_IsVisible) hace test point-in-quad usando los
    // 4 vertices en FrustrumX[0..3] (X) y FrustrumY[0..3] (Y).
    // Con 3/4 vertices en (0,0), el quad degenerado rechazaba TODOS los
    // chunks → mapa renderizaba vacío de objetos pese a que se spawn 2142.
    //
    // Per ghidra_backup line 6127-6128: loop 4 iterations, j stride 4 bytes
    // (= 1 float), escribiendo 4 vertices contiguos en cada array.
```

### Línea 266 — antes de `int __cdecl Frustum_TestSphere(float *param_1, float param_2)`

```cpp
// FUN_004F9590 @ 0x004F9590 — Frustum_TestSphere
// Tests if a world-space point is inside the view frustum.
// param_1: xyz position (float[3])
// param_2: radius (frustum half-width extension)
// Returns a short: low byte 1 if inside all planes, high byte flags if outside.
// Iterates 6 frustum planes stored at DAT_0838b7c8 (normal[3] stride=3) +
// corresponding plane-distances at FrustrumFaceD.
// 2026-05-03: AUTO-SKIP removed. The original Ghidra walked five plane normals
// at &DAT_0838b7c8 (= plane[0].Y) bound by literal `< 0x838b804`. In our build
// each plane component is a SEPARATE global (FrustrumFaceNormal..7fc, 15 floats) —
// the linker may not place them contiguously, so the pointer walk would read
// random memory between plane components. Camera_SetupFrustum (Camera.cpp:148)
// writes all 5 planes; here we read them by name. Unrolled 5×.
```

### Línea 323 — antes de `void __cdecl Camera_ProjectWorldToScreen(float *param_1,int *param_2,int *param_3)`

```cpp
//
// ── BUG-FIX 2026-04-26 ────────────────────────────────────────────────────────
// El decompile original tenía:
//   Vector_Transform(param_1, mat, local_c);
//   lVar1 = __ftol();   *param_2 = ViewportCenterX - lVar1;
//   lVar1 = __ftol();   *param_3 = lVar1 + ViewportCenterY;
// Ghidra perdió la aritmética FPU entre la transformación y __ftol — el código
// original calculaba perspective divide (x_view*scale/z_view) antes de truncar.
// Como `__ftol` aquí está stubbed a `GetTickCount()` (stdafx.h), el resultado
// era basura y rompía: name labels en char-select, hit-test del mouse, todo lo
// que dependa de proyección mundo→pantalla.
// Solución: usar gluProject con el GL state actual. Más robusto que recrear
// la perspective math; respeta cualquier viewport/projection set por
// GL_BeginViewport.
```

### Línea 359 en `Camera_ProjectWorldToScreen` — antes de `int ww = (int)DAT_0056156c;`

```cpp
  // 2026-08-22: aca se casteaba sx/sy a unsigned. Con un item fuera de pantalla
  // a la izquierda sx es negativo, y como unsigned pasaba a ~4.29e9: el
  // resultado salia positivo grande y el nombre del item saltaba al borde
  // DERECHO de la pantalla.
```

### Línea 400 en `GL_BeginViewport` — antes de `GL_SetPerspective(DAT_00561554,(float)uVar3 / (float)uVar4,DAT_0056154c,Ff(DAT_00561550) *`

```cpp
  // BUG-FIX: DAT_00561550 es DWORD (bit-pattern float). En original asm el FLD
  // lee como float. En C++ `DAT_00561550 * float` hace int→float (convierte el
  // bit-pattern 0x461c4000=10000.0f a 1.17e9), dando far plane astronómico.
  // Reinterpretar con Ff() antes de multiplicar.
  //
  // NOTA (2026-04-21): FOV y Near pasan crudos como `int` — GL_SetPerspective los
  // recibe como `int fov, int near_clip` y hace `Ff()` internamente (ver
  // stubs.cpp:2386-2388). Pasarles Ff() aquí causaría DOBLE reinterpretación:
  // 55.0f→int 55→Ff(55)=7.7e-44 → FOV≈0 → pantalla negra.
  // Solo el far necesita Ff() en el caller porque lo multiplicamos por _DAT_00552d34
  // (1.4f) ANTES de pasarlo — la multiplicación es en float-space aquí.
```

### Línea 415 en `GL_BeginViewport` — antes de `glRotatef(Ff(DAT_083a42bc), 0.0f, 1.0f, 0.0f);`

```cpp
  // BUG-FIX: 0x3f800000 es el bit pattern de 1.0f pero glRotatef espera GLfloat.
  // La conversion int→float daba 1065353216.0f (inofensivo porque glRotatef
  // normaliza el eje, pero ilegible). Usar 1.0f literal.
  // BUG-FIX 2: los ángulos DAT_083a42b8/bc/c0 son DWORD (bit-pattern de float).
  // En original asm FLD los lee como float. En C++, pasar DWORD→GLfloat hace
  // int→float: con pitch=-40.0f (bitpattern 0xc2200000), el valor pasado era
  // 3.26e9° (equivalente a ruido aleatorio tras glu). Reinterpretar con Ff().
```

### Línea 439 en `GL_BeginViewport` — antes de `glAlphaFunc(GL_GREATER, 0.25f);`

```cpp
  // BUG-FIX CRITICO: 0x3e800000 es bit pattern de 0.25f, glAlphaFunc espera
  // GLclampf. Como int se convierte a 1048576000.0f → clamp a 1.0 → test
  // "alpha > 1.0" siempre falla → TODO el UI con alpha-test activo era
  // invisible (login screen quedaba sin server list, botones, texto).
```

## `src/Render/Camera_Login.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Camera_Login.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 16 — antes de `void __cdecl Login_CameraUpdate(void) {`

```cpp
// Stub kept void() until Scene_Login.cpp callers are updated.
```

## `src/Render/Camera_Movement.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Camera_Movement.cpp
//
// Extracted from stubs_game.cpp.  Owns the intro/login camera walk animation.
// The function comment retains its original IDA symbol/address.
```

### Línea 25 en `MoveCamera` — antes de `float* CamWalk = CameraWalk_005615ec;        // concrete float[36]`

```cpp
    // BUG-FIX 2026-04-21: usar los arrays concretos en vez de `(float*)&DAT_*`
    // los DAT_* son referencias (float&/DWORD&) y tomar `&` sobre una referencia
    // no siempre da el address que uno espera con MSVC/extern. CurrentCameraAngle
    // quedaba en (0,0,0) aunque el init block corriera → CameraPitch nunca llegaba
    // a -80° → escena se veía sin pitch ("volteada").
```

### Línea 70 en `MoveCamera` — antes de `unsigned int r = rand();`

```cpp
            // BUG-FIX: el decompile original usaba DAT_07e11980 (una variable
            // que NO existe como xref en el binario; siempre 0). La instrucción
            // real en PE @ 0x0051E5D5 es `CMP [0x005615c0], 2` → SceneFlag.
            // Con la variable equivocada, la rama siempre caía al else y
            // elegía wp5 (200,-800,300, roll=-10°) → la cámara saltaba de
            // golpe a posición angulada tras ~128 frames (~3.2s).
            // SceneFlag==2 (login): random waypoint 1..4, random walk type 0..1
```

## `src/Render/Camera_Unproject.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Camera_Unproject.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.
```

### Línea 14 en `Camera_BuildMouseRay` — antes de `float view_dir[3];`

```cpp
    // BUG-FIX 2026-04-26 (deeper audit): el original usaba locals contiguas
    // en stack (local_18/14/10 era un vec3, local_c/8/4 era otro). El port
    // Ghidra los declaró como floats separados — el compilador C++ los puede
    // reubicar en CUALQUIER orden o slot, así que `&local_18` NO apuntaba a
    // un vec3 contiguo. Vector_InverseRotate leía/escribía 3 floats secuenciales
    // desde esa dirección, leyendo basura y stompeando otros locals.
    // Síntoma: CameraRayOriginX (camera pos) y el endpoint del ray quedaban en
    // valores de miles de millones, hit-test contra entidades nunca pasaba.
    // Logueado en HT slot=N rayO=(-79771616,...) rayT=(779717248,...).
```

## `src/Render/Effect_Combat.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Effect_Combat.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 54 — antes de `// ItemDrop_RenderGroundWeapon @ 0x0046B980 (sub_46B980, 377 bytes)`

```cpp
//
// 2026-09-26: aca habia una copia bajo el nombre RenderWheelWeapon.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.
```

### Línea 59 — antes de `void __cdecl ItemDrop_RenderGroundWeapon(int o) {`

```cpp
//
// 2026-09-26: la version anterior de este port estaba rota en cuatro puntos y
// por eso el Rageful Blow no mostraba el arma:
//   - el byte de clase se leia de Hero+0x2B8 (helper/pet) en vez de Hero+444;
//   - alpha se pasaba como 0.0f, y RenderPartObject (0x505A10) sale temprano
//     con `if (_DAT_005524f8 < param_5)` -> con 0 no dibujaba NADA;
//   - los argumentos 6..12 de RenderPartObject estaban corridos;
//   - faltaba BMD_Animation, o sea el arma nunca se posaba.
```

### Línea 119 — antes de `void __cdecl FUN_00466300(float* position)`

```cpp
// IDA compatibility bridges: stubs_IDA_ports.cpp intentionally retains these ABI names.
```

## `src/Render/Effect_Create.cpp`

### Línea 84 en `Tamachan_Create` — antes de `float __frame[28] = {0};`

```cpp
  // 2026-08-10 FIX (mismo patrón que MoveJoint): estos "locales" sueltos son en
  // realidad un bloque CONTIGUO del frame original (ebp-0x6C .. ebp), y el
  // código depende de esa contigüidad — MSVC no la garantiza:
  //   Vector_Rotate(&local_6c, local_3c + 3, &local_60)
  //     → entrada  = {local_6c, local_68, local_64}
  //     → salida   = {local_60, local_5c, local_58}
  //   Vector_Rotate(&local_6c, local_3c + 3, &local_48)
  //     → salida   = {local_48, local_44, local_40}
  // Además `Matrix_BuildFromEuler(ang, local_3c + 3)` escribe una matriz 3x4 (12 floats)
  // en local_3c[3..14], o sea un float FUERA del `float local_3c[14]` original.
  // Mapeo por offset de frame: -0x6C=[0] … -0x3C=[12]; a local_3c se le dan 16
  // slots para que la matriz entre completa.
```

### Línea 116 en `Tamachan_Create` — antes de `{`

```cpp
  // BUG-FIX 2026-05-03: was `if (0x7b2714f < (int)pfVar17)` — absolute source-
  // binary address. Pool DAT_07b11670 is sized 200 × 0x1bc bytes; walk by
  // explicit iteration count.
```

### Línea 277 en `Tamachan_Create`

```cpp
// lifetime: DWORD, no float (fix 2026-08-16)
```

### Línea 325 en `Tamachan_Create`

```cpp
// lifetime: DWORD, no float (fix 2026-08-16)
```

### Línea 415 en `Tamachan_Create` — antes de `if (*(int*)&pfVar17[1] != 4) {`

```cpp
        // 2026-08-15: IDA `if (*((_DWORD *)i + 1) == 4)` — SubType es un
        // DWORD. Leerlo como float y convertir (`(int)pfVar17[1]`) daba 0 para
        // cualquier SubType chico, asi que la rama NUNCA se ejecutaba.
```

### Línea 452 en `Tamachan_Create`

```cpp
// lifetime: DWORD, no float (fix 2026-08-16)
```

### Línea 939 en `Tamachan_Create` — antes de `fVar24 = (float10)RequestTerrainHeight(pfVar17[4], pfVar17[5]);`

```cpp
    // BUG-FIX 2026-04-28: pass explicit (xf, yf) — effect pos at pfVar17[4]/[5]
```

### Línea 1049 en `Tamachan_Create`

```cpp
// lifetime: DWORD, no float (fix 2026-08-16)
```

### Línea 1088 en `Tamachan_Create`

```cpp
// lifetime: DWORD, no float (fix 2026-08-16)
```

### Línea 3782 en `Tamachan_Create` — antes de `pfVar17[0x18] = 2.8026e-44;`

```cpp
  // 2026-09-03 (Aqua Beam desplazado): el vector de avance de la estela de los
  // tipos 1210/1211/1212 sale de `VectorRotate(in1, in2, (float *)i + 48)` con
  // IDA poniendo `in1 = (0, -50, 0)` (o `(0, -15, 0)` para el 1212).  El port
  // escribia `local_6c = fVar27; local_64 = fVar27;` -- o sea X y Z tomaban un
  // valor sobrante de otra rama de la funcion en vez de 0.  El campo +0xC0 es
  // el paso que `RenderEffects` usa para los 30 sprites 1176 de la estela, asi
  // que la estela avanzaba en una direccion arbitraria: el Aqua Beam nacia bien
  // pero se dibujaba corrido.
```

## `src/Render/Effect_Tick.cpp`

### Línea 25 en `Effect_TickAll` — antes de `float *pfVar1 = (float *)DAT_07b11670;`

```cpp
  // BUG-FIX 2026-04-28: pool real DAT_07b11670[200 × 0x1bc] (2026-08-15: era 124).
```

### Línea 31 en `Effect_TickAll` — antes de `__try {`

```cpp
      // ── OWNERDBG (temporal, 2026-08-16) — red de seguridad + diagnostico ───
      // Tras corregir la lectura del owner (slot+252, se leia como float y daba
      // ~0 en 39 sitios), las ramas que comparan `owner == Hero` por fin se
      // ejecutan — y ahi aparecio un AV `0xC0000005 param1=0x10`, o sea un
      // deref de `owner + 0x10` con **owner NULL**: algun caller crea el efecto
      // sin dueno. En el binario eso no pasa (el owner siempre es valido para
      // los tipos que lo deref), asi que el original no valida nada.
      // Hasta ubicar al caller, contenemos el crash: se desactiva ese slot y se
      // loguea su Type/SubType UNA vez por tipo — ese dato identifica al
      // culpable sin tener que reproducir con debugger.
```

### Línea 71 en `Joint_TickAll` — antes de `char *pcVar1 = DAT_07b27150;`

```cpp
  // BUG-FIX 2026-04-28: pool real DAT_07b27150[500 × 0x9d8] (2026-08-15: era 200).
```

### Línea 85 — antes de `void __cdecl`

```cpp
// Effect_DrawRing — Effect_DrawRing
// Draws a cylindrical ring effect by emitting GL_QUADS segments along a helical
// arc. Uses EulerToMatrix to build rotation matrix from Euler angles, and
// Vector_Rotate (EulerToMatrix3x4) to transform each ring-segment midpoint.
// param_1: texture slot
// param_2: center position float[3]
// param_3/4/5: radii bit-patterns (float bits in undefined4) for inner/outer ring corners
// param_6: z offset base
// param_7: alpha channel param (color for inner verts, packed bits)
// param_8: V-coord offset
//
// BUG-FIX 2026-04-26: el Ghidra-decomp original tenía decenas de variables
// `local_<N>` declaradas escalares pero usadas como vec3 contiguos pasados a
// Vector_Rotate (matrix×vector) y luego leídos en bloque por glVertex3fv con
// `((int)&local_f0 + iVar5)` y stride de 12. MSVC no preserva ese layout →
// las posiciones de los 4 vértices del quad eran basura → en char-select el
// efecto rojo del char seleccionado se renderizaba como una línea horizontal
// en el suelo (mile-of-vertices) en vez de un anillo vertical alrededor del
// personaje. Reemplazado por arrays float[4][3] explícitos.
// Mismo patrón que Camera_BuildMouseRay (mouse-ray) y FUN_004f70b0 (terrain normals).
```

### Línea 218 en `RenderPlane` — antes de `glRotatef(*(float*)&param_4, 0.0f, 0.0f, 1.0f);`

```cpp
  // BUG-FIX: 0x3f800000 son los bits de 1.0f. Pasarlos como int → C
  // los castea int→float = 1065353216.0f → eje Z mal definido +
  // glRotatef tira basura. Idem para los UVs (0x3f800000 → 1.06e9).
```

### Línea 257 en `Effect_TickFade` — antes de `int *piVar3 = (int*)DAT_07c74ec8;`

```cpp
  // BUG-FIX 2026-04-28: pool real DAT_07c74ec8[40 × 0x1bc].
```

### Línea 296 en `Effect_TickFlare` — antes de `pfVar3 = (float *)DAT_07c82cdc;`

```cpp
  // BUG-FIX 2026-04-28: pool real DAT_07c82cdc[63 × 0x70].
```

### Línea 301 en `Effect_TickFlare` — antes de `const int __pcnt = *(int*)&pfVar3[0xb] - 1;`

```cpp
      // 2026-09-02: IDA MovePointers (0x004794A0) L20-21 -> `v1 = v0[11] - 1;
      // v0[11] = v1;` con `v0` = _DWORD*: el contador y el TIPO del slot son
      // ENTEROS, no floats.  El port los leia con `(int)float`, o sea convertia
      // numericamente un bit-pattern -> casi siempre 0: el contador nunca bajaba,
      // el efecto no moria nunca y el ramp de alpha (+40) quedaba clavado en 0.
      // La mezcla dentro de la misma funcion delata el bug: dos lineas mas abajo
      // el MISMO campo se lee bien con `*(int*)&fVar1 == 7` / `== 1205`.
```

### Línea 346 — antes de `void DamageNumbers_Tick(void)`

```cpp
// MovePoints — Effect_TickSpark
// = IDA `MovePoints` @0x479380: tick de los NÚMEROS DE DAÑO. Por slot activo:
// decrementa el contador de delay; cuando dispara, sube el número (pos.z +=
// lifetime), decae el lifetime 0.3, lo desactiva al llegar a 0, y encoge la
// escala 5.0 por tick con piso en 15.
//
// 2026-08-15 BUG-FIX (los números salían pero quedaban FIJOS en pantalla y no
// desaparecían): esto iteraba `DAT_07c80128`, declarado en globals.cpp como un
// global PROPIO de 100×0x70 ("spark-effect pool"). En el binario `unk_7C80128`
// no es un pool aparte: es `DAT_07c80110 + 0x18` — el MISMO pool de números que
// llena `CreatePoint` y recorre `RenderPoints`, sólo que las tres funciones lo
// abordan desde offsets distintos del slot y usan índices relativos.
// Al tocar otra memoria, el tick corría sobre un pool siempre vacío: los
// números nacían y nadie los movía ni los expiraba.
// IDA: MovePoints
```

## `src/Render/Entity_AnimationLegacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

### Línea 89 en `Calc_RenderObject` — antes de `*(BYTE *)((int)this_+0x44) = 0;`

```cpp
        // Contorno de seleccion (hover sobre monstruo/NPC).  IDA 0x4FAA70
        // case 1: DOS pasadas en RENDER_COLOR (flag 64), primero un contorno
        // ancho y oscuro y despues uno mas fino y claro, con un color por
        // pasada que depende de Kind (4 = NPC).  2026-09-18: el port hacia
        // una sola pasada, con dos colores mal (0.4 y 0.02 en vez de 0.1 y
        // 0.01) y sin el caso Kind == 4 del factor de escala.
```

### Línea 157 en `BMD_Animation` — antes de `float priorFrame;`

```cpp
    // BUG-FIX CRÍTICO: param_3 es realmente `float PriorFrame` (IDA firma),
    // no un entero. La firma C nuestra lo declara `unsigned int` porque el
    // caller en Calc_RenderObject lo carga con *(DWORD*)(entity+0x10c) y los bits
    // del float caben en un DWORD. Hay que reinterpretar las bits → float
    // y truncar a int (== IDA: v12 = (__int64)PriorFrame; v39 = v12).
    // Antes calculábamos v39 = (int)(1.0 - fFrac) que es 0 ó 1 siempre →
    // cada bone muestreaba el MISMO keyframe → geometría explotada.
```

### Línea 185 en `BMD_Animation` — antes de `float posA[4], posB[4];`

```cpp
                    // root bone: leer Euler angles (3 floats, stride 0xc) y
                    // convertir a quat via EulerToQuat (EulerToQuat).
                    // BUG-FIX: antes faltaba el componente Y (posA[1]/posB[1]).
                    // EulerToQuat lee los 3 componentes (línea 25: v5 = a1[1]*0.5)
                    // → con Y=stack garbage el quat salía arbitrario.
                    // IDA: v43[1] = *(_DWORD *)(v21 + 4); (Y sin ajuste HeadAngle)
```

### Línea 222 en `BMD_Animation`

```cpp
// BUG-FIX: pass float bits, not truncated int
```

### Línea 224 en `BMD_Animation` — antes de `float rot33[12] = {0};`

```cpp
                // quaternion to 3x3 rotation matrix, embedded in a 3x4.
                // BUG-FIX: QuatToMatrix solo escribe posiciones [0,1,2,4,5,6,8,9,10]
                // (9 floats de 3x3). Las posiciones 3,7,11 (columna de translación)
                // quedaban sin inicializar. R_ConcatTransforms las lee como translation,
                // así que generaba bone matrices con translate = stack garbage →
                // vértices astronómicos. Zero-init + inyectar tX/tY/tZ abajo.
```

### Línea 259 en `BMD_Animation` — antes de `float sc = *(float*)((char*)this_ + 0x68);`

```cpp
                            // BUG-FIX 2026-05-03 (cross-ref con 5.2 ZzzBMD.cpp:153-159):
                            // El IDA decompile mostraba solo 3 escalas de diagonales
                            // (matrix[0][0], [1][1], [2][2] = offsets 0/5/10) — eso
                            // se LEÍA mal. El source 5.2 limpio muestra que se
                            // escalan los 9 elementos rotacionales (3x3 completa):
                            //   for (y=0; y<3; y++)
                            //     for (x=0; x<3; x++)
                            //       ParentMatrix[y][x] *= BodyScale;
                            // Sin escalar los 6 off-diagonal, la rotación queda
                            // deformada → bone matrices torcidas → vertices con
                            // posiciones distorsionadas → "imp-like" body parts.
                            // Translate flag escribe BodyOrigin en columna [3].
```

### Línea 320 en `Skeleton_Transform` — antes de `float* local_68_base = (float*)&DAT_077e298c;  // base del mesh actual`

```cpp
    // BUG-FIX: el original (IDA sub_4404E0 lines 84/172/206) mantiene DOS
    // punteros — v37 es la BASE del mesh actual y v35 = v37 al entrar a
    // cada mesh, para que ++v35 itere por normales y v37 += 15000 al
    // final del mesh avance a la base del siguiente mesh ABSOLUTAMENTE.
    // Usar un único puntero (como hacía el port anterior) acumulaba el
    // offset de normales a la suma de bases, desalineando los slots que
    // BMD_DrawMesh lee en DAT_077e298c + meshIdx*15000. Resultado: mesh 0
    // OK, mesh 1+ leía basura (ceros) → per-vertex intensity=0 → vertex
    // colors=0 → velas del ship y otros meshes se ven NEGROS.
```

### Línea 333 en `Skeleton_Transform` — antes de `static int s_xform_dbg = 0;`

```cpp
    // 2026-05-03 bumped 16→200 to capture in-game frames after login.
```

### Línea 365 en `Skeleton_Transform` — antes de `{`

```cpp
    // GUARD 2026-07-16: si el modelo (this_) tiene el puntero Meshs (+0x28) NULL/
    // garbage o un meshCount insano (preview char del panel crear-personaje sin
    // modelo válido), abortar antes de deferenciar → evita crash en Skeleton_Transform.
```

### Línea 386 en `Skeleton_Transform` — antes de `{`

```cpp
        // GUARD 2026-07-16: el preview char del panel crear-personaje puede quedar
        // con una malla cuyo array de vértices/normales es NULL (modelo sin cargar)
        // → deref de near-null en el loop → crash 0x5A3797 en Skeleton_Transform. Si la
        // malla es inválida, se saltea (avanzando los offsets per-mesh) para no
        // crashear. Loguea una vez el entity_type para diagnosticar la raíz.
```

### Línea 418 en `Skeleton_Transform` — antes de `const int nBonesGuard = (int)(short)*(short*)((char*)this_ + 0x22);`

```cpp
        // GUARD 2026-09-07: el indice de hueso del vertice (`*vs`) indexa
        // `BoneTransform` con stride 0x30, y ese bloque lo aloca
        // CreateCharacterPointer como `operator_new(48 * numBones)`.  Un vertice
        // que referencie un hueso fuera de rango lee cientos de KB despues del
        // bloque -> AV dentro de Vector_Transform (crash reportado al romper la
        // puerta de Blood Castle: addr 0x00505B72 = Vector_Transform+0x112,
        // param1=0x02B71000, page-aligned = tipico de salirse de una alocacion).
        //
        // El original no acota; aca se saltea el vertice y se loguea UNA vez con
        // el tipo de entidad, el modelo y los dos numeros, para poder atacar la
        // causa (que el modelo y el BoneTransform no correspondan) con datos.
```

### Línea 480 en `Skeleton_Transform` — antes de `local_68 = local_68_base;   // v35 — light buffer (1 float/vertex)`

```cpp
        // normal transform + per-vertex lighting (IDA sub_4404E0 L172-206).
        // BUG-FIX 2026-07-15: son DOS buffers separados —
        //   • normal transformada → NORMAL buffer (DAT_06f433bc + local_64,
        //     stride 3 floats), que lee el chrome env-map en BMD_DrawMesh.
        //   • intensidad de luz (dot con la dir de luz) → LIGHT buffer
        //     (DAT_077e298c, stride 1 float).
        // El port anterior escribía la normal al LIGHT buffer (local_68) y la
        // sobreescribía con la luz → la luz salía bien pero el NORMAL buffer
        // quedaba en cero → todos los UV chrome colapsaban a un texel → armas
        // y sets con glow chrome se veían como barra/relleno sólido dorado.
        // Ahora `v20 = &unk_6F433BC + v9` (normal) y `v35 = v37` (light) son
        // punteros independientes, fiel a IDA.
```

### Línea 515 en `Skeleton_Transform` — antes de `local_68_base += 15000;`

```cpp
        // FIX: avanzar la BASE del mesh 15000 floats (60000 bytes)
        // ABSOLUTAMENTE — equivale a `v37 += 15000` en IDA line 206.
```

### Línea 554 — antes de `// Chat_ValidateInputCommand — implemented in src/UI/Chat.cpp`

```cpp
// CSimpleModulus crypto (CSimpleModulus_Encode/cd20/cca0/ce30 + helpers) moved to
// src/Net/Crypto.cpp (B3 refactor 2026-05-07, 282 lines).
```

## `src/Render/Entity_DrawByType.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Entity_DrawByType.cpp — Draw_RenderObject @ 0x004fae00
// Dispatches entity render based on entity type (*(short*)(param_1+2)).
//
// param_1 = entity data pointer (stride 0x394, entity array at DAT_07abf5d0)
// param_2 = second entity pointer (unused in most cases)
// param_3 = extra param (used in type 0x14a HashTable block)
// param_4 = flag byte (player-focused override for some types)
//
// Render model context:
//   this = (void*)(DAT_05828d58 + entity_type * 0xbc)
//   this+0x48/4c/50 = tint RGB floats
//   this+0x88       = render layer mask
//
// Entity fields used:
//   param_1+0x02  = entity_type (short)
//   param_1+0x04  = unk_04 (int)
//   param_1+0x58  = facing/anim_id (float/int) — -2 = invisible guard
//   param_1+0x68  = world_x
//   param_1+0x6c  = world_y
//   param_1+0x70  = world_z
//   param_1+0x74  = render_mode byte
//   param_1+0x78  = entity_flags (uint) — bit0=player flag, bit1=PvP
//   param_1+0x7c  = unk_7c byte
//   param_1+0x84  = alive state
//   param_1+0x8b  = flash counter (10=full red, >0=fading)
//   param_1+0x105 = anim_state (byte) — 6=death trigger
//   param_1+0x168 = scale float
//   param_1+0x17c = base level (int)
//   param_1+100   = height_int
//
// ── Anti-tamper note ──────────────────────────────────────────────────────────
// Type 0x14a contains an ~300-line HashTable encode/decode block operating on
// DAT_083a7c00 (a local slot key). Per CLAUDE.md policy this is obfuscation,
// not game logic. The post-hash render logic IS implemented.
```

### Línea 49 — antes de `struct MoltSilhouette {`

```cpp
// ─────────────────────────────────────────────────────────────────────────────
// Silueta de Molt — clase de vtable off_552588 (IDA sub_40A660).
//
// Vtable: [0] 0x40A6C0 dtor, [1] 0x40A6F0 armar, [2] 0x40A830 liberar.  Layout
// (0x20 bytes): +0 vtable, +4 short, +8 buffer, +12..+20 direccion de la luz,
// +24 cantidad de aristas, +28 aristas (10 bytes c/u: v0, v1, malla, uv0, uv1).
//
// Desviacion: el binario hace `operator_new(0x20)` en CADA frame y nunca libera
// el objeto (el metodo [2] solo libera sus buffers), y llama a los metodos por
// la vtable.  Aca el objeto vive en el stack y los metodos se llaman directo:
// mismo resultado, sin la fuga.  2026-09-18: antes el port llamaba por una
// vtable que nunca se instalaba (WidgetB_CtorFull la saltea) y sin `this`.
// ─────────────────────────────────────────────────────────────────────────────
```

### Línea 178 en `Draw_RenderObject`

```cpp
// 0x3ca3d70a R (el port tenia 0.15)
```

### Línea 216 en `Draw_RenderObject`

```cpp
// 0x3f19999a G (el port tenia 0.575)
```

### Línea 359 en `Draw_RenderObject`

```cpp
// 0x3f4ccccd (el port tenia 0.6)
```

### Línea 491 en `Draw_RenderObject`

```cpp
// 0x3f4ccccd (el port tenia 0.6)
```

### Línea 540 en `Draw_RenderObject`

```cpp
// 0x3f19999a (el port tenia 0.575)
```

### Línea 566 en `Draw_RenderObject` — antes de `if (sType == 0x14a) {`

```cpp
    // 2026-09-04: aca habia un segundo bloque para sType 0x14a gateado por
    // `SceneFlag == 2` (login).  Draw_RenderObject (0x4FAE00) NO consulta
    // SceneFlag en ninguna parte -- su switch tiene UN solo `case 330`.  Era
    // una copia del bloque de abajo sin el chequeo de frames, o sea el patron
    // [[bloque-duplicado-dentro-de-una-funcion]].  Removido.
```

### Línea 573 en `Draw_RenderObject` — antes de `const bool bDead = (param_1[0x105] == 6);`

```cpp
        // ── Estatua de Blood Castle (entidad 330) ────────────────────────────
        // IDA Draw_RenderObject (0x4FAE00) case 330:
        //     if ( o->CurrentAction == 6 ) goto LABEL_137;   // MONSTER01_DIE
        //     <bloque de hash-table que descifra MoveSceneFrame>
        //     if ( MoveSceneFrame - o[380] >= 25 ) { <3 capas>; goto LABEL_146; }
        //     if ( o->CurrentAction == 6 ) { LABEL_137: *(BYTE *)o = 0; }
        //     PlayBuffer(106);
        //     sub_441BE0(o->model, 0, 260);
        //
        // O sea la rama de FRAGMENTOS (mesh 260 + sonido 106) se usa para dos
        // cosas: los primeros 25 frames desde el spawn (la estatua se
        // materializa rompiendose) y la MUERTE, que ademas desactiva la entidad
        // (`*(BYTE *)o = 0`) para que corra un unico frame.
        //
        // `o + 380` lo siembra CreateCharacterPointer con el MoveSceneFrame del
        // spawn.
        //
        // 2026-09-04: el port usaba `param_3` en vez de MoveSceneFrame -> la
        // resta daba siempre < 25 y la estatua quedaba rompiendose EN LOOP.
        // 2026-09-05: y con `CurrentAction == 6` caia a LAB_standard_render en
        // vez de a la rama de fragmentos, asi que al matarla no se veia la
        // animacion de romperse.
```

## `src/Render/Entity_DrawSetup.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Entity_DrawSetup.cpp
// IDA: RenderPartObjectEffect (0x00504B50)
//
// Entity_SetColorAndRender — resolves anim-mode, sets model color from light
// array, dispatches to BMD_SetupRenderByType / RenderPartObjectBodyColor / Entity_SetModelColorAlt, applies
// optional "flashing" effect on buffed entities.
//
// Signature (faithful Ghidra port):
//   void __cdecl RenderPartObjectEffect(int param_1, int param_2, float *param_3,
//                              float param_4, uint param_5, byte param_6,
//                              undefined4 param_7, uint param_8)
//
// param_1 — entity pointer
// param_2 — model slot index → this = DAT_05828d58 + param_2 * 0xbc
// param_3 — float[3] light color (R,G,B)
// param_4 — animation frame / scale
// param_5 — raw flags (anim_mode = (param_5 >> 3) & 0xf)
// param_6 — status bits; (param_6 & 0x3f) != 0 → buff flashing
// param_7 — pass-through (unused in default path)
// param_8 — draw flags (bit 0x400 forces anim_mode 0; bit 0x100 passed to subs)
//
// ── BUG-FIX (2026-04-20) ───────────────────────────────────────────────────────
// La port anterior pasaba `(int)(uintptr_t)param_3` (puntero heap) como 2do
// argumento `int flags` de BMD__RenderBody. Resultado en log:
//   BMD_Draw flags=0xa0b5790 bodyLight=(0,0,0)
// La función real de Ghidra NO llama BMD__RenderBody en el default path — sólo
// BMD_SetupRenderByType. Además BMD_SetupRenderByType toma 5 args (this, entity, model_slot,
// scale, flags), no 3. Esta re-port arregla ambos.
//
// ── Entity-type switches (Ghidra) ──────────────────────────────────────────────
// Hay ~15 branches para tipos especiales (0x1f9, 0x33f, 0x3be, 0x341, 0x342,
// 0x315, 0x316, 0x361-0x365, etc.). Para Player (0x186) y body-parts
// (0x390-0x3ac) NINGUNA coincide → todos caen al default path implementado aquí.
// Los branches especiales se dejan fuera; si aparecen sprites con tipo raro
// (dragones, phoenixes, bosses específicos), habrá que portarlos.
//
// Called from: Entity_DrawAt @ 0x00505A10
```

### Línea 387 en `RenderPartObjectEffect` — antes de `uint ItemLevel = (uint)ItemLevelFx;`

```cpp
        // ── BUG-FIX 2026-04-27: PORT del +N item-level glow logic IDA ───────
        // (Antes solo copiaba light directo y hacía UN render — el +9/+11 glow
        // visible del Mu Online viene de DOBLE render via RenderPartObjectBodyColor
        // con flags 0x44 + 0x48). Ver RenderPartObjectEffect IDA lines 415-510.
        //
        // ItemLevel se extrae de param_5 (flags con level en bits 3-6).
        // BUG-FIX: línea 52 ya hizo `param_5 = ((int)param_5 >> 3) & 0xf;` así que
        // param_5 ES el ItemLevel directamente. El shift adicional daba 0xb→1.
```

## `src/Render/Entity_Render.cpp`

### Línea 158 — antes de `// Entity_PrepareRender @ 0x004fc030`

```cpp
//
// 2026-09-25: aca habia una SEGUNDA copia de 183 lineas, marcada desde hacia
// tiempo como CODIGO MUERTO (nadie llamaba a Entity_Render()) y con un bug
// propio: caminaba el pool con el ancla equivocada, leyendo el flag `active` en
// Items+0 en vez de Items+72.  Se dejaba porque documentaba la funcion, pero una
// copia muerta con un bug adentro es una trampa -- el proximo arreglo podia caer
// ahi.  Borrada; la viva quedo con el nombre de IDA.
```

### Línea 203 — antes de `#if 0`

```cpp
// Entity_RenderAll_3D — Entity_TickAll  (DISABLED 2026-04-26: duplicate definition;
// active version is in src/Render/Entity_RenderAll_3D.cpp with diag tracers)
```

### Línea 255 — antes de `uint RenderBugs(void)`

```cpp
// RenderBugs — RenderBugs (Entity_VisibilityCheckAll)
// Itera el pool de butterflies/effect-entities (DAT_083a1218, 10 entries × 0x1BC).
// Por cada entry activo: frustum test, si visible y (owner es player o type==0x330)
// llama a Entity_PrepareRender (render). Type 0x330 además spawnea sparkle.
//
// BUGFIX 2026-04-26: tenía AUTO-SKIP early-return. Reactivado y reescrito para
// usar nuestro DAT_083a1218 array (stride 0x1bc, 10 entries). El layout original
// usaba pcVar4 = pool_base + 0x160 con índices negativos (-0x160 = +0); aquí
// indexamos directo desde el base del entry.
//
// Entry layout (offsets desde base):
//   +0x000  byte   active flag
//   +0x002  short  Type (816=butterfly, 195=elf-bug, 267=MG fire, 0x330=special)
//   +0x010  float  pos.x
//   +0x014  float  pos.y
//   +0x018  float  pos.z
//   +0x0FC  DWORD  owner (entity ptr)
//   +0x160  byte   visibility flag (escrito por frustum test)
```

### Línea 275 en `RenderBugs` — antes de `if (!(SceneFlag == 5 || SceneFlag == 4 || SceneFlag == 2)) return 0;`

```cpp
    // 2026-05-07: re-habilitado. Antes estaba TEMP DISABLED por flicker en
    // char-select. Ahora gated por SceneFlag == 5 (in-world) para evitar
    // ese path. Port FIEL desde IDA mu97k-src-IDA/raw/00500970_RenderBugs.c.
    //
    // Pool: DAT_083a1218 (10 entries × 0x1BC = 4440 bytes).
    // Slot layout:
    //   +0     active flag (byte)
    //   +2     type code (WORD): 195=elf-bug, 267=MG fire, 816=butterfly, 0x330=special
    //   +16    pos.x (float)
    //   +20    pos.y (float)
    //   +0xFC  owner entity ptr (DWORD)
    //   +0x160 visible flag (byte)
    //
    // Logic per IDA: if active → frustum test → if visible AND (owner is
    // player [type 390] or type==816) → PrepareRender + sparkle for type 816.
    // BUG-FIX 2026-07-16: se gateaba a state 5/2, EXCLUYENDO char-select (state 4).
    // Eso rompía el render de las monturas (Uniria bug=195 / Dinorant bug=267) que
    // se crean con CreateBug y se dibujan acá vía Entity_PrepareRender → Draw_RenderObject.
    // El IDA no tiene gate interno — Scene_CharSelect (0x523B30 L142) llama RenderBugs
    // directamente. Se agrega state 4.
```

## `src/Render/Entity_RenderAll_3D.cpp`

### Línea 105 en `Entity_RenderAll_3D` — antes de `if (pcVar1 == DAT_07abf5d8 && SceneFlag == 5 && *pcVar1 != '\0') {`

```cpp
        // BUG-FIX 2026-04-28: el IDA original tiene branches con conditions
        // que en nuestro build nunca matchean (flag bit 2, visibility flag).
        // Resultado: hero nunca renderiza. Detectamos hero ANTES que cualquier
        // otra cosa y forzamos el render.
```

### Línea 121 en `Entity_RenderAll_3D` — antes de `RenderCharacter((undefined4 *)pcVar1, (undefined4 *)pcVar1, (undefined4 *)0);`

```cpp
            // FIX 2026-07-24: el 3er param de RenderCharacter es
            // el flag de HOVER/highlight (dibuja el borde de selección).  El IDA
            // pasa `(slot == SelectedCharacter || SelectedNpc)`, y el Hero está
            // EXCLUIDO de esos → el original lo dibuja con 0.  Este forced-render
            // hardcodeaba 1 → el PJ tenía el borde de hover pegado siempre.
            // Debe ser 0 (el Hero nunca es su propio target de hover).
```

## `src/Render/Entity_Render_3D.cpp`

### Línea 116 — antes de `float * __cdecl Entity_SpawnEffects(int param_1)`

```cpp
// 2026-05-03: AUTO-SKIP removed. The only absolute-bound loop was the case
// 0x27 portal bone walker (`while (pfVar3 < 0x69716cd)`), already replaced
// with explicit count of 4. Other paths use proper bounds or symbol math.
```

### Línea 145 en `Entity_SpawnEffects` — antes de `float     local_pos_buf[3];`

```cpp
    // BUG-FIX: many paths pass &local_18 as if it were a contiguous float[3]
    // to TransformPosition / CreateSprite / Particle_Spawn. Keep the original
    // names as aliases over a real 3-float buffer so world positions are stable.
```

### Línea 165 en `Entity_SpawnEffects` — antes de `BMD_TransformPosition(pModel, (float *)&DAT_06970c1c, local_c, &local_18, '\0');`

```cpp
        // BUG-FIX 2026-07-12: translate=0 (fiel al IDA Entity_SpawnEffects). El
        // barco tiene 2 antorchas (bones c1c/c4c). El translate=1 de la sesión
        // previa se puso a ciegas (flares invisibles por bug texcoords) y sumaba
        // la pos world de más → 1 flare desplazado al lado del barco. La bone
        // matrix ya está en world-space, translate=0 da la pos correcta.
```

### Línea 316 en `Entity_SpawnEffects` — antes de `pfVar3 = (float *)&DAT_0697160c;`

```cpp
            // BUG-FIX 2026-05-03: bound `< 0x69716cd` was an absolute source-binary
            // address (= &DAT_0697160c + 0xC1). DAT_0697160c is a macro into
            // g_BoneScratch in our build; comparing pfVar3 to literal 0x69716cd
            // is junk. The iteration count is 4 ("array de 4 huesos portal,
            // stride 0x30" per the file header).
```

## `src/Render/Entity_UpdateRender.cpp`

### Línea 23 — antes de `// All FUN_* prototypes and DAT_* globals come from stdafx.h → functions.h / globals.h`

```cpp
// Entity_UpdateRender.cpp  —  RenderCharacter @ 0x00456770  (2195 lines in Ghidra)
//
// Per-frame visual update for a single entity.  Called from Entity_RenderAll_3D
// for every visible entity.  Drives:
//   - Skill-channel widget objects  (channeling beams / barriers)
//   - Entity_PrepareRender          (bone + AABB compute)
//   - Per-skill / per-anim-state particle effects on entity bones
//   - Weapon-slot rendering         (RenderLinkObject)
//   - Per-entity-type NPC / monster special effects (large outer switch)
//
// param_1  — player / local entity  (int*, stride 0x394, base DAT_07abf5d0[0])
// param_2  — entity being rendered  (undefined4* / puVar13 in Ghidra)
// param_3  — zone-id or context param (treated as int for zone-scale calc)
//
// Anti-tamper: ~30 HashTable_GetIndex / HashTable_Insert / XOR-encode blocks are
// interspersed throughout; per CLAUDE.md those are pure obfuscation and are omitted.
```

### Línea 48 en `PtrAsFloatBits` — antes de `int  RenderCharacterBackItem(int c, int o);`

```cpp
    // 2026-05-04: back-weapon render decision. Returns 1 if weapon was rendered
    // on back (LinkBone 47). When 1, Render_PlayerWeaponLoop should be skipped.
```

### Línea 51 en `PtrAsFloatBits` — antes de `void HeroEquipWatchdog(int c);`

```cpp
    // 2026-05-04: per-frame watchdog que restaura wings/weapons/pendant del
    // hero si fueron reseteados a -1 después de F3/03.
```

### Línea 107 en `RenderCharacter` — antes de `float local_60_buf[3] = { 1.0f, 1.0f, 1.0f }; // RGB color tint`

```cpp
    // ── BUG-FIX 2026-04-27: declarar como arrays contiguos para que `&local_X`
    // pasado a funciones que leen/escriben 3 floats consecutivos (CreateSprite,
    // BMD_TransformPosition, etc.) no caiga en stack slots aleatorios. Mismo patrón ya
    // arreglado en Sprite/Math_3D/Scene_CharSelect.
```

### Línea 135 en `RenderCharacter` — antes de `char cVar6 = *(char *)((int)param_1 + 0x2eb);  // tipo de monstruo`

```cpp
    // (2026-08-22: se llamaba "magic_channel_flag"; es el tipo que escribe
    //  CreateMonster.  La logica ya comparaba contra tipos, solo mentia el nombre.)
```

### Línea 146 en `RenderCharacter` — antes de `void *puVar8 = operator_new(100);`

```cpp
            // 2026-09-04 FIX (crash 0xC0000005 param1=0xCDCDCDD5 al romper la
            // puerta de Blood Castle).  IDA 0x456770 L308-323:
            //     v9 = (float *)(block + 4);
            //     *(_DWORD *)block = 1;                  // prefijo de count
            //     eh_vector_ctor(block + 4, 0x60, 1, sub_4093A0, sub_4093C0);
            //     sub_4093E0(v9, ...); sub_409250(v9, ...); sub_409250(v9, ...);
            //     *(_DWORD *)(c + 388) = v9;             // guarda el OBJETO
            //
            // El port construia en `block + 4` (bien) pero guardaba `block` en
            // c+388.  El tick de la tela (`sub_408CB0`) arranca con una llamada
            // por vtable -- `(*(void(**)(_DWORD*))(*a1 + 8))(a1)` -- asi que leia
            // el prefijo de count como si fuera la vtable.  Sin inicializar, el
            // CRT debug lo deja en 0xCDCDCDCD y `*(0xCDCDCDCD + 8)` da
            // 0xCDCDCDD5, que es exactamente el param1 del crash.
            //
            // Los tipos de monstruo de este case (+0x2EB: 89, 95, 112, 118, 124,
            // 130, 136) incluyen el 130 = "Magic Skeleton", que es el que aparece
            // al caer la puerta del evento.
```

### Línea 192 en `RenderCharacter` — antes de `// ── 4. Skill-state / anim-state particle effects ─────────────────────────`

```cpp
    // -- 3. (bloque removido 2026-09-04) --------------------------------------
    // Aca habia una "LOD / sparkle (distance check)" que NO existe en IDA: era
    // una copia mal leida del gate del render de cuerpo (RenderCharacter
    // L346-380), que ya esta portado completo mas abajo en la seccion 7a.
    // Confundia dos campos del objeto:
    //     puVar13 + 0x5a  = +360 -> NO es "distancia en pantalla", es el ALPHA
    //                       (por eso el umbral era _DAT_005528b8 = 0.3, que es
    //                       el `alpha >= 0.3` de IDA)
    //     puVar13 + 6     = +24  -> Position.Z
    // y hacia `if (World 11..16 && alpha < Z) Z = alpha;`, o sea clavaba la Z de
    // TODA entidad no-jugador de Blood Castle en ~1.0 -- los monstruos y el
    // Archangel quedaban por debajo del piso.  El heroe no, porque el gate
    // `entity_type != 390` lo excluye: de ahi que se viera al pj sobre el puente
    // y a todo lo demas hundido.
    // Lo que IDA hace en ese punto es
    //     if (World 11..16 && o->m_bActionStart && c->Dead) {
    //         th = RequestTerrainHeight(o->Position[0], o->Position[1]);
    //         if (th < o->Position[2]) o->Position[2] = th;
    //     }
    // que es exactamente lo que ya hace la seccion 7a.  Ademas llamaba
    // `GL_SetBlendAdditive()` suelto para cada entidad, ensuciando el estado GL.
```

### Línea 273 en `RenderCharacter` — antes de `GridSpring_Create(puVar8, PtrAsFloatBits(param_1), 0x13, 10.0f, 0.0f,`

```cpp
            // IDA L493: sub_408130(v33, c, 19, 10.0, 0, 5, 15, 30.0, 300.0, tex, tex, 0x1100)
            // El port tenia 240.0 / 500.0 — mal decodificados de los enteros del
            // decompile (1106247680 = 0x41F00000 = 30.0, no 240; 1133903872 =
            // 0x43960000 = 300.0, no 500). La capa salia 8x mas ancha.
```

### Línea 472 en `RenderCharacter` — antes de `*(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);`

```cpp
    // Luz base del cuerpo, desde el terreno + el ColorOffset del objeto.
    // IDA RenderCharacter L775-779.
    //
    // 2026-09-27 -- ACA ESTABA MEZCLADO EL TINTE DE PK, y ese era el bug.
    // El binario escribe la luz del cuerpo en DOS momentos distintos:
    //
    //   L775-779   c+800 = Light[] + ColorOffset      <- luz base (aca)
    //   L786       if (c == Hero) -> (1,1,1) x bebida
    //   L1020-1057 RenderPartObject ...................  el CUERPO usa esa luz
    //   L2310-2317 PK: si >= 6 -> (1.0, 0.1, 0.1)     <- segunda escritura
    //   L2342      RenderLinkObject(c + 672) .........  las ALAS usan el rojo
    //
    // O sea el rojo del PK se escribe DESPUES de dibujar el cuerpo, asi que en
    // el original solo alcanza a lo que se dibuja despues: las alas.  El port
    // habia fusionado las dos escrituras en esta sola, que corre ANTES del
    // render del cuerpo, y por eso el personaje PK salia rojo entero en vez de
    // con el cuerpo normal y las alas rojas.  La segunda escritura ahora vive
    // mas abajo, justo antes del bloque de armas y alas.
```

### Línea 502 en `RenderCharacter` — antes de `if ((void *)param_1 == DAT_07abf5d8) {`

```cpp
    // ── BodyLight del heroe + tinte de las bebidas (IDA RenderCharacter
    //    L786-L1009, `if (c == Hero)`) ──────────────────────────────────────
    // El heroe NO usa la luz del terreno que se acaba de calcular: la pisa con
    // (1,1,1) y despues le aplica el tinte segun los dos bits de bebida activa
    // de `CharacterAttribute + 40`, que escribe el handler del 0x29
    // (ReceiveHelperItem / PMSG_ITEM_SPECIAL_TIME_SEND) y limpia el timer.
    //   bit 0 -> Ale             (0.9, 0.5, 0.5) = rojizo
    //   bit 1 -> Remedy of Love  multiplica (0.5, 0.9, 0.5)
    // Todo lo que IDA tiene entre el gate y estas tres escrituras es el ruido
    // de hash-table que descifra CharacterMachine para leer el byte (omitido
    // por policy, ver CLAUDE.md).
    //
    // Va ANTES del render del cuerpo y ANTES de la segunda escritura de luz
    // (la del PK, mas abajo), igual que en IDA: L786 el heroe, L2310 el PK.
```

### Línea 541 en `RenderCharacter` — antes de `if (SceneFlag == 5 && param_1_ == DAT_07abf5d8) {`

```cpp
    // ── 2026-05-04: re-apply equipment stash si está reseteado ──────────────
```

### Línea 546 en `RenderCharacter` — antes de `}`

```cpp
        // 2026-08-10 — WATCHDOG REMOVIDO. Ya no hace falta: no había ningún
        // "escritor misterioso" del flag. +0x34E es **SafeZone**, no dead_flag,
        // así que valía 1 legítimamente con el héroe vivo parado en el pueblo;
        // el que estaba mal era el lector de abajo (bDead), que ahora usa el
        // dead real (+0x2FD, IDA ReceiveDie L18). El watchdog además forzaba
        // SafeZone=0 en cada frame, matando la música de pueblo, el bind del
        // arma a la espalda y el gate de "no atacar en zona segura".
```

### Línea 556 en `RenderCharacter` — antes de `if (sVar2 != 0x186 && *(BYTE *)((char *)puVar13 + 0x84) != 8) {`

```cpp
    // 2026-05-08: missing port — sin esto NPCs/monsters renderean SOLO efectos
    // especiales (entity-type switch al final) pero nunca su BODY geometry.
    // Resultado visual: monsters invisibles excepto por sparkles/particles.
    //
```

### Línea 640 en `RenderCharacter` — antes de `*(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);`

```cpp
            // IDA L1137-1139: la rama `< 6` REESCRIBE la luz base.  No es un
            // duplicado de la escritura de mas arriba: es el reset que separa
            // lo ya dibujado de lo que viene.  En el binario borra aca el
            // tinte de las bebidas que dejo el bloque del heroe, de modo que
            // el CUERPO (L1020, antes) lo tiene y las ALAS (L1239, despues) no.
            // Sacarlo hacia que el Ale tinara tambien las alas (2026-09-27).
```

### Línea 673 en `RenderCharacter` — antes de `bool Bind = false;`

```cpp
        // BUGFIX 2026-04-27: antes este bloque corría incondicionalmente y
        // duplicaba el render del crossbow del Elf — render en espalda Y en mano
        // → flicker visible cuando ambos paths competían por DAT_06989c9c.
        //
```

### Línea 759 en `RenderCharacter` — antes de `if ((World >= 11) && (World <= 16) &&`

```cpp
        // -- Arma del evento de Blood Castle sobre la espalda (EtcPart) -------
        // IDA LABEL_308: `if (World >= 11 && World <= 16 && c->EtcPart)`, con
        //     EtcPart 1 -> 570 (Staff)   2 -> 419 (Sword)   3 -> 546 (Bow)
        // y LinkBone 47.  `c->EtcPart` es el byte +0x2E8 (= param_1[0xba] con
        // param_1 como int*), que escribe el handler del 0x9B con el
        // EventItemLevel que manda el server.
        //
        // 2026-09-07: estaba DENTRO de `if (Bind)`, o sea sujeto a la rama de
        // "arma en la espalda".  En IDA vive en la rama contraria (`!Back ||
        // Type == -1`) y ademas Bind se fuerza a 0 en Blood Castle, asi que
        // siempre se alcanza.  Sintoma: el arco de la estatua no se dibujaba en
        // la espalda al levantarlo.  Verificado en el log del cliente: el server
        // manda `0x9B ... owner=9001 lvl=3` (3 = Bow) durante 80 paquetes.
```

### Línea 820 en `RenderCharacter` — antes de `int bBindBack = RenderCharacterBackItem((int)param_1, (int)puVar13);`

```cpp
        // ── Back-render decision (port WeaponView.cpp:49) ───────────────────
        // 2026-05-04: si bBindBack=1, renderiza armas en la espalda (LinkBone
        // 47) y se SALTA Render_PlayerWeaponLoop. Caso típico: safe-zone.
```

### Línea 834 en `RenderCharacter` — antes de `*(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);`

```cpp
    // ── Restaurar la luz base antes de las partes del cuerpo ────────────────
    //
    // DESVIACION FORZADA por el orden de este port.  En el binario las partes
    // del cuerpo se dibujan ANTES que las alas (L1020 vs L1239/L2342), asi que
    // el tinte de PK -- que se escribe entre medio, en L2310 -- alcanza solo a
    // las alas.  Aca el orden esta invertido: alas arriba, cuerpo abajo, con
    // lo cual una sola escritura no puede dejar las alas rojas y el cuerpo
    // normal: lo que tinta las alas tinta tambien el cuerpo.  Era el sintoma
    // reportado el 2026-09-27 ("el rojo continua al personaje").
    //
    // Se vuelve a poner la luz base justo antes del loop de partes.  El efecto
    // final es el del binario (alas rojas, cuerpo con su luz normal) sin tener
    // que reordenar los dos bloques de render, que estan muy anidados.
```

### Línea 862 en `RenderCharacter` — antes de `bool bSubTypeNpcRendered = false;`

```cpp
    // ── 7b. Body-part render loop (Ghidra RenderCharacter lines 1622-1700) ──────
    // Missing in previous port — this is what actually draws player geometry.
    // Player.bmd is skeleton-only (numMesh=0); body geometry lives in separate
    // BMD models (HelmClass##/ArmorClass##/PantClass##/GloveClass##/BootClass##)
    // referenced by entity equipment slots and rendered here via RenderPartObject
    // (Entity_DrawAt) using the parent entity's animated bones.
    //
    // Equipment slot layout (6 entries, stride 0x18 bytes):
    //   [0] +0x1e0  BodyPart[0] (extra / unused)
    //   [1] +0x1f8  BodyPart[1]  — Helm    (HelmClass## 0x390-0x393)
    //   [2] +0x210  BodyPart[2]  — Armor   (ArmorClass## 0x397-0x39a)
    //   [3] +0x228  BodyPart[3]  — Pant    (PantClass## 0x39e-0x3a1)
    //   [4] +0x240  BodyPart[4]  — Glove   (GloveClass## 0x3a5-0x3a8)
    //   [5] +0x258  BodyPart[5]  — Boot    (BootClass## 0x3ac-0x3af)
    // Per slot layout:
    //   +0x00 short  model_idx (-1 if empty)
    //   +0x02 byte   level
    //   +0x03 byte   option
    // Gate: *(param_1 + 0x34f) == 0 (not hide-equipment state).
    // ── 7a-bis. NPC con modelo de jugador + SubType propio (IDA L1012-1042) ──
    // 2026-08-08 PORT FALTANTE — "el Golden Archer / esqueleto de Lorencia no se
    // dibuja". CreateMonster case 236 (0x45CCF0 L803) crea la entidad como
    //     OpenNpc(390); c = CreateCharacter(Key, 390, ...);
    //     o->SubType (o+4) = 207;  o->Kind (o+132) = 4;  c+446 = 8;
    // o sea Type = 390 = MODEL_PLAYER. Con Type 390 el render cae en el loop de
    // body-parts de abajo, pero este NPC no tiene NINGUNA parte equipada
    // (+0x1e0…+0x258 todos -1) → no se dibujaba nada.
    //
    // El original tiene una rama previa: si Type == 390 y SubType está en
    // [206, 208] (MODEL_SKELETON1..3 — Data\Skill\Bones_Warrior / Bone_A /
    // Bone_C, cargados por OpenSkills 0x50B710), la entidad se dibuja como UN
    // solo modelo con RenderPartObject(SubType) y se SALTEA el loop de partes.
    // Hay dos variantes: NPC (Kind==4) en Lorencia (World==0) pasa
    // `8 * *(WORD*)(c+446)` como flags de render; el resto pasa 0.
```

### Línea 1016 en `RenderCharacter` — antes de `pvVar23 = model;`

```cpp
    // Antes era `pvVar23 = local_78` (= NULL) → TODOS los BMD_TransformPosition con
    // pvVar23 dereferenciaban NULL y crasheaban en model+0x68.
```

### Línea 1146 en `RenderCharacter` — antes de `{`

```cpp
        // ── Capa del Magic Gladiator (IDA RenderCharacter L667-743) ───────────
        // 2026-08-11: bloque que faltaba portar. La capa NO es una malla del
        // modelo: es un objeto de **tela** (cloth/verlet) construido con
        // `sub_408130`, texturizado con Robe01.jpg/Robe02.jpg = slots 490/491
        // (`OpenPlayerTextures` 0x507610 los carga y nuestro Model_Monsters.cpp
        // ya lo hacía; sólo faltaba el consumidor).
        //
        //   Targetd = 0;
        //   if ((c[444] & 7) == 3 && o->Type == 390) Targetd = 1;   // clase 3 = MG
        //   if (c[747] == 55)                        Targetd = 1;   // evento
        //   if (EnableSoccer && guild == hero-guild/rival-guild) Targetd = 1;
        //   if (SoccerObserver && guild == either observer team) Targetd = 1;
        //   if (Targetd && !c->Cloth) { crear + anclar; }
        //   luego: si sub_408900(0.005,5) → tick por vtable+0xC, si no DeleteCloth
        //
        // Slot: `o + 384` = flag creada, `o + 388` = puntero al sistema de tela
        // (mismo par que usan los otros dos cloth ya portados en este archivo).
```

### Línea 1219 en `RenderCharacter` — antes de `FUN_00408ff0((void *)pCloth);`

```cpp
                        // IDA: `(*(void (__thiscall **)(int,_DWORD))(*(_DWORD *)v49 + 12))(v49, 0);`
                        // La vtable `off_552520` no está portada (Widget_CtorBase
                        // tiene el vtable-set saltado), así que la indirección
                        // saltaba a basura → crash por ejecución (param0=8).
                        // 2026-08-11: leí la vtable del binario original
                        // (`Cliente armado/main.exe`, MD5 eb95ac…):
                        //     off_552520 = { 0x0045AAA0, 0x00408780,
                        //                    0x004089B0, 0x00408FF0 }
                        // o sea **+0xC = sub_408FF0**. Llamada directa al destino
                        // real en vez de la indirección.
```

## `src/Render/Font_Layout.cpp`

### Línea 23 en `Font_CreateTextDib` — antes de `memset(bmi, 0, 0x28);`

```cpp
    // BUG-FIX 2026-07-19: el "zero loop" de Ghidra escribía 10 veces sobre
    // biSize en vez de limpiar la cabecera (dejaba biSizeImage/biClrUsed con
    // basura). IDA hace `memset(DIB_INFO, 0, 0x28)`.
```

### Línea 29 en `Font_CreateTextDib` — antes de `const LONG fontW = (LONG)(*(float *)&DAT_083a7cc0);   // Bitmaps[0].Width`

```cpp
    // BUG-FIX 2026-07-19 (LA BURBUJA DE CHAT NO SE DIBUJABA): ancho y alto
    // estaban HARDCODEADOS EN 0 — eran artefactos `__ftol()` de Ghidra que
    // habían quedado neutralizados a `0` con el comentario al lado, así que el
    // barrido de los 47 sitios de `= __ftol();` no los detectó.
    // Con w=0/h=0 `CreateDIBSection` FALLA y deja `ppvBits` en nullptr → el
    // pixel-copy de FUN_0047f360 no puede componer el texto.
    //
    // IDA sub_50F5F0(HDC hdc, int a2):
    //     biWidth  = 2 * (__int64)*(float *)(a2 + 32);
    //     biHeight =    -(__int64)*(float *)(a2 + 36);
    // y `OpenFont` @0x50F690 lo llama como `sub_50F5F0(g_hDC, (int)Bitmaps)`,
    // o sea a2 = tabla de bitmaps; +32/+36 = Bitmaps[0].Width/Height (floats),
    // que en nuestro build son DAT_083a7cc0 / DAT_083a7cc4 — los mismos que ya
    // usa FUN_0047f360 para clampear la altura. Los puebla el OpenTGA de
    // "Interface/FontInput.tga" que corre justo antes en OpenFont.
```

### Línea 47 en `Font_CreateTextDib` — antes de `if (fontW <= 0 || fontH <= 0 || fontW > 4096 || fontH > 4096) {`

```cpp
    // SANITY 2026-07-19: si el OpenTGA de FontInput.tga no pobló Bitmaps[0]
    // todavía, estos floats vienen en 0/basura. Crear el DIB con dimensiones
    // absurdas hace que el pixel-copy de FUN_0047f360 (que recorre p1 columnas
    // × 3 bytes por fila sobre ppvBits) se salga del buffer → AV dentro de GDI.
    // Si los valores no son razonables, dejamos ppvBits en nullptr: el guard de
    // FUN_0047f360 saltea el copy (no se dibuja, pero NO crashea).
    // Valores correctos verificados en runtime: fontW=256 fontH=32 -> 512x32.
```

### Línea 68 en `Font_CreateTextDib` — antes de `m_hFontDC = hMemDC;`

```cpp
    // m_hFontDC ES DAT_055c9fec (macro en globals.h) — el font memory DC.
    // g_hDC es DAT_055ca004, el window DC que recibe SwapBuffers. NO confundirlos.
    // BUG-FIX 2026-07-19: antes existia un `HDC m_hFontDC` suelto que nunca se
    // asignaba (125 usos, 0 asignaciones), asi que quedaba NULL para siempre. IDA
    // sub_50F5F0 hace `m_hFontDC = CreateCompatibleDC(hdc)` — m_hFontDC es su
    // propio simbolo, y es el DC que usan sub_480C60 / RenderBooleans /
    // sub_47F360. En nuestro build m_hFontDC quedaba en NULL para siempre, asi
    // que GetTextExtentPoint32A fallaba, las dimensiones del texto salian 0 y la
    // burbuja no se componia ni se dibujaba.
```

## `src/Render/Font_LegacyFactory.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Font_LegacyFactory.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

## `src/Render/GL_2D.cpp`

### Línea 24 en `GL_Begin2D` — antes de `float fov  = *(float*)&DAT_00561554;`

```cpp
  // BUG-FIX: DAT_00561554/4c/50 son DWORDs que ALMACENAN bits de float (FOV/near/far).
  // El decompile de Ghidra los castea como (double)DWORD (interpretando como int) →
  // 45.0f bit-pattern (0x42340000 = 1110704128) se convierte en FOV=1.1e9 → matriz
  // degenerada (NaN) → el driver NVIDIA crashea en la siguiente llamada GL de estado.
  // Leer correctamente como float via puntero float*.
```

### Línea 39 en `GL_Begin2D` — antes de `glMatrixMode(GL_PROJECTION);`

```cpp
  // BUG-FIX 2026-06-28 (5.2 source ZzzOpenglUtil.cpp:1117): el 0.97k empuja la
  // 1ª matriz sobre el modo de ENTRADA (no explícito) y la 2ª sobre PROJECTION,
  // dejando el balance dependiente del modo actual.  5.2 empuja explícitamente
  // PROJECTION luego MODELVIEW.  Combinado con el fix de EndBitmap, balancea
  // exacto (1 PROJECTION + 1 MODELVIEW) y elimina el leak de PROJECTION.
```

### Línea 64 en `GL_End2D` — antes de `glMatrixMode(GL_PROJECTION);`

```cpp
  // BUG-FIX 2026-06-28 (5.2 source ZzzOpenglUtil.cpp:1136): el 0.97k original
  // hacía `glPopMatrix(); glPopMatrix();` SIN cambiar de modo → ambos pops caían
  // sobre MODELVIEW (el modo activo al salir de BeginBitmap).  Resultado: nunca
  // se popeaba PROJECTION (que BeginBitmap había empujado) → PROJECTION acumula
  // +1/frame → GL_STACK_OVERFLOW (0x503); y EndOpengl, al popear MODELVIEW de
  // nuevo, generaba GL_STACK_UNDERFLOW (0x504).  Corrige popeando explícito
  // 1 PROJECTION + 1 MODELVIEW, balanceando exacto con BeginBitmap.
  // NOTA vs 5.2: 5.2 popea MODELVIEW→PROJECTION (queda en modo PROJECTION).
  // Acá popeamos PROJECTION→MODELVIEW para DEJAR el modo en MODELVIEW, porque
  // nuestro EndOpengl (0.97k IDA) asume MODELVIEW como modo activo de entrada
  // (su 1er glPopMatrix es sobre el modo actual).  Balance idéntico (1+1).
```

### Línea 83 — antes de `void __cdecl GL_DrawRect(float param_1,float param_2,float param_3,float param_4)`

```cpp
// FUN_005124C0 @ 0x005124C0 — GL_DrawRect
// Draws a 2D filled rectangle (no texture) using GL_TRIANGLE_FAN (glBegin(6)).
// Vertices TL, BL, BR, TR forman un abanico (fan) con v0=TL como pivote:
//   tri1 = (TL, BL, BR), tri2 = (TL, BR, TR).
// Coordinates are in screen pixels; Y is flipped relative to viewport height.
// param_1: x,  param_2: y,  param_3: width,  param_4: height
//
// 2026-05-04 BUG-FIX: Ghidra decompile splittered the original contiguous
// stack array of 8 floats into `local_20[5] + local_c + local_8 + local_4`.
// The walker `for(i=0;i<4;i++) { glVertex2f(*p, p[1]); p+=2; }` assumed
// 8 contiguous floats, but MSVC is free to reorder/separate the named
// locals → vertex 4 (and possibly 3) read garbage from stack → 4th corner
// degenerated → rect renders as a triangle (visible as the yellow EXP bar
// and hover highlights showing as triangles instead of bars).
// Fix: explicit contiguous float[8].
```

### Línea 176 en `GL_DrawRect` — antes de `glBegin(6);   // GL_TRIANGLE_FAN`

```cpp
  // BUG-FIX: la decompile original usaba *(undefined4*) = *(unsigned int*) lo
  // cual al pasar a glTexCoord2f/glVertex2f hacia conversion int→float,
  // corrompiendo las coords (0x3f800000 → 1065353216.0f en vez de 1.0f).
  // Leer como float via puntero float*.
```

### Línea 201 en `GL_DrawBillboard` — antes de `float in1[12], out[12];`

```cpp
  // ── 2026-08-16: patron [[locales-contiguos-ghidra]] (5ta instancia) ────────
  // IDA `sub_511C10` recorre `in1[0..11]` y `v[0..11]` como 4 vec3 cada uno:
  //     for (i = 0; i < 12; i += 3) VectorTransform(&in1[i], in2, &v[i]);
  // Ghidra emitio ese frame como escalares SUELTOS (local_60[4] + local_50,
  // local_4c, local_48, local_44, local_40, local_3c, local_38, local_34 /
  // local_30[3] + local_24 + local_18 + local_c). MSVC no garantiza que queden
  // contiguos, asi que los vertices 2, 3 y 4 salian de memoria basura y se
  // escribian en lugares arbitrarios => quads desbocados = los CUADROS BLANCOS
  // de los efectos de skill (este es el billboard 3D que usa SkillEffect_Render
  // en todos los mapas salvo World 2).
  // Mapeo por offset de frame (ebp):
  //   in1[0..2]  = -0x60,-0x5c,-0x58   in1[3..5]  = -0x54,-0x50,-0x4c
  //   in1[6..8]  = -0x48,-0x44,-0x40   in1[9..11] = -0x3c,-0x38,-0x34
  //   out[0..2]  = -0x30 (v)   [3..5] = -0x24 (v16)
  //   out[6..8]  = -0x18 (v17) [9..11]= -0x0c (v18)
```

## `src/Render/GL_LegacyState.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"
```

## `src/Render/HUD_Pass1.cpp`

### Línea 4 — antes de `//   * RenderMainFrameWindow — el marco inferior del HUD (5 llamadas a RenderBitmap + el`

```cpp
// Estas cuatro faltaban por completo en nuestro build o estaban stubeadas vacías
// en src/Render/Render_Frame.cpp. Traerlas restaura:
//   * RenderPartyHP        — las barritas de HP sobre los miembros del party en el mundo
//   * RenderBooleans       — floating-numbers iterator (damage/heal popups)
//                            Nota: la llamada a RenderBoolean por entrada (0x480E00,
//                            ~3000 bytes) todavía NO está portada; el iterador
//                            does its layout/de-overlap work but then falls
//                            por un RenderBoolean stubeado por ahora.
```

### Línea 58 — antes de `#define ScreenCenterX        ViewportCenterX`

```cpp
// ── Globals que el original referencia por nombre simbólico ─────────────────
// Definimos #defines para que los nombres estilo IDA coincidan con el storage que
// ya tenemos, y el código portado quede visualmente cerca del decompile.
// 2026-08-22 FIX: estos cuatro apuntaban a 0x00561558..0x00561564 con el
// comentario "already in our build (assumed)" — una suposición que nunca se
// verificó y que estaba mal.  `ida_xrefs_to` da las direcciones reales, las
// cuatro escritas por `gluPerspective2` (0x511220):
//     ScreenCenterX 0x083A429C · ScreenCenterY 0x083A42A0
//     PerspectiveX  0x083A42A4 · PerspectiveY  0x083A42A8
// 0x00561558 es otra cosa (la escribe `BeginOpengl`, 0x5119B0).
// Nadie usaba estos nombres todavía, así que el bug nunca se disparó — pero eran
// cuatro trampas armadas para el próximo port que los usara, porque nuestro
// `GL_SetPerspective` sí escribe en los DAT_083a42xx.
```

### Línea 237 — antes de `void Render_HPBars(void) { RenderPartyHP_(); }`

```cpp
// Entrada pública — reemplaza al stub vacío que antes estaba en Render_Frame.cpp.
```

### Línea 248 — antes de `//`

```cpp
//   3. Pasada de render: llama a RenderBoolean por entrada (hoy stubeada).
```

### Línea 396 — antes de `extern "C" DWORD DAT_07e91388;`

```cpp
// =============================================================================
// Render_HotbarItems3D — sub_4BFDE0. Renderiza los items de la barra de accesos rápidos del inventario como
// mallas 3D vía RenderItem3D, desplazados a lo largo de x=208..301 en pasos de 31 px.
// Guarda el estado ortográfico 2D actual, arma un frustum en perspectiva nuevo
// (FOV=1.0 rad) usando la CameraMatrix guardada, dibuja los items y después
// restores 2D state.
//
// La rama `g_bEventChipDialogEnable in {0, 3}` elige la barra normal
// path; other values delegate to sub_4F54B0 (event-chip dialog renderer)
// que no tenemos portado. El valor por defecto de 0 toma el camino normal.
//
// Deps:
//   EndBitmap / BeginBitmap (GL_End2D / GL_Begin2D)
//   sub_482BE0(slot)        — devuelve el índice de OffsetInventoryItems del slot de la barra
//   OffsetInventoryItems    — array of {Type, Level, ...}
//   RenderItem3D            — RenderItem3D
//   CreateScreenVector      — Camera_BuildMouseRay
//   CameraPosition[]        — float[3] world-space camera
//   CameraMatrix[]          — 4x4 GL matrix (already in our globals)
// =============================================================================
// Item_FindQuickSlotByCategory, RenderItem3D y Camera_BuildMouseRay ya están declaradas en
// functions.h (que entra vía stdafx.h). FUN_004f5ce0 / FUN_004f6420 están
// declaradas pero sin implementar en nuestro build — acá dejamos stubs para que
// enlacen los call sites del render de la barra. Son renderers de efectos de
// skill / teleport; que no tengan cuerpo sólo significa que esos overlays no se dibujan, lo cual es
// fine until they get their own port.
```

## `src/Render/HUD_Pass2.cpp`

### Línea 3 — antes de `#include "stdafx.h"`

```cpp
//
// Functions in this TU (all 1:1 from IDA sub_xxxxxx of the original mu.exe):
//   * Render_ChatBox          (sub_4BE4F0)  — chat input box + scroll history
//   * Render_CharInfoPanel    (sub_4BC220)  — guild war / soccer score banner
//   * RenderEquipedHelperLife (sub_4BEC00)  — pet/helper life bar (top centre)
//   * RenderBrokenItem        (sub_4BE710)  — durability warnings (right side)
//   * RenderExperience        (sub_4BF990)  — XP bar + tooltip on hover
//
// Helpers ported here (because no callers existed yet):
//   * RenderBar       (sub_4BBDD0) — border + filled bar (used by 3 funcs)
//   * RenderNumber2D  (sub_5122F0) — bitmap-glyph integer renderer
//   * sub_47F6F0      — RenderText with shadow style (returns SIZE*)
//   * GetScreenWidth  (sub_4CB520) — UI panel-aware screen-width
//
// Helper STUBS (the real bodies are 600+ bytes each, port follow-up):
//   * RenderInputText (sub_47F0B0)
//   * RenderTipText   (sub_47F7F0)
//   * CreateGuildMark (sub_4F0100)
//
// =============================================================================
```

### Línea 46 — antes de `#define SummonLife               DAT_05826d24`

```cpp
// 2026-08-22: `SummonLife` NO es DAT_07e11d28.  Una nota vieja lo aliaseaba
// ahí y colisionaba con el contador de debounce del walker
// (Player_InputTick.cpp), que lo incrementa cada frame — por eso la barra de HP
// del monstruo invocado se dibujaba siempre (el triángulo cyan).  El parche de
// entonces fue una variable local nueva, o sea el global quedó partido en dos.
// `ida_xrefs_to("SummonLife")` da la dirección real: **0x05826D24**
// (= DAT_05826d24), escrita por InitGame, ReceiveRevival (3 sitios) y el
// F3/0x20 de ProtocolCore, y leída por RenderEquipedHelperLife.  Ahora el alias
// apunta ahí, así que el reset de Recv_Revival lo ve este render.
// (Pendiente: portar el F3/0x20 `SummonLife = ReceiveBuffer[4]`, el único
//  productor del valor; hasta entonces queda en 0 y la barra no se dibuja.)
```

### Línea 87 en `HUD_IsQuestPanelOpenRuntime` — antes de `(*(BYTE*)((BYTE*)(uintptr_t)g_csQuest + 0x1C87F) != 0);`

```cpp
           // 2026-08-21: era 0x1C8FF.  El flag "panel de quest abierto" esta en
           // g_csQuest + 116863 (0x1C87F) — IDA lo usa asi en 9 sitios, uno de
           // ellos este mismo GetScreenWidth (0x4CB520 L103).  Con 0x1C8FF se
           // leia un byte 0x80 mas adelante.
```

### Línea 291 — antes de `#define DAT_07e11e10_alias  DAT_05826c08   // SoccerTime`

```cpp
// 2026-08-22: `SoccerTime` y `SoccerObserver` NO son DAT_07e11e10 / DAT_07e11e14.
// `ida_xrefs_to` da 0x05826C08 y 0x05826D33; de 0x07E11E10 el unico xref en todo
// el binario es sub_494520 (el bloque anti-tamper de IME/RC4), o sea el alias
// viejo era inventado.  Mientras estuvo mal, los escritores (InitGame y ahora
// los handlers F3/22 y F3/23) y este lector estaban en memorias distintas, asi
// que el reloj del evento y el marcador nunca se dibujaban.  Ver
// [[global-partido-en-dos]].
```

### Línea 412 en `RenderEquipedHelperLife_` — antes de `int retY = 15;                       // 'mov esi, 0Fh' del prologo`

```cpp
    // 2026-08-22 FIX (la barra del pet no se dibujaba): el gate leia el SLOT DE
    // ITEM (`CharacterMachine + 536 + 68*8`) a traves de una cadena de fallbacks
    // inventada por el port.  IDA (0x4BEC00, verificado en el disassembly del
    // prologo) lee la ENTIDAD del heroe:
    //     mov ax, [eax+2B8h]   ; Hero + 696 = tipo del helper
    //     cmp ax, 330h / 333h  ; 816..819
    // Ese campo lo escriben SetCharacterClass, ChangeCharacterExt y el handler
    // 0x25.  El slot de item se usa SOLO para la vida y para el nombre por
    // defecto.
    //
    // (`+0x2B8` esta etiquetado como `char_class` en la tabla de offsets de
    //  CLAUDE.md — es falso, es el tipo del helper/pet equipado.)
```

### Línea 435 en `RenderEquipedHelperLife_` — antes de `const float x = (float)GetScreenWidth()`

```cpp
        // x = GetScreenWidth() - 50.0 - (PartyNumber > 0 ? 50.0 : 0.0) - 15.0
        // (flt_552598 = 50.0 y flt_552834 = 15.0, leidos del binario).  Queda
        // arriba a la DERECHA; el port anterior la centraba en pantalla.
```

### Línea 551 en `RenderBrokenItem_` — antes de `unsigned int attrBase = ItemAttribute_Base();`

```cpp
            // 2026-05-08: usar ItemAttribute_Base() para recuperar de
            // corrupción de DAT_07d78068 a 0x1.
```

### Línea 588 en `RenderBrokenItem_` — antes de `if ((uintptr_t)v19 < 0x100000 || (uintptr_t)v19 >= 0x80000000) {`

```cpp
                // 2026-05-08: defensive — v19->Name is `char[30]` inline,
                // address = v19. Crash at addr 0x74F3DBCC param1=0x2A01 came
                // from wsprintfA reading bogus v19->Name. Validate v19 is in
                // heap range before reading.
```

## `src/Render/HUD_Pass3.cpp`

### Línea 73 en `PointerBitsAsFloat` — antes de `initPool(OffsetWarehouseItems,  120 * 0x44);`

```cpp
    // 2026-07-27 FIX (baúl abre "lleno" de Kris sin data): OffsetWarehouseItems
    // era el ÚNICO pool que no se inicializaba → quedaba todo en ceros, y Type=0
    // es un item válido (Kris) en vez del marcador de celda vacía (0xFFFF) → las
    // 120 celdas del baúl mostraban un Kris fantasma con durabilidad 0/20.
```

### Línea 78 en `PointerBitsAsFloat` — antes de `// 2026-08-22: acá se estampaba Type=0xFFFF sobre un 'g_EquipGridBuf' suelto`

```cpp
    // 2026-08-22: acá se estampaba Type=0xFFFF sobre `g_InventoryGridPool`, otro
    // buffer suelto que nadie llenaba.  Sus consumidores ya leen el inventario
    // real, así que no hace falta.
```

### Línea 82 en `PointerBitsAsFloat` — antes de `}`

```cpp
    // 2026-08-22: acá se estampaba Type=0xFFFF sobre un `g_EquipGridBuf` suelto
    // para que sus celdas vacías no dieran falso positivo.  Ese buffer no existía
    // en el binario: unk_7EA9504 / unk_7EA9328 son posiciones dentro del
    // inventario real (ver globals.cpp).  Con los punteros ya reenraizados, este
    // loop escribiría 0xFFFF encima del Type de slots REALES.  Removido — el
    // inventario ya lo inicializa initPool() unas líneas más arriba.
```

### Línea 108 en `PointerBitsAsFloat` — antes de `// Inventory pools.`

```cpp
    // SetTextColor_0 (= 0x00559C7C) YA NO se define acá.  2026-08-15: estaba
    // duplicado — esta copia era la que escribían RenderBoolean_IDA y
    // RenderPartyHP, mientras que el LECTOR real (FUN_0047f360, stubs_game.cpp)
    // leía `DAT_00559c7c`, otro global distinto que quedaba en 0.  Resultado: el
    // prefijo de guild de la burbuja de chat se pintaba con color 0x00000000
    // (transparente/negro).  Ahora vive en globals.cpp y `DAT_00559c7c` es un
    // alias del mismo símbolo (ver globals.h).  Verificado en IDA:
    // xrefs de 0x559C7C = { sub_47F360 (lee), RenderBoolean ×3 (escribe),
    // RenderPartyHP (escribe) } — o sea UNA sola memoria.
```

### Línea 118 en `PointerBitsAsFloat` — antes de `BYTE  Inventory[160 * 68]            = {0};`

```cpp
    // Inventory pools.
    // 2026-07-25 (#2 shops): Inventory era [64*68] pero el pool de TIENDA es un
    // overlay 8×15 (120 slots) que arranca en &Inventory[32].WalkSpeed y llega
    // hasta ~Inventory+10348 (HUD_Pass3:382 sub_4E38B0 + Net_Process ShopInsert).
    // Con 64 slots el render leía OOB (shop salía vacío) y poblarlo corrompería
    // memoria. Agrandado a 160 slots (10880 bytes) para cubrir el grid completo.
```

### Línea 128 en `PointerBitsAsFloat` — antes de `BYTE  OffsetWarehouseItems[120 * 68]  = {0};`

```cpp
    // 2026-05-08: warehouse grid is 8x15 = 120 slots * 0x44 = 8160 bytes.
    // Used by FUN_004d23b0 (Inventory click handler) when WarehouseOpened.
```

### Línea 131 en `PointerBitsAsFloat` — antes de `BYTE  ShopItems[120 * 68]             = {0};`

```cpp
    // 2026-07-27: pool de la TIENDA (8×15 = 120 slots). ANTES era un overlay
    // dentro de Inventory (&Inventory[32].WalkSpeed) — como en el binario
    // original esas direcciones son distintas, en nuestro build el overlay
    // colisionaba con otros usos de Inventory[32] (scratch de coords de paneles,
    // loops de trade, etc.) que lo pisaban cada frame → "tienda abre vacía"
    // intermitente (diag SHOPREND: populate 15/15 y 1s después occ=0 SIN ningún
    // paquete de red). Con array propio el pool es inmune a ese aliasing.
```

### Línea 140 en `PointerBitsAsFloat` — antes de `int   dword_7EAA0CC          = 0;`

```cpp
    // 2026-04-30: Inventory/Trade panel origin globals — unified with the
    // IDA-side DAT_ addresses that RenderEquipment3D / RenderItem3D
    // already read.  The Ghidra-era phantoms (InventoryStartX = 380 etc.)
    // were independent of DAT_07ea5288 → equipment items rendered at
    // x≈0 while the panel frame rendered at x=450.
```

### Línea 218 — comentario al final de `extern "C" int __cdecl FUN_00482850_(void);`

```cpp
extern "C" int __cdecl FUN_00482850_(void);   // we prefix to avoid clash with stubs.cpp
```

### Línea 240 en `FUN_00482850_` — antes de `int* base = &DAT_07ea9328;      // slot 56, campo Key`

```cpp
    // Recorre la grilla 8x8 del inventario hacia atras, igual que IDA.
    // 2026-08-22: esto caminaba `g_InventoryGridPool`, otro buffer suelto que
    // nadie llenaba (mismo caso que g_EquipGridBuf).  unk_7EA9328 y unk_7EA9504
    // son posiciones dentro de OffsetInventoryItems — ver la derivacion en
    // globals.cpp.  Encima, con la base equivocada el `cell -= 136` (544 bytes)
    // se iba 3876 bytes por DEBAJO del buffer.
```

### Línea 347 en `Render_HudPass_4BCD20_` — antes de `GL_Begin2D();`

```cpp
    // 2026-09-04 -- DESVIACION NECESARIA (barra de AG invisible).
    // `sub_4BCD20` dibuja con RenderBitmap / RenderNumber2D / RenderTipText, que
    // no arman matrices propias: dependen de la ortho de `BeginBitmap`.  Y el
    // pase anterior (`sub_4BD650`) TERMINA con `EndBitmap`.
    //
    // En el binario eso funciona por accidente: `EndBitmap` (0x5124B0) hace dos
    // `glPopMatrix` seguidos SIN cambiar de modo, o sea los dos caen sobre
    // MODELVIEW y la PROJECTION ortho que empujo `BeginBitmap` queda activa (a
    // costa de desbordar esa pila, que es el GL_STACK_OVERFLOW 0x503 conocido).
    // Nuestro `GL_End2D` esta balanceado a proposito (fix 2026-06-28), asi que
    // al salir de sub_4BD650 la proyeccion vuelve a la perspectiva 3D y todo lo
    // que dibuja esta funcion cae fuera de pantalla.
    //
    // Sonda AGBAR (2026-09-04): confirmaba `x=551 y=437 h=36 blend=1 tex=1
    // texsz=(16,64) gl257=47 glerr=0`, o sea el draw se emitia perfecto y no se
    // veia -- ni la barra, ni el numero, ni el tooltip.
```

### Línea 397 — antes de `extern "C" void __cdecl Render_HudPass_4F6050_(void);`

```cpp
// =============================================================================
// sub_4F6050 — inventory-grid render dispatch.  Calls sub_4E38B0 four times
// to draw the active grid panel (Inventory / Shop / Trade / Warehouse /
// ChaosMix) using its own offset ints and 8×N item array.
//
// The IDA decomp interleaves heavy hash-table ref-count noise on
// ShopOpened / TradeOpened — those serve only to satisfy anti-tamper hash
// tracking and are skipped here.
//
// Without sub_4E38B0 ported the panels won't show inventory items, but the
// rest of the HUD is unaffected.
// =============================================================================
// 2026-05-08 NOTE: previously had FUN_004d23b0/Inventory_DropDispatch hook here.
// That was wrong — sub_4F6050 (this fn) is NOT called in-world. The actual
// per-frame in-world inventory render is RenderInventoryWindow (sub_4F0A50)
// invoked from Render_QuickButtons_ (sub_4F5820), HUD_Pass4.cpp:246. The
// click-handler hook lives there now (HUD_Pass6.cpp:RenderInventoryWindow).
```

### Línea 609 — antes de `void __cdecl RenderBoolean(int x, int y, DWORD c)`

```cpp
// =============================================================================
// RenderBoolean — sub_480E00.  La copia VIVA es la de mas abajo.
//
// 2026-09-25: aca habia una SEGUNDA implementacion de la misma direccion (90
// lineas) que su propio comentario ya daba por muerta -- "0 callers de codigo:
// solo la referencian comentarios" -- y quedaba como candidata a borrar.  Se
// borro: el simbolo que corre es el de abajo, al que llegaba el unico caller
// (HUD_Pass1) pasando por un wrapper llamado FUN_00480e00 que tambien se
// elimino.  Ver [[simbolo-duplicado-patron]].
// =============================================================================
```

### Línea 637 en `RenderBoolean` — antes de `BYTE kind = *(BYTE*)(c + 36);`

```cpp
    // 2026-08-15: los 6 colores estaban mal transcritos (el decompile los muestra
    // como decimales con signo).  Valores de IDA RenderBoolean L121-144:
    //   -983146=0xFFF0FF96  -34716=0xFFFF7864  -19316=0xFFFFB48C
    //   -9016=0xFFFFDCC8  -12806401=0xFF3C96FF  -14790401=0xFF1E50FF
    //   default -16776961=0xFF0000FF
```

### Línea 670 en `RenderBoolean` — antes de `BYTE mode = *(BYTE*)(c + 37);`

```cpp
    // 2026-08-15: constantes corregidas contra IDA RenderBoolean L180-195.
    //   mode 0: back=-1773129196=0x96503214  SetTextColor_0=-14116=0xFFFFC8DC
    //   mode 1: back=-1778359236=0x9600643C  SetTextColor_0=-16711736=0xFF00FFC8
    //   otros : back=-1778384796=0x96000064  SetTextColor_0=-16776961=0xFF0000FF
```

## `src/Render/HUD_Pass4.cpp`

### Línea 32 — antes de `#define MacroTime   DAT_07e11d7c`

```cpp
// ── Aliases: WM_CHAR + RenderInputText share storage via DAT_07db8710 /
// DAT_07d780a8 globals (matching the IDA original where InputText is at
// 0x07db8710 and InputLength at 0x07d780a8). See globals.h.
// MacroTime es 0x07E11D7C (IDA lo decrementa en Game_MainLoop y lo pone en
// 100 al disparar una macro).  Hasta 2026-09-24 este archivo definia una
// variable propia con ese nombre, asi que la barra "Macro Time" nunca se
// dibujaba: el contador que se escribia no era el que se leia.
```

### Línea 46 — antes de `int   m_iMatchCountDownType   = 0;`

```cpp
    // 0x07E11D8C / 0x07E11D90.  Los escribe StartMatchCountDown (0x47EC00),
    // que atiende el opcode 0x92; declarados en globals.h para que el handler
    // los vea (antes eran estaticos de este .cpp y nadie los seteaba).
```

### Línea 51 — antes de `int   AlphaBlendType          = 0;`

```cpp
    // m_iMatchTime vive en globals.cpp (0x00559CCC): lo escribe SetMatchInfo
    // desde el handler 0x9B.  Tenerlo aca como local dejaba al renderer
    // leyendo una copia que nadie escribia (2026-09-04).
    // MixState vive en globals.h como alias de DAT_07eaa140 (0x07EAA140).
    // Estaba partido en dos: los ESCRITORES (Item_ClickHandler al mandar el mix
    // = 1, y el handler del 0x86 con 0 o 2) usan DAT_07eaa140, y los LECTORES
    // -- la animacion de la caja de Chaos de abajo y el `++MixState` que la hace
    // avanzar -- leian este `int` propio, que nadie escribia.  Efecto: la
    // maquina de Chaos nunca mostraba su animacion al mezclar.
```

### Línea 62 — antes de `BYTE  InputTextHide[10]       = {0};`

```cpp
    // Input fields — IDA exposes 10 slots (chat + whisper-target + ...).
    // BUG-FIX 2026-05-04: previously this declared a separate `InputText[2][256]`
    // local storage and `int InputLength[2]`, which DESYNCED from WM_CHAR (which
    // writes to DAT_07db8710 / DAT_07d780a8). Net effect: typing in chat
    // appended to the global, but RenderInputText read this dead local → user
    // saw no characters appearing. Now both use the same storage.
```

### Línea 70 — antes de `#define InputIndex   DAT_07e11d78`

```cpp
    // 2026-08-26 — mismo bug que el de arriba, que quedo a medias en 2026-05-04:
    // `InputIndex` e `InputFrame` eran copias LOCALES de dos globals reales, asi
    // que este archivo nunca veia lo que escribia el resto del cliente.
    //
    //   InputIndex = DAT_07e11d78 — indice del campo de input activo. Lo rota el
    //     Tab en WndProc (`DAT_07e11d78 = (DAT_07e11d78 + 1) % InputNumber`) y
    //     lo lee `RenderInputText` (Chat.cpp) para saber en que campo va el
    //     caret. Con la copia local clavada en 0, el `_` se dibujaba SIEMPRE en
    //     el campo de chat aunque se estuviera escribiendo en el de whisper.
    //
    //   InputFrame = DAT_07e11d2c — contador del parpadeo del caret; Chat.cpp
    //     usa el mismo `% 2` sobre el global. Con dos contadores separados los
    //     dos caret parpadeaban desfasados.
```

### Línea 86 — antes de `#define MarkColor DAT_07e11f34`

```cpp
    // Guild mark colour palette (16 entries × DWORD ARGB).
    // 2026-08-25: esto era una copia LOCAL del array. El global real es
    // 0x7E11F34 (= DAT_07e11f34), que es el que lee `RenderGuildMark`
    // (0x4F02F0, nuestro RenderGuildMark): `CreateGuildMark` llenaba esta
    // copia y el render leia el global, que quedaba en ceros — y encima estaba
    // declarado como UN DWORD, asi que indexarlo 0..15 desbordaba.
    // Ver [[global-partido-en-dos]].
```

### Línea 298 — antes de `extern "C" void __cdecl Render_QuickButtons_(void);`

```cpp
// =============================================================================
// Render_QuickButtons — sub_4F5820.  HUD UI dispatcher.  Calls 9 sub-panels
// in sequence (party / inventory / trade / shop / chaos-mix / warehouse /
// event / golden-archer / server-division) plus a quest-state refresh.
// All 9 are stubs at the moment — when they get ported individually their
// visuals appear without touching this dispatcher.
// =============================================================================
```

### Línea 311 en `Render_QuickButtons_` — antes de `const int panelStartX = 450;`

```cpp
    // 2026-08-08 FIX "el panel de Character (C) se ve negro si se abre despues
    // del inventario": esto era `GetScreenWidth()`, que devuelve 260 cuando
    // Inventory+Character estan abiertos a la vez -> el panel de Character se
    // dibujaba ENCIMA del inventario (que tambien va a 260) y la franja 450..640
    // quedaba sin pintar = rectangulo negro. En el binario los 3 paneles del
    // lado derecho son CONSTANTES 0x1C2 (=450), no el ancho del viewport:
    //   sub_4F5820+0x39  push 0 / push 1C2h / call RenderGuildCreation
    //   sub_4F5820+0x253 push 0 / push 1C2h / call RenderGuildList
    //   sub_4F5820+0x379 push 0 / push 1C2h / call RenderCharacterInfoWindow
    // (GetScreenWidth solo recorta el viewport 3D, no posiciona paneles.)
```

### Línea 334 en `Render_QuickButtons_` — antes de `float btnX = (float)g_GuildCreatorScratchX + _DAT_005524fc;`

```cpp
        // 2026-08-26 FIX "los botones OK/CANCEL salen abajo a la izquierda, fuera
        // del panel": el origen era `DAT_07ea5b1c/20` (= Inventory[32].Level/Part),
        // el scratch que el port ABANDONO el 2026-07-27 al mover el origen del
        // creador a `g_GuildCreatorScratchX/Y` (Inventory[32] es el slot 0 del pool
        // de la tienda y lo estaba pisando). Quedo en 0, asi que los botones se
        // dibujaban en (0+20, 0+350) absoluto — abajo a la izquierda — mientras los
        // El hit-test SecondPassword_Screen1 ya usaba el origen bueno: se dibujaban en un
        // lado y se clickeaban en otro.
        // Los tres offsets coinciden con esos hit-tests: +20/+350 y +100 el segundo.
        // Es el tercer hermano del fix del 2026-08-08 b (GuildList y CharacterInfo
        // ya habian pasado a sus globals reales; este quedo sin actualizar).
```

### Línea 353 en `Render_QuickButtons_` — antes de `btnX = (float)g_GuildCreatorScratchX + 100.0f;`

```cpp
        // 2026-08-26 FIX (CANCEL sobresalia del panel): el segundo boton
        // arranca en +100 desde el origen del panel, NO en +20+100.
        // IDA `sub_4E4760` L446-447 da los dos rects:
        //     boton 1 (crear):  [origin+20 , origin+90 )   ancho 70
        //     boton 2 (cancel): [origin+100, origin+170)   ancho 70
        // El port partia del primero (+20) y le sumaba `_DAT_005524f0` (que
        // ademas es TERRAIN_SCALE, no un offset de UI: valia 100 de casualidad),
        // dejando el boton en +120..+190. Como el panel mide 190 de ancho, ese
        // rect terminaba exactamente en el borde y se veia sobresalir; el
        // desalineado contra el hit-test crecia con la resolucion.
        // No es mezcla de espacios: los dos estaban en logico. Es el offset.
```

### Línea 404 en `Render_QuickButtons_` — antes de `if (HUD_IsGuildListRuntime())      RenderGuildList(panelStartX, 0);`

```cpp
    // 2026-05-04: en el original (sub_4F5820) hay 3 `if` sites antes/alrededor
    // de RenderParty que llaman a RenderCharacterInfoWindow/RenderGuildList/
    // RenderGuildCreation cuando los flags correspondientes están seteados.
    // Sin ellos, los menús C/G renderizan como rectángulo negro afuera del
    // 3D viewport (que es 450 wide cuando esos flags están on).
    // 2026-09-04: el ORDEN importa -- los tres paneles se dibujan en el mismo
    // x=450 y el ultimo tapa a los anteriores.  IDA sub_4F5820:
    //     if (GuildCreatorOpened) RenderGuildCreation(450, 0);
    //     else if (GuildOpened)   RenderGuildList(450, 0);
    //     RenderParty(450, 0);
    //     if (CharacterOpened)    RenderCharacterInfoWindow(450, 0);
    // El port lo tenia al reves (personaje primero, guild despues), asi que con
    // GuildOpened en 1 el panel de guild pintaba ENCIMA del de personaje y del
    // de party: de ahi "los botones de party y character abren el panel de
    // guild".
```

### Línea 432 en `Render_QuickButtons_` — antes de `FUN_00403f30((void*)(uintptr_t)g_csQuest);`

```cpp
    // 2026-08-21: faltaba el render del panel de quest.  IDA sub_4F5820 L38:
    // `sub_403F30((_BYTE *)g_csQuest);` entre RenderGoldenArcherWindow y
    // RenderServerDivision.  Sin esto, con el flag del panel prendido
    // GetScreenWidth angostaba el viewport a 450 y esa franja quedaba negra.
```

### Línea 438 en `Render_QuickButtons_` — antes de `if (DAT_07eaa14c == 0 && DAT_083a7c24 == 0 &&`

```cpp
    // ── In-world drop dispatcher (2026-05-08) ────────────────────────────────
    // Mirrors what Net_PacketSession.cpp:284 does for state=4 char-select but
    // for in-world. Gated internally on dword_7E91388 > 0 (= player carrying
    // an item picked up via FUN_004d23b0 inside RenderInventoryWindow). This
    // is the function that builds and SENDS the 0x24 PMSG_ITEM_MOVE_RECV
    // packet via SendRequestEquipmentItem → Net_SendSmallPacket (C3).
    // 2026-09-16: este llamado es un DUPLICADO del port — en IDA el dispatcher
    // (sub_4DF410) solo lo llama UpdateWindowsMouse (0x4ECB00), que corta antes
    // mientras el teclado del PIN esta abierto (SecondPassword_Handler devuelve
    // 1 y el widget en foco no coincide).  Sin este gate el preview azul del
    // drop seguia al mouse con el teclado abierto.  Se deja el llamado (quitarlo
    // requiere probar el drop) pero con el mismo corte que el original.
    // Condicion de corte de UpdateWindowsMouse: teclado del PIN activo, un
    // cartel abierto (ErrorMessage) o un widget con foco.  El caso del cartel
    // aparecia con el quick-move del click derecho: tras un PIN incorrecto el
    // item queda en la mano con el cartel "Contrasena incorrecta" encima.
```

### Línea 470 — antes de `//`

```cpp
// glow particles via sub_5126E0 (stubbed).
```

### Línea 626 en `sub_4E38B0` — antes de `TextSize.cx = (LONG)((double)TextSize.cx / g_fScreenRate_x);`

```cpp
    // 2026-08-26: acá había un workaround. En 2026-07-19 se detectó que el
    // caret quedaba corto (~80% del largo) y se lo compensó guardando el ancho
    // SIN dividir, porque en ese momento `UI_DrawText` -> `CUIRenderText_RenderText`
    // dibujaba en píxeles crudos: la mitad "lógico -> físico" del pipeline no
    // existía, así que un offset en espacio-640 se dibujaba como si fuera píxel.
    //
    // Esa mitad ya está implementada (CUIRenderText_RenderText convierte con
    // g_fScreenRate_x/y, igual que `sub_410AF0` en el binario), así que el
    // workaround quedó obsoleto y ahora es él quien descoloca el caret: sumaba
    // un ancho en PÍXELES a una `x` en LÓGICO.
    //
    // IDA (RenderInputText 0x47F0B0) posiciona el caret con el TextSize ya
    // dividido, o sea en espacio lógico, que es lo que se restaura acá.
    //   ancho medido -> píxel -> / g_fScreenRate_x -> lógico -> + x (lógico)
```

## `src/Render/HUD_Pass5.cpp`

### Línea 387 en `Render_HudPass_4BD650_` — antes de `GL_SetBlendSrcOver(1);`

```cpp
    // ── Coin / kill counters at (24,462) and (48,462) ────────────────────────
    // 2026-09-04 BUG-FIX ("los numeros de la barra tienen fondo negro"): faltaba
    // el `EnableAlphaTest(1)` que IDA (sub_4BD650) hace justo antes del
    // `glColor3f(0.6, 0.6, 0.6)`.  Los digitos salen del TGA `FontTest` (bitmap
    // 1) y necesitan SRC_ALPHA/ONE_MINUS_SRC_ALPHA; sin la llamada heredaban el
    // estado que dejo el pase anterior (blend apagado) y los texeles
    // transparentes se pintaban NEGROS Y OPACOS = el recuadro detras de cada
    // numero.  0x511680 -> GL_SetBlendSrcOver en este arbol.
```

## `src/Render/HUD_Pass6.cpp`

### Línea 1 — antes de `//`

```cpp
// HUD_Pass6.cpp — final pass: closes every remaining stub.
```

### Línea 45 — antes de `#define CharacterInfoStartX  (*(int*)&DAT_07ea982c)`

```cpp
// Origen (esquina superior izquierda) de los paneles Character / Guild.
//
// 2026-08-08 FIX "el botón X no cierra guild/party/character (en el inventario
// sí anda)": esto eran DOS `static int` de este .cpp, o sea una SEGUNDA copia
// de globals que sí existen (CharacterInfoStartX/Y = DAT_07ea982c/30,
// GuildListStartX/Y = DAT_07e91788/84). El render escribía las copias locales
// y los hit-tests de cierre (SecondPassword_Screen1 / SecondPassword_Screen3 en Net/SecondPassword,
// port de sub_4E4760 L617-629 y sub_4E5DE0 L336-357) leían los globals reales,
// que quedaban en 0 → el rect de la X caía en (25..49, 395..419) de PANTALLA en
// vez de (panelX+25, panelY+395), y encima SecondPassword_Screen3/Party_MemberClickHandler hacen
// early-return cuando el origen es 0, así que el hit-test ni corría.
// El inventario funcionaba porque InventoryStartX/Y sí es el global real.
```

### Línea 246 en `sub_4F6420` — antes de `if (v7->Type == -1) {`

```cpp
                // 2026-07-27 FIX "todos los slots se ven iguales": ITEM.Type es
                // short (signed). Una celda vacía = -1 (0xFFFF). El check estaba
                // como `== (WORD)-1`: (WORD)-1 = 0xFFFF = 65535, pero v7->Type
                // (short -1) promociona a int como -1 → `-1 == 65535` SIEMPRE
                // falso → toda celda vacía caía al else y dibujaba la textura 278
                // (ocupada) → grid uniforme. IDA usa `v7->Type == -1`.
```

### Línea 253 en `sub_4F6420` — antes de `InventoryColor(v7);`

```cpp
                    // 2026-09-09: aca habia un `glColor3f(1,1,1)` fijo.  IDA
                    // (0x4E37B0) llama `InventoryColor(v7)` en las DOS ramas, y
                    // esa es justamente la que muestra la SILUETA del item que
                    // se esta arrastrando: el hit-test escribe ITEM.Color = 2
                    // (no entra) / 3 (entra) / 4 (moneda) sobre las celdas
                    // VACIAS bajo el cursor, y con el color fijo ese marcado no
                    // se veia nunca.
```

### Línea 318 — antes de `extern "C" void FUN_004f5ce0_realbody(void) { sub_4F5CE0_(); }`

```cpp
// Wire over the previous stub of FUN_004f5ce0 (declared in HUD_Pass1.cpp).
// HUD_Pass1's stub took (int,int,int,int) — we redirect from there.
```

### Línea 389 en `sub_4F5CE0_` — antes de `FUN_004d23b0((char*)(uintptr_t)(InventoryStartX + 15),`

```cpp
    // ── In-world click handler hook (2026-05-08) ────────────────────────────
    // FUN_004d23b0 = grid hit-test + pickup + right-click use dispatcher.
    // Must run BEFORE RenderItemsBoxes so highlight bytes are set when the
    // item bitmaps are painted. Drop dispatcher (Inventory_DropDispatch) is invoked
    // once after all the panel-specific hit-tests in Render_QuickButtons_.
```

### Línea 539 en `sub_4F5CE0_` — antes de `g_PartyPanelScratchX = a1;`

```cpp
    // ── Full Party panel ───────────────────────────────────────────────────
    // 2026-07-27 FIX (tienda vacía intermitente): estos scratch escribían en
    // Inventory[32], que es EXACTAMENTE el primer slot del overlay del pool de
    // la TIENDA (&Inventory[32].WalkSpeed). Cada frame con el panel de Party
    // abierto pisaba el slot 0 → el render lo veía vacío (diag SHOPREND:
    // slot0Type=-1 occ=0 sin ningún paquete de red en el medio). En el binario
    // original ese scratch vive en otra dirección; acá le damos storage propio.
```

### Línea 572 en `sub_4F5CE0_` — antes de `DWORD* slot = (DWORD*)Party;`

```cpp
        // 2026-08-25 FIX (el panel mostraba coords y HP en basura): la base
        // estaba en `Party + 24`, o sea TODOS los campos corridos 24 bytes —
        // se leia dentro del registro SIGUIENTE. IDA `RenderParty` (0x4EF160
        // L435) usa `v4 = 36 * row + Party`, la base del registro:
        //     +0        name[10]
        //     +12       map      -> GlobalText[map + 30]
        //     +13, +14  X, Y
        //     +16, +20  CurLife, MaxLife (DWORD)
        // El +24 SI es correcto para las barras del HUD de mas arriba (ahi el
        // byte de paso de vida vive en +24, IDA L297 `v67 = &Party + 6`), pero
        // no para este panel.
        //
        // Los offsets relativos ya estaban bien; lo unico que fallaba era la
        // base — y el nombre lo compensaba a mano con `v4 - 24`.
```

### Línea 667 en `sub_4F5CE0_` — antes de `}`

```cpp
        // 2026-05-04: Party panel X close button click handler.  The original
        // 0.97k routes this via a __thiscall hit-test (sub_402F40 pattern at
        // MouseX in [475,499) × MouseY in [395,419) when MouseLButtonPush);
        // we add the equivalent inline here so the X button actually closes
        // the panel, mirroring the bottom-HUD Party toggle behaviour.
```

### Línea 675 — antes de `#define g_iKeyPadEnable DAT_07eaa144`

```cpp
// 2026-08-25: esto era un `static` propio del archivo, o sea una SEGUNDA copia
// del flag. El global real es 0x7EAA144 (= DAT_07eaa144), que es el que lee el
// hit-test de la creacion de guild en `SecondPassword_Screen1`: el handler del 0x55
// seteaba esta copia y el hit-test leia la otra, que nunca pasaba de 0.
// Ver [[global-partido-en-dos]].
```

### Línea 702 — antes de `extern "C" void GuildCreator_OpenQuestionFromServer(void)`

```cpp
// 2026-08-25: el dialogo previo "¿crear guild?" — modo 0 del mismo panel.
// El server lo abre con `[C1][03][54]` (`GCGuildMasterQuestionSend`,
// Protocol.cpp:1795) cuando hablas con el NPC Guild Master Y cumplis los
// requisitos; si ya estas en un guild o te falta nivel/resets manda un chat o
// un notice y no abre nada (NpcTalk.cpp:197-220).
//
// IDA: ProtocolCore 0x54. Cierra las ventanas de NPC y abre el creador con el
// keypad APAGADO; así ambos botones responden con 0x54 en vez de 0x55/0x57
// (ver el hit-test en SecondPassword_Screen1).
```

### Línea 755 — antes de `#define byte_7E919BC  DAT_07e919bc`

```cpp
// `byte_7E919BC` NO es un buffer propio: es la tabla global `DAT_07e919bc`
// (stride 80). Antes se definía acá un array separado de 1280 bytes, así que
// Net_Process escribía en una memoria y el render leía otra — el panel de
// guild mostraba "no tenés guild" aunque la lista hubiera llegado.
```

### Línea 763 — antes de `extern "C" void __cdecl RenderCharacterInfoWindow(int iPosX, int iPosY)`

```cpp
// =============================================================================
// RenderCharacterInfoWindow — sub_4ECC60 @ 0x004ECC60 (9398 bytes).
// Faithful port: panel skeleton + name/class banner with sin alpha pulse +
// 5 stat-row backgrounds (sprite 245) + Level/Exp + Strength/Damage row +
// Agility/Defense row + Vitality/HP row + Energy/Mana row + DK/MG skill
// damage line (when applicable) + Charisma/Command (DK/MG) line.
// Anti-tamper CharacterMachine hash-table refcount blocks (~60% of original
// bytes) skipped per CLAUDE.md project policy — they no-op when the table
// is empty (`dword_55C9BD4 == 0`) which is our default state.
// =============================================================================
```

### Línea 814 en `sub_4F5CE0_` — antes de `{`

```cpp
    // Zone label: "ServerName - Channel" via GlobalText[460]/[461].
    // 2026-05-04: el sprintf usaba `"%s"` con GlobalText[460] como dato → si
    // GlobalText[460] contenía un format spec como "%s - %d" lo copiaba
    // literal al pszText y el HUD mostraba "%s - %d" en pantalla.
    // IDA `sub_4ECC60:280-287` hace:
    //   if (sub_406B10(ServerSelectHi, dword_56169C))
    //     sprintf(pszText, GlobalText[460], &ServerList[idx], channel);
    //   else
    //     sprintf(pszText, GlobalText[461], &ServerList[idx], channel);
    // ServerSelectHi=ServerSelectHi (index del server elegido), channel=ServerLocalSelect.
```

### Línea 863 en `sub_4F5CE0_` — antes de `sprintf(Buffer, GlobalText[200],`

```cpp
    // ── Level (label) ───────────────────────────────────────────────────────
    // 2026-05-04: GlobalText[200] format = "Nivel: %d / %d" (CharacterLevel /
    // MaxCharacterLevel). El IDA Hex-Rays decompile perdió el 2º arg; el
    // companion source PrintPlayer.cpp:472 confirma 2 args.
    // g_MaxCharacterLevel se popula por opcode 0xDF (Net_Process); default 400.
    // Declarado en globals.h con extern "C".
```

### Línea 916 en `sub_4F5CE0_` — antes de `int availPts = (int)*(unsigned short*)(CA + 84);`

```cpp
    // ── Stat-add [+] buttons (sprites 0x120 / 0x121) ────────────────────────
    // 2026-08-08: la geometría venía de OpenMU (PrintPlayer.cpp) y NO coincidía
    // con el hit-test real. Ahora sale de IDA sub_4E5DE0 L94-98, que es donde el
    // binario testea estos botones:
    //     x ∈ [CharacterInfoStartX+125, +149)      (24 px)
    //     y ∈ [CharacterInfoStartY+115+60*row, +24)
    // Gate: LevelUpPoint = *(WORD*)(CharacterAttribute+0x54) != 0.
    //   Sprite 0x120 (288): normal   0x121 (289): hover/pressed
    // El click y el envío del F3/06 los hace SecondPassword_Screen3 — acá sólo se dibuja.
```

### Línea 932 en `sub_4F5CE0_` — antes de `bool pressed = hover && (DAT_083a4124 != 0 || DAT_083a413c != 0);`

```cpp
        // 2026-05-04: tap clicks (DOWN+UP within one frame) lose DAT_083a4124
        // before this render runs — WM_LBUTTONUP clears it.  Use the latched
        // click-event flag DAT_083a413c instead (set on UP, sticks until
        // consumed). Hover-without-click still highlights via 4124 for held
        // clicks.  Player_InputTick already gates on g_MouseOnWindow so it
        // won't double-fire ground walk.
        // 2026-08-08: SOLO render + highlight. El CLICK (y el envío del
        // F3/06) lo maneja `SecondPassword_Screen3` (port de sub_4E5DE0 L85-244), que es
        // donde el binario original tiene el hit-test de estos botones — mismo
        // rect (+125..+149 × +115+60*row ..+24) y mismo gate (LevelUpPoint).
        // Tener el send acá TAMBIÉN mandaba el paquete dos veces por click.
```

### Línea 1193 en `sub_4F5CE0_` — antes de `if (g_nGuildMemberCount > 0 && DAT_055c9ff4) {`

```cpp
    // ── Listado de miembros ───────────────────────────────────────────────
    // IDA `RenderGuildList` @0x4F0810 termina con:
    //     if ( g_nGuildMemberCount > 0 )
    //         (*(void (**)(void))(*(_DWORD *)dword_55C9FF4 + 16))();
    // o sea despacha el slot 4 de la vtable del widget de lista (el que dibuja
    // las filas de miembros).
    //
    // 2026-08-15: el AV de EJECUCIÓN que había acá NO era "el objeto no está
    // construido" — `WinMain.cpp:844` sí lo construye vía
    // `ChatListBox_ConstructWhisper()`.  El problema era que ese constructor le
    // instalaba la vtable del CHAT (`s_ChatLB_VTable`, off_5525CC) cuando el
    // binario le pone off_5526EC, y 13 de los 30 slots DIFIEREN — entre ellos
    // los cuatro que usa este dispatch (20/22/23/24).  Corregido en
    // `UI/ChatListBox.cpp` (bloque "WIDGET DE LISTA DE GUILD").
```

### Línea 1229 en `sub_4F5CE0_` — antes de `g_GuildCreatorScratchX = iPosX;`

```cpp
    // 2026-07-27 FIX: idem Party panel — Inventory[32] es el slot 0 del pool de
    // la tienda; usar storage propio en vez de pisarlo.
```

### Línea 1447 en `sub_4F5CE0_` — antes de `{`

```cpp
    // IDA RenderShopInterface (0x4F1F50) L130-265: fila de botones en y+365.
    // El hit-test lo hace FUN_004ec330 (sub_4EC330).  Hasta 2026-09-12 no se
    // dibujaba ninguno: el herrero no mostraba los de reparacion.
```

### Línea 1514 en `sub_4F5CE0_` — antes de `}`

```cpp
    // 2026-09-12: aca el port dibujaba una X de cierre (bitmap 280) en
    // (+25, +395).  RenderShopInterface (0x4F1F50) no la tiene: la tienda se
    // cierra con la X del inventario (sub_4EC330, InventoryStartX + 25).  Esa
    // X inventada tapaba la etiqueta de "reparar todo".
```

### Línea 1642 — antes de `extern "C" void __cdecl RenderWarehouse(void)`

```cpp
// RenderWarehouse — sub_4F3170 @ 0x004F3170 (2776 bytes).
//
// 2026-08-08: port completo. Antes era un esqueleto (interfaz + grid + un botón
// suelto 280) al que le faltaba TODA la fila inferior: el zen guardado, el
// impuesto de retiro y los 3 botones (guardar zen / sacar zen / candado).
// El hit-test de esos botones ya estaba portado (SecondPassword_Screen9, mal llamado
// "SecondPassword_Screen9" en Net/SecondPassword.cpp) pero no se veía nada,
// así que había que adivinar dónde clickear.
//
// Ruido anti-tamper (hash table sobre CharacterMachine alrededor de CADA lectura
// de +0x54C/+0x548 y del Level) omitido per policy — el efecto neto es la
// lectura directa.
```

### Línea 1786 — antes de `extern "C" void __cdecl RenderEventWindow(void)`

```cpp
// The 0.97K event dialog is not an item grid.  It is a selector of four Devil
// Square or six Blood Castle levels (IDA RenderEventWindow @ 0x004F3C50).
// Keeping it as a generic InventoryInterface skeleton was why these NPCs looked
// like a shop even when ReceiveTalk had correctly selected EventWindowOpened.
// ── RenderEventWindow (0x004F3C50) ───────────────────────────────────────────
// Selector de nivel del evento: 4 filas para Devil Square, 6 para Blood Castle.
//
// 2026-09-07: el port anterior manejaba el CLICK y mandaba el paquete de entrada
// desde aca (`SendEventEntry`).  En el binario esta funcion **solo dibuja**: el
// click lo atiende `sub_4E6C40`, que es donde viven el chequeo de nivel, los dos
// carteles de "nivel muy bajo/alto" y el envio.  Tener las dos cosas hacia que el
// click se consumiera aca (`MouseLButtonPush = 0`) antes de llegar al handler
// bueno, y que se enviara un paquete con el slot mal calculado (usaba +12 para
// los dos eventos, cuando Devil Square usa +24).
//
// Diferencia real entre los dos paneles, y por que Devil Square muestra las
// cuatro filas habilitadas: **solo Blood Castle** grisa las filas fuera de rango
// (`iPos_x = 0` -> glColor 0.4/0.4/0.5).  Devil Square las dibuja todas iguales y
// deja que `sub_4E6C40` conteste con el cartel.  Es asi en el binario.
```

### Línea 1906 en `ChaosMixFormatZen` — antes de `Net_SendNpcTalkClose();`

```cpp
            // 2026-09-20: aca se mandaba `Net_SendEventWindowClose()` = un
            // [C1][03][97], copiado del boton de cerrar del Golden Archer.  Es
            // invencion del port: la ventana de evento (Devil Square / Blood
            // Castle) la abre un NPC via 0x30, `CloseInventoryRelatedWindows`
            // (0x4CBA60) no manda NADA al cerrarla, y el unico paquete que el
            // binario emite por esa ventana es el 0x31 de `SendMove`
            // (0x491C40 L742: `buf[4] = 49` con EventWindowOpened).
            //
            // Ademas era peligroso contra MuEmu: su `case 0x97` es un paquete
            // con sub-opcode (`lpMsg[3]`), asi que con un frame de 3 bytes leia
            // un byte FUERA del paquete y, si caia en 0x02 o 0x03, despachaba
            // un canje del Golden Archer con datos basura.
```
