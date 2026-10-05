# Historial de comentarios: `src/Render/` (parte 2: archivos I–Z y Texture/)

Comentarios de desarrollo movidos desde `src/Render/` (archivos cuyo nombre empieza
con I–Z, más `src/Render/Texture/`) según el criterio de [README.md](README.md).
La parte 1 (archivos A–H) está en su propio historial. El texto está copiado tal
cual; la línea indica dónde estaba en `fase/1` antes de esta limpieza.

## `src/Render/ItemDrop_Render2.cpp`

### Línea 42 en `RenderParticles` — antes de `uint  *puVar8  = (uint*)((char*)DAT_07abf5f0 + 0x44);  // +0x44 from slot 0 base`

```cpp
    // Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODO el render del particle
    // pool DAT_07abf5f0 — particles spawneadas via CreateParticle (Particle_Spawn)
    // (lightning ELS=10/11, waterfall +9 glow, fire/smoke, rain/snow) NUNCA se
    // dibujaban. Ahora itera por índice acotado a 3000 slots.
```

### Línea 54 en `RenderParticles` — antes de `uint entityType = puVar8[-0x10];`

```cpp
            // BUG-FIX 2026-04-28: bounds-check entity_type. La tabla en
            // DAT_083a7cc0 tiene 0x600 entradas (stride 0x38). Tipos fuera
            // de rango leen memoria inválida → AV. Causa: el pool se sembraba
            // con tipos garbage (ej. bits de un float) cuando un particle se
            // marcaba activo pero no se inicializaba bien.
```

### Línea 63 en `RenderParticles` — antes de `float fVar13 = *(float *)((char *)&DAT_083a7cc0 + iVar2) * *(float *)(puVar8 - 0xe);`

```cpp
            // BUG-FIX (sistema de partículas): el campo scale (slot+0x0c) es un FLOAT.
            // IDA: Width = Bitmaps[type].Width * *((float*)v1 - 14). El port hacía
            // `(float)puVar8[-0xe]` = conversión int→float de los BITS del float →
            // p.ej. 0.5f (bits 0x3F000000 = 1056964608) se convertía en 1e9 → billboard
            // gigante → whiteout. Reinterpretamos los bits como el binario original.
```

### Línea 75 en `RenderParticles` — antes de `GL_SetBlendSrcOver('\0');      // EnableAlphaTest(0)`

```cpp
                // ── 2026-08-16: CAUSA DEL CUADRO BLANCO ───────────────────────
                // IDA 00478C00 L66-72:
                //     if (Bitmaps[v2].Components == 3) EnableAlphaBlend();
                //     else                             EnableAlphaTest(0);
                // `EnableAlphaTest` es **0x00511680**. El comentario anterior
                // afirmaba que estaba "mapped at 00511590" y es FALSO: 0x511590
                // es `DisableTexture(bool)`, que hace glDisable(GL_TEXTURE_2D).
                // O sea TODA particula con Components != 3 apagaba el
                // texturizado y su quad salia pintado con el color plano
                // (blanco). Como las particulas se dibujan de a cientos
                // (parts llego a 439 en el log) y el estado GL queda pegado,
                // se veia una masa blanca de bordes escalonados que ademas
                // contaminaba lo que se dibujara despues.
                // Mismo error que ya estaba en SkillEffect_Render; es la 4ta
                // vez que esta familia muerde (ver la tabla en CLAUDE.md).
```

## `src/Render/Joint_Create.cpp`

### Línea 44 — antes de `alignas(8) unsigned char __fr[0x98] = {0};`

```cpp
  // 2026-08-10 FIX (haces oscuros de las alas) — tercera instancia del patrón
  // "locales que Ghidra separó y el código asume contiguos". Acá el código hace:
  //     local_90 = Scale * -0.5f;  local_8c = 0;  local_88 = 0;
  //     Vector_Rotate(&local_90, local_6c, &local_78);   // in y out son vec3
  //     *(float*)(slot+0x58) = local_78 + Position.x;   // out[0]
  //     *(float*)(slot+0x5c) = local_74 + Position.y;   // out[1]
  //     *(float*)(slot+0x60) = local_70 + Position.z;   // out[2]
  // o sea depende de que {local_90,local_8c,local_88} y {local_78,local_74,
  // local_70} sean tríos contiguos del frame. Con locales sueltos sólo out[0]
  // caía donde el código lo lee → la esquina X del segmento salía bien y la Y/Z
  // quedaban con basura, que es justo lo que mostró el probe JOINTDBG
  // (v0=(13480.6, 1.08e9, 3.86e10)).
  // Varios de estos slots son dual-use (float o byte*), así que se respalda todo
  // con un bloque de bytes y los nombres quedan como referencias tipadas al
  // offset correcto: ebp-0x98 → __fr[0x00] … ebp-0x3C → __fr[0x5C], total 0x98.
```

### Línea 73 — antes de `float &local_94f = *(float *)(__fr + 0x04);`

```cpp
  // 2026-08-10 — vistas FLOAT de los slots que Ghidra tipó como `byte*`.
  // Ghidra los llamó punteros, pero cuando forman el vec3 de Vector_Rotate
  // guardan FLOATS. El port los leía con `local_8cf`, que
  // CONVIERTE el valor entero en vez de reinterpretar los bits: un 2.16
  // (bits 0x400B0000 = 1074413568) salía como 1074413568.0 ≈ 1.07e9 — la
  // causa exacta de los haces (x usaba local_90, que sí era float, y salía
  // sana; y/z pasaban por esta conversión y explotaban).
```

### Línea 106 — antes de `*(int *)(pcVar14 + 0x9c8) = 0;`

```cpp
  // 2026-09-04 -- DESVIACION DOCUMENTADA (no esta en IDA 0x46D840).
  // +0x9C8 y +0x9CC son el avance por tick de la TargetPosition que aplica
  // LABEL_439 de MoveJoint (`TargetPos.x += o+2504`, `TargetPos.y += o+2508`),
  // o sea la velocidad del joint.  El binario limpia +0x9C0 aca pero NO estos
  // dos, asi que un subtipo que no los escriba en su propio case hereda los
  // del joint anterior que ocupo el slot.  El 1249/sub7 -- los disparos del
  // ataque de Alquamos (AttackEffect case 0x45) -- es justamente uno de esos:
  // su case en CreateJoint fija vida, escala, segMax y fase, y nada mas.  Si
  // el slot venia de un 1249/sub14, que pone +0x9C8 en `rand()%500 - 250`, el
  // disparo arranca con hasta 250 unidades de deriva por tick y se va de lado.
  // Limpiarlos deja a cada joint con la velocidad que define su propio case.
```

### Línea 2055 — antes de `const int __rowBytes = 0x30;`

```cpp
    // 2026-09-03 -- DESVIACION DOCUMENTADA (no esta en IDA 0x46D840).
    //
    // El binario inicializa UNICAMENTE la fila 0 del anillo de segmentos
    // (`v11+22..v11+33` = 0x58..0x87, los 4 vertices) y deja el resto como
    // estaba.  En el juego original eso no se nota porque el pool de joints se
    // recicla sin parar: las filas altas conservan las coordenadas del joint
    // anterior, que son valores de mundo plausibles, asi que los quads de mas
    // salen diminutos o degenerados.
    //
    // En nuestro build el pool es un global en BSS: la PRIMERA vez que se usa
    // un slot esas filas valen 0, y el renderer (0x00473710) dibuja un quad
    // entre la ultima fila con datos y una fila en el origen -- la banda que
    // cruza la pantalla desde el personaje.  La muestran justo los tres joints
    // reportados: aura del Soul Barrier (266), efecto de subir de nivel y halo
    // del set +11 (1249).  Medido con la sonda JROWS: el vertice lejano salia
    // en (1046, 0, 295) con el cercano en (1112, 1489, 381).  Y verificado por
    // contraste: cuando el slot venia RECICLADO (con datos del joint anterior)
    // el anillo se comportaba perfecto durante 200 muestras seguidas.
    //
    // Replicar la fila 0 en todo el anillo reproduce la condicion que el
    // original obtiene gratis por reciclaje: los quads sobrantes quedan
    // degenerados sobre la propia posicion del joint en vez de barrer el mapa.
    // 2026-09-12: arranca DESPUES de las filas que la creacion ya construyo
    // (0..segCount).  Antes empezaba en la 1 y pisaba los segmentos armados por
    // los bucles de creacion (1254 sub 14 / 1253 sub 4 de las alas del MG, y
    // cualquier subtipo que llame a sub_46FE90 dentro de CreateJoint): la estela
    // quedaba colapsada en un punto (sonda JOINTWING, largo 0.0) y la luz del
    // ala no recorria las plumas.
```

### Línea 2338 — antes de `*(float *)(pcVar14 + 0xc) = param_7;`

```cpp
    // 2026-09-02: Ghidra tipo este slot como `float**` y le asigno `param_4`,
    // que es el PUNTERO al vec3 de angulos -- una direccion de pila.  El campo
    // es la **Scale** del joint (+0x0C).  Confirmado con MU 5.2 CreateJoint,
    // case 0 de BITMAP_JOINT_SPIRIT:  Velocity = 70; LifeTime = 49;
    // Scale; MaxTails = 6  -- los otros tres valores de este mismo
    // bloque coinciden exacto.  Medido con la sonda ESPIRIT JOINT:
    // `scaleBits=001AF32C` (una direccion de stack) en vez de 42A00000 (80.0f).
```

### Línea 2595 — antes de `*(float *)(pcVar14 + 0xc) = param_7;`

```cpp
    // 2026-09-02: Ghidra tipo este slot como `float**` y le asigno `param_4`,
    // que es el PUNTERO al vec3 de angulos -- una direccion de pila.  El campo
    // es la **Scale** del joint (+0x0C).  Confirmado con MU 5.2 CreateJoint,
    // case 0 de BITMAP_JOINT_SPIRIT:  Velocity = 70; LifeTime = 49;
    // Scale; MaxTails = 6  -- los otros tres valores de este mismo
    // bloque coinciden exacto.  Medido con la sonda ESPIRIT JOINT:
    // `scaleBits=001AF32C` (una direccion de stack) en vez de 42A00000 (80.0f).
```

### Línea 2610 — antes de `void* __cdecl CreateJoint(int type, float* p1, float* p2, float* p3, unsigned int subType,`

```cpp
// IDA compatibility bridge: stubs_IDA_ports.cpp intentionally retains this ABI name.
```

## `src/Render/Joint_Render.cpp`

### Línea 44 — antes de `void Trail_RenderAll(void)`

```cpp
// 2026-05-03: AUTO-SKIP removed. Pool now properly sized in globals.cpp
// (g_RenderPool_07c608a8 = 100 slots × 0x2f0). DAT_07c608b4 is the +12
// anchor inside slot[0]. Walk replaced with explicit count.
// IDA: Trail_RenderAll (0x0046C3E0)
```

### Línea 81 en `Trail_RenderAll` — antes de `glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2`

```cpp
            // BUG-FIX 2026-07-15: IDA Trail_RenderAll lee el color como FLOAT
            // (`*((float*)v0+2)`). Leerlo como `(float)piVar2[N]` (cast int de
            // los bits float, ej. 0.1f=0x3DCCCCCD → 1.03e9) hacía glColor clampear
            // a 1.0 → los beams/crackles salían a brillo MÁXIMO dorado en vez del
            // color tenue → haz dorado brillante sobre la espada del +11 set.
```

### Línea 88 en `Trail_RenderAll`

```cpp
// BUG-FIX: era 0x3f800000 (int=1e9), IDA usa 1.0
```

### Línea 96 en `Trail_RenderAll` — antes de `glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2`

```cpp
            // BUG-FIX 2026-07-15: IDA Trail_RenderAll lee el color como FLOAT
            // (`*((float*)v0+2)`). Leerlo como `(float)piVar2[N]` (cast int de
            // los bits float, ej. 0.1f=0x3DCCCCCD → 1.03e9) hacía glColor clampear
            // a 1.0 → los beams/crackles salían a brillo MÁXIMO dorado en vez del
            // color tenue → haz dorado brillante sobre la espada del +11 set.
```

### Línea 106 en `Trail_RenderAll`

```cpp
// BUG-FIX: era 0x3f800000 (int=1e9), IDA usa 1.0
```

## `src/Render/MoveEffect.cpp`

### Línea 191 en `MoveEffect` — antes de `alignas(16) unsigned char __mfr[0x374];`

```cpp
  // 2026-08-23 FIX [[locales-contiguos-ghidra]]: el codigo pasa `&local_XXX` a
  // funciones que leen 3 floats consecutivos (Position, Light, matrices), o sea
  // depende de que los locales queden contiguos y en el orden del frame
  // original.  MSVC no lo garantiza.  Se reconstruye el frame como UN bloque y
  // cada `local_XXX` es una referencia a su offset real (idx = 0x36c - offset),
  // asi las direcciones relativas son las del binario.
```

### Línea 199 en `MoveEffect` — antes de `int __owner_fVar13 = 0;`

```cpp
  // 2026-08-23: `param_1[0x3f]` es el PUNTERO al owner del efecto.  El port lo
  // leia como float y despues hacia `(int)`, que convierte NUMERICAMENTE: los
  // bits de una direccion dan ~1e-27 y `(int)` de eso es 0, o sea se
  // deferenciaba NULL+offset.  IDA lo lee como DWORD:
  // `*((_DWORD *)param_1 + 63)`.  Son los efectos que siguen a su entidad duena.
```

### Línea 369 en `MoveEffect` — antes de `const int __cnt = *(int*)&param_1[0x18];`

```cpp
    // 2026-08-15: el counter (+96) es un DWORD. IDA: `v4 = *(int *)(o + 96)`
    // y compara `v4`; `v356 = *(float *)&v4` es solo la copia de los BITS.
    // El port hacia `(int)fVar13`, o sea convertia el float numericamente -> 0.
```

### Línea 1963 en `MoveEffect` — antes de `local_340 = 0.0;`

```cpp
      // 2026-09-26 (Rageful Blow): las cuatro ramas de abajo comparaban
      // `*(int*)&fVar13` -- un valor FILTRADO de otro case, porque en este
      // no hay ningun `fVar13 = param_1[1]` previo.  En IDA la variable es
      // `v4 = *(int *)(o + 96)` (L419), o sea el LIFETIME, que aca es
      // `__cnt`.  El efecto nace con vida 20 y va bajando:
      //   15     -> invierte el signo del termino vertical (o+216)
      //   13     -> estela de efectos 254 + sonido 89
      //   10..15 -> el arma DESCIENDE (-8/frame)
      //    3     -> impacto en el suelo (grietas, chispas, 247/245/246)
      //    2     -> reporte de blancos (sub_45FEC0) y muerte del efecto
      // Ninguna se cumplia: el arma solo subia y no habia ni impacto ni
      // dano multi-objetivo.  El test `9 < __cnt < 16` ya estaba bien.
```

### Línea 1996 en `MoveEffect` — antes de `param_1[0x5d] = local_324 + param_1[5];`

```cpp
        // 2026-09-26 (Rageful Blow): dos errores en el punto de impacto.
        //  - Y se truncaba a int.  IDA: `v355 = TargetPosition[1] + *(float*)(o+20)`
        //    es float; el `(int)` venia del slot SLODWORD que Ghidra reusa.
        //  - RequestTerrainHeight recibia la posicion ORIGINAL del efecto en vez
        //    del punto YA rotado (IDA usa v60/v299, o sea o+368 y o+372), asi que
        //    el golpe al suelo muestreaba el terreno bajo los pies del caster y no
        //    donde cae, 80 unidades adelante.
```

## `src/Render/MoveEffect_Helpers.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// MoveEffect_Helpers.cpp
// Helper functions called from FUN_00466AD0 (MoveEffect).
//
// Effect_SpawnSmokeBurst @ 0x004660F0  — Effect_SmokeBurst   (smoke + optional sparkle)
// Effect_SpawnSmokeExplosion @ 0x004661F0  — Effect_SmokeExplosion (int-coord variant)
// Effect_SpawnLightningBurst @ 0x00460C30  — Effect_LightningBurst (3 random lightning beams)
// Effect_SpawnProximityHit @ 0x00465E60  — Effect_OnHitProximity (proximity hit fx by entity type)
// Ring_ComputeOrbit @ 0x00473D90  — Ring_ComputeOrbit    (Lissajous ring position calculator)
//
// NOTE: Effect_CollisionCheck @ 0x00466440 — NOT implemented here.
//   Uses unaff_retaddr + unaff_EBP (phantom return-address / frame-pointer params)
//   and heavy HashTable reference-count obfuscation — cannot be ported safely.
//   Kept as empty stub in stubs.cpp.
```

## `src/Render/MoveJoint.cpp`

### Línea 57 — antes de `static void MoveJoint_GenericTail(unsigned char *o)`

```cpp
// -----------------------------------------------------------------------------
// 2026-09-03 - COLA GENERICA de MoveJoint (IDA 0x00470030 L1922-2217)
//
// En el binario, todo joint cuyo TIPO no matchea ningun bloque del dispatch cae
// en esta cola.  Es la que CONSTRUYE la estela: recorre `segMax` pasos y en cada
// uno mueve el joint con `MoveHumming` hacia su objetivo y empuja un segmento
// con `sub_46FE90` -- o sea el rayo entero se dibuja en UN tick.
//
// El port no la tenia, y por eso faltaban tres cosas que parecian no
// relacionadas: el rayo del Lightning (1254 sub 0), los rayos del Twister
// (1253 sub 0, ocho joints) y los rayos de fondo de Icarus (1254 sub 6, que
// ademas es el unico subtipo que el epilogo LABEL_487 excluye del tick de
// segmentos justamente porque los construye aca).
//
// Offsets: +8 SubType - +12 Scale - +16 Position - +40 Angle - +52 Light
//          +64 Owner - +68 TargetPosition - +84 segMax - +2488 lifetime
//          +2496 Velocity - +2500/2504/2508 jitter de angulo
```

### Línea 303 en `MoveJoint` — antes de `float   __frame[54] = {0};`

```cpp
    // 2026-08-10 FIX (haces oscuros): estos "locales" que produjo Ghidra son en
    // realidad UN bloque contiguo del frame original (ebp-0xD8 .. ebp), y el
    // código de abajo depende de esa contigüidad — MSVC no la garantiza:
    //   L99  Vector_Rotate(&local_b4, local_30, local_d8 + 6)  → escribe el vec3
    //        de salida en local_d8[6],[7],[8]; L102 lee out[2] como `local_b8`,
    //        o sea `local_d8[8] == local_b8` (¡y local_d8 estaba dimensionado a
    //        8, así que era además una escritura fuera de rango en el stack!).
    //   L138 Vector_Rotate(local_d8 + 6, local_a8 + 6, &local_b4) → L140/L141
    //        leen out[1] y out[2] como `local_b0` y `local_ac`, o sea
    //        `(&local_b4)[1] == local_b0` y `(&local_b4)[2] == local_ac`.
    // Con locales sueltos, la entrada de Vector_Rotate traía basura en y/z y la
    // salida se perdía → la Position del joint (+0x14/+0x18) quedaba con valores
    // de ~1e9 y el render dibujaba quads que cruzaban toda la pantalla.
    // Mapeo por offset de frame: -0xD8=[0] … -0xB8=[8] … -0xA8=[12] …
    // -0x60=[30] … -0x30=[42], total 0xD8/4 = 54 floats.
```

### Línea 355 en `MoveJoint` — antes de `const float dist_4e8 =`

```cpp
        // IDA case 1256 (BITMAP_JOINT_FIRE en el source de MU 5.2,
        // ZzzEffectJoint.cpp:4243):
        //     Distance = MoveHumming(Position, Angle, TargetPosition, 0.f);
        //     ... if (Distance <= o->Velocity) { o->Live = false; ... }
        // IDA lo muestra como `sub_43E4A0(...); v295 = v4;` — `v4` es el retorno
        // FPU que Hex-Rays no tipa. Antes se aproximaba con `local_e4_f`
        // (distancia recalculada a mano); ahora se usa el valor real.
```

### Línea 650 en `MoveJoint` — antes de `int seg_rows = *(int *)(param_1 + 0x50) + 1;`

```cpp
            // Subtract anchor from segment positions (un-translate)
            // 2026-09-03 FIX (la banda de Icarus): el bucle cubria las filas
            // 0..segCount-1, pero el renderer (0x00473710) dibuja segCount
            // quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**; y `sub_46FE90` tambien escribe esa fila.  La
            // fila de mas quedaba fuera del par restar/sumar y derivaba: la
            // sonda JSPAN la cazo con un quad de 1572 unidades entre
            // (1648,1571,299) y (1638,0,285) -- una raya cruzando el mapa.
            // Ahora se cubren segCount+1 filas, acotado a segMax.
```

### Línea 691 en `MoveJoint` — antes de `int seg_rows = *(int *)(param_1 + 0x50) + 1;`

```cpp
            // Add anchor back to segment positions (re-translate)
            // 2026-09-03 FIX (la banda de Icarus): el bucle cubria las filas
            // 0..segCount-1, pero el renderer (0x00473710) dibuja segCount
            // quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**; y `sub_46FE90` tambien escribe esa fila.  La
            // fila de mas quedaba fuera del par restar/sumar y derivaba: la
            // sonda JSPAN la cazo con un quad de 1572 unidades entre
            // (1648,1571,299) y (1638,0,285) -- una raya cruzando el mapa.
            // Ahora se cubren segCount+1 filas, acotado a segMax.
```

### Línea 732 en `MoveJoint` — antes de `const int frameSeed = (int)DAT_083a7c00;          // MoveSceneFrame`

```cpp
        // 2026-08-24 FIX (Soul Barrier: el efecto se deformaba y cubria al pj en
        // vez de orbitar): la semilla era `local_dc_f`, que es el DELTA X
        // (Position.x - Target.x) — un valor geometrico. IDA (0x470030 L1035-1039)
        // usa MoveSceneFrame, el contador de frames:
        //     v49 = LODWORD(v297);                      // v297 = MoveSceneFrame
        //     if ( !(iIndex % 2) ) v49 = -LODWORD(v297);
        //     v52 = iIndex + v49 + 53730 * iIndex;
        // Con el delta X la posicion del joint se calcula a partir de si misma:
        // realimentacion -> la orbita se abre en cada frame hasta cubrir al pj.
        // Hex-Rays reusa el slot `v297` varias veces en esta funcion y el port
        // tomo el valor de otro tramo.
        //
        // `LODWORD` sobre un float toma sus BITS: MoveSceneFrame es un contador
        // entero (DAT_083a7c00, DWORD) que Hex-Rays tipeo float, asi que el valor
        // correcto es el entero directo — no `float_as_int` de un float negado,
        // que solo invierte el bit de signo y da otra cosa.
        //
        // Ojo el sentido: IDA niega cuando el indice es PAR (`if (!(iIndex % 2))`),
        // al reves de lo que hacia el port.
```

### Línea 915 en `MoveJoint` — antes de `if (iVar16 == 0x4e6) {`

```cpp
    // 2026-09-02 (Lightning sin rayo): el tipo 1254 NO tiene bloque propio en
    // IDA MoveJoint (0x00470030) -- su unica mencion en toda la funcion es la
    // exclusion del epilogo (L2226).  El bloque que habia aca era invencion del
    // port y hacia DOS cosas daninas:
    //   1. Llamaba `sub_46FE90` una segunda vez por frame (el epilogo ya la
    //      llama), o sea DOBLE scroll de la historia de segmentos: la estela
    //      perdia la mitad de su largo y duplicaba la cabeza.
    //   2. Aplicaba el fade `if (lifetime < 5) color *= 0.76923078`, que en
    //      IDA pertenece al tipo **1176** (L785-791), no al 1254.
    // Con lifetime 2 (CreateJoint case 1254 sub 0 -> LABEL_57), las dos cosas
    // juntas dejaban el rayo del Lightning practicamente invisible.
```

### Línea 939 en `MoveJoint` — antes de `*pfVar15                   = *(float *)(param_1 + 0x1c);`

```cpp
            // 2026-09-03 (haz que cruzaba Icarus): IDA (0x00470030 L1880-1886)
            //     *v2 = *(float *)(o + 28);
            //     *(_DWORD *)(o + 20) = *(_DWORD *)(o + 32);
            //     *(_DWORD *)(o + 24) = *(_DWORD *)(o + 36);
            // Esos 28/32/36 son DECIMALES = 0x1C/0x20/0x24, donde `CreateJoint`
            // guarda el ORIGEN del rayo (LABEL_61: `*((float *)v11 + 7) = *v12`).
            // El port los leyo como HEX y reseteaba desde 0x28/0x2c/0x30, que es
            // el vector Angle (~0,0,0 para estos joints): el rayo renacia en la
            // esquina del mapa en cada rebuild y la estela lo unia con el cielo
            // -- el haz azul que cruzaba Icarus.  Medido con la sonda JBEAM:
            // `type=1255 sub=9 P=(3167,928,58) v0=(1251,54,139)`.
```

### Línea 1015 en `MoveJoint` — antes de `const float dist_4ea =`

```cpp
                // 2026-08-16: `Distance` es el RETORNO de MoveHumming, no la Z
                // del target. Hex-Rays tipaba MoveHumming como void (retorno en
                // st0) y este port comparaba `target[2]` = ownerZ + 120, que en
                // cualquier mapa es >> 35 → las esferas de EXP nunca llegaban a
                // absorberse y orbitaban al pj acumulandose. Confirmado contra el
                // source de MU 5.2 (ZzzEffectJoint.cpp:3368).
```

### Línea 1320 en `MoveJoint` — antes de `{`

```cpp
    // ── IDA LABEL_301 — movimiento compartido por los tipos 1249 (0x4E1) y
    // 1277 (0x4FD). Es un switch sobre el SubType (+0x08).
    //
    // 2026-08-10 FIX (destello dorado errático del set +11): el port mandaba
    // TODOS los subtipos a la órbita pseudo-aleatoria de más abajo, sembrada con
    // el índice de slot — de ahí que el efecto saltara de un lado a otro en
    // cualquier altura. El subtipo 0 (que es el halo del set +11, spawneado por
    // `Entity_UpdateRender` sección 6) en el original es un **círculo que sube**:
    //     a = (lifetime + phase) * 0.1        (PKKey == -1)
    //     Position.x = cos(a) * 40 + TargetPos.x
    //     Position.y = TargetPos.y - sin(a) * 40
    //     Position.z += riseSpeed
    //
    // 2026-09-26: TODOS los subtipos estan portados y verificados contra IDA.
    //
    // El comentario anterior decia que 4/6/7/8/9/11/12 quedaban sin portar porque
    // su decompile "esta entrelazado con ruido de hash-table (LABEL_438/439)".
    // Las dos cosas eran falsas: MoveJoint no tiene ruido anti-tamper (6 lineas
    // de 2244, o sea 0%), y LABEL_438/439 no es anti-tamper sino la cola comun a
    // la que saltan varios subtipos.  Lo que despista es que el switch de IDA
    // (L1085) solo lleva los cases 0/2/3/10/14 y NO tiene default: el resto se
    // resuelve con ifs encadenados desde L1162, asi que no aparecen como `case`.
    //
    // De los que el comentario daba por pendientes, 6/7/8/9/11/12 ya estaban
    // portados (y verificados ahora termino a termino); el unico que faltaba de
    // verdad era el 4.
```

### Línea 1407 en `MoveJoint` — antes de `const int life = *(int *)(param_1 + 0x9b8);`

```cpp
            // 00470030 LABEL_301 subtipos 4, 6 y 12: los tres convergen en
            // LABEL_438/439 y solo cambian como calculan lateral/vertical.
            //
            // 2026-09-26: el 4 faltaba.  En IDA llega aca por el `if (v168 != 4)`
            // de L1162, que SALTEA todo el bloque de los demas subtipos, asi que
            // cae junto al 12 en L1681 (`v16 = v168 == 12`).  Su rama es la del
            // else de L1702.
```

### Línea 1444 en `MoveJoint` — antes de `float jointMatrix_v304[12];               // IDA: v304[3][4]`

```cpp
        // 2026-09-01 FIX — los subtipos 8 y 9 PISABAN `local_30`, que es el
        // `in2` de IDA: la matriz que arma el prologo con
        // `AngleMatrix((float *)(o + 40), in2)` (L319) y que el epilogo
        // LABEL_487 (L2228) le pasa a `sub_46FE90` para construir las 4
        // esquinas del segmento nuevo.  IDA usa una matriz APARTE (`v304`,
        // declarada en L307) en estos dos cases; reusar `in2` dejaba el
        // scroll de segmentos del epilogo con la matriz equivocada.
```

### Línea 1482 en `MoveJoint` — antes de `if (jsub == 7 || jsub == 11) {`

```cpp
        // 2026-09-04 -- PORTADO: subtipos 7 y 11 (IDA 0x00470030 L1302-1682).
        //
        // Estos dos NO caen en la helice generica de mas abajo.  Rehice la traza
        // de anidamiento de LABEL_301 contando llaves: `if (v168 != 7)` cierra en
        // L1301 y la ejecucion sigue en L1302, dentro del bloque de
        // `if (v168 != 12)`, que llega hasta L1683 y termina en `goto LABEL_447`.
        // Solo los subtipos 4 y 12 alcanzan la helice.
        //
        // Es un PROYECTIL que converge sobre el owner:
        //   - la posicion sale de una espiral alrededor de TargetPosition, con
        //     radio que crece de 0 a 150 a medida que baja la vida;
        //   - un bucle de tres pasos (x, y, z) interpola esa posicion hacia el
        //     owner con peso `w`, que decrece con la vida -> se va pegando al
        //     objetivo;
        //   - y emite tres CreateSprite por tick (1231 + dos 1150), que es lo
        //     unico visible del efecto.
        //
        // Es el disparo del ataque de Alquamos: AttackEffect case 0x45 crea ocho
        // 1249/sub7 con el heroe como owner.  Nuestro port los mandaba a la
        // helice, que ni converge ni emite sprites -- de ahi que primero se
        // vieran lineas hacia cualquier lado y, una vez limpia la velocidad
        // heredada del slot, no se viera nada.
```

### Línea 1566 en `MoveJoint` — antes de `const int srcBase = (jsub == 11) ? 352 : 0;`

```cpp
                // 2026-09-04 FIX (la flecha del arco colapsaba en un destello):
                // el subtipo 11 NO converge hacia `owner + 16/20/24` sino hacia
                // `owner + 368/372/376`.  IDA lo escribe ofuscado --
                // `LODWORD(v295) = 352 - o;` y luego
                // `*(float *)(LODWORD(v295) + *(_DWORD *)(o + 64) + v228)` --
                // pero 352 + {16,20,24} = {368,372,376}, que es la posicion
                // DERIVADA del efecto de la flecha: la que fija CreateEffect
                // case 243 (`i+92..94`) 100 unidades por delante y la que usa su
                // propio AddTerrainLight.
                //
                // Con `owner + 16` la cosa se realimenta: el joint con
                // SkillIndex==1 escribe su posicion EN `owner + 16`, asi que
                // convergia hacia si mismo y los cuatro joints se apelotonaban
                // en el punto de salida -- el destello sin estela.
```

### Línea 1621 en `MoveJoint` — antes de `{`

```cpp
    // 2026-09-01 — PORTADA la cola real de LABEL_301 (IDA L1706-1724 ->
    // LABEL_438 -> LABEL_439).  Aca habia una "orbita pseudo-aleatoria"
    // INVENTADA (sembrada con el indice de slot y unos globals de ruido) que
    // ademas llamaba `Joint_SegmentTick` de mas: el epilogo LABEL_487 ya lo llama,
    // asi que se scrolleaba la historia de segmentos DOS veces por tick — el
    // anillo avanzaba al doble y quedaban segmentos duplicados/entrelazados.
    //
    // Lo que hace el binario para los subtipos que no matchean ningun case
    // (4, 7, 11 y el resto) es la MISMA helice que 6 y 12, con otra formula de
    // radio:
    //     v237 = *(_DWORD *)(o + 2488);                       // life
    //     v238 = ((double)v237 + *(float *)(o + 2500)) * 0.1;  // angulo
    //     v240 = v237 + 40;  v296 = (v240 <= 10) ? 10 : v240;  // radio
    //     v234 = -(cos(v238) * (v296 * 0.64999998));
    //     v236 =   v296 * 0.64999998;  v235 = sin(v238);
    //   LABEL_438: v239 = v235 * v236;
    //   LABEL_439: <ancla + heading + escritura de Pos>
```

### Línea 1685 en `MoveJoint` — antes de `iVar16 = *(int *)(param_1 + 4);`

```cpp
    // 2026-09-04 -- ORDEN CORREGIDO.  En IDA el epilogo es
    //     LABEL_182 (re-ancla SubType 7)  ->  LABEL_487 (scroll de segmentos)
    // y aca estaban al reves.  Con el orden invertido, el `goto _skipLabel182`
    // que usa el bloque de los subtipos 7/11 -- que en IDA salta LABEL_182 pero
    // SI pasa por LABEL_487 -- terminaba salteando tambien el scroll, asi que
    // esos joints nunca acumulaban segmentos y su cinta no se dibujaba.  Es la
    // estela de la flecha del arco (cuatro 1249/sub11 que crea CreateEffect
    // case 243) y la cadena de Queen Rainer.
```

## `src/Render/Particle.cpp`

### Línea 133 — antes de `// IDA: CreateSprite (0x004795C0)`

```cpp
// SetAction @ 0x0043E820 — DEFINICION UNICA en stubs_externs.cpp.
// 2026-08-16: acá había una segunda definición con firma (int, uint). Como C++
// las trata como sobrecargas distintas, ambas compilaban y cada caller elegía
// por el tipo de sus argumentos — el mismo patrón que causó el bug del Magic
// Gladiator con SetPlayerStop. Las dos eran equivalentes al binario, así que
// consolidar no cambia comportamiento; sólo elimina la trampa.
```

### Línea 166 en `Particle_PathUpdate` — antes de `// BUG-FIX 2026-05-01: null pointer guard. El binary original asume que`

```cpp
  // Pool fix 2026-04-27: el AUTO-SKIP previo (return 0 al inicio) bloqueaba
  // TODOS los efectos (glow +9 set, wing FX, weapon sparkles, lightning).
  // Ahora con DAT_07c85890[1002*0x1bc] correctamente dimensionado, iteramos
  // por índice acotado por 1002 slots en vez de la dirección absoluta original.
```

### Línea 171 en `Particle_PathUpdate` — antes de `if (!param_2 || !param_4) {`

```cpp
  // BUG-FIX 2026-05-01: null pointer guard. El binary original asume que
  // param_2 (pos) y param_4 (dir) siempre son pointers válidos, pero algún
  // caller en el char-select pipeline pasa NULL → AV en char-select crash.
  // Loguear UNA vez para identificar al caller y eventualmente arreglar la
  // raíz. Por ahora, retornar 0 (slot inválido) para evitar el AV.
```

## `src/Render/Particle_Legacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 11 — antes de `void __cdecl Character_UpdateAll(void) {`

```cpp
// Character_UpdateAll @ 0x00479730 — Particle_RenderAll.
// Iterates effect/particle pool (base DAT_07C85890, stride 0x1BC).
// For each active slot: sets GL blend mode (0=normal,1=additive,2=alpha),
// calls Render_DrawSprite to draw it, then clears the active flag.
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODOS los efectos (glow +9,
// wing FX, etc.). Ahora itera por índice acotado (1002 slots).
```

### Línea 31 — antes de `void __cdecl Effect_UpdateAll(void) {`

```cpp
// Effect_UpdateAll @ 0x00479790 — marks all active particle entries dirty (+0x160 = 1).
// Pool fix 2026-04-27: ahora itera por índice acotado (1002 slots).
```

### Línea 42

```cpp
// 2026-09-21: aca vivia un wrapper de 5 argumentos
// `Particle_Spawn(type, x, y, z, flags)` que delegaba en el real pasando
// `nullptr` como Position.  Particle_Spawn hace `*param_2` sin guard, asi que
// cualquier llamada habria sido una lectura de la direccion 0.  No tenia
// callers (Combat/Skills.cpp solo lo declaraba), o sea era una trampa armada:
// la misma familia que ya causo dos crashes, en RenderBoids y en la caida de
// la puerta de Blood Castle.  Se borra en vez de ponerle un guard, porque el
// binario tampoco lo tiene: ahi Position nunca llega en NULL.
```

## `src/Render/Particle_Move.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.
```

### Línea 34 — antes de `#define P_BASE      (DAT_07abf5f0)`

```cpp
// Helper macros for particle field access
// BUG-FIX 2026-04-27: era `&DAT_07abf5f0` cuando DAT era `char` solo. Ahora es
// array `char[N]` y `&array` sería pointer-to-array (aritmética × sizeof[N]).
// Usar `DAT_07abf5f0` directo decae a char* correcto.
```

## `src/Render/Particle_Render.cpp`

### Línea 96 — antes de `void __cdecl Particle_RenderAll(void)`

```cpp
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 648-789 (142 lines)
// =============================================================================
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/0046BE40_Particle_Render.c.
```

## `src/Render/Particle_Spawn.cpp`

### Línea 47 en `Particle_Spawn` — antes de `float in1_3c[3];`

```cpp
    // 2026-09-02 -- patron [[locales-contiguos-ghidra]] (otra instancia).
    // IDA Particle_Spawn (0x00475220) declara `float in1[3]` en [ebp-3Ch] y lo
    // pasa entero a VectorRotate.  Ghidra lo emitio como TRES escalares
    // sueltos (local_3c/38/34 = ebp-0x3C/-0x38/-0x34) y los seis call sites de
    // esta funcion hacen `Vector_Rotate(&local_3c, ...)`, o sea leen 3 floats
    // contiguos desde el primero.  MSVC no garantiza ese layout: las
    // componentes Y/Z salian de pila basura.
    //
    // Sintoma (sonda FXQUAD, 2026-09-02): la particula de SANGRE (tipo 1206,
    // Effect/blood.tga) se dibujaba con la posicion en NaN -- el log mostraba
    // `pos=(-2147483648,...)`, que es `(int)NaN`.  La velocidad basura entra
    // en el slot (+19) y `MoveParticles` case 0x4B6 la suma a la posicion en
    // cada frame.  Con vertices NaN el quad se estira sin limite; en los mundos
    // 2, 7 y 10 `SkillEffect_Render` usa blending ADITIVO, asi que se ve como
    // un haz brillante (de ahi el reporte en Icarus).
```

### Línea 68 en `Particle_Spawn` — antes de `iVar6 = 0;`

```cpp
    // ── scan pool for free slot ────────────────────────────────────────────────
    // Pool fix 2026-04-27: era end-bound absoluto 0x7b1166f. Con DAT_07abf5f0
    // como 1 byte el primer slot estaba "ocupado" (basura) y el guard
    // retornaba 0 inmediatamente → NUNCA spawneaba particles. Ahora pool real
    // de 3000 slots, iteramos por índice.
```

### Línea 131 en `Particle_Spawn` — antes de `*(float *)(pcVar11 + 0x40) = (float)(int)((__int64)WorldTime % 360);`

```cpp
                // 2026-08-11: acá había un `__ftol()` sin argumentos (el artefacto
                // de Ghidra para la conversión x87 de WorldTime) cuyo resultado
                // alimentaba el shift/OR de abajo — o sea el ángulo inicial de la
                // partícula salía de basura. Es el mismo patrón ya barrido en los
                // otros 47 sitios; éste sobrevivió porque el tipo 1220 (humo)
                // nunca llegaba a spawnearse.
```

### Línea 418 en `Particle_Spawn` — antes de `}`

```cpp
            // 2026-09-12: aca estaba el cuerpo de 0x47f (vida rand%8+20) SUELTO,
            // sin condicion: atrapaba todos los tipos entre 0x498 y 0x4a5 y los
            // devolvia con vida 20-27 antes de llegar a sus case de abajo.  Con
            // eso 0x498 (1176, Teleport) nunca sorteaba angulos ni recibia
            // velocidad -- la columna del teleport quedaba recta -- y las
            // particulas del Energy Ball (1180/1176) vivian 20-27 ticks en vez
            // de 2.  IDA: esos tipos van a su case o salen por el default con
            // la vida por defecto.
```

### Línea 472 en `Particle_Spawn` — antes de `goto particleSpawn_outerSwitch;`

```cpp
        // 2026-08-23 FIX: aca habia un `return iVar6;` que se tragaba TODOS los
        // tipos entre 0x49c y 0x4c4 sin case propio en este arbol — o sea 9 tipos
        // de particula que SI tienen su inicializacion en el switch de mas abajo:
        //   0x4a7, 0x4ab (fuego), 0x4ac, 0x4ad, 0x4b0 (Flame01), 0x4b5, 0x4b6,
        //   0x4bf, 0x4c0.
        // Quedaban con el `life` en el default 2 (en vez de 24 para el fuego) y
        // sin escala/rotacion propias: se creaban ~150 por segundo y morian a los
        // 2 frames, asi que en pantalla habia ~10 a la vez y el fuego se veia como
        // un puntito en vez de una masa.
        //
        // El `if (param_1 < 0x4c5)` de arriba NO es un rango real: es el arbol
        // binario de busqueda que genera Ghidra para un switch disperso, y el port
        // lo convirtio en un if/else con `return` que corta el fall-through.
```

## `src/Render/Player_Render.cpp`

### Línea 70 en `Player_Render` — antes de `DWORD* puVar1 = DAT_07c74f54;`

```cpp
    // 2026-05-07: UN-SKIPPED. El pool ahora se aloca propiamente como
    // g_PlayerRenderPool[100 × 0x1BC] (per IDA bound 0xAD70 = 100 × 444), con
    // DAT_07c74f54 apuntando a g_PlayerRenderPool + 0xEC (= v1 anchor del
    // slot 0). Los reads negativos (puVar1 - 0x3b = -0xEC bytes) ahora caen
    // dentro del buffer.
    //
    // Walker iter: v1 starts at DAT_07c74f54 (= slot 0 + 0xEC), advances 0x6F
    // floats (= 0x1BC bytes) per iter. Total 100 slots.
    //
    // Slot fields (relative to v1 = slot+0xEC):
    //   v1 - 0x3b (= slot+0)    byte: active flag
    //   v1 - 0x38 (= slot+0xC)  float: rotation
    //   v1 - 0x37 (= slot+0x10) float[3]: pos xyz
    //   v1 - 0x32 (= slot+0x24) float: scale
    //   v1 - 1, v1, v1 + 1      float: color RGB
    //   v1 - 0xea (= slot - 0xC2 from v1) short: class_code
```

## `src/Render/RenderLinkObject.cpp`

### Línea 94 en `RenderLinkObject` — antes de `float afStack_264[12];`

```cpp
    // BUGFIX 2026-04-26: el `return;` AUTO-SKIP estaba mal copiado del template
    // de Player_Render. Esta función NO tiene end-bound loop — es el render de
    // items linked al hueso (alas, armas, helper, escudo). Sin esto las alas
    // (model_idx=0x313..0x316) y armas no se dibujan en char-select ni in-game.
    // Ghidra emitía 642 líneas válidas que sí están en el cuerpo abajo.
    // ── local storage ─────────────────────────────────────────────────────────
    // afStack_264 layout used for various vec3/matrix temps throughout:
    //   [0..2]  = Light color vec3
    //   [3..5]  = Angle vec3  (input to Matrix_BuildFromEuler / BMD_TransformPosition)
    //   [6..8]  = Position vec3 (world pos scratch / BodyOrigin temp)
    //   [9..11] = extra (matches IDA `Position[3]` at ebp-240h)
    // BUGFIX 2026-04-26: era float[7] pero los callees (BMD_TransformPosition,
    // CreateSprite, Joint_Create) leen/escriben 3 floats desde
    // `afStack_264 + 6` → [6][7][8] OOB. /GS canary check tripeaba al return.
    // local_248/local_244 eran las falsas vars que Ghidra emitió por las
    // posiciones [7] y [8].
```

### Línea 113 en `RenderLinkObject` — antes de `float local_240_buf[3];  // pos_out: [0]=x, [1]=y, [2]=z (BMD__RotationPosition target)`

```cpp
    // BUGFIX 2026-04-27: local_240/_23c_f/_238_f eran 3 vars separadas (void* +
    // float + float). BMD__RotationPosition escribe 3 floats consecutivos a través de
    // (float*)&local_240. Si el compilador NO ubica las 3 vars contiguas (no
    // está obligado), los writes 4-11 caen en stack canary u otros locales →
    // comportamiento NO determinístico entre builds (flicker variable, locales
    // pisados). Ahora un solo array contiguo. local_23c_f/_238_f redirigidos
    // vía macro al uso "como float", local_240 mantiene su uso "as void*" en
    // el bloque hash-table (lee/escribe los 4 bytes como pointer).
```

### Línea 126 en `RenderLinkObject` — antes de `float local_228[12];     // AngleMatrix output: 3×4 matrix, row-major`

```cpp
    // BUGFIX 2026-04-26: era float[3] con local_21c/20c/1fc separadas; pero
    // Matrix_BuildFromEuler escribe 12 floats (matriz 3×4 [0..0xb]) → overflow masivo
    // dentro del propio buffer. Ahora declarado como matriz completa y los
    // accesos legacy local_21c/20c/1fc redirigen vía macro.
```

### Línea 135 en `RenderLinkObject` — antes de `float afStack_204[3];    // = IDA v70[3] — anim param scratch`

```cpp
    // BUGFIX 2026-04-26: afStack_204 era float[2], pero IDA `v70[3]` y los
    // callees (sub_4404E0 anim1/anim2) leen 3 floats. → OOB read garbage.
```

### Línea 138 en `RenderLinkObject` — antes de `unsigned char local_1ec[0x1c0];     // local OBJECT — ItemObjectAttribute target`

```cpp
    // BUGFIX 2026-04-26: el OBJECT local era unsigned char[2] + un short suelto.
    // `ItemObjectAttribute` (ItemObjectAttribute) escribe hasta offset 0x168 (360 bytes)
    // → smasheaba TODO el frame, devolvía a Entity_RenderAll con param_1
    // corrupto a la siguiente lectura (+0xae). Ahora dimensionado al stride
    // del effect-entity pool (0x1bc = 444 bytes) y `local_1ea` redirigido al
    // offset +2 dentro del buffer (campo Type según IDA `o[1]`).
```

### Línea 161 en `RenderLinkObject` — antes de `memset(local_1ec, 0, sizeof(local_1ec));`

```cpp
    // BUGFIX 2026-04-27: zero local_1ec antes de ItemObjectAttribute. La stack
    // tiene garbage cada llamada → ItemObjectAttribute no escribe TODOS los
    // campos del OBJECT struct, sólo los que le importan. Los bytes uninit
    // pueden cambiar comportamiento de RenderPartObjectEffect / BMD_SetupRenderByType entre frames
    // → flicker visible en weapons.
```

### Línea 196 en `RenderLinkObject` — antes de `*(float*)(iVar7 + 0x6c) = *(float*)&local_240 + *(float*)(param_4 + 0x10);`

```cpp
        // BodyOrigin = TransformedPosition + entity world position
        // BUGFIX 2026-04-26: era `(float)(int)local_240` que tomaba la
        // representación entera de los bits del float y la convertía a float
        // (devolvía 1065353216.0f para un 1.0f real). Ghidra había tipado
        // local_240 como void* y aplicó el cast equivocado. IDA línea 173
        // usa `Position[0]` directo. Reinterpretamos correctamente.
```

### Línea 257 en `RenderLinkObject` — antes de `bool poseSet = false;`

```cpp
                // ── Tabla de poses de escudo / segundo item ─────────────────
                // PORT DEL DLL (CWeaponView::SecondWeaponViewFix), 2026-09-01.
                // El DLL engancha en 0x0045568B (la rama generica de abajo, que
                // vanilla resuelve con trans (-20, 5, 40)) y salta de vuelta a
                // 0x004556AA.  Constantes decodificadas de su bloque _asm:
                //   0xC20C0000=-35  0x41200000=10   0x41F00000=30  0xC1200000=-10
                //   0x43070000=135  0x42B40000=90   0xC1A00000=-20 0x42480000=50
                //   0xC1E00000=-28  0xC1C80000=-25  0xC2DC0000=-110 0x43340000=180
                //   0x41A00000=20   0x41700000=15   0x42200000=40  0x40A00000=5
                // Sin esto un escudo cae en la pose generica de arma
                // (Angle 70,0,90 / trans -20,5,40) y queda flotando por encima
                // de la cabeza.
```

### Línea 347 en `RenderLinkObject` — antes de `// BoneTransform for LinkBone`

```cpp
        // Translation column already written via local_21c/20c/1fc macros, which
        // alias local_228[3]/[7]/[11] (BUGFIX 2026-04-26 — see decl block).
        // R_ConcatTransforms takes: (bone_mat, angle_mat_12, out_parentmat)
```

### Línea 375 en `RenderLinkObject` — antes de `{`

```cpp
    // ── 4. Hash-table ref-count block on param_4+0x302 (anti-tamper) ─────────
    // This is the obfuscation pattern (see CLAUDE.md). It manipulates a reference
    // count stored at param_4+0x302 via a hash table keyed on the pointer.
    // The block is transcribed faithfully; net game effect = zero.
```

### Línea 693 en `RenderLinkObject` — antes de `float Light[3] = { 0.0f, 0.0f, 0.0f };`

```cpp
    // BUGFIX 2026-09-01: locales no contiguos que Ghidra separo.  Los 9 cases
    // del switch de abajo construian el `Light[3]` de IDA como TRES escalares
    // sueltos (ebp-270h / -26Ch / -268h) y pasaban `&pbStack_270_f` como vec3.
    // MSVC no garantiza ese layout, asi que CreateSprite leia G y B de basura:
    // el brillo de cada arma/escudo salia con color arbitrario.  El Grand Soul
    // Shield (tipo 607) deberia tirar a azul (Light[2] = fLum*2, ~3x el R/G) y
    // se veia blanco.
```

### Línea 715 en `RenderLinkObject` — antes de `float* pfVar10 = (float*)&DAT_06970afc;`

```cpp
        // 2026-09-08: el bound era `< 0x6970c4c`, una direccion ABSOLUTA del
        // binario fuente (IDA: `while ((int)v43 < (int)flt_6970C4C)`).  En este
        // build g_BoneScratch vive muy por debajo de esa direccion, asi que el
        // bucle recorria ~100 MB de memoria transformando basura y spawneando
        // sprites en posiciones arbitrarias -- los "circulitos volando" -- hasta
        // pegar en una pagina no mapeada.  Ese era el crash de Blood Castle:
        // `Vector_Transform <- BMD_TransformPosition <- RenderLinkObject`.
        // Solo se disparaba con la ESPADA del evento (Type 419), por eso con el
        // arco el evento terminaba bien.
        //   base = flt_6970AFC = g_BoneScratch + 0x60 = hueso 2
        //   fin  = flt_6970C4C                        = hueso 9
        //   (0xC4C - 0xAFC) / 0x30 = 7 iteraciones -> huesos 2..8
```

## `src/Render/Render_Frame.cpp`

### Línea 212 — antes de `// Forward decls for HUD helpers defined later in this TU.`

```cpp
// 2026-05-08: backup of DAT_07d78068 — defined here (not in globals.cpp) so
// it lives in a different .obj's BSS, NOT adjacent to DAT_07d78068. The
// unknown writer that sets DAT_07d78068=0x1 also clobbers the next 4 bytes
// to 0 (8-byte write). Putting the backup far away keeps it intact.
// Plus a CANARY before/after to detect if even this gets clobbered.
// (g_ItemAttribute_Backup here)
```

### Línea 250 en `Game_RenderTick` — antes de `while (glGetError() != GL_NO_ERROR) {}`

```cpp
    // BUG-FIX 2026-05-04: drain residual GL errors antes del frame para que el
    // diagnostic logging de GL_DisableDepthTest no spamee con 0x504 stale (de pops
    // sin push del frame previo durante la transición login→in-game).
```

### Línea 255 en `Game_RenderTick` — antes de `Render_Scene3D();`

```cpp
    // BUG-FIX 2026-04-28: anteriormente esto llamaba Render_GameFrame que a su
    // vez llamaba Render_Scene3D al final → recursión infinita cuando porteamos
    // Render_Scene3D para que invocara los UI sub-renderers internamente.
    // Ahora Game_RenderTick → Render_Scene3D directamente (que ES la función
    // RenderMainScene de IDA @ 0x00525A00, hace todo el flujo: BeginOpengl,
    // 3D passes, BeginBitmap, HUD via Render_GameFrame, EndBitmap, EndOpengl).
```

### Línea 265 — antes de `static inline float ConvertX_RF(float x) { return x * (float)((double)WindowWidth  / 640.0`

```cpp
// Render_GameFrame — full HUD render pass.  Reconciled 2026-04-29 against
// IDA sub_4BBFB0 of the original mu.exe.  Previously several entries were
// mislabeled "AntiTamper_HashMaintain_*" which they are NOT — IDA shows
// them as plain UI render functions, and several real call sites
// (RenderMainFrameWindow vtable dispatch on dword_55C9FF0, sub_4BFDE0 3D
// hotbar, sub_4F6050 / sub_4EB070) were missing entirely.
//
// IDA verified call order (off + name + size in bytes):
//   if (World==8) { swirling-water bg via RenderBitmapUV }       — TODO
//   glColor3f(1,1,1)
//   sub_4BC220   Render_CharInfoPanel       guild-war/soccer
//   RenderPartyHP                            party HP bars (0x4BCA20, 735 b)
//   RenderNumArrow                           number/arrow overlay (0x4BF540, 1083 b)
//   RenderEquipedHelperLife                  party member helper life (0x4BEC00, 1168 b)
//   RenderBrokenItem                         broken-item warning (0x4BE710, 1255 b)
//   sub_4BF090   Render_MacroTimer
//   sub_4BF2D0   Render_MapLoadText
//   RenderBooleans                           floating-numbers/booleans (0x4BD090, 534 b)
//   sub_4F5820   Render_QuickButtons
//   (*dword_55C9FF0 + 0x10)(self)            chat scroll listbox render
//   RenderMainFrameWindow                    main HUD frame (0x4BD2B0, 919 b)
//   sub_4BE4F0   Render_ChatBox              chat input box
//   RenderExperience                         exp bar/level text (0x4BF990, 1097 b)
//   sub_4BD650                               LARGE HUD pass (3734 b — was misnamed C)
//   sub_4BCD20                               HUD pass D (867 b — was misnamed D)
//   sub_4BFDE0                               3D-projected hotbar items (434 b)
//   sub_4F6050                               unknown HUD pass (973 b)
//   sub_4EB070                               unknown HUD pass (1342 b)
//
// Functions tagged "TODO port" below are scaffold-only; their bodies are
// pending 1:1 IDA ports in dedicated sessions because each pulls in 5-15
// new globals (GuildWarScore[], HeroSoccerTeam, EnableGuildWar, ...) plus
// CRT/Win32 helpers (CreateGuildMark, RenderText_1, RenderBitmap, ...).
```

### Línea 302 — antes de `static void RenderBitmapUV(int Texture, float x, float y, float Width, float Height,`

```cpp
// ── RenderBitmapUV (0x005128C0) ─────────────────────────────────────────────
// 2026-08-23: no estaba implementada (functions.h la declaraba mal, como
// `(int,int,int,int)`), asi que el unico caller —la tormenta de arena de
// Tarkan— usaba `RenderBitmap` (0x5125A0) en su lugar.  No son intercambiables:
//
//   RenderBitmap   toma (u0, v0, u1, v1) y mapea un RECTANGULO de UV.
//   RenderBitmapUV toma (u, v, uWidth, vHeight) y mapea un cuadrilatero
//                  SESGADO — la V de las dos esquinas izquierdas va a
//                  `v + 0.25*vHeight` y `v + 0.75*vHeight`, mientras las
//                  derechas van a `v + vHeight` y `v`.
//
// Ese sesgo es lo que da el arrastre/perspectiva de la arena; con el rectangulo
// plano de RenderBitmap la textura se lee como un mosaico.
//
// Decodificado del decompile por offsets de stack: el loop
// `glTexCoord2f(t[v9-1], t[v9]); glVertex2f(ya[v9-1], ya[v9])` con v9 = 0,2,4,6
// desborda los arrays `t[5]`/`ya[2]` a proposito y toca los locales vecinos
// (`s`, `v15`, `v16`, `xa`, `v19`..`v23`), o sea depende del layout del frame
// original — [[locales-contiguos-ghidra]].  Aca se escriben las 4 esquinas
// explicitas, que es lo mismo sin depender del stack.
```

### Línea 355 en `Render_GameFrame` — antes de `RenderBitmapUV(0x494, 0.0f, 0.0f, 640.0f, 435.0f, scrollA, 0.0f, 0.30000001f, 0.30000001f)`

```cpp
        // 2026-08-23: antes esto llamaba a `RenderBitmap` (0x5125A0) con
        // (u0,v0,u1,v1), que mapea un RECTANGULO — la textura se leia como un
        // mosaico.  El original usa `RenderBitmapUV` (0x5128C0), que mapea un
        // cuadrilatero SESGADO en V y produce el arrastre de la arena.
```

### Línea 478 — antes de `// AntiTamper_HashMaintain_A (= RenderNumArrow @ 0x004BF540) is now`

```cpp
// 2026-04-29: name "AntiTamper_HashMaintain_X" was a misidentification.
// IDA shows these are plain HUD render passes, NOT anti-tamper code.
// Renamed conceptually but symbol kept (callers remain in Render_GameFrame
// only) until the bodies are ported and a final naming pass happens.
```

### Línea 570 en `Render_Scene3D` — antes de `// ── 3. Top strip viewport for frustum ─────────────────────────────────────`

```cpp
    // ── 2b. MoveMainCamera ────────────────────────────────────────────────────
    // Llama al port mínimo de MoveMainCamera (stubs.cpp), que setea
    // CameraAngle/CameraPosition relativos al Hero. Pitch -48.5° (= EarthQuake
    // - 48.5° per IDA), seguimiento 3rd-person.
```

### Línea 601 en `Render_Scene3D` — antes de `if (worldId != 10) {`

```cpp
    // ── 6. 3D render passes ──────────────────────────────────────────────────
    // BUG-FIX 2026-04-28: faltaba la llamada a RenderTerrain que
    // dibuja la malla de tiles del terreno. Sin ella, el cliente entraba al
    // mundo pero quedaba 100% negro.
```

### Línea 609 en `Render_Scene3D` — antes de `Particle_RenderAll();                         // particle system draw`

```cpp
    // 2026-05-07: Particle_Render (FUN_0046BE40) — port FIEL desde IDA
    // Game_RenderTick:113. Itera el effect pool y renderiza partículas
    // (gate sparks, magic glow, etc). ANTES no estaba wireado.
```

### Línea 621 en `Render_Scene3D` — antes de `RenderFishs(0, 0, 0, 0);                    // RenderFishs`

```cpp
    // 2026-05-07: RenderFishs + RenderBugs — port FIEL desde IDA
    // Game_RenderTick:124-125. Fauna decorativa (peces, mariposas).
```

### Línea 638 en `Render_Scene3D` — antes de `RenderPoints(0, 0, 0, 0);                    // RenderPoints (damage)`

```cpp
    // 2026-05-06: damage popup numbers (port FIEL desde IDA Game_RenderTick:139).
    // Llamado entre RenderParticles y glPopMatrix para que los números floten en
    // world-space. CreatePoint (= FUN_004792c0 en stubs.cpp:3407) los populeya
    // desde Net_Process case 0x15 (ReceiveAttackDamage).
```

### Línea 650 en `Render_Scene3D` — antes de `RenderMonsterName(0, 0, 0, 0);`

```cpp
    // 2026-05-07: sub_4CB6F0 (Target_Render) — port FIEL desde IDA
    // Game_RenderTick:143. Renderiza nombre del NPC/mob/player hovered.
    // Sin esto el user no ve qué está hovereando.
```

## `src/Render/Render_LegacyBillboards.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 95 — antes de `// ── Weapon/Entity color helpers ───────────────────────────────────────────────`

```cpp
// BMD__PlayAnimation (BMD::PlayAnimation / BMD_AnimTick) — moved to src/Render/BMD_Anim.cpp
// CharacterAnimation @ 0x00448600       — moved to src/Render/BMD_Anim.cpp
// (B3 refactor 2026-05-07)
```

## `src/Render/Render_LegacyDamageNumbers.cpp`

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

### Línea 47 — antes de `extern "C" double __cdecl RenderNumber2D(float x, float y, int Num,`

```cpp
// RenderPoints @ 0x00479330 — RenderPoints (damage popup renderer)
// 2026-05-06: ported from IDA mu97k-src-IDA/raw/00479330_RenderPoints.c.
//
// Itera el pool DAT_07c80110[100 × 0x70] de damage popups (poblado por
// CreatePoint en Net_Process case 0x15 / ReceiveAttackDamage). Para cada
// slot activo proyecta su world position a screen via gluProject y
// renderiza el número con RenderNumber2D usando el color del slot.
//
// Slot layout (per IDA CreatePoint):
//   +0x00 byte  active (1 if displayed)
//   +0x04 int   Value (damage to display; -1 = MISS)
//   +0x0c float scale (text size — typically 15 normal, 50 special)
//   +0x10 float pos.x (world)
//   +0x14 float pos.y (world)
//   +0x18 float pos.z (world, +140 elevation pre-applied)
//   +0x1c float color.r
//   +0x20 float color.g
//   +0x24 float color.b
//   +0x38 float frame counter (init 0, MovePoints increments)
//   +0x48 float lifetime (init 10.0, MovePoints decrements 0.3/tick)
```

### Línea 72 — antes de `extern "C" void __cdecl RenderNumber(float Position[3], int Num,`

```cpp
// 2026-08-15 — REESCRITO FIEL A IDA. La versión anterior era una invención en
// dos fases (project con `gluProject` + draw con `RenderNumber2D` en ortho 2D).
// Tres síntomas venían de ahí:
//   · nada se veía — se proyectaba con la matriz MODELVIEW leída de GL, que en
//     ese punto del frame es la IDENTIDAD (el call site corre después de
//     `GL_BeginSprite`/BeginSprite);
//   · los dígitos salían invertidos — `RenderNumber2D` usa V de 0.0→0.5 y el
//     original usa 0.5→0.0;
//   · los MISS salían como barras blancas — el original tiene un sprite propio
//     para `Num == -1`, no dibuja dígitos.
//
```

### Línea 168 — antes de `// GL helpers — cached OpenGL state wrappers`

```cpp
// ── Missing stubs added for linker fix ───────────────────────────────────────
```

### Línea 170

```cpp
// GL helpers — cached OpenGL state wrappers
```

## `src/Render/Render_LegacyLinker.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 40 — antes de `void __cdecl EnableAlphaBlend(void) {`

```cpp
// EnableAlphaBlend @ 0x00511710 — GL additive blending setup.
// 2026-06-29 BUG-FIX (depth-mask cache desync → estructuras opacas desaparecen):
// esta copia llamaba glDepthMask(0) y glDisable(GL_CULL_FACE) DIRECTOS, sin tocar
// los caches DAT_083a42e8 (depth mask) ni DAT_083a411c (cull). El render de meshes
// usa la versión cacheada GL_SetBlendAdditive (GL_State.cpp); cuando algún caller pasaba
// por ESTA copia, el cache quedaba en "depth write ON" mientras el GL real estaba
// en OFF → el EnableDepthMask cacheado de DisableAlphaBlend se volvía no-op → el
// objeto opaco siguiente renderizaba con mask=0, no escribía depth, y geometría
// más lejana lo tapaba. El source 5.2 (ZzzOpenglUtil.cpp:468) confirma que
// EnableAlphaBlend usa el DisableDepthMask CACHEADO, nunca glDepthMask directo.
// Fix: delegar a la versión canónica cacheada (idéntico address 0x00511710).
```

### Línea 55 — antes de `void __cdecl EnableAlphaTest(bool enable) {`

```cpp
// EnableAlphaTest @ 0x00511680 — GL standard alpha blend + alpha test.
// 2026-06-29 BUG-FIX (mismo desync de cache que EnableAlphaBlend): esta copia
// llamaba glDepthMask(1)/glDisable(GL_CULL_FACE) DIRECTOS sin tocar los caches.
// El source 5.2 (ZzzOpenglUtil.cpp:443) confirma que EnableAlphaTest(DepthMask)
// usa el EnableDepthMask CACHEADO condicional. Fix: delegar a la versión canónica
// cacheada GL_SetBlendSrcOver (idéntico address 0x00511680; param = flag DepthMask).
```

### Línea 66 — antes de `// AccessModelWithTextures - DESVIACION DEL PORT, no existe en IDA.`

```cpp
// Linker stubs — external functions called by OpenNpc/RenderEquipment3D/RenderItems3D
// These are placeholders until the actual implementations are decompiled.
// ═══════════════════════════════════════════════════════════════════════════════
```

### Línea 70 — antes de `void __cdecl AccessModelWithTextures(int id, char* path, char* name, int param) {`

```cpp
// AccessModelWithTextures - DESVIACION DEL PORT, no existe en IDA.
//
// Envuelve a AccessModel (0x005060B0, el loader BMD crudo) y le agrega los dos
// pasos que el port necesita para que un NPC quede utilizable: cargar su
// textura y sembrar las velocidades de animacion.  Los ~42 call sites que la
// usan son los que antes llamaban al nombre AccessModel cuando el loader crudo
// todavia se llamaba FUN_005060b0.
//
// 2026-09-25: hasta el renombrado esta funcion SE llamaba AccessModel y convivia
// con FUN_005060b0.  Al renombrar el loader crudo a AccessModel las dos quedaron
// como sobrecargas (char* vs const char*), functions.h solo declaro la del loader
// y este puente quedo muerto: los NPC cargaban su BMD pero sin velocidades de
// animacion, o sea congelados -- y el herrero, cuyo sonido se dispara por rango
// de frame, lo reproducia en loop.  Ver [[simbolo-duplicado-patron]].
//
// 2026-05-05: AccessModel era stub vacio -> ningun BMD de NPC se cargaba.
// Solo el guardia (type=249) renderizaba porque usa player model 390 ya
// cargado. Los demas NPCs (Storage, Smith, Wizard, etc.) llamaban a
// AccessModel(0x149, "Data\\Npc\\", "Storage", 1) etc pero el modelo nunca
// cargaba -> invisible.
//
// 2026-05-05 (followup): ademas llamar OpenTexture post-BMD load. Sin esto los
// NPCs cargaban geometria pero las texturas no se resolvian en los slots
// (IndexTexture[]) -> render en blanco. El cliente original si hace este paso
// despues del BMD load para NPCs.
```

### Línea 102 en `AccessModelWithTextures` — antes de `int slotBase = DAT_05828d58 + id * 0xbc;`

```cpp
    // 2026-05-05: setup de animation speeds (idéntico al patrón que
    // OpenMonsterModel hace para monsters). Sin esto, los NPCs cargan
    // geometry/textures pero entity[+0x105] action speed = 0 →
    // CharacterAnimation no avanza el frame → NPCs estáticos.
    //
    // CharacterAnimation lee de model+48 (=bones table per BMD__Open
    // alloc) con stride 16 bytes. Esa tabla tiene `numBones` entries de 0x10
    // bytes c/u. Para evitar buffer overflow (crashes vimos con NPCs de
    // pocos bones), solo escribir speeds hasta el límite de bones disponibles.
```

### Línea 250 en `RenderItem3D` — antes de `if (!resolved) {`

```cpp
    // 2026-08-11 — REMOVIDO: bloque inventado por el port (el comentario original
    // decía que estos items "expect to be centered ... not biased downward",
    // o sea una heurística a ojo, no un decompile). Forzaba ofsYmul = 0.50 para
    // 416-419/428/429 y, al no llevar guard `!resolved`, PISABA el valor correcto
    // de IDA para el rango [416,448) que asigna la rama de arriba (0.50/0.70).
    // Efecto: la Uniria (tipo 418) se anclaba en el centro de la casilla en vez
    // de al 70% → se veía más arriba que en el original. IDA `RenderItem3D`
    // (0x4E1BE0): `if (Type >= 416 && Type < 448) { _sx += W*0.5; _sy += H*0.7; }`
    // sin ninguna excepción para esos tipos.
```

### Línea 269 en `RenderItem3D` — antes de `case 459:`

```cpp
            // 2026-08-11 FIX: el 460 estaba agrupado con 465-467 (0.5/0.5 fijo)
            // y el 459 tenía ramas inventadas (lvl3 13/14/15) que no están en el
            // 0.97k.
```

### Línea 306 en `RenderItem3D` — antes de `}`

```cpp
            // 2026-08-11 — REMOVIDOS los casos 475/476/477/478/479: no existen en
            // el 0.97k. En IDA caen al final de la cadena y, por estar dentro de
            // [448,480), terminan en `goto LABEL_90` = 0.50/0.95 (el mismo
            // fallback de más abajo). Los valores que había (0.90 / 0.5-0.5 /
            // 0.55-0.80) eran invenciones del port.
```

### Línea 320 en `RenderItem3D` — antes de `if (!resolved && Type >= 448 && Type < 480) {`

```cpp
    // 2026-08-11 FIX: el rango era [448,512), así que los tipos 480-511 tomaban
    // 0.95 cuando en IDA les corresponde 0.60.
```

### Línea 402 — antes de `void __cdecl MoveObject_Special(int a1)`

```cpp
// Batch 21 — helper function stubs (called by MoveObjects, CollisionDetectLineToMesh, CheckMixRecipe)
// MoveObject_Special (IDA-activated, was Ghidra stub)
```

## `src/Render/Render_LegacyRotatedRect.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 67 en `GL_DrawRotatedRect` — antes de `static const float uvs[8] = { 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f };`

```cpp
    // IDA sub_5126E0: esquinas (-w/2, h/2), (-w/2, -h/2), (w/2, -h/2),
    // (w/2, h/2) con UV (0,0), (0,1), (1,1), (1,0).  2026-09-12: el port las
    // tenia en orden cruzado (+,+ / +,- / -,+ / -,-), asi que el TRIANGLE_FAN
    // salia como un mono y el martillo animado del cursor se veia roto.
```

## `src/Render/Render_LegacyTrails.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 54 en `Entity_TeleportAnim` — antes de `char *slot = &DAT_07c80110[0];`

```cpp
             // 4-arg overload in stubs.cpp:3324 (Entity_TeleportAnim with float* dst_pos).
```

### Línea 73 — antes de `void __cdecl Particle_Update(void)`

```cpp
// Particle_Update @ 0x0046C3E0 — Trail_RenderAll: render weapon/beam trails in pool.
// Pool: g_RenderPool_07c608a8 (= shared joint/trail pool, 100 slots × 0x2f0).
// 2026-05-03: AUTO-SKIP removed. Pool now properly sized; iteration count
// is 100 (matching IDA bound `< 0x7c72e74` = base + 100*0x2f0).
```

## `src/Render/Render_LegacyTransforms.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 151 — antes de `void *__cdecl Entity_InitRenderState(void *a1)`

```cpp
// Entity_InitRenderState @ 0x004FF580 — Entity_InitRenderState(entity)
// Scans render-state pool at DAT_083a2370 (stride 0xc, 128 slots).
// Finds first free slot (byte[0]==0), marks it active and stores entity ptr.
// Entity_InitRenderState (IDA-activated, was Ghidra stub)
```

## `src/Render/Render_MatrixStack.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 11 — antes de `unsigned int __cdecl GL_PopMatrixAll(void) {`

```cpp
// ── GL_PopMatrixAll ───────────────────────────────────────────────────────────
// BUG-FIX CRÍTICO: antes hacía un solo glPopMatrix() asumiendo modo actual.
// Pero GL_BeginViewport (GL_SetupView) pushea DOS matrices (PROJECTION + MODELVIEW),
// y Scene_Login sólo hace un glPopMatrix antes de Begin2D. Resultado: cada frame
// quedaba un push acumulado en PROJECTION → stack overflow tras 2 frames →
// matrices corruptas → UI 2D invisible. Aquí forzamos reset completo de ambos
// stacks a identidad. Los glGetError() limpian el GL_STACK_UNDERFLOW que
// generan los pops sobrantes (son inocuos, sólo setean el flag de error).
```

## `src/Render/Render_PlayerEquipment.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Render_PlayerEquipment.cpp
//
// Port directo del IDA RenderCharacter (sub_456770) líneas 1267-1981 — render
// de Helper, Wing y Weapons del player. Este bloque se llamaba en Entity_UpdateRender
// pero estaba parcialmente portado (solo wing+algunos weapons), faltaba:
//   - Render de Helper (819 = pet hada / 817 = otro)
//   - Loop principal de armas (v234=0..1) que dibuja Weapon[0] y Weapon[1]
//   - 30+ weapon-specific particle effects (glows de espadas/staffs/bows)
//
// 2026-05-04: agregado port directo de `CWeaponView::RenderCharacterBackItem`
// (DLL source `Mu-linux-97K/Source/Client/Main/WeaponView.cpp:49`) que decide
// si el arma se renderiza en la ESPALDA (LinkBone 47) o en la mano. Conditions:
//   - safe-zone (entity+0x34E set por terrain bit 0)
//   - greeting anim (93..124)
//   - Atlans swim (World 7, anim 21/29)
//   - NOT Blood Castle (World 11..16)
// Cuando bBindBack=true: weapon → bone 47 (espalda), Render_PlayerWeaponLoop
// se SALTA. Cuando false: Render_PlayerWeaponLoop renderiza en mano (default).
//
// Mapping de offsets (entity = c, model = o):
//   c+0x270 = Weapon[0] slot (24 bytes: model/lvl/opt/bone/action/...)
//   c+0x288 = Weapon[1] slot
//   c+0x2A0 = Wing slot
//   c+0x2B8 = Helper slot
//   c+0x108 = float AnimationFrame
//   c+0x105 = byte CurrentAction
//
// Models[player_type=390] action table:
//   Models + 188*390 + 48 = pointer a Actions[]
//   Actions[N].PlaySpeed @ +4 dentro del struct (16 bytes/action)
//
// API mapping IDA → nuestra:
//   CreateSprite(type, pos, scale, color, owner, alpha, mode) → CreateSprite
//   CreateJoint(type, p0, p1, color, flag, owner, scale, ?, mode) → Joint_Create
//   Particle_Spawn(type, pos, size, color, flag, alpha, mode) → Particle_Spawn
//   TransformPosition(model, mat3x4, pos_in, pos_out, translate) → BMD_TransformPosition
//   sub_4553C0(model, type, bone, scale, color, owner) → Model_BoneParticle
//
// Anti-tamper hash-table operations (líneas IDA 1290-1505) elididas — pure
// obfuscation por CLAUDE.md, no afectan render.
```

### Línea 48 — antes de `extern "C" void EquipWipe_Tick(int op, int sub)`

```cpp
// ── Diagnostico EQUIPWIPE, version dirigida ───────────────────────────────
//
// El detector que vive mas abajo (en HeroEquipWatchdog) avisa QUE los 12 wear
// slots de CharacterMachine pasaron a -1 de golpe, pero no QUIEN lo hizo: corre
// en el render, frames despues del escritor.  Como los wipes observados caen
// junto a cambios de mapa, el sospechoso es un handler de red, asi que esto se
// llama una vez por paquete desde Net_ProcessPacket y nombra el paquete que
// estaba en curso.
//
// Reporta el opcode de la pasada ANTERIOR a proposito: si el wipe lo produce el
// paquete N, esta funcion lo ve recien en la pasada N+1.
//
// Es barato (12 lecturas por paquete) y queda permanente hasta encontrar al
// escritor -- lleva sin aparecer desde 2026-08-08.
```

### Línea 163 en `CheckFullSet` — antes de `EquipmentLevelSet = 0;`

```cpp
            // 2026-08-08 FIX (glow pegado al desequiparse la armadura): acá el
            // port devolvía `true` ("tiene las 5 piezas aunque no matcheen").
            // En IDA ese camino es un `break` del while EXTERNO, y justo
            // después del while está `v26 = 0;` — o sea devuelve **false**.
            // El único camino que deja `v26 = 1` es el `goto LABEL_15` de
            // arriba (set completo Y todos los niveles >= 9).
            //
            // Por qué se notaba al desequiparse: al sacar la armadura las
            // body-parts de la entidad NO quedan en 0xFFFF sino en el modelo
            // por defecto de la clase, así que el while externo recorre las 5
            // piezas y entra acá; con niveles 0 el while interno no corre y
            // caía en este `return true` → `v230 = true` en el case 0x186 de
            // Entity_UpdateRender → se seguía ejecutando la sección 5
            // (PartObjectColor + 6 sprites en los huesos del arma) = el glow.
            // Un pj que nunca tuvo armadura sale antes por el while externo
            // (alguna pieza en 0xFFFF) y por eso se veía normal.
```

### Línea 231 en `RenderWeaponFX` — antes de `short Type = *(short*)v121;`

```cpp
    // BUGFIX 2026-09-01: aca habia `float WorldTime = (float)DAT_05826e08;`, pero
    // `WorldTime` es un MACRO a DAT_05826e08 (structs.h:438), asi que declaraba un
    // local que se sombreaba a si mismo y quedaba con basura (C4700): los cases que
    // animan con sin(WorldTime * ...) usaban tiempo random.  Removido: los usos de
    // abajo ya resuelven al global por el macro.
```

### Línea 489 — antes de `extern "C" void HeroEquipWatchdog(int c)`

```cpp
// Llamada por Entity_UpdateRender al inicio del render del hero in-game.
// Restaura equipment slots si fueron borrados.
```

### Línea 500 en `RenderWeaponFX` — antes de `{`

```cpp
        // ── 2026-08-08: RE-SIEMBRA DESDE EL STASH — REMOVIDA ─────────────────
        // Era el último resto del watchdog: si un wear slot de CharacterMachine
        // estaba en -1, lo rellenaba desde `g_HeroEquipStash_*`. O sea disparaba
        // EXACTAMENTE al desequipar → el item volvía a aparecer en su caja
        // (pants que "siguen equipados", escudo que se dibuja con un arma a dos
        // manos, etc.).
        //
        // Validado contra IDA: en el binario NADA re-siembra CharacterMachine
        // por frame. El equipo lo escribe sólo el server —  F3/10
        // (ReceiveInventory), el ack del move 0x24 y los acks de equipar —, y
        // el render (`RenderEquipment3D` 0x4E3100) lo lee directo. Este watchdog
        // era 100% invención del port para tapar que el equipo se reseteaba
        // después del F3/03.
        //
        // Si el reseteo tras F3/03 vuelve a aparecer, el diagnóstico de abajo
        // (EQUIPWIPE) lo registra: es el bug real a arreglar, no a tapar.
```

### Línea 528 en `RenderWeaponFX` — antes de `{`

```cpp
        // ── 2026-08-08: RE-SIEMBRA DESDE EL INVENTARIO — REMOVIDA ────────────
        // Acá había un loop que, para cada slot de equipo VACÍO en
        // CharacterMachine, lo rellenaba desde `((ITEM*)OffsetInventoryItems)[slotIdx]`
        // con slotIdx = 0..11. Eso es memoria EQUIVOCADA por construcción:
        // `OffsetInventoryItems` es el pool del grid 8×8 y su índice de celda es
        // `slotIdx - 12` (ver AddItemToGrid:396) — o sea `[0..11]` son las
        // CELDAS 0..11 del grid visible (la primera fila y media del
        // inventario), NO los wear slots. Los wear slots viven sólo en
        // `CharacterMachine + 536 + 68*slot`.
        //
        // Consecuencias que explicaba, las dos reportadas por el usuario:
        //  · Render equivocado: la celda 1 del grid (p. ej. un item de mascota,
        //    tipo 418) se copiaba al slot 1 = caja del ESCUDO → el escudo se
        //    dibujaba como Uniria (modelo = Type+400 = 818). Las celdas 9/10/11
        //    caían en Ring1/Ring2/Pendant → "los anillos figuran como guantes".
        //  · El "clon" al levantar un item equipado: UI_Main limpiaba el slot y
        //    este loop lo volvía a llenar al frame siguiente con lo que hubiera
        //    en la celda del grid — de ahí el "a veces sale otro item".
        //
        // (El stash `g_HeroEquipStash_*` se borro el 2026-09-18: no tenia lectores.)
        // IDA/source base path: let SetCharacterClass rebuild the world hero
        // from CharacterMachine, instead of keeping a partial local mirror.
        //
        // ── 2026-08-15: el rebuild ya NO corre por frame ─────────────────────
        // `SetCharacterClass` (0x45C130) termina cancelando la animación en
        // curso cuando la acción está fuera de [0x22, 0x5B]:
        //     if (!(v11 >= 0x85 && v11 <= 0x8C) && (v11 < 0x22 || v11 > 0x5B))
        //         SetPlayerStop(c);
        // La caminata es la acción 13 (0x0D) → entraba SIEMPRE. Como este
        // watchdog corre en cada frame del render del hero, el ciclo por tick
        // era: acción 13 → SetPlayerStop pone 1 (frame=0) → SetPlayerWalk la
        // devuelve a 13 (frame=0 otra vez). El frame nunca pasaba de 0.3 y la
        // caminata se veía "mueve un pie y se resetea".
        // Medido con el probe FRAMEDBG (2026-08-15):
        //     act=13 spd=0.300 f=0.000->0.300   ← en CADA tick
        // mientras que las acciones de idle (1 y 9), que no pasan por este
        // camino, progresaban normal (0.28 → 0.56 → 0.84 … loop en nF=6).
        //
        // En IDA `SetCharacterClass` se llama sólo cuando CAMBIA el equipo
        // (ReceiveAddPoint / ProtocolCore / char-select), nunca por frame.
        // Reproducimos eso: rebuild sólo si los wear slots (o la entidad del
        // hero) cambiaron respecto del frame anterior.
```

### Línea 621 en `IsBackItem` — antes de `if (SceneFlag != 5)`

```cpp
    // DESVIACION DEL PORT (2026-05-04, conservada): gate por state=5 (in-game).
    // IDA no lo tiene.  Sin el, char-select dibujaba el arma dos veces.
```

### Línea 647 en `IsBackItem` — antes de `bool bBack = false;`

```cpp
    // ── Que items van a la espalda ──────────────────────────────────────────
    // PORT DEL DLL (CWeaponView::RenderCharacterBackItem, WeaponView.cpp:49),
    // 2026-09-01.  Este main.exe cuelga UN SOLO item: verificado instruccion por
    // instruccion en 0x458370-0x4584C0 --
    //     004583bb  LEA   ECX, [EAX+EAX*2+0x4E]      ; 3*Hand + 78
    //     004583c7  MOVSX EBX, word ptr [EDI+ECX*8]  ; un unico Type
    //     0045849f  CALL  0x00455430                 ; una unica llamada
    //     004584c0  ...                              ; cae a LABEL_308, sin salto atras
    // -- y elegia la mano con un `Hand` que solo pasa a 1 con arco (528..534,
    // 545) o flechas (535), asi que el escudo no se dibujaba nunca.
    //
    // El cliente de referencia SI muestra los dos, y la funcion que lo hace esta
    // en el DLL de inyeccion: itera los dos slots (igual que MU 5.2, cuyo
    // RenderLinkObject ademas tiene un parametro `bRightHandItem` que el de
    // 0.97k no tiene) y parchea 0x0045568B con una tabla de poses por tipo de
    // escudo mas un offset para el segundo item.  Portado: el bucle aca, la
    // tabla en RenderLinkObject.
    //
    // Weapon[0] = c+624 (0x270), Weapon[1] = c+648 (0x288).
```

## `src/Render/Render_SpriteHelpers.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Render_SpriteHelpers.cpp
//
// Extracted from stubs_game.cpp.  Owns the additive blend setup and sprite/
// digit-atlas draw helpers.  Function comments retain IDA provenance.
```

### Línea 11 en `GL_SetBlendInvSrcColor` — antes de `if (DAT_083a412c != 5) {           // AlphaBlendType`

```cpp
    // 0x00511810 — OpenGL additive blending (ONE_MINUS_DST_COLOR, ONE)
    // 2026-04-30 BUG-FIX: previously cached state in DAT_07eaa160/164/168
    // — those addresses are CheckInventory + adjacent ITEM ptrs, NOT GL
    // state.  IDA shows the real cache is at DAT_083a412c (AlphaBlendType),
    // DAT_083a411d (AlphaTestEnable), DAT_083a4125 (TextureEnable).
    // Writing 2/3/5 to CheckInventory was crashing Scene_MapTick when it
    // dereferenced CheckInventory as ITEM*.
```

### Línea 101

```cpp
//
// 2026-09-26: aca habia una copia bajo el nombre RenderNumber2D.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.
```

## `src/Render/Render_WorldHelpers.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Render_WorldHelpers.cpp
//
// Formerly stubs_render_helpers.cpp.  This module owns the world-render
// helpers called from Render_Scene3D and adjacent render passes.
//
// IDA provenance is intentionally retained at every entry point as
// `FUN_XXXXXXXX @ 0xXXXXXXXX`.  Functions are not renamed until their
// contracts are confirmed against the 0.97k binary; do not use 5.2 code to
// infer behaviour.
```

### Línea 38 — antes de `void __cdecl RenderBoids(void)`

```cpp
// 2026-04-28: minimal-impl stubs so we can wire them up in Render_Scene3D without
// link errors. Each will be ported per-IDA when the corresponding pool/entity
// system gets activated.
//
```

### Línea 63 en `RenderBoids` — antes de `if (!DAT_05828d58) return;`

```cpp
    // 2026-09-03 -- BOIDS QUE NUNCA SE DIBUJABAN.
    // IDA RenderBoids (0x00500AA0) arranca con `v0 = (float *)dword_839BE18` y
    // recorre `v0 += 111` hasta `&unk_83A0378`, o sea 40 slots de 444 bytes
    // anclados en +0x168 del slot: `v0 - 90` floats = la base del slot,
    // `*((BYTE*)v0 - 360)` = el flag de activo y `*((WORD*)v0 - 179)` = el tipo.
    // Ese 0x0839BE18 NO es un puntero suelto: es el campo +0x168 del slot 0 del
    // mismo pool que ya vive en `g_WeatherSlotPool` (base 0x0839BCB0, y
    // 0x839BE18 - 0x839BCB0 = 0x168).  El port lo habia dejado como un DWORD
    // aparte inicializado en 0, asi que este `return` se tomaba SIEMPRE y las
    // criaturas voladoras del mapa no se dibujaban nunca -- se nota sobre todo
    // en Atlans e Icarus, que son los dos mundos donde `Weather_Update` las
    // spawnea (tanto que el DLL de inyeccion NOPea esas dos ramas en 0x00501292
    // con el comentario "Fix Atlans and Icarus Goldens Overflow").
```

### Línea 104 en `RenderBoids` — antes de `if (entType == 301 && *((int*)v0 - 89) == 1) {`

```cpp
                // El port llamaba en su lugar a PartObjectColor (0x503CF0),
                // que es OTRA funcion, con otra firma y otra semantica -- el
                // comentario decia "Hero body color".  Resultado: el pase 0x48
                // nunca corria y el dragon dorado salia con el render normal,
                // o sea rojo.  Reportado 2026-09-29: "no aparecen dorados ni
                // con la textura correcta".
```

### Línea 145 en `RenderBoids` — antes de `if (entType == 175) {`

```cpp
                // Type 175: random fire-cloud sparkle.
                //
                // IDA RenderBoids L61:
                //   CreateSprite(1150, v0 - 86, 1.0, v14, (DWORD)(v0 - 90), 0.0, 0);
                //
                // El port llamaba a Particle_Spawn, que es OTRA funcion con
                // otra firma, y para que los argumentos entraran metia un
                // nullptr como Position pasando la posicion real en el slot
                // del Angle.  Particle_Spawn hace `*param_2` sin guard, asi
                // que esto crasheaba leyendo la direccion 0 apenas aparecia
                // una entidad de tipo 175 (reporte: al entrar a Noria).
```

### Línea 172 en `RenderBoids` — antes de `BMD_TransformPosition(model, (float*)&DAT_06970acc, locOffsetL, Position, 1);`

```cpp
                    // IDA L80-95: TransformPosition(v4, flt_6970ACC, ...) y
                    // CreateSprite(1150, Position, 0.1, Light, owner, 0.0, 0).
                    // Mismos dos errores que en el caso 175, mas un nullptr
                    // como matriz de hueso: Vector_Transform la deferencia, o
                    // sea era otro crash latente.  flt_6970ACC es nuestro
                    // DAT_06970acc (g_BoneScratch + 0x30).
                    // Left jet
```

### Línea 213 — antes de `void __cdecl Entity_Render(void)`

```cpp
// 2026-08-21: acá había una tabla local inventada con valores 0..99, así que
// `% 360` daba ángulos de sólo 0..99° → las monedas salían en una cuña en vez
// de en círculo, y siempre en el mismo patrón.
```

### Línea 269 en `Entity_Render` — antes de `BMD_Animation(model, (int)&DAT_06970a9c,`

```cpp
                // AnimationFrame / PriorFrame: IDA los toma en v0+3 y v0+7 —
                // offsets de BYTE (disasm 0x5039E9/0x5039ED: `mov ecx,[esi+7]`,
                // `mov edx,[esi+3]`).  2026-08-21: el port usaba v0+12 y v0+28,
                // que es la misma confusión float*/BYTE* que el Alpha de arriba.
```

### Línea 307 en `Entity_Render` — antes de `RenderPartObject((int)v1, type, 0, Light, *(float*)(v0 + 99),`

```cpp
                        // 2026-08-21: el port tenía v0 + 396 (Items+729).
```

### Línea 332 en `Entity_Render` — antes de `RenderPartObject((int)v1, *(short*)(v1 + 2), 0, Light,`

```cpp
                // 2026-07-27 FIX (item del suelo renderizaba mal, "árbol"):
                // el IDA (L161-163) pasa `*((short*)v1 + 1)` = v1+2 = el TYPE
                // del entity (= model del item, ej 662) a RenderPartObject.
                // El port usaba v1+4 (= el flag "1" que escribe CreateItem en
                // ip+76) → renderizaba el modelo equivocado.
```

### Línea 346 en `Entity_Render` — antes de `{`

```cpp
                // 2026-07-27 FIX: proyectar la posición del item a pantalla y
                // guardarla en v1+92/94 (= word idx 46/47, IDA Entity_Render
                // L182-183). RenderItemName lee esa pos en o+0x5c/0x5e (= base+
                // 164/166 = v1+92/94) para dibujar el nombre SOBRE el item. El
                // port la zereaba en v1+184 (offset equivocado) → nombre en
                // (0,0). Camera_ProjectWorldToScreen (World_ToScreen) sí está implementado.
```

### Línea 392 en `ItemDrop_Render` — antes de `const int kEntries = (int)(sizeof(DAT_07b27150) / 0x9d8);   // 200`

```cpp
    // El binario itera de &unk_7B27B08 a &unk_7C5B4E8 con stride 0x9d8 = 500
    // entradas; nuestro pool DAT_07b27150 está dimensionado a 200 slots, así que
    // iteramos los que entran (antes eran 84 fijos → los trails de los slots
    // 84..199 nunca se dibujaban).
```

### Línea 405 en `ItemDrop_Render` — antes de `bool useMinus = (type == 1253 || type == 1250);`

```cpp
        // Sonda temporal, estrictamente acotada: el artefacto de Icarus se
        // manifiesta al activar aura; el log demostró que el candidato visible
        // en esa zona es el joint de alas 1254/subtipo 14. Antes de
        // tocar la inicializacion de esos slots, capturamos la geometria que
        // el renderer recibe realmente, no la que un creador supone haber
        // escrito.
```

### Línea 420 en `ItemDrop_Render` — antes de `if (useMinus) GL_SetBlendSrcAlpha(); else GL_SetBlendAdditive();`

```cpp
        // 2026-08-10 FIX (picos duros / líneas negras de los joints): el IDA
        // llama `EnableAlphaBlend()` en la rama else, que es **0x00511710 =
        // GL_SetBlendAdditive** (blend tipo 3). El port llamaba `GL_SetBlendSrcOver`
        // (GL_SetBlendSrcOver, alpha normal SRC_ALPHA/ONE_MINUS_SRC_ALPHA +
        // depth-mask ON). Con alpha normal los téxeles OSCUROS de la textura de
        // glow se pintan negros y opacos en vez de no sumar nada → el rayo
        // aparecía como una forma sólida de bordes duros (picos azules del MG)
        // y como líneas negras (mago). `EnableAlphaBlendMinus()` = 0x00511790,
        // que sí estaba bien.
```

### Línea 463 en `ItemDrop_Render` — antes de `{`

```cpp
        // 2026-08-08 GUARD (crash 0xC0000005 dentro del driver GL, llamado desde
        // acá): segCount sale del slot del pool; si un slot queda con basura, el
        // loop avanza v6 de a 12 ints sin techo y termina pasándole a
        // glVertex3fv un puntero fuera de todo lo mapeado. El binario original
        // tampoco acota, pero acá el pool se corrompe por otros bugs de port, así
        // que clampeamos al espacio de datos del propio slot (cada entrada son
        // 630 ints; desde v0-599 entran (630-599+31)/12 segmentos con margen).
```

### Línea 582 — antes de `namespace {`

```cpp
// 2026-05-07: NPC interaction packet helpers
// =============================================
// Wire formats per server source Mu-linux-97K/Source/MuServer/GameServer/
// {NpcTalk.h, ItemManager.h, Warehouse.h}.
// Cada send: plain C1 + chain XOR forward + MuEmu byte XOR + raw socket send.
```

### Línea 740 — antes de `extern "C" SIZE* __cdecl RenderCenteredText(int iPos_x, int iPos_y, const char* pszText);`

```cpp
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/004CB6F0_sub_4CB6F0.c.
```

### Línea 763 en `RenderMonsterName` — antes de `if (SceneFlag != 5) return;`

```cpp
    // 2026-05-07: solo activo in-world. CharSelect tiene su propio path con
    // entity pool poblado de chars; queremos que Target_Render solo procese
    // mob/NPC/player hovers en el mundo de juego.
```

### Línea 768 en `RenderMonsterName` — antes de `extern void __cdecl RenderItemName(int, DWORD, int, int, bool);`

```cpp
    // 2026-09-21: reordenada segun el flujo de IDA (sub_4CB6F0).  El port
    // dibujaba PRIMERO todos los nombres de items y despues el del monstruo.
    // RenderItemName deja el glColor del ultimo item (IDA tampoco lo
    // restaura), y como el texto sale como m_dwTextColor x glColor, el nombre
    // del monstruo heredaba el color de ese item.  Reporte del tester: "el
    // nombre de los monsters cambia de color segun el ultimo item pickeado,
    // solo con el Alt activado" -- con Alt se dibujan todos, de ahi el "solo".
    //
    // Flujo real: PASO 1 dibuja UNO solo, por prioridad, con el glColor todavia
    // en blanco; PASO 2 (LABEL_39) recien ahi los items de Alt.
```

### Línea 817 en `RenderMonsterName` — antes de `const DWORD savedBack = SetBackgroundTextColor;`

```cpp
                // Monstruo: el nombre va arriba del todo, centrado.
                //
                // 2026-09-21, fix del DLL: IDA pone el fondo en rojo oscuro
                // (0xFF000064; el formato es ABGR) y el texto en celeste, y NO
                // los restaura.  Como el bucle de Alt (LABEL_39) viene justo
                // despues, en el original los nombres de items del suelo se
                // ponen rojos mientras se apunta a un monstruo.  Antes no se
                // veia porque el port dibujaba los items antes que el monstruo.
                // El DLL lo tapa en su hook de esta rama (HealthBar.cpp,
                // DrawPointingHealthBar en 0x004CB7AD): despues del nombre hace
                // `SetBackgroundTextColor = Color4b(0,0,0,0)`.  Aca se restaura
                // el valor ANTERIOR en vez de forzar 0, para que los items
                // queden igual que cuando no se apunta a nada.
```

### Línea 834 en `RenderMonsterName` — antes de `RenderCenteredText(GetScreenWidth() / 2, 10, name);`

```cpp
                // IDA LABEL_35: `RenderCenteredText(v13 / 2, 10, v3)`, con v13
                // del MISMO arbol que GetScreenWidth (0x4CB520): 260 con
                // inventario + panel lateral, 450 con cualquier panel, 640 sin
                // ninguno.  (2026-08-22: aca habia un criterio inventado que
                // leia CharacterAttribute + 0x14E como "inventario abierto".)
```

### Línea 869 — antes de `void __cdecl RenderFishs(int /*unused*/, int /*unused*/, int /*unused*/, int /*unused*/)`

```cpp
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/00502200_RenderFishs.c.
```

### Línea 1029 en `EffectPool_RenderAll` — antes de `if (!DAT_05828d58) return;`

```cpp
    // BUG-FIX 2026-05-01: HeadAngle (0x07B11698) está en offset +40 dentro del
    // effect pool DAT_07b11670 (200 entries × 0x1bc bytes = 0x1bc stride = 444B).
    // En IDA: HeadAngle iter es float*, offsets negativos cubren la cabecera del
    // entry. v0 inicia en (float*)(pool + 40), recorre 200 entries de 111 floats.
```

### Línea 1060 en `EffectPool_RenderAll` — antes de `if (type == 238 || type == 243) {`

```cpp
        // ── 2026-08-15: CAUSA DE LOS "CUADROS BLANCOS" ───────────────────────
        // El binario NO manda todo el rango 190..268 a Entity_PrepareRender: su
        // switch (IDA L110-137) aparta cuatro tipos ANTES de caer al rango:
        //     case 238: case 243:  break;                  // no se dibujan
        //     case 239:  RenderWheelWeapon(o);             // renderer propio
        //     case 244:  sub_46B980(o);                    // renderer propio
        //     default:   if (190 <= t < 269) Entity_PrepareRender(o);
        // Esos cuatro slots NO tienen modelo cargado por `OpenSkills` (que sólo
        // llena 190-237, 240-242, 245-255, 259, 266-268), asi que dibujarlos
        // como entidad normal produce un quad sin geometria/textura = el
        // cuadrado blanco que se ve al lanzar skills.
```

### Línea 1168 — antes de `bool __cdecl MoveMainCamera(void) {`

```cpp
// MoveMainCamera @ 0x00524CB0 — MoveMainCamera  (port 1:1 desde IDA, 2026-06-27)
// Setea los parámetros de cámara que consume Camera_SetupFrustum:
//   CameraFOV = 35.0  (antes el port no lo seteaba → quedaba stale 45/55/10)
//   CameraViewFar = 2000 (o 3200 en topview)
//   CameraDistance = 1000 + smoothing (CameraDistanceTarget)
//   CameraPosition vía AngleMatrix(CameraAngle)+VectorIRotate del offset (0,-1000,0)
//   CameraAngle[0] = EarthQuake - 48.5  (pitch SET después de la posición)
// Símbolos IDA: CameraTopViewEnable=CameraTopViewEnabled, CameraDistance,
//   CameraDistanceTarget. Retorna 0 (no-spectator) como IDA.
// Sin force-yaw ni DIAG: el yaw lo preserva el estado de cámara, igual que IDA.
```

### Línea 1223 — antes de `void __cdecl Resource_LoadOrFatal(char* param_1) {`

```cpp
// Resource_LoadOrFatal @ 0x00406F50 — Resource_LoadOrFatal(filename).
// Original: calls Resource_Load (Resource_Load). On failure: shows "IError"
// MessageBox + Window_FatalError to terminate.
//
// PORT FIX (2026-04-25): the resource manager context (DAT_083bbb14) is never
// initialized in our port — Resource_Load always returns 0, which would make
// every caller fatal-error. The most visible offender is Game_SceneUpdate.cpp
// case 0x14 (post-login Character list ready) which passes the username
// "tester" as a filename → IError MessageBox blocks user from ever reaching
// char-select, which is what the user reports happens "siempre".
//
// Neutralized: still calls Resource_Load (so any future side effects remain
// once the manager is wired up) but suppresses the modal + fatal exit. Once
// resource loading is fully ported this guard can be removed.
```

## `src/Render/SMD_Legacy.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_bulk_misc.cpp.
```

## `src/Render/SMD_Parser.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// SMD_Parser.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 4346-5298 (953 lines).
//
// SMD (Half-Life skeletal model format) parsers and SMD2BMD converters.
// El cliente 0.97k usa BMD compresado para distribución, pero el código
// de loader SMD original está aquí como referencia + fallback path.
//
//   ParseNodes / ParseSkeleton / ParseTriangles — SMD section parsers
//   FUN_0040b350 — SMD tokenizer (read next token)
//   FixupSMD     — post-process skeleton + triangles
//   SMD2BMDModel / SMD2BMDAnimation — convert SMD parsed data to BMD slots
//   OpenSMDFile (probe stub)
```

### Línea 44 — antes de `extern bool __cdecl OpenSMDFile(char *FileName, int Type, bool Flip);  // C++ impl, line 1`

```cpp
// ── SMD parser stubs ─────────────────────────────────────────────────────────
// Los archivos SMD (Half-Life skeletal model format) NO se distribuyen con el
// BUG-FIX 2026-05-03: previously this stub returned false unconditionally,
// shadowing the real implementation at line 18532 which has the parser logic.
// The shadow happened because both definitions exist (C-linkage here +
// C++-linkage further below); call sites in this file resolve to the
// extern "C" stub via the forward decl at line 3943 → SMD models never load.
// Now we delegate to the parser proper. Cast the const-char arg since the
// real impl mutates the input via fopen handle but signature is `char*`.
```

### Línea 86 — antes de `// CWsctlc_Startup @ 0x0043DB30 — Net_WSAStartup(__fastcall int param_1)`

```cpp
// OpenJPG (Texture_Load OZJ), OpenTGA (OpenTGA), UnloadImage (Texture_FreeSlot)
// moved to src/Render/Texture/Texture.cpp (B3 refactor 2026-05-07, 395 lines).
```

### Línea 254 en `DeleteObjects` — antes de `memset(DAT_083a2370, 0, sizeof(DAT_083a2370));`

```cpp
            // ── Pool zero-clear loops (DESACTIVADOS) ─────────────────────────────
            // El binario original limpiaba 9 pools de partículas/efectos/entidades
            // usando direcciones ABSOLUTAS del .bss original (rangos 0x07c85890..0x83a3fe8).
            // En nuestro port:
            //   • DAT_07c85890, _0839bcb0, _07abf5f0, _07c80110, _07b27150, _07b11670
            //     son `char = 0` stubs de 1 byte — iterar con stride 0x1bc/0x70 escribe
            //     cientos de KB hacia globals adyacentes y luego en memoria no mapeada.
            //   • Los loops `(char*)0x83a2e90` y `(char*)0x7c5ab30` arrancan en literal
            //     pointers que en nuestro binario están sin commit → segfault inmediato.
            //   • DAT_083a2370 sí es un array real (0x960 bytes) pero el bound 0x83a3ae0
            //     también es absoluto.
            // Estas pools están vacías (no se llenan en login/char-select) así que
            // saltearlas es seguro hasta que migremos cada uno a símbolos con tamaño.
            //
            // 2026-09-04: la de `Operates` (DAT_083a2370) SI hay que limpiarla.
            // IDA la borra aca (`v12 = &unk_83A2370; do { *v12 = 0; v12 += 12; }`)
            // y es la lista de objetos interactuables que arma `sub_4FF580` desde
            // CreateObject.  Sin el clear, al cambiar de mapa quedan punteros a
            // objetos ya liberados y el picker (sub_4B0240) los deferencia; ademas
            // la lista se llena y los objetos del mapa nuevo no entran.
            // El array es real y esta dimensionado, asi que se acota con sizeof en
            // vez del bound absoluto del binario.
```

### Línea 282 — antes de `void __cdecl ClearCharacters(int param_1) {`

```cpp
// ClearCharacters @ 0x0045ABB0 — Entity_ClearByType(map_id)
// Loops over entity array (base DAT_07abf5d0, stride 0x394).
// For each active entity whose type (+0x1dc) != map_id: clears active flag,
// also clears matching emitter pool entries (DAT_083a1218, stride 0x1bc).
// Then calls DeleteCloth on every slot.
//
// Inner loop bound: el binario original usaba el literal 0x83a2370 (= DAT_083a1218
// + 0x1158, fin del array Butterfles). En nuestro port DAT_083a1218 es un array
// real (10 × 0x1bc = 0x1158 bytes) pero el linker lo coloca en otra dirección,
// así que el literal es basura — pcVar2 sigue iterando hasta crashear.
// Se reemplaza por DAT_083a1218 + 0x1158 (end-pointer real).
```

### Línea 313 — antes de `void __cdecl Effect_CollisionCheck(int Target) {`

```cpp
// Effect/particle
// MoveEffect @ 0x00466AD0 — MoveEffect: implemented in Render/MoveEffect.cpp
// Effect_SpawnSmokeBurst @ 0x004660F0 — Effect_SmokeBurst: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnSmokeExplosion @ 0x004661F0 — Effect_SmokeExplosion: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnLightningBurst @ 0x00460C30 — Effect_LightningBurst: implemented in Render/MoveEffect_Helpers.cpp
// Effect_SpawnProximityHit @ 0x00465E60 — Effect_OnHitProximity: implemented in Render/MoveEffect_Helpers.cpp
// Ring_ComputeOrbit @ 0x00473D90 — Ring_ComputeOrbit: implemented in Render/MoveEffect_Helpers.cpp
// STUB: Effect_AutoAttack — proximity-check all entities against param_1, fire
// attack effect (CreateBomb/CreateJoint 0x4E1) at nearby targets.
// Real logic: iterates CharactersClient[0..399], distance check <= DAT_005524f0,
// sub_466440 @ 0x00466440 — Effect/projectile collision/trigger handler.
// Port FIEL desde IDA decompile (2026-05-02). Anti-tamper hash table noise
// (CharacterMachine encrypt/decrypt wrappers) skipped per project policy.
//
// Called per-frame from MoveEffect (3 sites) and MoveJoint (1 site) when
// a projectile/effect entity is moving. Two paths:
//   A) Target[+132] != 0: targeted skill — 50% rand check, hero-only,
//      cooldown decrement (skill 52), radius 100 collision, then dispatch
//      CreateJoint (skill 52 chain) / CreateBomb (skill 51 explosion) /
//      sound + bomb (other).
//   B) Target[+132] == 0: AOE — scan CharactersClient for any non-self
//      visible entity within 100u, then dispatch similar.
//
// Used skill IDs (read from CharacterAttribute[+87 + Target[+133]]):
//   51 = explosion / bomb
//   52 = chain / joint hit
//
// Entity types triggering bomb FX:
//   223 (0xDF), 243 (0xF3) — produce CreateBomb on hit
// Effect_SpawnSmokeBurst declared in functions.h as (float*, char). Using through normal
// linkage (no extern decl needed here).
```

### Línea 483 en `Joint_BoneOffsetApply` — antes de `float out[3] = {0.0f, 0.0f, 0.0f};`

```cpp
    // PORT FIX: Ghidra decompile split a contiguous float[3] output buffer into
    // three separate locals (local_3c/38/34). MSVC does not guarantee they're
    // adjacent in memory, so Vector_Rotate (which writes 3 contiguous floats)
    // only landed in local_3c and the other two reads picked up uninitialised
    // stack. Use a proper array to guarantee contiguity. Same pattern as the
    // Terrain_Light.cpp Entity_GetLightScale fix.
```

### Línea 506 — antes de `void __cdecl AddTerrainLight(float xf, float yf, float *Light, int Range, float *Buffer) {`

```cpp
// 2026-09-25: se llamaba AddTerrainLight y convivia con un wrapper inline
// AddTerrainLight en structs.h que solo existia para castear los punteros --
// Ghidra los habia tipado como int.  Ahora la firma es la real y el wrapper se
// elimino, asi que hay un unico simbolo para esta direccion.
```

### Línea 620 en `Entity_FindNearby_SendPacket` — antes de `BYTE pkt[3 + 5 + 5 * 2];`

```cpp
    // 3. Paquete C1:1D — PMSG_MULTI_SKILL_ATTACK_RECV.
    //
    // IDA sub_45FEC0 anexa, en este orden (L228, 277, 326, 374, 427 y el bucle
    // de L476/L525):
    //     [0x1D]
    //     v111 = *(BYTE *)(CharacterAttribute + a1 + 87)   // skill
    //     v29  = (int)(a2[0] * 0.01)                       // x  (grilla)
    //     v32  = (int)(a2[1] * 0.01)                       // y
    //     a4                                               // serial
    //     LOBYTE(v109) = count
    //     por entidad:  [id >> 8][(BYTE)id]                // index[2] big-endian
    //
    // Coincide 1:1 con el server (GameServer/SkillManager.h:67):
    //     struct PMSG_MULTI_SKILL_ATTACK_RECV { PBMSG_HEAD header; BYTE skill;
    //                                           BYTE x; BYTE y; BYTE serial; BYTE count; };
    //     struct PMSG_MULTI_SKILL_ATTACK      { BYTE index[2]; };
    //
    // El port anterior mandaba [rand][serial++][a4][count][id>>8 ...]: los cinco
    // campos corridos y UN solo byte por entidad en vez de dos.  El server leia
    // `skill` = rand() -> `GetSkill()` devolvia 0 y salia por
    // CGMultiSkillAttackRecv sin aplicar dano.  Este es el paquete que cierra
    // los skills multi-objetivo (Penetration, Twisting Slash, Rageful Blow,
    // Death Stab, Hell Fire, Twister, Evil Spirit, Aqua Beam, Blast, Inferno,
    // Flame, Fire Slash): el C3:1E solo arma `MultiSkillIndex` y es el 0x1D el
    // que trae la lista de blancos y dispara gAttack.Attack().
```

### Línea 673 en `Joint_SegmentTick` — antes de `float local_18[6] = {0};`

```cpp
    // PORT FIX: Ghidra decompile produced `float local_18[4], local_8, local_4;`
    // and wrote Vector_Rotate's 3-float output at `local_18 + 3`, expecting
    // local_18[4]==local_8 and local_18[5]==local_4. MSVC doesn't guarantee that
    // layout, so local_8/local_4 reads picked up uninitialised stack. Expanding
    // the array to 6 elements makes the 3 output slots genuinely contiguous.
    // Slots used: local_18[0..2] = input vec, local_18[3..5] = output vec.
```

### Línea 689 en `Joint_SegmentTick` — antes de `char* rowBase = (char*)param_1 + 0x88 + (iVar1 - 1) * 0x30;`

```cpp
        // 2026-08-10 FIX (haces oscuros saliendo del personaje): el port había
        // COLAPSADO los dos punteros del IDA en uno solo. El original lleva
        // `v6` = base de la fila (retrocede 0x30 por segmento) y `v7` = cursor
        // que camina esa fila; `v7` se RE-INICIALIZA desde `v6` en cada vuelta:
        //     v6 = 48*(count-1) + a1 + 136;
        //     do { v7 = v6; <4 × copiar vec3, v7 += 3>; v6 -= 48; } while(--v5);
        // Nosotros hacíamos `puVar3 += 48` en el inner y después `-= 0x78`,
        // o sea un paso neto de -72 en vez de -48: el cursor se corría 24 bytes
        // por vuelta y terminaba escribiendo POR DEBAJO del array de segmentos
        // (0x58), encima de la cabecera — segCount (0x50) y segMax (0x54)
        // quedaban con floats, y el render dibujaba vértices basura = los haces.
```

### Línea 744 — antes de `float __cdecl MoveHumming(float *param_1, float *param_2, float *param_3, float param_4)`

```cpp
// MoveHumming @ 0x0043E4A0 — MoveHumming(Position, Angle, TargetPosition, Turn)
// Gira Angle hacia el target y **devuelve la distancia** al target.
// NO mueve la posicion (de eso se encarga el tick generico del joint).
//
// 2026-08-16: el retorno FALTABA. Hex-Rays la tipa `void` porque el valor sale
// en st0 y no lo detecta — el mismo artefacto de FPU que ya mordio antes. El
// source original de MU 5.2 (ZzzAI.cpp:131) lo deja explicito:
//     float MoveHumming(...) { ...; return VectorLength(Range); }
// y el consumidor lo usa como distancia (ZzzEffectJoint.cpp:3368):
//     Distance = MoveHumming(...);
//     if (Distance <= 35.f)  { o->Live = false; ... }        // absorber
//     else if (Distance <= 70.f && ...) { Velocity -= 10; }  // frenar
// Sin el retorno, `MoveJoint` case 0x4ea comparaba la Z ABSOLUTA del target
// (ownerZ + 120, siempre > 35) → las esferas de EXP nunca se absorbian y
// quedaban orbitando al personaje acumulandose.
```

### Línea 782 — antes de `// 2026-05-08 BUG-FIX (item +N glow): el wrapper estaba pre-shifting 'Level'`

```cpp
// RenderItem3D @ 0x004E1BE0 — RenderItem3D
//
// 2026-04-30: la versión anterior estaba MAL identificada como
// `ItemDrop_SpawnEffect` y llamaba `RenderObjectScreen(type+400, ...)` (RenderObjectScreen)
// con effect-ids inventados.  Para items "normales" (helmet=0x4E1, etc.) eso
// resolvía a un BMD inexistente y crasheaba en BMD_Animation con AV.
//
// El IDA companion confirma 0x004E1BE0 = RenderItem3D
// `(float sx, sy, Width, Height, int Type, Level, Option1, bool PickUp)`.
//
// Mientras no portemos el render 3D real con BMD models, redirigimos al
// placeholder 2D que vive en RenderItem3D (línea 26253 abajo) — dibuja un
// quad texturado con el icono del item type en la posición pasada.
// RenderItem3D forward decl already in functions.h (non-extern-C C++ linkage).
```

### Línea 797 — antes de `void __cdecl BMD__RenderBody(void *model, int flags, float f1, int f2, float f3, float f4,`

```cpp
// 2026-05-08 BUG-FIX (item +N glow): el wrapper estaba pre-shifting `Level`
// con `>> 3 & 0x0F` antes de llamar `RenderItem3D`, pero per IDA
// `RenderItem3D` (0x004E1BE0) toma el RAW Level byte y lo pasa así a
// `RenderObjectScreen` (0x004E13A0) que internamente hace `Level = (ItemLevel
// >> 3) & 0xF`. Pre-shifting acá producía un DOUBLE-shift → para items +9
// (Level byte = 0x48), el valor llegaba a Entity_DrawSetup como 1 en vez
// de 9 → ItemLevel<3 → no entra en la rama de glow +9/+11 → items en
// inventario sin halo dorado/azul.
//
// Per IDA: pasamos raw Level. Entity_DrawSetup (línea 52 de su archivo)
// hace el shift una sola vez (la cadena solo shifteaba después).
// BMD__RenderBody @ 0x00441E00 — BMD::RenderBodyTranslate
// Signature IDA: __thiscall(this, Flag, Alpha, BlendMesh, BlendMeshLight,
//                           BlendMeshTexCoordU, BlendMeshTexCoordV, HiddenMesh, Texture8)
// BlendMesh y HiddenMesh son INT pero los callers nuestros pasan como float
// (bit-pattern reinterpreted). Por eso usamos union para reinterpretar bits sin
// romper signatures.
//
// BUG-FIX 2026-04-28: lógica del branch NULL estaba INVERTIDA (skipping cuando
// debería render). IDA: `if (NULL && i != HiddenMesh) goto render;`. Y la
// comparación `i != HiddenMesh` debe ser INT, no float (NaN para -1, etc.).
```

### Línea 819 en `BMD__RenderBody` — antes de `if (model == nullptr) return;`

```cpp
    // BUG-FIX 2026-04-29: validar model + meshBase antes de iterar. Crash AV en
    // glPopMatrix con stack KernelBase+opengl32 venía de un BMD__RenderMesh que
    // dereferenciaba un mesh pointer wild (VBO inválido).
    // BUG-FIX 2026-05-01: range check del pointer model. Algún caller pasa
    // direcciones tipo 0xE5E90005 (kernel space) → AV en glDrawElements / lectura
    // de model+0x24. User-space address válido es < 0x80000000 y > 0x100000.
```

### Línea 832 en `BMD__RenderBody` — antes de `if (f1 < _DAT_00552544) glColor4f(*(float*)((char*)model+0x48),*(float*)((char*)model+0x4c`

```cpp
        // BUG-FIX 2026-04-26: IDA usa < 0.99f (_DAT_00552544), no < 1.0f.
```

### Línea 873 — antes de `// SetHall (0x00404BB0) vive en src/Sound/Sound_DS3D.cpp.  Aca habia una segunda`

```cpp
// CreateCharacter, CreateMonster moved to
// src/Monster/Monster.cpp (B3 refactor 2026-05-07, 925 lines).
```

### Línea 929 — antes de `// Net_Connect @ 0x0043DC70 — connect socket to server (TCP) + arm WSAAsyncSelect.`

```cpp
// GetScreenWidth @ 0x004CB520 — `GetScreenWidth` per IDA companion (Offsets.h).
// Returns the "logical width" of the 3D world viewport based on which UI
// panel is open: 260 (right pane open) / 450 (right pane open, narrower
// content) / 640 (no panel — full width).
//
// 2026-04-30: Ghidra labelled this `SecondPassword_GetAnimFrame` because
// it reads DAT_07eaa117/116/118/119/11a/11b/11c — but per IDA's
// Offsets.h:59-69 those addresses ARE the UI-panel flags
// (InventoryOpened / CharacterOpened / ShopOpened / WarehouseOpened /
//  ChaosMixOpened / TradeOpened / EventWindowOpened). The magic values
// 0x280=640, 0x1c2=450, 0x104=260 confirm screen-width semantics.
//
// Body kept verbatim to original IDA decompile (matches the safe path
// of the anti-tamper hash-table-decorated original).
// Net PacketSession helpers
// SecondPassword screens (SecondPassword_Handler / 004df410 / 004e4760-004ec330) moved to
// src/Net/SecondPassword.cpp (B3 refactor 2026-05-07, ~1535 lines).
```

### Línea 947 — antes de `int __cdecl Net_Connect(void* ctx, char* ip, unsigned short port, unsigned int wMsg)`

```cpp
// Net_Connect @ 0x0043DC70 — connect socket to server (TCP) + arm WSAAsyncSelect.
// ctx layout:  +0x00 = HWND (msg target)   +0x08 = SOCKET
// wMsg        = Windows message ID for WSAAsyncSelect (WinMain/0x423920 pass 0x400 = WM_USER)
// Returns: 1 on success, 0 on failure (matches caller in Net_Connect.cpp:46).
```

## `src/Render/SkillEffect_Render.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// SkillEffect_Render.cpp
// SkillEffect_Render @ 0x0046CB70  (44 lines, decompile completo)
//
// Renderiza todos los efectos de habilidades activos. Soporta dos modos:
// en-mundo 2D (World == 2) y billboard 3D para otras escenas.
//
// ── POOL DE EFECTOS ───────────────────────────────────────────────────────────
//
//   Base:   DAT_07c5ab3c  (inicio de iteración, pfVar1[-3] = flag activo)
//   Stride: 0x1c * 4 = 0x70 bytes por efecto (pfVar1 += 0x1c por iteración)
//   Límite: pfVar1 < 0x7c602bc
//   Entradas: (0x7c602bc - 0x07c5ab3c) / 0x70 = 0x5780/112 = 200 efectos
//
//   Struct de efecto (relativo a pfVar1):
//     pfVar1[-3]    — char: active flag
//     pfVar1[-2]    — int/float: tipo de efecto (texture ID)
//                     0 (1.42932e-43 = 0x00000001 float): tipo especial grande (20×1)
//                     otros: tipo normal (3×3)
//     pfVar1[+0]    — float: escala / intensidad
//     pfVar1[+1..3] — float[3]: posición XYZ
//     pfVar1[+4..6] — float[3]: rotación XYZ (Euler angles)
//     pfVar1[+7..N] — datos adicionales (pasados a RenderSprite_0)
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void FUN_0046cb70(void)
//   {
//     // Modo: update timing vs GL state
//     if (World == 2 || World == 7 || World == 10)
//       GL_SetBlendAdditive();           → Frame_UpdateTimer()
//     else
//       GL_SetBlendSrcOver('\x01');     → GL_SetMode(1) — 2D ortho setup para otras escenas
//
//     glColor3f(1.0, 1.0, 1.0);  // color blanco (textura sin tinte)
//
//     pfVar1 = &DAT_07c5ab3c;
//     do {
//       if (pfVar1[-3] != '\0') {             // efecto activo
//         BindTexture(*((_DWORD *)v0 - 2));    ← DWORD, no float (ver fix 2026-08-15)
//
//         if (World == 2) {
//           // Modo en-mundo: draw 2D en espacio mundo
//           RenderSprite_0((int)pfVar1[-2], pfVar1+1, *pfVar1, *pfVar1,
//                        pfVar1+7, 0.0, 0.0, 0.0, 1.0, 1.0);
//                        → SkillEffect_Draw2D(type, pos[3], r, g, scale, 0,0,0, 1, 1)
//         }
//         else {
//           // Modo billboard: draw 3D con matriz de rotación
//           glPushMatrix();
//           glTranslatef(pfVar1[1], pfVar1[2], pfVar1[3]);  // trasladar a posición efecto
//           Matrix_BuildFromEuler(pfVar1+4, local_30);               → Matrix_FromEuler(rot[3], mat)
//           if (pfVar1[-2] == 1.42932e-43) {    // tipo 1 (1 en float = tipo especial)
//             fVar3 = 20.0; fVar2 = 1.0;        // billboard grande y delgado
//           } else {
//             fVar3 = 3.0; fVar2 = 3.0;         // billboard cuadrado estándar
//           }
//           GL_DrawBillboard(fVar2, fVar3, local_30); → SkillEffect_DrawBillboard(w, h, rot_mat)
//           glPopMatrix();
//         }
//       }
//       pfVar1 += 0x1c;   // siguiente efecto
//     } while (pfVar1 < 0x7c602bc);
//   }
//
// ── NOTA ──────────────────────────────────────────────────────────────────────
//
//   Esta función se llama dos veces en Game_RenderTick:
//   1. Siempre (línea 106) — para todas las escenas
//   2. Solo si World == 2 && DAT_07e118e8 not in {3, >=10} (línea 121)
//      → segunda pasada solo en modo in-world normal
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   GL_SetBlendAdditive  → Frame_UpdateTimer()
//   GL_SetBlendSrcOver  → GL_SetMode(mode)
//   GL_BindTextureSlot  → Particle_SetTexture(type) — glBindTexture
//   RenderSprite_0  → SkillEffect_Draw2D(type, pos, r, g, scale, ...)
//   Matrix_BuildFromEuler  → Matrix_FromEuler(angles[3], out_mat[12])
//   GL_DrawBillboard  → SkillEffect_DrawBillboard(width, height, rot_mat)
```

### Línea 86 — antes de `void SkillEffect_Render(void)`

```cpp
// SkillEffect_Render @ 0x0046CB70 (44 lines)
// Renders all active skill effects. In sub-states 2/7/10 uses timer-driven mode;
// otherwise sets GL blend mode. Each effect: bind texture, then draw 2D or billboard.
// BUG-FIX 2026-04-28: AUTO-SKIP removed — pool DAT_07c5ab3c ahora es array
// de 200 × 0x70 bytes en globals.cpp. Loop bound count-based.
//
// Pool layout per slot (start at +0x0c offset from pfVar1, so pfVar1[-3] = +0):
//   +0x00 char  active flag
//   +0x04 int   type / texture id
//   +0x0c float scale / intensity   (= pfVar1[0])
//   +0x10..0x18 float[3] position   (= pfVar1[1..3])
//   +0x1c..0x24 float[3] euler rot  (= pfVar1[4..6])
//   +0x28..    additional data passed to RenderSprite_0
```

### Línea 105 en `SkillEffect_Render` — antes de `GL_SetBlendSrcOver('\x01');`

```cpp
        // ── 2026-08-16: CAUSA REAL DE LOS CUADROS BLANCOS ────────────────────
        // IDA llama `EnableAlphaTest(1)` = **0x00511680**. El port llamaba
        // `GL_SetAlphaTest`, que es **DisableTexture(bool)** (0x00511590) y termina
        // con `glDisable(GL_TEXTURE_2D)` incondicional. Con el texturizado
        // apagado, cada quad se pinta con el `glColor3f(1,1,1)` de abajo = un
        // CUADRADO BLANCO. Y como el estado GL es global y queda "pegado",
        // contaminaba todo lo que se dibujara después (de ahí los cuadros
        // blancos sobre los mobs al atacar, y el Inferno "verde" del principio).
        // Por eso el probe TEXDBG dio 0 hits: la textura se bindeaba bien, sólo
        // que GL_TEXTURE_2D estaba deshabilitado.
        // Ojo con esta familia (3ra vez que muerde, ver CLAUDE.md 2026-08-10):
        //   0x00511590 DisableTexture   0x00511680 EnableAlphaTest
        //   0x00511710 EnableAlphaBlend 0x00511790 EnableAlphaBlendMinus
```

### Línea 127 en `SkillEffect_Render` — antes de `const int texId = *(int*)(pfVar1 - 2);`

```cpp
        // ── 2026-08-15: CAUSA DE LOS "CUADROS BLANCOS" DE LOS SKILLS ─────────
        // IDA lee este campo como **DWORD**: `BindTexture(*((_DWORD *)v0 - 2))`.
        // El port hacía `(int)pfVar1[-2]`, o sea lo leía como FLOAT y lo
        // convertía NUMÉRICAMENTE. El campo guarda un entero (el id de textura),
        // así que p.ej. 102 leído como float da 1.43e-43 y `(int)` de eso es 0
        // → se bindeaba el slot 0 (sin textura) y el sprite salía blanco.
        // Afecta a TODOS los efectos de skill: esta función dibuja sus sprites.
        // Nótese que la comparación `== 102` de más abajo ya leía bien el campo
        // (`*(int*)(pfVar1 - 2)`): el mismo campo se leía de dos formas
        // distintas dentro de la misma función.
        // Mismo primo del patrón `(float)(uintptr_t)` que corrompía los joints
        // (ver CLAUDE.md 2026-08-10).
```

## `src/Render/Sprite.cpp`

### Línea 81 — antes de `float local_corners_buf[12];`

```cpp
  // ── BUG-FIX 2026-04-27: las 12 vars (local_60[0..4] + local_4c..local_34)
  // se iteraban como 12 floats consecutivos vía pfVar4 = local_60; pfVar4 += 3
  // 4× para emitir glVertex3fv. MSVC no garantiza contigüidad → corners 1-3
  // leían stack basura → sprites se veían como streaks horizontales en lugar
  // de quads. Fix: declarar como UN SOLO array de 12 floats con macros para
  // mantener nombres originales como aliases.
```

### Línea 99 — antes de `float TPos_buf[3];`

```cpp
  // ── BUG-FIX 2026-04-27: local_ac/_a8/_a4 son 3 variables locales separadas;
  // MSVC no garantiza layout contiguo. Pasar &local_ac a Vector_Transform (que
  // escribe 3 floats consecutivos) puede dejar TPos[1]/TPos[2] en memoria
  // unrelated → sprite quad corner depth basura → sprites invisibles o
  // mal-clipeados. Usamos un array TPos_buf[3] contiguo y copiamos a las vars.
```

### Línea 112 — antes de `float depth_eye = *(float*)&local_a4;`

```cpp
  // ── BUG-FIX 2026-04-27: local_a4 está declarado `undefined4` (int32). El
  // decompile original hace `(float)local_a4` que es CAST int→float (NO
  // reinterpret-bits). Como almacenamos el bit-pattern de TPos[2] (e.g.
  // -290.0f → 0xC3910000), el cast int→float lo lee como -1.01e9 → quad
  // corners con depth astronómica → fuera del frustum → invisible.
  // Usamos directamente el bit-pattern almacenado vía reinterpret.
```

### Línea 134 — antes de `float halfW = local_9c[3];   // param_3 * _DAT_00552504`

```cpp
    // ── BUG-FIX 2026-07-15: el caso ROTADO (param_6 != 0) leía los offsets de
    // las 4 esquinas de local_9c[0..4] + local_88/84/80/7c/78/74/70 — locales
    // SEPARADOS — como 12 floats contiguos vía `(float*)((int)local_9c+iVar1)`.
    // MSVC NO garantiza ese layout → esquinas 2-4 leían stack basura → el sprite
    // rotado se estiraba a posiciones random (haces/beams saliendo de los bordes
    // de la pantalla). Mismo patrón que los corners axis-aligned y los texcoords
    // (ya arreglados). Fix: array contiguo con los 4 offsets en el orden de IDA:
    //   BL(-hw,-hh) BR(+hw,-hh) TR(+hw,+hh) TL(-hw,+hh), z = depth_eye.
    // TODOS los sprites/flares/glows con rotación pasan por acá (partículas
    // 0x4e1 con frame≠0, weapon glow, etc.), no solo el char-select.
```

### Línea 174 — antes de `float local_tex[8] = {`

```cpp
  // ── BUG-FIX 2026-07-12: los 8 texcoords (local_9c[0..4] + local_80/84/88)
  // se iteraban como 8 floats consecutivos vía pfVar3 = local_9c; pfVar3 += 2
  // 4× para emitir glTexCoord2f. Igual que los corners, MSVC NO garantiza que
  // local_80/84/88 sigan contiguos a local_9c[4] → los texcoords de los corners
  // 3-4 leían stack basura → la textura del sprite muestreaba coords fuera de
  // rango (parte negra) → sprites/flares/glows INVISIBLES (samplean negro).
  // Fix: array contiguo de 8 floats con el layout correcto del IDA:
  //   corner BL=(u, v+vH), BR=(u+uW, v+vH), TR=(u+uW, v), TL=(u, v)
```

### Línea 218

```cpp
// BUG-FIX: array contiguo (era local_9c → texcoords basura)
```

## `src/Render/Terrain_Render.h`

### Línea 2 — antes de `void Terrain_Render(void);`

```cpp
// 2026-09-26: aca habia una declaracion `void Terrain_Render(int, int, int)` sin
// definicion y sin un solo call site, mientras la funcion real vivia como
// Terrain_Render(void).  Dos firmas para la misma direccion son dos simbolos.
```

## `src/Render/Terrain_Water.cpp`

### Línea 59 en `Terrain_Water` — antes de `*(unsigned int *)((char*)&DAT_081cb608 + iVar3) = *(unsigned int*)&DAT_0828b608[iVar2 * 3]`

```cpp
                    // BUG-FIX: DAT_081cb608 está declarado DWORD, así que
                    // `&DAT_081cb608 + iVar3` hace aritmética DWORD* (= +iVar3*4
                    // = +iVar2*48 bytes). El stride real es 12 bytes (3 floats).
                    // Disasm @ 0x004f9620-23 confirma byte-offset = iVar2*3*4=12.
                    // Castear a char* para que la suma sea aritmética de bytes.
                    // FIX 2026-06-27: el RHS era `DAT_0828b608[iVar2*3]` (float[] → VALOR
                    // float); asignado a `*(unsigned int*)` hacía conversión float→int =
                    // truncación → R quedaba 0 (luz 0.x<1.0). G/B usan el macro DWORD*
                    // (bits). IDA hace PrimaryTerrainLight[i][0]=BackTerrainLight[i][0]
                    // (copia float). Bit-cast del source para preservar el float, igual a G/B.
```

### Línea 103 en `Terrain_Water` — antes de `pfVar6 = (float *)((char*)&DAT_07eab200 + (iVar9 + FrustrumBoundMinX_1) * 4);`

```cpp
                // BUG-FIX: DAT_07eab200 es DWORD → &DAT_07eab200 + N*4 hace
                // aritmética DWORD* (=+N*16 bytes). Disasm @ 0x004f96d1 muestra
                // LEA EDX,[EAX*0x4 + 0x7eab200] = byte offset N*4. Castear a char*.
```

## `src/Render/Texture/Texture.cpp`

### Línea 142 en `Texture_BindLocalCached` — antes de `if (g_bound_texture_id == id)`

```cpp
    // ⚠ ESTE **NO** ES EL BIND QUE USA EL RENDER.
    //
    // El bind real del pipeline es `GL_BindTextureSlot` (0x00511480, en GL_State.cpp):
    // ése es el que llaman BMD_DrawMesh, los efectos, los sprites y el HUD.
    // Esta función es un equivalente funcional sin callers vivos — su único
    // caller es `Texture_Draw2D`, que a su vez tampoco tiene callers.
    //
    // Peor: mantiene su PROPIO cache (`g_bound_texture_id`) distinto del de
    // GL_BindTextureSlot (`DAT_00561574`). Si algún día se la vuelve a usar, los dos
    // caches se desincronizan y se omiten binds → textura equivocada.
    //
    // Se conserva porque forma parte del port de este módulo, pero NO
    // instrumentar acá para diagnosticar el render: en la sesión del 2026-08-16
    // se puso un probe en esta función, dio 0 hits, y ese silencio se tomó como
    // evidencia de que las texturas estaban bien — costó una ronda entera.
```

### Línea 356 en `tex_load_ozt` — antes de `if (bit != 32 || nx > 1024 || ny > 1024) {`

```cpp
    // BUG-FIX (2026-04-21): mismo issue que en OZJ — 256 era muy bajo.
```

### Línea 535 — antes de `int __cdecl OpenJPG(const char* path, int id, int filter, int wrap, int flags, char show_e`

```cpp
// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 4805-5199 (395 lines)
// OpenJPG (Texture_Load OZJ/JPEG raw), OpenTGA (OpenTGA), UnloadImage (Texture_FreeSlot)
// =============================================================================
// ── OpenJPG @ 0x00529740 — Texture_Load (OZJ/JPEG) ─────────────────────
// Loads JPEG or OZJ texture from disk, decompresses with libjpeg, uploads to GL.
// Path mode:
//   DAT_0055a7c4 == 0 → full_path = g_tex_base_dir + filename
//   DAT_0055a7c4 != 0 → strip extension, try g_tex_ext_hq then g_tex_ext_lq
// OZJ files: fseek(f, 24, SEEK_SET) to skip 24-byte Webzen header before JPEG data.
// Limits: 256x256 max, rounds to power-of-2 before GL upload.
```

### Línea 551 en `OpenJPG` — antes de `const char* nameForPath = path;`

```cpp
    // 2026-05-05: Strip "Data\" / "Data/" prefix si el BMD lo guardó como
    // path absoluto desde root. Sin esto el path final queda
    // "Data\Data\Npc\foo.OZJ" y fopen falla.
```

### Línea 594 en `OpenJPG` — antes de `fseek(f, 0x18, SEEK_SET);`

```cpp
    // --- OZJ header (skip 24 bytes) ---
    // NOTA / BUG PENDIENTE (sistema de texturas): varios .OZJ (Effect\Spark02.OZJ,
    // Local\Webzenlogo.OZJ, ships, logos) son JPEG PLANO de Photoshop (SOI 0xFFD8
    // sin header OZJ) con estructura: THUMBNAIL RGB embebido en APP1/APP13 +
    // imagen PRINCIPAL en CMYK. El skip-24 a ciegas hace que libjpeg lea el SOI del
    // THUMBNAIL (RGB, con color) — por eso los ships/logos se ven bien pero Spark02
    // queda 4x4 (su thumbnail es diminuto). Intentos previos:
    //   - Detectar SOI + leer imagen principal → crash (CMYK, 4 comp, overflow).
    //   - + JCS_RGB → sin crash pero TODO gris (CMYK Adobe invertido mal convertido).
    // Solución correcta pendiente: leer el thumbnail RGB SIEMPRE (que es lo que hace
    // el skip-24 por casualidad), o portar el decode CMYK-Adobe fiel. Por ahora se
    // mantiene skip-24 (colores OK); Spark02 queda chico como efecto secundario.
```

### Línea 727 en `OpenTGA` — antes de `const char* nameForPath = szFileName;`

```cpp
    // 2026-05-05: BMDs de NPC almacenan el nombre de textura como path
    // completo "Data\Npc\foo.OZT". Sin strip, la concatenación con base
    // "Data\" produce "Data\Data\Npc\foo.OZT" → fopen FAIL → NPCs blancos.
    // Si szFileName empieza con "Data\" o "Data/", strip ese prefijo.
```

### Línea 805 en `OpenTGA` — antes de `if (depth == 0x20) {`

```cpp
    // [FIX #4 2026-06-30] Límite de tamaño <=256 removido — match companion-DLL
    // Patchs.cpp "Remove TGA size limit" (NOPea el size-check + fuerza los jumps
    // de width/height). Las object textures del mundo (chair2.OZT etc.) son >256,
    // el límite original las rechazaba → renderizaban cyan (textura sin subir).
    // Se mantiene `depth == 0x20` (32bpp) — el patch tampoco lo toca.
```

### Línea 866 en `OpenTGA` — antes de `glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (float)GL_MODULATE); // 0x2100 = 8448.0f`

```cpp
        // BUG-FIX: el valor original era 8192.0f. GL_MODULATE = 0x2100 = 8448.0f.
        // 8192 = 0x2000 = GL_NICEST, que NO es un valor válido para
        // GL_TEXTURE_ENV_MODE → setea glGetError = GL_INVALID_ENUM (0x500),
        // que el driver NVIDIA acumula y eventualmente convierte en AV en una
        // llamada GL siguiente (visto en GL_DisableDepthTest → glDisable).
```

## `src/Render/Trail_Render.cpp`

### Línea 113 en `FUN_0046c3e0_DISABLED` — antes de `if (++nSlotTR >= 100) break;`

```cpp
        // 2026-09-08: el bound era `< 0x7c72e74`, direccion absoluta del binario
        // (IDA: `while ((int)v0 < (int)&unk_7C72E74)`).  Base unk_7C608B4 =
        // g_RenderPool_07c608a8 + 0x0C; (0x7C72E74 - 0x7C608B4) / 0x2F0 = 100,
        // que es el tamano real del pool.
```

## `src/Render/Weather.cpp`

### Línea 111 — antes de `#define WSLOT_DW(off) (*(unsigned int *)&g_WeatherSlotPool[(off) + iVar14 * 0x1bc])`

```cpp
// 2026-09-03 FIX (la banda que cruzaba la pantalla desde el personaje).
//
// Dos campos DWORD del slot de clima se accedian con el indice mal escalado --
// el patron `&DAT_x + i*stride` sobre puntero tipado que ya mordio antes en
// este proyecto:
//
//   (&DAT_0839bd8c)[iVar2]          -- DAT_0839bd8c es `unsigned int`, o sea el
//     compilador multiplica el indice POR 4.  Pero `iVar2` ya es el offset en
//     BYTES (`iVar14 * 0x1bc`), asi que el paso real era 0x6f0 en vez de 0x1bc.
//     A partir del slot 10 escribe FUERA del pool: en Icarus el bucle llega
//     hasta el slot 12 (`if (0xc < iVar14) return`), mientras que en los demas
//     mapas corta en 4 -- por eso el sintoma salia solo ahi.
//     Medido: g_WeatherSlotPool = 016DC520..016E0A80 y el pool de joints
//     arranca en 016E0AA0, o sea 32 bytes despues.  Con iVar14 = 10 la
//     escritura cae en 0xdc + 0x6f0*10 = 0x463C = **joint slot 0 + 0xBC**, que
//     es la Y del vertice 0 de la fila 2 del anillo de segmentos.  Escribe `1`,
//     y `1` leido como float es el denormal 1.4e-45: el vertice se iba al
//     origen del mapa y el quad entre esa fila y la anterior barria la pantalla.
//     De ahi que lo mostraran el aura del Soul Barrier, el efecto de subir de
//     nivel y el halo del set +11 -- los joints que ocupan los primeros slots.
//
//   (&DAT_0839bcb4)[iVar14*0x6f]    -- el caso simetrico: DAT_0839bcb4 esta
//     declarado `char`, asi que el indice NO se escala y el paso quedaba en
//     0x6f en vez de 0x1bc.  No sale del pool, pero pisa los slots vecinos.
//     Que el campo es un DWORD lo confirma su propio uso mas abajo:
//     `(int)... + 1` y `if (1 < (int)...)` -- es un contador.
//
// Los dos pasan por este accesor, que fija el paso en 0x1bc bytes y el ancho
// en 4, que es lo que el binario hace (`0x6f * 4 == 0x1bc`).
```

### Línea 166 en `Weather_Update` — antes de `float   __fr[12] = { 0.0f };`

```cpp
    //
    // Medido: las nubes de Icarus (Weather_Update -> CreateEffect 1150) nacian en
    // pos=(6936, 0, 0) — X bien, Y y Z en cero — o sea fuera del mapa y por eso
    // no se veian. Mismo patron que ya mordio en MoveJoint, CreateJoint,
    // CreateEffect y los texcoords de los sprites.
```

### Línea 379 en `Weather_Update` — antes de `bool __spawn = (iVar12 == 0) || (iVar12 == 1) || (iVar12 == 3) ||`

```cpp
            // Non-storm: spawn by game state.
            //
            // 2026-09-03 -- IDA (0x00500E80 L690-706) manda los TRES caminos al
            // MISMO spawn (los tres hacen `break` del while y caen en el
            // `memset(v17, 0, 0x1BC)` + init del slot):
            //     if ( !v15 || v15 == 1 || v15 == 3 || v15 == 4 || v15 == 10 ) break;
            //     if ( v15 == 7 ) { v24 = TerrainWall[v113];
            //                       if (!v24 || v24 == 2) break; }
            //     else if ( v15 >= 11 && v15 <= 16 ) break;
            // El port tenia los dos ultimos como `else if` con el cuerpo VACIO y
            // un comentario ("Same as state 0..4 spawn"), asi que Atlans (7) y
            // los mundos 11-16 no spawneaban nada.
            //
            // Nota: el DLL de inyeccion NOPea justo estos dos tests en 0x00501292
            // ("Fix Atlans and Icarus Goldens Overflow"), o sea el binario SI los
            // usa -- lo que el DLL hace es desactivarlos.
```

### Línea 679 en `Weather_Update` — antes de `Particle_PathUpdate((int)pcVar3, iVar14, (int)(uintptr_t)&DAT_0839bcb0, 0x28);`

```cpp
                        // BUG-FIX 2026-05-04: era literal `0x839bcb0` (dirección absoluta del binario
                        // fuente). En nuestro build DAT_0839bcb0 vive en otra dirección; pasar el
                        // literal hacía que Particle_PathUpdate leyera memoria random → AV at 0x004BF712 al
                        // entrar al mundo (param0=0 read, param1=0x0839BCB0).
```

## `src/Render/Weather_Particles.cpp`

### Línea 64 en `WeatherParticles_Update` — antes de `_DAT_00559b9c = (int)(10 * ((longlong)sin((double)DAT_05826e08 * 0.001) + 3));`

```cpp
    // ── Per-frame wind / oscillation globals ─────────────────────────────────
    // BUG-FIX 2026-07-19: acá había un bloque con `__ftol()` sin argumentos
    // (artefacto de Ghidra) y debajo una "aproximación" que OMITÍA el sin()
    // por completo — o sea RainSpeed/RainAngle crecían monótonamente con
    // WorldTime en vez de oscilar. IDA MoveLeaves (0x46CC80) es explícito:
    //   RainSpeed = 10 * ((__int64)sin(WorldTime * 0.001) + 3);
    //   RainAngle = 20 * (__int64)sin(WorldTime * 0.00050000002 + 50.0);
    // (_DAT_00559b9c = RainSpeed, DAT_07c74ae8 = RainAngle.)
```

### Línea 78 en `WeatherParticles_Update` — antes de `int iVar9 = (iVar7 != 9) ? 80 : 200;`

```cpp
    // ── Compute active particle count ─────────────────────────────────────────
    // sub-state 9 (snow/logout): 200; others: 0x88 (136) unless sub-state is 9
    // IDA: iMaxLeaves = World != 9 ? 80 : 200;  (el port tenia 0x88 = 136)
```

### Línea 84 en `WeatherParticles_Update` — antes de `float *pfVar10 = (float *)&DAT_07c5ab5c;`

```cpp
    // ── Main particle loop ────────────────────────────────────────────────────
    // IDA: for ( i = (float *)&unk_7C5AB5C; ; i += 28 )
    // El port sumaba 0x44 bytes de mas sobre un alias que ya estaba corrido 12,
    // asi que escribia el flag de activo 56 bytes fuera de donde lo lee
    // SkillEffect_Render: el pool quedaba siempre vacio (medido: active=0).
```

## Comentarios al final de línea corregidos a mano

### `src/Render/SMD_Parser.cpp` línea 53

```cpp
extern bool __cdecl OpenSMDFile(char *FileName, int Type, bool Flip);  // C++ impl, line 18532
```

### `src/Render/Render_WorldHelpers.cpp` línea 396 en `ItemDrop_Render`

```cpp
    const int kEntries = (int)(sizeof(DAT_07b27150) / 0x9d8);   // 200
```

### `src/Render/Render_Frame.cpp` líneas 394-395 en `Render_GameFrame`

```cpp
    Render_HudPass_4F6050();        // sub_4F6050 (TODO port)
    Render_HudPass_4EB070();        // sub_4EB070 (TODO port)
```

### `src/Render/Texture/Texture.cpp` línea 811 en `OpenTGA`

```cpp
        // Round up to next power-of-2 (max 256)
```
