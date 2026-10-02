// 0x00412180 FUN_00412180 — nunca compilado: src/stubs_IDA_ports.cpp no estaba en el .vcxproj; la versión viva es FUN_00412180 en src/Sound/Music.cpp (__fastcall(int*)). Archivo copiado entero.
// stubs_IDA_ports.cpp
//
// IDA Hex-Rays ports — reference / inactive code.
//
// Este archivo NO esta en mu97k.vcxproj ni en CMakeLists.txt: no se compila.
// Los ports gated por IDA_PORT_xxxxxxxx (ninguna de esas macros estaba
// definida) se movieron a docs/codigo-muerto/ida-ports/, uno por funcion.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// Forward decls for symbols that live in stubs.cpp but are referenced by
// IDA-activated (non-gated) functions in this file.
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);

// ── FUN_00412180 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_00412180(DWORD *_this)
{
  int v2; // ecx
  int result; // eax
  int v4; // ecx
  int *v5; // eax
  int v6; // edx
  int v7; // ecx
  int v8; // eax
  int v9; // ecx
  int v10; // edx
  int v11; // edx
  int v12; // ecx

  v2 = _this[4];
  switch ( v2 )
  {
    case 7:
      (*(void (__cdecl **)(DWORD *))(*_this + 108))(_this);
      result = 0;
      break;
    case 12:
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, -100);
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      v4 = _this[23];
      _this[28] = v4;
      result = 0;
      _this[28] = *(DWORD *)(v4 + 4);
      break;
    case 13:
    case 14:
      if ( _this[26] != 1 || !_this[24] )
      {
        goto LABEL_23;
      }
      if ( v2 == 13 )
      {
        v5 = (int *)_this[28];
        if ( v5 == (int *)_this[23] )
        {
          goto LABEL_23;
        }
        v6 = *v5;
        _this[28] = *v5;
        if ( v6 == _this[23] )
        {
          _this[28] = *(DWORD *)(v6 + 4);
        }
      }
      else if ( v2 == 14 )
      {
        v7 = _this[28];
        if ( v7 == *(DWORD *)_this[23] )
        {
          goto LABEL_23;
        }
        _this[28] = *(DWORD *)(v7 + 4);
      }
      if ( _this[24] <= _this[35] )
      {
        goto LABEL_23;
      }
      v8 = 0;
      v9 = *(DWORD *)_this[23];
      _this[25] = v9;
      if ( v9 != _this[23] )
      {
        do
        {
          if ( _this[28] == v9 )
          {
            break;
          }
          ++v8;
          v10 = *(DWORD *)_this[25];
          _this[25] = v10;
          v9 = v10;
        }
        while ( v10 != _this[23] );
      }
      v11 = _this[35];
      v12 = _this[34];
      if ( v8 < v12 + v11 )
      {
        if ( v8 < v12 )
        {
          (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - v8);
        }
        goto LABEL_23;
      }
      (*(void (__cdecl **)(DWORD *, int))(*_this + 48))(_this, v12 - (v8 - v11 + 1));
      result = 0;
      break;
    default:
LABEL_23:
      result = 0;
      break;
  }
  return result;
}
