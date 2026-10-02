// Combat_AttackEffect.cpp
//
// AttackEffect (IDA 0x00445230) -- los efectos visuales del ataque de los
// MONSTRUOS.  Es el port crudo del decompile (la macro IDA_PORT_00445230 está
// definida en globals.h, así que este código está vivo).
//
// El selector del switch es `Owner+747`, el TIPO DE MONSTRUO de Monster.txt.
// Para el heroe ese byte vale 0xFF, asi que no matchea ningun case: la funcion
// es un no-op para el jugador POR DISENO del binario.  Los efectos de los
// skills del jugador viven en los tres switches de MoveCharacter.
//
// Los `AE_*` de abajo son shims que neutralizan el ruido anti-tamper del
// decompile (hash-table, CErrorReport, operator_new) sin tocar la logica.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"


// === AttackEffect (0x00445230) ===
// Gateada por IDA_PORT_00445230, que esta definida.
extern "C" void DbgLogPublic(const char* msg);   // probe AEDBG (temporal)

// ─────────────────────────────────────────────────────────────────────────────
// Shims de compatibilidad para el port crudo de AttackEffect.
//
// El decompile de IDA trae el ruido anti-tamper tal cual (hash-table +
// CErrorReport + operator_new/delete alrededor de CADA lectura del byte de
// skill en Owner+770). Por policy del proyecto ese ruido no se ejecuta.
//
// Camino real en nuestro build: g_HashTableCtx tiene capacity=1 con el slot 0
// como centinela (key=0), asi que el primer `memcmp(&v245 /*0*/, keys[0] /*0*/)`
// da match, rompe el while y cae directo al `LABEL_*: Ownerx = *vN;` que lee el
// byte EN CLARO — que es exactamente lo que queremos (todo el resto del port
// escribe/lee Owner+770 sin encriptar). El segundo bloque (re-encrypt) no entra
// al while por la misma razon y no toca nada.
//
// Estos shims neutralizan las llamadas para que compile sin editar los ~60
// bloques a mano (menos riesgo de romper la logica de gameplay al reescribir).
// Se hace #undef de todo justo despues del #endif de esta funcion.
// ─────────────────────────────────────────────────────────────────────────────
namespace CErrorReport { inline void Write(DWORD, const char*) {} }
static const char aHashTableFullG[] = "hash table full";
static DWORD      g_ErrorReport = 0;

// Scratch fijo: el "nodo" que crea el ruido anti-tamper nunca se lee (el insert
// es no-op), asi que devolver un buffer estatico evita el leak de operator_new.
static BYTE g_AE_HashScratch[16];
static inline DWORD AE_new(size_t)            { return (DWORD)(uintptr_t)g_AE_HashScratch; }
static inline void  AE_delete(DWORD)          {}
// Reemplaza el dispatch inline `(*(int(**)(int*,DWORD))(MAIN_HASH_CLASS+12))(...)`.
// En nuestro build ese slot es HashFn_Sentinel (globals.cpp) y devuelve 0; el
// shim evita el deref de vtable sin cambiar el comportamiento.
static inline int AE_ht_hash(DWORD) { return 0; }
static inline unsigned int AE_ht_index(const void*, DWORD) { return 0xFFFFFFFFu; }
static inline void  AE_ht_noop2(const void*, DWORD)        {}
static inline void  AE_ht_noop3(const void*, DWORD, DWORD) {}
static inline unsigned char AE_SkillRead(const void* ctx, const void* p)
{ return FUN_0045fae0((DWORD)(uintptr_t)ctx, (unsigned char*)p); }

// Wrappers de tipo: Hex-Rays pasa enteros/DWORD donde functions.h declara
// float*/void*. Se castea aca en vez de tocar los ~50 call sites.
static inline float* AE_CreateEffect(int type, float* p1, float* p2, float* p3,
                                     int a4, int a5, int a6, int a7, int flag)
{ return CreateEffect(type, p1, p2, p3, (float*)(intptr_t)a4, (float*)(intptr_t)a5,
                      (float*)(intptr_t)a6, (float*)(intptr_t)a7, (byte)flag); }
static inline float* AE_CreateEffect(int type, float* p1, float* p2, float* p3,
                                     int a4, int a5, float* a6, int a7, int flag)
{ return CreateEffect(type, p1, p2, p3, (float*)(intptr_t)a4, (float*)(intptr_t)a5,
                      a6, (float*)(intptr_t)a7, (byte)flag); }
static inline void* AE_CreateJoint(int type, float* p1, float* p2, float* p3,
                                   int sub, int owner, double scale, int a, int b)
{ return CreateJoint(type, p1, p2, p3, (unsigned int)sub, owner, (float)scale,
                      (short)a, (unsigned char)b); }
static inline void AE_TransformPosition(DWORD model, const void* mat,
                                        float* pos, float* out, int flag)
{ BMD_TransformPosition((void*)(uintptr_t)model, (float*)mat, pos, out, (char)flag); }

#define Models                        DAT_05828d58
#define Matrix                        DAT_07abf444   // IDA `Matrix` @ 0x07ABF444
#define operator_new(n)               AE_new((size_t)(n))
#define delete__(p)                   AE_delete((DWORD)(uintptr_t)(p))
#define FUN_004041e0(ctx, key)        AE_ht_index((const void*)(ctx), (DWORD)(key))
#define HashTable_Insert(ctx, node, key)  AE_ht_noop3((const void*)(ctx), (DWORD)(uintptr_t)(node), (DWORD)(key))
#define Packet_EncryptByte(node, key)       AE_ht_noop2((const void*)(uintptr_t)(node), (DWORD)(uintptr_t)(key))
#define PACKET_ENCRYPT(ctx, key)      AE_ht_noop2((const void*)(ctx), (DWORD)(uintptr_t)(key))
#define FUN_0045fae0(ctx, p)          AE_SkillRead((const void*)(ctx), (const void*)(p))
#define FUN_00466300(pos)             FUN_00466300((float*)(uintptr_t)(pos))
#define CreateJoint                   AE_CreateJoint
#define TransformPosition(a,b,c,d,e)  AE_TransformPosition((DWORD)(uintptr_t)(a), (const void*)(b), (c), (d), (e))
#define PlayBuffer(a,b,c)             PlayBuffer((a), (DWORD)(b), (BOOL)(c))
#undef  CreateEffect
#define CreateEffect                  AE_CreateEffect

void __cdecl AttackEffect(int Owner)
{
  // Guard de port (no esta en IDA): si la tabla de modelos todavia no esta
  // cargada, `Models + 188*type` seria un puntero basura que TransformPosition
  // deferenciaria. La version parcial de Combat_LegacyAttackEffects.cpp tenia el mismo guard.
  if (!Owner || !DAT_05828d58) return;

  DWORD v2; // edi
  char v3; // al
  char *v4; // ebx
  unsigned int v5; // eax
  bool v6; // cf
  int v7; // eax
  unsigned int v8; // eax
  char *v9; // esi
  unsigned char v10; // al
  char *v11; // eax
  char v12; // cl
  char v13; // cl
  unsigned int v14; // eax
  BYTE *v15; // edi
  char v16; // al
  BYTE *v17; // esi
  char v18; // al
  bool v19; // zf
  signed int v20; // eax
  double v21; // st7
  int v22; // edi
  DWORD v23; // eax
  float *v24; // ebx
  float *v25; // esi
  char v26; // al
  char *v27; // ebx
  unsigned int v28; // edx
  const char *v29; // edi
  int v30; // eax
  unsigned int v31; // eax
  char *v32; // esi
  unsigned char v33; // al
  char *v34; // eax
  char v35; // cl
  char v36; // cl
  unsigned int v37; // eax
  BYTE *v38; // edi
  char v39; // al
  BYTE *v40; // esi
  char v41; // al
  int v42; // esi
  int v43; // eax
  char v44; // al
  int v45; // esi
  float v46; // eax
  double v47; // st7
  float v48; // ecx
  float v49; // edx
  float v50; // eax
  int v51; // eax
  float v52; // eax
  float v53; // ecx
  double v54; // st7
  int v55; // edx
  float v56; // ecx
  float v57; // edx
  float v58; // eax
  double v59; // st7
  int v60; // ecx
  char *v61; // ebx
  unsigned int v62; // edx
  const char *v63; // edi
  int v64; // eax
  unsigned int v65; // eax
  char *v66; // esi
  unsigned char v67; // al
  char *v68; // eax
  char v69; // cl
  char v70; // cl
  unsigned int v71; // eax
  BYTE *v72; // edi
  char v73; // al
  BYTE *v74; // esi
  char v75; // al
  char v76; // al
  int v77; // esi
  int v78; // eax
  char v79; // al
  char *v80; // ebx
  unsigned int v81; // edx
  const char *v82; // edi
  int v83; // eax
  unsigned int v84; // eax
  char *v85; // esi
  unsigned char v86; // al
  char *v87; // eax
  char v88; // cl
  char v89; // cl
  unsigned int v90; // eax
  BYTE *v91; // edi
  char v92; // al
  BYTE *v93; // esi
  char v94; // al
  int v95; // esi
  float v96; // ecx
  double v97; // st7
  char *v98; // ebx
  unsigned int v99; // edx
  const char *v100; // edi
  int v101; // eax
  unsigned int v102; // eax
  char *v103; // esi
  unsigned char v104; // al
  char *v105; // eax
  char v106; // cl
  char v107; // cl
  unsigned int v108; // eax
  BYTE *v109; // edi
  char v110; // al
  BYTE *v111; // esi
  char v112; // al
  int v113; // ebx
  int v114; // eax
  float v115; // eax
  float v116; // ecx
  char *v117; // ebx
  int v118; // eax
  unsigned int v119; // eax
  char *v120; // esi
  unsigned char v121; // al
  char *v122; // eax
  char v123; // cl
  char v124; // cl
  unsigned int v125; // eax
  BYTE *v126; // eax
  char v127; // cl
  int v128; // esi
  float v129; // eax
  double v130; // st7
  char *v131; // ebx
  unsigned int v132; // edx
  const char *v133; // edi
  int v134; // eax
  unsigned int v135; // eax
  char *v136; // esi
  unsigned char v137; // al
  char *v138; // eax
  char v139; // cl
  char v140; // cl
  unsigned int v141; // eax
  BYTE *v142; // edi
  char v143; // al
  BYTE *v144; // esi
  char v145; // al
  double v146; // st7
  float v147; // ecx
  float v148; // edx
  float v149; // eax
  double v150; // st7
  int v151; // esi
  char *v152; // ebx
  unsigned int v153; // edx
  const char *v154; // edi
  int v155; // eax
  unsigned int v156; // eax
  char *v157; // esi
  unsigned char v158; // al
  char *v159; // eax
  char v160; // cl
  char v161; // cl
  unsigned int v162; // eax
  BYTE *v163; // edi
  char v164; // al
  BYTE *v165; // esi
  char v166; // al
  float v167; // eax
  float v168; // ecx
  double v169; // st7
  float v170; // ecx
  char *v171; // ebx
  unsigned int v172; // edx
  const char *v173; // edi
  int v174; // eax
  unsigned int v175; // eax
  char *v176; // esi
  unsigned char v177; // al
  char *v178; // eax
  char v179; // cl
  char v180; // cl
  unsigned int v181; // eax
  BYTE *v182; // edi
  char v183; // al
  BYTE *v184; // esi
  char v185; // al
  int v186; // esi
  unsigned int v187; // ebx
  int v188; // eax
  char *v189; // edi
  char v190; // bl
  double v191; // st7
  unsigned int v192; // eax
  char *v193; // esi
  unsigned char v194; // al
  char *v195; // eax
  char v196; // cl
  char v197; // cl
  double v198; // st7
  int v199; // ebx
  int v200; // eax
  int v201; // ecx
  short v202; // ax
  DWORD v203; // esi
  float v204; // ecx
  float v205; // edx
  float v206; // ecx
  float v207; // edx
  float v208; // edx
  float v209; // eax
  float v210; // ecx
  double v211; // st7
  int v212; // edx
  float *v213; // ebp
  int v214; // edi
  int k; // ebx
  int m; // ebx
  int j; // edi
  int i; // edi
  int v219; // edi
  int v220; // edi
  int v221; // edi
  int ii; // edi
  int n; // edi
  int jj; // edi
  int v225; // edi
  int v226; // eax
  float v227; // eax
  float (*v228)[4]; // eax
  float v229; // ecx
  float (*v230)[4]; // [esp-10h] [ebp-7Ch]
  float (*v231)[4]; // [esp-10h] [ebp-7Ch]
  float (*v232)[4]; // [esp-10h] [ebp-7Ch]
  DWORD This; // [esp+10h] [ebp-5Ch]
  unsigned int v234; // [esp+14h] [ebp-58h]
  unsigned int v235; // [esp+14h] [ebp-58h]
  unsigned int v236; // [esp+14h] [ebp-58h]
  unsigned int v237; // [esp+14h] [ebp-58h]
  unsigned int v238; // [esp+14h] [ebp-58h]
  unsigned int v239; // [esp+14h] [ebp-58h]
  unsigned int v240; // [esp+14h] [ebp-58h]
  unsigned int v241; // [esp+14h] [ebp-58h]
  unsigned int v242; // [esp+14h] [ebp-58h]
  DWORD v243; // [esp+18h] [ebp-54h] BYREF
  const char *v244; // [esp+1Ch] [ebp-50h] BYREF
  DWORD v245; // [esp+20h] [ebp-4Ch] BYREF
  float v246[3]; // [esp+24h] [ebp-48h] BYREF
  float TargetPosition[3]; // [esp+30h] [ebp-3Ch] BYREF
  float v248[3]; // [esp+3Ch] [ebp-30h] BYREF
  float Light[3]; // [esp+48h] [ebp-24h] BYREF
  float Angle[3]; // [esp+54h] [ebp-18h] BYREF
  float Position[3]; // [esp+60h] [ebp-Ch] BYREF
  DWORD Ownera; // [esp+70h] [ebp+4h]
  char Ownerb; // [esp+70h] [ebp+4h]
  DWORD Ownerc; // [esp+70h] [ebp+4h]
  char Ownerd; // [esp+70h] [ebp+4h]
  DWORD Ownere; // [esp+70h] [ebp+4h]
  char Ownerf; // [esp+70h] [ebp+4h]
  DWORD Ownerg; // [esp+70h] [ebp+4h]
  char Ownerh; // [esp+70h] [ebp+4h]
  int Ownery; // [esp+70h] [ebp+4h]
  signed int Owneri; // [esp+70h] [ebp+4h]
  DWORD Ownerj; // [esp+70h] [ebp+4h]
  char Ownerk; // [esp+70h] [ebp+4h]
  unsigned int Ownerl; // [esp+70h] [ebp+4h]
  char Ownerm; // [esp+70h] [ebp+4h]
  signed int Ownern; // [esp+70h] [ebp+4h]
  DWORD Ownero; // [esp+70h] [ebp+4h]
  char Ownerp; // [esp+70h] [ebp+4h]
  DWORD Ownerq; // [esp+70h] [ebp+4h]
  char Ownerr; // [esp+70h] [ebp+4h]
  DWORD Owners; // [esp+70h] [ebp+4h]
  char Ownert; // [esp+70h] [ebp+4h]
  signed int Owneru; // [esp+70h] [ebp+4h]
  DWORD Ownerv; // [esp+70h] [ebp+4h]
  float Ownerw; // [esp+70h] [ebp+4h]
  float Ownerx; // [esp+70h] [ebp+4h]

  v2 = Models + 188 * *(short *)(Owner + 2);
  This = v2;
  rand();
  v3 = *(BYTE *)(Owner + 747);
  memset(v248, 0, sizeof(v248));
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  switch ( v3 )
  {
    case 35:
      v171 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v172 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Owners = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v173 = (const char *)(DAT_055c9bd0 + 4 * v172);
          v244 = v173;
          if ( !memcmp((const char *)&v245, v173, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v172 == -1 )
            {
              break;
            }
            v175 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v175 == -1 )
            {
              v176 = 0;
            }
            else
            {
              v176 = *(char **)(DAT_055c9bcc + 4 * v175);
            }
            v177 = v176[1] + 1;
            v176[1] = v177;
            if ( v177 < 2u )
            {
              v178 = (char *)operator_new(1u);
              v179 = *v176;
              *v178 = *v176;
              v179 -= 35;
              *v178 = v179;
              v180 = (DAT_00559050[0] ^ v179) - 71;
              *v178 = v180;
              *v171 = v180;
              delete__(v178);
            }
            goto LABEL_265;
          }
          v172 = (v172 + 1) % DAT_055c9bd4;
          if ( ++Owners >= DAT_055c9bd4 )
          {
            goto LABEL_263;
          }
        }
      }
      else
      {
LABEL_263:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v174 = operator_new(2u);
      *(BYTE *)(v174 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v174, Owner + 770);
LABEL_265:
      Ownert = *v171;
      v244 = (const char *)(Owner + 770);
      v242 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v242), 4) )
          {
            if ( v242 != -1 )
            {
              v181 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v182 = v181 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v181);
              v183 = v182[1] - 1;
              v182[1] = v183;
              if ( !v183 )
              {
                v184 = (BYTE *)operator_new(1u);
                v185 = *v171 + 71;
                *v184 = v185;
                *v184 = (DAT_00559050[0] ^ v185) + 35;
                *v171 = rand();
                *v182 = *v184;
                delete__(v184);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v242 = (v242 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_269;
          }
        }
      }
      else
      {
LABEL_269:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownert == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v186 = 0;
        Owneru = 0;
        do
        {
          v246[2] = (double)Owneru * 20.0;
          v246[0] = 0.0;
          v246[1] = 0.0;
          CreateEffect(191, (float *)(Owner + 16), v246, (float *)(Owner + 232), 1, Owner, -1, 0, 0);
          Owneru = ++v186;
        }
        while ( v186 < 18 );
        PlayBuffer(46, 0, 0);
      }
      break;
    case 38:
    case 67:
      v244 = (const char *)(Owner + 770);
      v187 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerv = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v187), 4) )
          {
            if ( v187 == -1 )
            {
              break;
            }
            v189 = (char *)(Owner + 770);
            v192 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v192 == -1 )
            {
              v193 = 0;
            }
            else
            {
              v193 = *(char **)(DAT_055c9bcc + 4 * v192);
            }
            v194 = v193[1] + 1;
            v193[1] = v194;
            if ( v194 < 2u )
            {
              v195 = (char *)operator_new(1u);
              v196 = *v193;
              *v195 = *v193;
              v196 -= 35;
              *v195 = v196;
              v197 = (DAT_00559050[0] ^ v196) - 71;
              *v195 = v197;
              *v189 = v197;
              delete__(v195);
            }
            goto LABEL_293;
          }
          v6 = ++Ownerv < DAT_055c9bd4;
          v187 = (v187 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_291;
          }
        }
      }
      else
      {
LABEL_291:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v188 = operator_new(2u);
      v189 = (char *)(Owner + 770);
      *(BYTE *)(v188 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v188, Owner + 770);
LABEL_293:
      v190 = *v189;
      PACKET_ENCRYPT(&MAIN_HASH_CLASS, v189);
      if ( v190 == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          CreateEffect(200, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          CreateEffect(201, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          PlayBuffer(89, 0, 0);
        }
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v191 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v191 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        goto LABEL_75;
      }
      break;
    case 42:
      v152 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v153 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerq = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v154 = (const char *)(DAT_055c9bd0 + 4 * v153);
          v244 = v154;
          if ( !memcmp((const char *)&v245, v154, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v153 == -1 )
            {
              break;
            }
            v156 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v156 == -1 )
            {
              v157 = 0;
            }
            else
            {
              v157 = *(char **)(DAT_055c9bcc + 4 * v156);
            }
            v158 = v157[1] + 1;
            v157[1] = v158;
            if ( v158 < 2u )
            {
              v159 = (char *)operator_new(1u);
              v160 = *v157;
              *v159 = *v157;
              v160 -= 35;
              *v159 = v160;
              v161 = (DAT_00559050[0] ^ v160) - 71;
              *v159 = v161;
              *v152 = v161;
              delete__(v159);
            }
            goto LABEL_238;
          }
          v153 = (v153 + 1) % DAT_055c9bd4;
          if ( ++Ownerq >= DAT_055c9bd4 )
          {
            goto LABEL_236;
          }
        }
      }
      else
      {
LABEL_236:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v155 = operator_new(2u);
      *(BYTE *)(v155 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v155, Owner + 770);
LABEL_238:
      Ownerr = *v152;
      v244 = (const char *)(Owner + 770);
      v241 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v241), 4) )
          {
            if ( v241 != -1 )
            {
              v162 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v163 = v162 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v162);
              v164 = v163[1] - 1;
              v163[1] = v164;
              if ( !v164 )
              {
                v165 = (BYTE *)operator_new(1u);
                v166 = *v152 + 71;
                *v165 = v166;
                *v165 = (DAT_00559050[0] ^ v166) + 35;
                *v152 = rand();
                *v163 = *v165;
                delete__(v165);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v241 = (v241 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_242;
          }
        }
      }
      else
      {
LABEL_242:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerr == 50 )
      {
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
          v230 = (float (*)[4])(*(DWORD *)(Owner + 276) + 528);
          memset(v248, 0, sizeof(v248));
          TransformPosition(This, v230, v248, TargetPosition, 1);
          v167 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) - 30.0;
          v246[1] = v167;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v168 = *(float *)(Owner + 36);
          v169 = *(float *)(Owner + 28) - 30.0;
          v246[1] = *(float *)(Owner + 32);
          v246[0] = v169;
          v246[2] = v168;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          v170 = *(float *)(Owner + 32);
          v246[0] = *(float *)(Owner + 28) - 20.0;
          v246[2] = *(float *)(Owner + 36) + 30.0;
          v246[1] = v170;
          CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 2, 0, -1, 0, 0);
          PlayBuffer(46, 0, 0);
        }
        goto LABEL_31;
      }
      break;
    case 45:
      v199 = 4;
      do
      {
        v248[0] = (float)(rand() % 32 - 16);
        v248[1] = (float)(rand() % 32 - 16);
        v200 = rand() % 32;
        v201 = *(DWORD *)(Owner + 276);
        v248[2] = (float)(v200 - 16);
        TransformPosition(v2, (float (*)[4])(v201 + 96), v248, TargetPosition, 1);
        Particle_Spawn(1241, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        Particle_Spawn(1206, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
        --v199;
      }
      while ( v199 );
      break;
    case 49:
      if ( *(unsigned char *)(Owner + 757) % 5 == 1 )
      {
        TransformPosition(v2, (float (*)[4])(*(DWORD *)(Owner + 276) + 3024), v248, TargetPosition, 1);
        CreateEffect(1211, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      v131 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v132 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownero = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v133 = (const char *)(DAT_055c9bd0 + 4 * v132);
          v244 = v133;
          if ( !memcmp((const char *)&v245, v133, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v132 == -1 )
            {
              break;
            }
            v135 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v135 == -1 )
            {
              v136 = 0;
            }
            else
            {
              v136 = *(char **)(DAT_055c9bcc + 4 * v135);
            }
            v137 = v136[1] + 1;
            v136[1] = v137;
            if ( v137 < 2u )
            {
              v138 = (char *)operator_new(1u);
              v139 = *v136;
              *v138 = *v136;
              v139 -= 35;
              *v138 = v139;
              v140 = (DAT_00559050[0] ^ v139) - 71;
              *v138 = v140;
              *v131 = v140;
              delete__(v138);
            }
            goto LABEL_210;
          }
          v132 = (v132 + 1) % DAT_055c9bd4;
          if ( ++Ownero >= DAT_055c9bd4 )
          {
            goto LABEL_208;
          }
        }
      }
      else
      {
LABEL_208:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v134 = operator_new(2u);
      *(BYTE *)(v134 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v134, Owner + 770);
LABEL_210:
      Ownerp = *v131;
      v244 = (const char *)(Owner + 770);
      v240 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v240), 4) )
          {
            if ( v240 != -1 )
            {
              v141 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v142 = v141 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v141);
              v143 = v142[1] - 1;
              v142[1] = v143;
              if ( !v143 )
              {
                v144 = (BYTE *)operator_new(1u);
                v145 = *v131 + 71;
                *v144 = v145;
                *v144 = (DAT_00559050[0] ^ v145) + 35;
                *v131 = rand();
                *v142 = *v144;
                delete__(v144);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v240 = (v240 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_214;
          }
        }
      }
      else
      {
LABEL_214:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerp == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        v146 = *(float *)(Owner + 36) + 20.0;
        v147 = *(float *)(Owner + 32);
        v148 = *(float *)(Owner + 16);
        v246[0] = *(float *)(Owner + 28);
        v149 = *(float *)(Owner + 20);
        v246[1] = v147;
        v246[2] = v146;
        v150 = *(float *)(Owner + 24) + 50.0;
        v248[0] = v148;
        v248[1] = v149;
        Light[0] = 0.42000002;
        Light[1] = 0.84000003;
        Light[2] = 1.4;
        v151 = 9;
        v248[2] = v150;
        do
        {
          v246[2] = v246[2] + 40.0;
          CreateEffect(1210, v248, v246, Light, 0, 0, (float *)-1, 0, 0);
          --v151;
        }
        while ( v151 );
      }
      break;
    case 53:
    case 58:
    case 59:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        FUN_00466300(Owner + 16);
      }
      if ( *(BYTE *)(Owner + 757) == 14 && *(BYTE *)(Owner + 747) == 59 )
      {
        v117 = (char *)(Owner + 770);
        v244 = (const char *)(Owner + 770);
        Ownerl = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * Ownerl), 4) )
            {
              if ( Ownerl == -1 )
              {
                break;
              }
              v119 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              if ( v119 == -1 )
              {
                v120 = 0;
              }
              else
              {
                v120 = *(char **)(DAT_055c9bcc + 4 * v119);
              }
              v121 = v120[1] + 1;
              v120[1] = v121;
              if ( v121 < 2u )
              {
                v122 = (char *)operator_new(1u);
                v123 = *v120;
                *v122 = *v120;
                v123 -= 35;
                *v122 = v123;
                v124 = (DAT_00559050[0] ^ v123) - 71;
                *v122 = v124;
                *v117 = v124;
                delete__(v122);
              }
              goto LABEL_181;
            }
            v6 = ++v243 < DAT_055c9bd4;
            Ownerl = (Ownerl + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_179;
            }
          }
        }
        else
        {
LABEL_179:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v118 = operator_new(2u);
        *(BYTE *)(v118 + 1) = 1;
        HashTable_Insert(&MAIN_HASH_CLASS, v118, Owner + 770);
LABEL_181:
        Ownerm = *v117;
        v244 = (const char *)(Owner + 770);
        v239 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v239), 4) )
            {
              if ( v239 != -1 )
              {
                v125 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
                v126 = v125 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v125);
                v127 = v126[1] - 1;
                v126[1] = v127;
                if ( !v127 )
                {
                  Packet_EncryptByte(v126, (BYTE *)(Owner + 770));
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v239 = (v239 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_185;
            }
          }
        }
        else
        {
LABEL_185:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerm == 50 )
        {
          v128 = 0;
          Ownern = 0;
          do
          {
            v129 = *(float *)(Owner + 32);
            v130 = (double)Ownern * 20.0 + *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v129;
            v246[2] = v130;
            CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
            Ownern = ++v128;
          }
          while ( v128 < 18 );
        }
      }
      break;
    case 54:
    case 57:
    case -105:
      if ( *(BYTE *)(Owner + 757) == 1 )
      {
        v113 = *(DWORD *)(Owner + 276);
        v114 = *(unsigned char *)(Owner + 628);
        v248[0] = 60.0;
        v248[1] = -110.0;
        v248[2] = 0.0;
        TransformPosition(v2, (float (*)[4])(v113 + 48 * v114), v248, TargetPosition, 1);
        CreateEffect(223, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        if ( *(BYTE *)(Owner + 747) == 57 )
        {
          v115 = *(float *)(Owner + 28);
          v116 = *(float *)(Owner + 32);
          Angle[2] = *(float *)(Owner + 36) + 20.0;
          Angle[0] = v115;
          Angle[1] = v116;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
          Angle[2] = Angle[2] - 40.0;
          CreateEffect(223, (float *)(Owner + 16), Angle, (float *)(Owner + 232), 0, Owner, -1, 0, 0);
        }
      }
      break;
    case 61:
    case 63:
      v19 = v3 == 63;
      v79 = *(BYTE *)(Owner + 757);
      if ( v19 )
      {
        if ( v79 == 1 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
        }
        v80 = (char *)(Owner + 770);
        v243 = Owner + 770;
        v81 = AE_ht_hash(Owner + 770);
        v245 = 0;
        Ownerg = 0;
        if ( DAT_055c9bd4 )
        {
          while ( 1 )
          {
            v82 = (const char *)(DAT_055c9bd0 + 4 * v81);
            v244 = v82;
            if ( !memcmp((const char *)&v245, v82, 4) )
            {
              break;
            }
            if ( !memcmp((const char *)&v243, v244, 4) )
            {
              if ( v81 == -1 )
              {
                break;
              }
              v84 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              if ( v84 == -1 )
              {
                v85 = 0;
              }
              else
              {
                v85 = *(char **)(DAT_055c9bcc + 4 * v84);
              }
              v86 = v85[1] + 1;
              v85[1] = v86;
              if ( v86 < 2u )
              {
                v87 = (char *)operator_new(1u);
                v88 = *v85;
                *v87 = *v85;
                v88 -= 35;
                *v87 = v88;
                v89 = (DAT_00559050[0] ^ v88) - 71;
                *v87 = v89;
                *v80 = v89;
                delete__(v87);
              }
              goto LABEL_116;
            }
            v81 = (v81 + 1) % DAT_055c9bd4;
            if ( ++Ownerg >= DAT_055c9bd4 )
            {
              goto LABEL_114;
            }
          }
        }
        else
        {
LABEL_114:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        v83 = operator_new(2u);
        *(BYTE *)(v83 + 1) = 1;
        HashTable_Insert(&MAIN_HASH_CLASS, v83, Owner + 770);
LABEL_116:
        Ownerh = *v80;
        v244 = (const char *)(Owner + 770);
        v237 = AE_ht_hash(Owner + 770);
        v245 = 0;
        v243 = 0;
        if ( DAT_055c9bd4 )
        {
          while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
          {
            if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v237), 4) )
            {
              if ( v237 != -1 )
              {
                v90 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
                v91 = v90 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v90);
                v92 = v91[1] - 1;
                v91[1] = v92;
                if ( !v92 )
                {
                  v93 = (BYTE *)operator_new(1u);
                  v94 = *v80 + 71;
                  *v93 = v94;
                  *v93 = (DAT_00559050[0] ^ v94) + 35;
                  *v80 = rand();
                  *v91 = *v93;
                  delete__(v93);
                }
              }
              break;
            }
            v6 = ++v243 < DAT_055c9bd4;
            v237 = (v237 + 1) % DAT_055c9bd4;
            if ( !v6 )
            {
              goto LABEL_120;
            }
          }
        }
        else
        {
LABEL_120:
          CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
        }
        if ( Ownerh == 50 )
        {
          if ( *(BYTE *)(Owner + 747) == 63 )
          {
            TargetPosition[0] = (double)(rand() % 800) + *(float *)(Owner + 16) - 400.0;
            Ownery = rand() % 800;
            TargetPosition[2] = *(float *)(Owner + 24);
            TargetPosition[1] = (double)Ownery + *(float *)(Owner + 20) - 400.0;
            CreateEffect(240, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v95 = 0;
            Owneri = 0;
            do
            {
              v96 = *(float *)(Owner + 32);
              v97 = (double)Owneri * 20.0 + *(float *)(Owner + 36);
              v246[0] = *(float *)(Owner + 28);
              v246[1] = v96;
              v246[2] = v97;
              CreateEffect(568, (float *)(Owner + 16), v246, (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
              Owneri = ++v95;
            }
            while ( v95 < 18 );
          }
        }
      }
      else if ( v79 == 1 )
      {
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 66:
      v98 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v99 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerj = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v100 = (const char *)(DAT_055c9bd0 + 4 * v99);
          v244 = v100;
          if ( !memcmp((const char *)&v245, v100, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v99 == -1 )
            {
              break;
            }
            v102 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v102 == -1 )
            {
              v103 = 0;
            }
            else
            {
              v103 = *(char **)(DAT_055c9bcc + 4 * v102);
            }
            v104 = v103[1] + 1;
            v103[1] = v104;
            if ( v104 < 2u )
            {
              v105 = (char *)operator_new(1u);
              v106 = *v103;
              *v105 = *v103;
              v106 -= 35;
              *v105 = v106;
              v107 = (DAT_00559050[0] ^ v106) - 71;
              *v105 = v107;
              *v98 = v107;
              delete__(v105);
            }
            goto LABEL_148;
          }
          v99 = (v99 + 1) % DAT_055c9bd4;
          if ( ++Ownerj >= DAT_055c9bd4 )
          {
            goto LABEL_146;
          }
        }
      }
      else
      {
LABEL_146:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v101 = operator_new(2u);
      *(BYTE *)(v101 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v101, Owner + 770);
LABEL_148:
      Ownerk = *v98;
      v244 = (const char *)(Owner + 770);
      v238 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v238), 4) )
          {
            if ( v238 != -1 )
            {
              v108 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v109 = v108 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v108);
              v110 = v109[1] - 1;
              v109[1] = v110;
              if ( !v110 )
              {
                v111 = (BYTE *)operator_new(1u);
                v112 = *v98 + 71;
                *v111 = v112;
                *v111 = (DAT_00559050[0] ^ v112) + 35;
                *v98 = rand();
                *v109 = *v111;
                delete__(v111);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v238 = (v238 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_152;
          }
        }
      }
      else
      {
LABEL_152:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerk == 50 && *(BYTE *)(Owner + 757) == 1 )
      {
        goto LABEL_30;
      }
      break;
    case 70:
      if ( *(BYTE *)(Owner + 757) == 5 && CharactersClient )  // guard de port
      {
        v22 = 20;
        v23 = CharactersClient + 916 * *(short *)(Owner + 784);
        v24 = (float *)(v23 + 28);
        v25 = (float *)(v23 + 16);
        do
        {
          CreateEffect(1271, v25, v24, Light, 0, 0, (float *)-1, 0, 0);
          --v22;
        }
        while ( v22 );
      }
      break;
    case 71:
    case 74:
      v26 = *(BYTE *)(Owner + 261);
      if ( (v26 == 3 || v26 == 4) && *(BYTE *)(Owner + 757) == 5 )
      {
        FUN_00466300(Owner + 16);
        CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
      }
      break;
    case 72:
      v27 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v28 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownerc = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v29 = (const char *)(DAT_055c9bd0 + 4 * v28);
          v244 = v29;
          if ( !memcmp((const char *)&v245, v29, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v28 == -1 )
            {
              break;
            }
            v31 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v31 == -1 )
            {
              v32 = 0;
            }
            else
            {
              v32 = *(char **)(DAT_055c9bcc + 4 * v31);
            }
            v33 = v32[1] + 1;
            v32[1] = v33;
            if ( v33 < 2u )
            {
              v34 = (char *)operator_new(1u);
              v35 = *v32;
              *v34 = *v32;
              v35 -= 35;
              *v34 = v35;
              v36 = (DAT_00559050[0] ^ v35) - 71;
              *v34 = v36;
              *v27 = v36;
              delete__(v34);
            }
            goto LABEL_46;
          }
          v28 = (v28 + 1) % DAT_055c9bd4;
          if ( ++Ownerc >= DAT_055c9bd4 )
          {
            goto LABEL_44;
          }
        }
      }
      else
      {
LABEL_44:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v30 = operator_new(2u);
      *(BYTE *)(v30 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v30, Owner + 770);
LABEL_46:
      Ownerd = *v27;
      v244 = (const char *)(Owner + 770);
      v235 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v235), 4) )
          {
            if ( v235 != -1 )
            {
              v37 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v38 = v37 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v37);
              v39 = v38[1] - 1;
              v38[1] = v39;
              if ( !v39 )
              {
                v40 = (BYTE *)operator_new(1u);
                v41 = *v27 + 71;
                *v40 = v41;
                *v40 = (DAT_00559050[0] ^ v41) + 35;
                *v27 = rand();
                *v38 = *v40;
                delete__(v40);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v235 = (v235 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_50;
          }
        }
      }
      else
      {
LABEL_50:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerd == 50 && *(BYTE *)(Owner + 757) == 14 )
      {
        memset(Angle, 0, sizeof(Angle));
        v42 = 36;
        do
        {
          Angle[0] = (float)(rand() % 360);
          Angle[1] = (float)(rand() % 360);
          v43 = rand();
          Position[1] = *(float *)(Owner + 20);
          Position[0] = *(float *)(Owner + 16);
          Angle[2] = (float)(v43 % 360);
          Position[2] = *(float *)(Owner + 24) + 100.0;
          CreateJoint(1253, Position, Position, Angle, 1, 0, 60.0, 0, 0);
          --v42;
        }
        while ( v42 );
      }
      break;
    case 73:
    case 75:
      v44 = *(BYTE *)(Owner + 757);
      if ( *(BYTE *)(Owner + 261) == 3 )
      {
        if ( v44 == 11 )
        {
          FUN_00466300(Owner + 16);
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          v45 = 5;
          do
          {
            v46 = *(float *)(Owner + 36);
            v47 = *(float *)(Owner + 28) + 45.0;
            v48 = *(float *)(Owner + 16);
            v246[1] = *(float *)(Owner + 32);
            v49 = *(float *)(Owner + 20);
            v246[2] = v46;
            v50 = *(float *)(Owner + 24);
            Light[0] = 1.0;
            v246[0] = v47;
            Light[1] = 0.5;
            Light[2] = 0.0;
            TargetPosition[0] = v48;
            TargetPosition[1] = v49;
            TargetPosition[2] = v50;
            TargetPosition[0] = (double)(rand() % 1001 - 500) + v48;
            v51 = rand();
            *(float *)(Owner + 368) = TargetPosition[0];
            TargetPosition[1] = (double)(v51 % 1001 - 500) + TargetPosition[1];
            TargetPosition[2] = TargetPosition[2] + 500.0;
            v52 = TargetPosition[2];
            *(float *)(Owner + 372) = TargetPosition[1];
            *(float *)(Owner + 376) = v52;
            CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
            --v45;
          }
          while ( v45 );
        }
      }
      else
      {
        if ( v44 == 13 )
        {
          v53 = *(float *)(Owner + 32);
          v54 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v55 = *(DWORD *)(Owner + 276);
          v246[1] = v53;
          v246[0] = v54;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          v248[0] = -50.0;
          v248[1] = 100.0;
          v248[2] = 0.0;
          TransformPosition(v2, (float (*)[4])(v55 + 528), v248, TargetPosition, 1);
          v56 = TargetPosition[1];
          v57 = TargetPosition[2];
          *(float *)(Owner + 368) = TargetPosition[0];
          *(float *)(Owner + 372) = v56;
          *(float *)(Owner + 376) = v57;
          CreateEffect(256, TargetPosition, v246, Light, 1, Owner, -1, 0, 0);
          goto LABEL_75;
        }
        if ( v44 == 9 )
        {
          v58 = *(float *)(Owner + 32);
          v59 = *(float *)(Owner + 28) + 45.0;
          v246[2] = *(float *)(Owner + 36);
          v60 = *(DWORD *)(Owner + 276);
          v246[1] = v58;
          v246[0] = v59;
          Light[0] = 1.0;
          Light[1] = 0.5;
          Light[2] = 0.0;
          memset(v248, 0, sizeof(v248));
          TransformPosition(v2, (float (*)[4])(v60 + 528), v248, TargetPosition, 1);
        }
      }
      break;
    case 77:
      v61 = (char *)(Owner + 770);
      v243 = Owner + 770;
      v62 = AE_ht_hash(Owner + 770);
      v245 = 0;
      Ownere = 0;
      if ( DAT_055c9bd4 )
      {
        while ( 1 )
        {
          v63 = (const char *)(DAT_055c9bd0 + 4 * v62);
          v244 = v63;
          if ( !memcmp((const char *)&v245, v63, 4) )
          {
            break;
          }
          if ( !memcmp((const char *)&v243, v244, 4) )
          {
            if ( v62 == -1 )
            {
              break;
            }
            v65 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v65 == -1 )
            {
              v66 = 0;
            }
            else
            {
              v66 = *(char **)(DAT_055c9bcc + 4 * v65);
            }
            v67 = v66[1] + 1;
            v66[1] = v67;
            if ( v67 < 2u )
            {
              v68 = (char *)operator_new(1u);
              v69 = *v66;
              *v68 = *v66;
              v69 -= 35;
              *v68 = v69;
              v70 = (DAT_00559050[0] ^ v69) - 71;
              *v68 = v70;
              *v61 = v70;
              delete__(v68);
            }
            goto LABEL_84;
          }
          v62 = (v62 + 1) % DAT_055c9bd4;
          if ( ++Ownere >= DAT_055c9bd4 )
          {
            goto LABEL_82;
          }
        }
      }
      else
      {
LABEL_82:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v64 = operator_new(2u);
      *(BYTE *)(v64 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v64, Owner + 770);
LABEL_84:
      Ownerf = *v61;
      v244 = (const char *)(Owner + 770);
      v236 = AE_ht_hash(Owner + 770);
      v245 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v236), 4) )
          {
            if ( v236 != -1 )
            {
              v71 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v72 = v71 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v71);
              v73 = v72[1] - 1;
              v72[1] = v73;
              if ( !v73 )
              {
                v74 = (BYTE *)operator_new(1u);
                v75 = *v61 + 71;
                *v74 = v75;
                *v74 = (DAT_00559050[0] ^ v75) + 35;
                *v61 = rand();
                *v72 = *v74;
                delete__(v74);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v236 = (v236 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_88;
          }
        }
      }
      else
      {
LABEL_88:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerf == 50 )
      {
        v76 = *(BYTE *)(Owner + 757);
        if ( v76 == 2 || v76 == 6 )
        {
          memset(Angle, 0, sizeof(Angle));
          v77 = 40;
          do
          {
            Angle[0] = (float)(rand() % 360);
            Angle[1] = (float)(rand() % 360);
            v78 = rand();
            Position[1] = *(float *)(Owner + 20);
            Position[0] = *(float *)(Owner + 16);
            Angle[2] = (float)(v78 % 360);
            Position[2] = *(float *)(Owner + 24) + 100.0;
            CreateJoint(1253, Position, Position, Angle, 3, 0, 50.0, 0, 0);
            --v77;
          }
          while ( v77 );
        }
      }
      break;
    case 89:
    case 95:
    case 112:
    case 118:
    case 124:
    case -126:
    case -120:
      v4 = (char *)(Owner + 770);
      v244 = (const char *)(Owner + 770);
      v5 = AE_ht_hash(Owner + 770);
      v243 = 0;
      Ownera = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v243, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
        {
          if ( !memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v5), 4) )
          {
            if ( v5 == -1 )
            {
              break;
            }
            v8 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
            if ( v8 == -1 )
            {
              v9 = 0;
            }
            else
            {
              v9 = *(char **)(DAT_055c9bcc + 4 * v8);
            }
            v10 = v9[1] + 1;
            v9[1] = v10;
            if ( v10 < 2u )
            {
              v11 = (char *)operator_new(1u);
              v12 = *v9;
              *v11 = *v9;
              v12 -= 35;
              *v11 = v12;
              v13 = (DAT_00559050[0] ^ v12) - 71;
              *v11 = v13;
              *v4 = v13;
              delete__(v11);
            }
            goto LABEL_8;
          }
          v6 = ++Ownera < DAT_055c9bd4;
          v5 = (v5 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_6;
          }
        }
      }
      else
      {
LABEL_6:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      v7 = operator_new(2u);
      *(BYTE *)(v7 + 1) = 1;
      HashTable_Insert(&MAIN_HASH_CLASS, v7, Owner + 770);
LABEL_8:
      Ownerb = *v4;
      v245 = Owner + 770;
      v234 = AE_ht_hash(Owner + 770);
      v244 = 0;
      v243 = 0;
      if ( DAT_055c9bd4 )
      {
        while ( memcmp((const char *)&v244, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
        {
          if ( !memcmp((const char *)&v245, (const char *)(DAT_055c9bd0 + 4 * v234), 4) )
          {
            if ( v234 != -1 )
            {
              v14 = FUN_004041e0(&MAIN_HASH_CLASS, Owner + 770);
              v15 = v14 == -1 ? 0 : *(BYTE **)(DAT_055c9bcc + 4 * v14);
              v16 = v15[1] - 1;
              v15[1] = v16;
              if ( !v16 )
              {
                v17 = (BYTE *)operator_new(1u);
                v18 = *v4 + 71;
                *v17 = v18;
                *v17 = (DAT_00559050[0] ^ v18) + 35;
                *v4 = rand();
                *v15 = *v17;
                delete__(v17);
              }
            }
            break;
          }
          v6 = ++v243 < DAT_055c9bd4;
          v234 = (v234 + 1) % DAT_055c9bd4;
          if ( !v6 )
          {
            goto LABEL_12;
          }
        }
      }
      else
      {
LABEL_12:
        CErrorReport::Write((DWORD)&g_ErrorReport, aHashTableFullG);
      }
      if ( Ownerb == 50 )
      {
        v20 = rand() & 0x80000001;
        v19 = v20 == 0;
        if ( v20 < 0 )
        {
          v19 = (((BYTE)v20 - 1) | 0xFFFFFFFE) == -1;
        }
        if ( !v19 )
        {
LABEL_31:
          TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
          v21 = (double)(rand() % 1024) + *(float *)(Owner + 20);
          TargetPosition[2] = *(float *)(Owner + 24);
          TargetPosition[1] = v21 - 512.0;
          CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
          goto LABEL_75;
        }
        if ( *(BYTE *)(Owner + 757) == 1 )
        {
LABEL_30:
          CreateEffect(241, (float *)(Owner + 16), (float *)(Owner + 28), (float *)(Owner + 232), 1, 0, -1, 0, 0);
        }
      }
      break;
    case 103:
      if ( FUN_0045fae0(&MAIN_HASH_CLASS, (BYTE *)(Owner + 770)) == 50 )
      {
        TargetPosition[0] = (double)(rand() % 1024) + *(float *)(Owner + 16) - 512.0;
        v198 = (double)(rand() % 1024) + *(float *)(Owner + 20);
        TargetPosition[2] = *(float *)(Owner + 24);
        TargetPosition[1] = v198 - 512.0;
        CreateEffect(191, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 0, (float *)-1, 0, 0);
LABEL_75:
        PlayBuffer(46, 0, 0);
      }
      break;
    default:
      break;
  }
  v202 = *(WORD *)(Owner + 784);
  // `&& CharactersClient` no esta en IDA: en nuestro build el pool arranca en 0
  // hasta que se aloca, y `0 + 916*idx` seria un puntero basura. Mismo guard que
  // Render_Frame.cpp:395.
  if ( v202 >= 0 && v202 < 400 && CharactersClient )
  {
    v203 = CharactersClient + 916 * v202;
    if ( FUN_0045fae0(&MAIN_HASH_CLASS, (BYTE *)(Owner + 770)) == 17 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( i = 0; i < 4; ++i )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (i >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x2E:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( j = 0; j < 4; ++j )
          {
            memset(v246, 0, sizeof(v246));
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (j >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            CreateJoint(1166, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 10.0, -1, 0);
          }
          break;
        case 0x3D:
          for ( k = 0; k < 6; ++k )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (k >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( m = 0; m < 4; ++m )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (m >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x42:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v219 = 0;
          Ownerw = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v219 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerw;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v219;
            Ownerw = Ownerw + 270.0;
          }
          while ( v219 < 4 );
          break;
        case 0x45:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            // DESVIACION DOCUMENTADA (bug del binario original).
            // Este case (Alquamos, MonsterID 69) usa `TargetPosition` y `v246`
            // SIN inicializarlos: son locales del frame que en IDA arrastran lo
            // que dejo un case anterior.  En nuestro build el CRT de Debug los
            // llena con 0xCCCCCCCC (= -1.07e8 como float), asi que la mitad de
            // los joints nacia en una coordenada absurda y sus lineas salian
            // disparadas al lado contrario del personaje.
            // Se anclan al propio monstruo, que es lo unico coherente con el
            // resto del case: `v213` ya es Owner+16 y el segundo CreateJoint
            // usa el mismo SubType 7, cuyo re-anclado de LABEL_182 deja la
            // Position pegada a la TargetPosition.
            TargetPosition[0] = *(float *)(Owner + 16);
            TargetPosition[1] = *(float *)(Owner + 20);
            TargetPosition[2] = *(float *)(Owner + 24);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = *(float *)(Owner + 32);
            v246[2] = *(float *)(Owner + 36);
            v213 = (float *)(Owner + 16);
            v214 = 4;
            do
            {
              CreateJoint(1249, v213, v213, v246, 7, v203, 50.0, -1, 0);
              CreateJoint(1249, TargetPosition, TargetPosition, v246, 7, v203, 50.0, -1, 0);
              --v214;
            }
            while ( v214 );
          }
          break;
        case 0x49:
        case 0x4B:
          if ( *(BYTE *)(Owner + 261) == 4 && *(BYTE *)(Owner + 757) == 13 )
          {
            v210 = *(float *)(Owner + 32);
            v211 = *(float *)(Owner + 28) + 45.0;
            v246[2] = *(float *)(Owner + 36);
            v212 = *(DWORD *)(Owner + 276);
            v246[1] = v210;
            v246[0] = v211;
            Light[0] = 1.0;
            Light[1] = 0.5;
            Light[2] = 0.0;
            v248[0] = -50.0;
            v248[1] = 100.0;
            v248[2] = 0.0;
            TransformPosition(This, (float (*)[4])(v212 + 528), v248, TargetPosition, 1);
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])Matrix, v248, TargetPosition, 1);
            v208 = *(float *)(Owner + 32);
            v209 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v208;
            v246[2] = v209;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        case 0x57:
        case 0x5D:
        case 0x63:
        case 0x74:
        case 0x7A:
        case 0x80:
        case 0x86:
          if ( *(BYTE *)(Owner + 757) == 13 )
          {
            v232 = (float (*)[4])(*(DWORD *)(Owner + 276) + 288);
            Light[0] = 1.0;
            Light[1] = 1.0;
            Light[2] = 1.0;
            v248[0] = 60.0;
            v248[1] = 30.0;
            v248[2] = 0.0;
            TransformPosition(This, v232, v248, TargetPosition, 1);
            v206 = *(float *)(Owner + 32);
            v207 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v206;
            v246[2] = v207;
            CreateEffect(191, TargetPosition, v246, (float *)(Owner + 232), 5, 0, -1, 0, 0);
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 14 )
          {
            v231 = (float (*)[4])(*(DWORD *)(Owner + 276) + 1584);
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, v231, v248, TargetPosition, 1);
            v204 = *(float *)(Owner + 32);
            v205 = *(float *)(Owner + 36);
            v246[0] = *(float *)(Owner + 28);
            v246[1] = v204;
            v246[2] = v205;
            CreateEffect(256, TargetPosition, v246, Light, 1, 0, -1, 0, 0);
            CreateJoint(1254, TargetPosition, TargetPosition, v246, 2, v203, 50.0, -1, 0);
          }
          break;
        default:
          return;
      }
    }
    else if ( FUN_0045fae0(&MAIN_HASH_CLASS, (BYTE *)(Owner + 770)) == 3 )
    {
      switch ( *(BYTE *)(Owner + 747) )
      {
        case 0x22:
          for ( n = 0; n < 4; ++n )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (n >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
            Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          }
          break;
        case 0x25:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(87, 0, 0);
          }
          for ( ii = 0; ii < 4; ++ii )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (ii >= 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
          }
          break;
        case 0x27:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v225 = *(DWORD *)(Owner + 276);
          v226 = *(unsigned char *)(Owner + 628);
          v248[2] = 0.0;
          TransformPosition(This, (float (*)[4])(v225 + 48 * v226), v248, TargetPosition, 1);
          v227 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v227;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
        case 0x30:
          for ( jj = 0; jj < 6; ++jj )
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (jj >= 3) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = (float)(rand() % 360);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 50.0, -1, 0);
            CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 2, v203, 10.0, -1, 0);
          }
          break;
        case 0x4D:
          if ( *(BYTE *)(Owner + 757) >= 8u )
          {
            memset(v248, 0, sizeof(v248));
            TransformPosition(This, (float (*)[4])DAT_07abf3e4, v248, TargetPosition, 1);
            v221 = 4;
            do
            {
              v246[0] = 0.0;
              v246[1] = 0.0;
              v246[2] = (float)(rand() % 360);
              CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 80.0, -1, 0);
              --v221;
            }
            while ( v221 );
          }
          break;
        case 0x59:
        case 0x5F:
        case 0x70:
        case 0x76:
        case 0x7C:
        case 0x82:
        case 0x88:
          if ( *(BYTE *)(Owner + 757) == 1 )
          {
            PlayBuffer(60, 0, 0);
          }
          v220 = 0;
          Ownerx = 45.0 - (double)((int)((__int64)WorldTime / 10 + 3 * *(unsigned char *)(Owner + 757)) % 90) + 180.0;
          do
          {
            TransformPosition(
              This,
              (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 24 * (v220 % 2) + 628)),
              v248,
              TargetPosition,
              1);
            v246[0] = 0.0;
            v246[1] = 0.0;
            v246[2] = Ownerx;
            CreateJoint(1261, TargetPosition, (float *)(v203 + 16), v246, 1, v203, 50.0, -1, 0);
            Particle_Spawn(1195, TargetPosition, (float *)(Owner + 28), (float *)(Owner + 232), 0, 1.0, 0);
            ++v220;
            Ownerx = Ownerx + 270.0;
          }
          while ( v220 < 4 );
          break;
        default:
          v19 = *(WORD *)(Owner + 2) == 390;
          v248[0] = 0.0;
          v248[1] = 0.0;
          if ( !v19 )
          {
            v248[1] = -130.0;
          }
          v228 = (float (*)[4])(*(DWORD *)(Owner + 276) + 48 * *(unsigned char *)(Owner + 628));
          v248[2] = 0.0;
          TransformPosition(This, v228, v248, TargetPosition, 1);
          v229 = *(float *)(Owner + 36);
          v246[0] = -60.0;
          v246[1] = 0.0;
          v246[2] = v229;
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 50.0, -1, 0);
          CreateJoint(1254, TargetPosition, (float *)(v203 + 16), v246, 0, v203, 10.0, -1, 0);
          Particle_Spawn(1180, TargetPosition, (float *)(Owner + 28), Light, 0, 1.0, 0);
          break;
      }
    }
  }
}

// Fin de los shims de AttackEffect — restaurar el estado global de macros.
#undef Models
#undef Matrix
#undef operator_new
#undef delete__
#undef FUN_004041e0
#undef HashTable_Insert
#undef Packet_EncryptByte
#undef PACKET_ENCRYPT
#undef FUN_0045fae0
#undef FUN_00466300
#undef CreateJoint
#undef TransformPosition
#undef PlayBuffer
#undef CreateEffect
#define CreateEffect CreateEffect
