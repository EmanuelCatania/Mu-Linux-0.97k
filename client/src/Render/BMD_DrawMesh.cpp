// BMD_DrawMesh.cpp
// BMD__RenderMesh @ 0x00440D50  [Kayito: unnamed; S6 equivalent = BMD::RenderMesh]
//
// Draws one mesh slot of a BMD (Bone Mesh Data) model using OpenGL.
// Called __thiscall in binary; reconstructed as __cdecl with explicit bmd_obj.
//
// Parameters (matched to S6 BMD::RenderMesh):
//   bmd_obj   = BMD object pointer (ECX in original)
//   meshIdx   = mesh/bone-slot index (param_1 as float→int)
//   flags     = render mode bitmask (param_2)
//   alpha     = transparency (param_3)
//   blendMesh = blend-mesh index or -1.0f (param_4); reused as mode-tag internally
//   blendLight= blend mesh luminosity (param_5)
//   uvU       = blend mesh texcoord U offset (param_6)
//   uvV       = blend mesh texcoord V offset (param_7)
//   texOverride= texture slot override (param_8, 0xffffffff = use mesh default)
//
// BMD object offsets:
//   +0x28 = Meshs array ptr  (Mesh_t stride 0x28)
//   +0x38 = IndexTexture ptr (texture lookup table)
//   +0x44 = LightEnable (char)
//   +0x48..0x50 = BodyLight[3] (RGB floats)
//   +0x88 = StreamMesh index (char)
//   +0x98 = Skin index (char)
//   +0x99 = HideSkin flag (char)
//
// Mesh_t offsets (stride 0x28):
//   +0x00 = flags/active (char)
//   +0x02 = Texture index (short) → IndexTexture lookup
//   +0x06 = NumNormals (short)
//   +0x0a = NumTriangles (short)
//   +0x18 = TexCoords ptr  (UV array, float[2] per entry)
//   +0x1c = Triangles ptr  (Triangle_t array, stride 0x24)
//   +0x24 = SubMesh ptr (optional, checked for NULL)
//
// Triangle_t offsets (stride 0x24):
//   +0x00 = vertex count (char)
//   +0x0a = first vertex record (shorts)
//       vertex[i][-4] = bone/vertex position index (sVar3)
//       vertex[i][0]  = light vertex index
//       vertex[i][4]  = texcoord index
//
// Global buffers:
//   DAT_077e298c = IntensityTransform[meshIdx * 15000] (float per normal)
//   DAT_060db65c = LightTransform RGB [vertex * 12 bytes = float R,G,B]
//   DAT_0584621c = BoneVertex world positions [vertex * 3 floats]
//   DAT_05828d5c = Chrome UV array [packed: U0,V0,U1,V1,...] (floats)
//   DAT_0839bc8c = WaterTextureNumber (DWORD)
//   DAT_083a7cc8 = Texture table (stride 0x38, byte[0] = bpp/type)
//
// Texture special values:
//   0x12c (300) = BITMAP_HIDE → skip mesh
//   0x12d (301) = BITMAP_SKIN → use BodySkin variant
//   0x041  (65) = BITMAP_WATER → use animated water texture
//
// Render flag bits:
//   0x400 = animated variant → delegate to BMD__RenderMeshTranslate
//   0x001 = RENDER_COLOR (no texture, flat color)
//     0x040 = RENDER_BRIGHT (additive blend)
//     0x080 = RENDER_DARK (subtractive blend)
//   0x004 = RENDER_CHROME (environment map, wavy V texcoords)
//   0x008 = RENDER_OBJ (some object variant)
//   0x200 = RENDER_OIL (wavy U+V chrome variant)
//   0x002 = RENDER_TEXTURE (explicit texture bind)
//   0x010 = RENDER_LIGHTMAP
//
// Mode-tag constants (param_4 reused as local int tag via float bits):
//   MODE_COLOR   = bits of 0x00000002 as float = 1.4013e-45
//   MODE_CHROME  = bits of 0x0000000a as float = 5.60519e-45
//   MODE_TEXTURE = bits of 0x00000004 as float = 2.8026e-45
//   MODE_METAL   = bits of 0x00000080 as float = 8.96831e-44

#include "stdafx.h"
#include "../globals.h"
#include "../functions.h"

// Additional globals used by this function
// DAT_060db65c defined via macro in globals.h (backed by g_BoneLightBuf).
// DAT_05828d5c / DAT_06f433bc / DAT_06f433c0 defined via macros in globals.h
// (chrome UV scratch + transformed-normal buffer). No re-declarar acá.
extern float   _DAT_00552868;  // 0.0001f  (1/10000 for WorldTime wave)
extern float   _DAT_00552544;  // alpha < this → use alpha channel
extern float   _DAT_00552580;  // 0.0f
extern float   _DAT_005528c0;  // chrome U scale
extern float   _DAT_005528b4;  // chrome offset
extern float   _DAT_00552504;  // chrome scale factor
extern float   _DAT_0055256c;  // chrome V offset
extern float   _DAT_005526e4;  // some chrome constant
extern float   _DAT_00552540;  // blend tex scale
extern float   _DAT_00552530;  // blend tex factor

// Render mode tags (see header comment)
#define RENDER_MODE_COLOR   1.4013e-45f
#define RENDER_MODE_TEXTURE 2.8026e-45f
#define RENDER_MODE_CHROME  5.60519e-45f
#define RENDER_MODE_METAL   8.96831e-44f

// GL state helpers — aliases to the actual FUN_ addresses
#define BindTexture(x)          GL_BindTextureSlot(x)
#define EnableAlphaBlend()      GL_SetBlendAdditive()
#define EnableAlphaBlendMinus() GL_SetBlendSrcAlpha()
#define EnableLightMap()        GL_EnableLightMap()
#define EnableAlphaTest(x)      GL_SetBlendSrcOver(x)
#define DisableAlphaBlend()     GL_ResetState()
#define DisableTexture(x)       GL_SetAlphaTest(x)
#define DisableDepthMask()      GL_DisableDepthWrites()



static int DecodeMeshIndex(float meshIdx)
{
    unsigned int bits = 0;
    memcpy(&bits, &meshIdx, sizeof(bits));
    if ((bits & 0x7f800000u) == 0 && bits != 0) {
        return (int)bits;
    }
    return (int)meshIdx;
}

void __cdecl BMD__RenderMesh(void *bmd_obj, float meshIdx, int flags,
                           float alpha, int blendMesh, float blendLight,
                           float uvU, float uvV, unsigned int texOverride)
{
    char *pcVar1;
    int iVar2;
    short sVar3;
    unsigned char bVar4;
    bool bVar5;
    unsigned int uVar6;
    unsigned int uVar7;
    float *pfVar8;
    float fVar9;
    float *pfVar10;
    float fVar11;
    int iVar12;
    char *pcVar13;
    short *psVar14;
    unsigned int local_10;
    int meshIndex = DecodeMeshIndex(meshIdx);
    // IDA sub_440D50: a5 (BlendMesh) es un ENTERO -- indice de malla o -1.
    // El port lo habia tipado float y convivian dos codificaciones incompatibles
    // (bit-pattern desde o+100 vs valor numerico en los literales -1.0f), asi que
    // `-1.0f` caia en `a5 <= -2` y disparaba blend aditivo donde el binario
    // texturaba normal.  Ahora es int, como en el binario.
    int blendMeshIndex = blendMesh;
    // v47 del decompile: modo de emision de coordenadas/color del bucle de vertices.
    enum { MODE_NONE = 0, MODE_COLOR = 1, MODE_TEXTURE = 2, MODE_CHROME = 4, MODE_METAL = 64 };
    int renderMode = MODE_NONE;
    uVar6 = flags;


    // Get mesh slot pointer: Meshs[meshIdx] (stride 0x28)
    pcVar1 = (char *)(*(int *)((int)bmd_obj + 0x28) + meshIndex * 0x28);

    // Early-out: NumTriangles == 0
    if (*(short *)(*(int *)((int)bmd_obj + 0x28) + 10 + meshIndex * 0x28) == 0) {
        return;
    }

    // Animated variant: delegate to BMD_DrawBoneSlot_Anim
    if ((flags & 0x400) != 0) {
        // El indice de malla llega como BIT-PATTERN (denormal): los callers usan
        // 1.4013e-45f para la malla 1, y BMD__RenderMeshTranslate lo consume con `(int)frame`
        // -> (int)1.4013e-45f == 0, o sea dibujaba SIEMPRE la malla 0.
        // Se le pasa el indice ya decodificado. Sintoma: Queen Rainer (ModelID
        // 321) oculta su malla 1 en el pase principal y la redibuja por aca; al
        // dibujarse la 0 en su lugar, el vestido no aparecia nunca.
        BMD__RenderMeshTranslate(bmd_obj, '\x01', 0, (float)meshIndex, flags, alpha, blendMesh, uvU, uvV, blendLight, texOverride);
        return;
    }

    // WorldTime wave value (for animated texcoords)
    // IDA sub_440D50: `v42 = (__int64)WorldTime % 10000 * 0.0001`. Hay que leer
    // WorldTime = DAT_05826e08 explícitamente: `__ftol()` lee el tope de la pila
    // x87 (ST0), que no tiene por qué ser WorldTime, y con una fase basura el
    // scroll de texcoords del glow +N (chrome, textura 1170) sale sólido y
    // desincronizado entre partes.
    long long worldTime = (long long)DAT_05826e08;
    float fVar16 = (float)(int)(worldTime % 10000) * _DAT_00552868;

    // Texture lookup: local_10 = IndexTexture[Mesh.Texture]
    local_10 = (unsigned int)*(short *)(*(int *)((int)bmd_obj + 0x38) + *(short *)(pcVar1 + 2) * 2);

    if (local_10 == 300) {  // BITMAP_HIDE
        return;
    }
    if (local_10 == 0x12d) {  // BITMAP_SKIN
        if (*(char *)((int)bmd_obj + 0x99) != '\0') {  // HideSkin
            return;
        }
        local_10 = (int)*(char *)((int)bmd_obj + 0x98) + 0x12d;  // BITMAP_SKIN + Skin
    } else if (local_10 == 0x41) {  // BITMAP_WATER
        local_10 = DAT_0839bc8c + 0x41;  // BITMAP_WATER + WaterTextureNumber
    }

    // Override texture if specified
    if (texOverride != 0xffffffff)
        local_10 = texOverride;

    bVar5 = false;
    // StreamMesh index
    int streamMeshIndex = (int)*(char *)((int)bmd_obj + 0x88);  // StreamMesh
    int waveMeshIndex = streamMeshIndex;

    // Check blend sub-mesh activation
    if ((*(int *)(pcVar1 + 0x24) != 0) && (*(char *)(*(int *)(pcVar1 + 0x24) + 2) != '\0'))
        waveMeshIndex = meshIndex;

    // Enable blend wave if both conditions
    if (((meshIndex == blendMeshIndex) || (meshIndex == waveMeshIndex)) &&
        ((uvU != _DAT_00552580 || (uvV != _DAT_00552580))))
        bVar5 = true;

    // Lighting setup
    unsigned char param_2_b0 = *(char *)((int)bmd_obj + 0x44);  // LightEnable

    // ── BRIGHTNESS BOOST ──────────────────────────────────────────────────────
    // El port se ve oscuro comparado con el armed. Sin GL_LIGHTING activo y
    // con bodyLight bajo (p.ej. 0.20,0.20,0.20) el texel × glColor pinta casi
    // negro. Aplicamos un boost CLAMPEADO a 1.0: los objetos ya brillantes
    // (bodyLight=(1,1,1) — sky, banner, wave) siguen pintando color × 1.0
    // (evita saturación a blanco); los oscuros (ships, chars) se iluminan.
    if (meshIndex == streamMeshIndex) {
        // StreamMesh: flat body color, no per-vertex lighting
        glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
        param_2_b0 = '\0';
    } else if ((param_2_b0 != '\0') && (iVar12 = 0, 0 < *(short *)(pcVar1 + 6))) {
        // Per-vertex lighting: LightTransform[mesh * 15000 + normal] * BodyLight * boost
        pfVar8  = (float *)(&DAT_077e298c + meshIndex * 15000);
        pfVar10 = (float *)(&DAT_060db65c + 4 + meshIndex * 180000);  // G-component base
        do {
            iVar12++;
            pfVar10[-1] = *pfVar8 * *(float *)((int)bmd_obj + 0x48);  // R
            *pfVar10    = *pfVar8 * *(float *)((int)bmd_obj + 0x4c);  // G
            pfVar10[1]  = *pfVar8 * *(float *)((int)bmd_obj + 0x50);  // B
            pfVar8++;
            pfVar10 += 3;
        } while (iVar12 < *(short *)(pcVar1 + 6));
    }

    bVar4 = (unsigned char)uVar6;

    if ((bVar4 & 1) == 1) {
        // RENDER_COLOR: flat color, disable texture
        renderMode = MODE_COLOR;
        if ((bVar4 & 0x40) == 0x40)
            EnableAlphaBlend();
        else if ((bVar4 & 0x80) == 0x80)
            EnableAlphaBlendMinus();
        else
            DisableAlphaBlend();
        DisableTexture('\0');

        if (alpha < _DAT_00552544) {
            EnableAlphaTest('\x01');
            glColor4f(*(float *)((int)bmd_obj + 0x48),
                      *(float *)((int)bmd_obj + 0x4c),
                      *(float *)((int)bmd_obj + 0x50), alpha);
        } else {
            glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
        }
    } else {
        uVar7 = uVar6 & 4;
        if (((uVar7 == 4) || ((bVar4 & 8) == 8)) || ((uVar6 & 0x200) == 0x200)) {
            // RENDER_CHROME / OIL / OBJ: compute wavy texcoords
            if ((*(int *)(pcVar1 + 0x24) != 0) && (*(char *)(*(int *)(pcVar1 + 0x24) + 3) != '\0')) {
                return;
            }
            if (*pcVar1 != '\0') {
                return;
            }

            renderMode = MODE_CHROME;

            // Recompute WorldTime-based phase for OIL (misma corrección: usar
            // WorldTime explícito, no ST0 basura). IDA: `(__int64)WorldTime % 5000`.
            iVar12 = 0;
            fVar11 = (float)(int)(worldTime % 5000) * _DAT_005528c0 - _DAT_005528b4;

            if (0 < *(short *)(pcVar1 + 6)) {
                // Ojo: Ghidra decompiló mal la base.
                // Disasm @ 0x00441194:  MOV ECX,0x5828d60   (NO 0x5828d58)
                // Layout real del array: d5c=U0, d60=V0, d64=U1, d68=V1, ...
                // Con la base en d58, pfVar10[-1] escribiría en DAT_05828d54 y *pfVar10 en
                // DAT_05828d58, que es el puntero base a la tabla de modelos (g_Models).
                pfVar10 = ((float *)&DAT_05828d5c) + 1;   // = &DAT_05828d60 (V0 slot)
                // DAT_06f433c0 está declarado como `float`, así que &DAT_06f433c0 +
                // meshIdx*180000 sería aritmética float* (= +meshIdx*720000 bytes). El stride
                // real per-mesh son 180000 BYTES (disasm @ 0x004411a8: ADD EAX,0x6f433c0
                // tras EBP*180000). Se castea a char* para que el +N sea byte arith.
                pfVar8  = (float *)((char*)&DAT_06f433c0 + meshIndex * 180000);  // source normals
                do {
                    if ((uVar6 & 0x200) == 0x200) {
                        // OIL mode
                        pfVar10[-1] = fVar11 + fVar11 + (pfVar8[1] + pfVar8[-1]) * _DAT_00552530;
                        fVar9 = fVar11 * _DAT_00552540 + *pfVar8 + pfVar8[-1];
                    } else if (uVar7 == 4) {
                        // CHROME mode
                        pfVar10[-1] = pfVar8[1] * _DAT_00552504 + fVar16;
                        fVar9 = *pfVar8 * _DAT_00552504 + fVar16 + fVar16;
                    } else {
                        // OBJ mode
                        pfVar10[-1] = pfVar8[1] * _DAT_00552504 + _DAT_005526e4;
                        fVar9 = (*pfVar8 + _DAT_0055256c) * _DAT_00552504;
                    }
                    iVar12++;
                    *pfVar10 = fVar9;
                    pfVar8  += 3;
                    pfVar10 += 2;
                } while (iVar12 < *(short *)(pcVar1 + 6));
            }

            if ((bVar4 & 0x40) == 0x40) {
                if (alpha < _DAT_00552544) {
                    *(float *)((int)bmd_obj + 0x48) = alpha * *(float *)((int)bmd_obj + 0x48);
                    *(float *)((int)bmd_obj + 0x4c) = alpha * *(float *)((int)bmd_obj + 0x4c);
                    *(float *)((int)bmd_obj + 0x50) = alpha * *(float *)((int)bmd_obj + 0x50);
                }
                EnableAlphaBlend();
            } else if ((bVar4 & 0x80) == 0x80) {
                EnableAlphaBlendMinus();
            } else if ((bVar4 & 0x10) == 0x10) {
                EnableLightMap();
            } else if (alpha < _DAT_00552544) {
                EnableAlphaTest('\x01');
            } else {
                DisableAlphaBlend();
            }

            // Select chrome texture
            if (((uVar6 & 0x200) == 0x200) && (texOverride == 0xffffffff))
                local_10 = 0x4f2;
            else if ((uVar7 == 4) && (texOverride == 0xffffffff))
                local_10 = 0x492;
            else if (((bVar4 & 8) == 8) && (texOverride == 0xffffffff))
                local_10 = 0x4ce;
            BindTexture(local_10);

        } else if (
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
            //   - Divergencia aceptada: el sentinel real que usan los callers es
            //     0xffffffff (ver Entity_DrawByType.cpp).
            // MUGAME type 0xa2: obj+100 = DWORD 1 → como int == mesh.Texture=1 (backdrop)
            //   → cond2 TRUE → EnableAlphaBlend (aditivo) sobre backdrop naranja.
            blendMesh <= -2 || (int)*(short *)(pcVar1 + 2) == blendMesh
        ) {
            // Blend mesh / animated UV variant.
            renderMode = MODE_TEXTURE;
            BindTexture(local_10);
            if ((bVar4 & 0x80) == 0x80)
                EnableAlphaBlendMinus();
            else
                EnableAlphaBlend();
            glColor3f(blendLight * *(float *)((int)bmd_obj + 0x48),
                      blendLight * *(float *)((int)bmd_obj + 0x4c),
                      blendLight * *(float *)((int)bmd_obj + 0x50));
            param_2_b0 = '\0';

        } else if ((bVar4 & 2) == 2) {
            // RENDER_TEXTURE: explicit bind with blend
            renderMode = MODE_TEXTURE;
            BindTexture(local_10);
            if ((bVar4 & 0x40) == 0x40)
                EnableAlphaBlend();
            else if ((bVar4 & 0x80) == 0x80)
                EnableAlphaBlendMinus();
            else if ((alpha < _DAT_00552544) || ((&DAT_083a7cc8)[local_10 * 0x38] == '\x04'))
                EnableAlphaTest('\x01');
            else
                DisableAlphaBlend();

        } else if ((bVar4 & 0x40) == 0x40) {
            // Bright-only chrome: skip if texture type is 4
            if ((&DAT_083a7cc8)[local_10 * 0x38] == '\x04') {
                return;
            }
            renderMode = MODE_METAL;
            EnableAlphaBlend();
            DisableTexture('\0');
            DisableDepthMask();

        } else {
            renderMode = MODE_TEXTURE;
        }
    }


    // Triangle render loop — estructura verbatim del binario original
    // (IDA @ 0x00440D50): glBegin(GL_TRIANGLES) UNA sola vez antes del
    // face loop, glEnd() UNA vez después. Las BMD 97k están pre-trianguladas
    // (nv==3 para todas las faces).

    // No se toca glColor antes de glBegin: cada rama del switch ya dejó el suyo
    // (la rama blend-mesh del banner Mu, type 0xA2, setea
    // glColor3f(blendLight * OBJECT.Light[0..2]) y pisarlo rompería su ramp).
    //
    // Helper1 (modelo 816) — el "hada"/Guardian Angel. Su BMD tiene 2 meshes:
    //   mesh 0 = fairy.jpg   (el cuerpo, 46 triangulos)
    //   mesh 1 = fairy2.jpg  (el glow, 4 triangulos = 2 quads)
    // Ninguna de las dos lleva el marcador `_R` en el nombre (verificado leyendo
    // Helper01.bmd, que es version 10 y NO esta encriptado), asi que
    // `TextureScriptParsing::parsingTScript` (0x40C190 — reconoce R/H/S/N tras
    // un `_`) no las marca como bright y el mesh del glow queda RENDER_TEXTURE
    // opaco: el fondo negro del JPG se dibuja como un recuadro negro.
    // El gate es por MESH, no por escena: solo el glow (mesh 1) lleva aditivo y el
    // cuerpo (mesh 0) sigue opaco, igual en el mundo y en el inventario.
    //
    // DESVIACION documentada: no encontre en IDA el mecanismo por el que el
    // original decide este blend — no sale del asset (el BMD v10 no tiene campo
    // de RenderType; se deriva del nombre de textura) ni de `RenderLinkObject`
    // (0x455430), que no toca BlendMesh. Queda como forzado explicito.
    if (bmd_obj == (void*)(DAT_05828d58 + 816 * 0xbc) && meshIndex == 1) {
        EnableAlphaBlend();
    }

    glBegin(GL_TRIANGLES);
    texOverride = 0;

    if (0 < *(short *)(pcVar1 + 0x0a)) {
        float param_5_f = 0.0f;  // face byte offset
        do {
            pcVar13 = (char *)((int)param_5_f + *(int *)(pcVar1 + 0x1c));
            iVar12 = 0;
            if ('\0' < *pcVar13) {
                psVar14 = (short *)(pcVar13 + 10);
                do {
                    sVar3 = psVar14[-4];  // bone/position index

                    if (renderMode == MODE_TEXTURE) {
                        // Standard UV from texcoord table
                        pfVar8 = (float *)(*(int *)(pcVar1 + 0x18) + psVar14[4] * 8);
                        if (bVar5) {
                            // Blend wave offset
                            glTexCoord2f(*pfVar8 + uvU, pfVar8[1] + uvV);
                        } else {
                            glTexCoord2f(*pfVar8, pfVar8[1]);
                        }
                        if (param_2_b0 != '\0') {
                            iVar2 = meshIndex * 15000 + (int)*psVar14;
                            if (alpha < _DAT_00552544) {
                                glColor4f(*(float *)(&DAT_060db65c + iVar2 * 12),
                                          *(float *)(&DAT_060db65c + iVar2 * 12 + 4),
                                          *(float *)(&DAT_060db65c + iVar2 * 12 + 8), alpha);
                            } else {
                                glColor3fv((const GLfloat *)(&DAT_060db65c + iVar2 * 12));
                            }
                        }
                    } else if (renderMode == MODE_CHROME) {
                        // Chrome UV from precomputed chrome table
                        if (alpha < _DAT_00552544) {
                            glColor4f(*(float *)((int)bmd_obj + 0x48),
                                      *(float *)((int)bmd_obj + 0x4c),
                                      *(float *)((int)bmd_obj + 0x50), alpha);
                        } else {
                            glColor3fv((const GLfloat *)((int)bmd_obj + 0x48));
                        }
                        // Ghidra emitió `(&DAT_05828d5c + 4)[idx*2]`, que es aritmética float*
                        // (+4 = +16 bytes) y leería V de d6c+idx*8 en vez de d60+idx*8.
                        // Disasm @ 0x004413bd-c4:
                        //   MOV ECX,[EAX*0x8 + 0x5828d60]   ; V
                        //   MOV EDX,[EAX*0x8 + 0x5828d5c]   ; U
                        // El array es {U,V,U,V,...} contiguo desde d5c → V está en idx*2+1.
                        glTexCoord2f((&DAT_05828d5c)[*psVar14 * 2 + 0],
                                     (&DAT_05828d5c)[*psVar14 * 2 + 1]);
                    }

                    // Emit vertex from BoneVertex buffer
                    {
                        const float *_v = (const float*)(&DAT_0584621c + ((int)sVar3 + meshIndex * 15000) * 3);
                        glVertex3fv((const GLfloat *)_v);
                    }

                    iVar12++;
                    psVar14++;
                } while (iVar12 < *pcVar13);
            }
            texOverride++;
            param_5_f = (float)((int)param_5_f + 0x24);  // face stride = 36 bytes
        } while ((int)texOverride < (int)*(short *)(pcVar1 + 0x0a));
    }
    glEnd();
    // Restaurar el depth-mask al salir: el path chrome (flag&0x40) llama
    // DisableDepthMask() arriba y, si no se restaura, los meshes siguientes del
    // mismo frame se dibujan sin depth-write (efecto "ghost"). El binario lo hace
    // en los setters de estado entre meshes, pero acá el cache DAT_083a42e8 puede
    // quedar desincronizado.
    GL_EnableDepthWrites();  // EnableDepthMask
}
