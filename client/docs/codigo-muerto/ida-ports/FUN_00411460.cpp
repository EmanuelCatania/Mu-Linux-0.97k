// 0x00411460 FUN_00411460 — nunca activado: IDA_PORT_00411460 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00411460 (IDA-only, gated) ──
#if defined(IDA_PORT_00411460)
DWORD *__cdecl FUN_00411460(DWORD *_this, DWORD *a2, int a3, DWORD *a4, DWORD *a5)
{
  DWORD *v6; // ebp
  DWORD *v7; // eax
  int v8; // eax
  DWORD *v9; // eax
  DWORD *v10; // eax
  DWORD *v11; // ecx
  DWORD *v12; // esi
  DWORD *v13; // edx
  int v14; // edx
  DWORD *v15; // ecx
  int v16; // edx
  DWORD *v17; // edx
  DWORD *v18; // ecx
  DWORD *v19; // edx
  int v20; // esi
  int v21; // esi
  DWORD *v22; // esi
  DWORD *v23; // ecx
  int v24; // edx
  int v25; // edx
  DWORD *v26; // edx
  int v27; // esi
  DWORD *v28; // esi
  DWORD *result; // eax

  v6 = (DWORD *)operator_new(0x138u);
  v6[1] = a4;
  v6[77] = 0;
  *v6 = DAT_055c9b98;
  v6[2] = DAT_055c9b98;
  FUN_004124d0(v6 + 3, a5);
  v7 = (DWORD *)_this[1];
  ++_this[3];
  if ( a4 == v7 || a3 != DAT_055c9b98 || *a5 < a4[3] )
  {
    *a4 = v6;
    v9 = (DWORD *)_this[1];
    if ( a4 == v9 )
    {
      v9[1] = v6;
      *(DWORD *)(_this[1] + 8) = v6;
    }
    else if ( a4 == (DWORD *)*v9 )
    {
      *v9 = v6;
    }
  }
  else
  {
    a4[2] = v6;
    v8 = _this[1];
    if ( a4 == *(DWORD **)(v8 + 8) )
    {
      *(DWORD *)(v8 + 8) = v6;
    }
  }
  v10 = v6;
  while ( v10 != *(DWORD **)(_this[1] + 4) )
  {
    v11 = (DWORD *)v10[1];
    if ( v11[77] )
    {
      break;
    }
    v12 = (DWORD *)v11[1];
    v13 = (DWORD *)*v12;
    if ( v11 == (DWORD *)*v12 )
    {
      v14 = v12[2];
      if ( *(DWORD *)(v14 + 308) )
      {
        if ( v10 == (DWORD *)v11[2] )
        {
          v10 = (DWORD *)v10[1];
          v15 = (DWORD *)v11[2];
          v10[2] = *v15;
          if ( *v15 != DAT_055c9b98 )
          {
            *(DWORD *)(*v15 + 4) = v10;
          }
          v15[1] = v10[1];
          v16 = _this[1];
          if ( v10 == *(DWORD **)(v16 + 4) )
          {
            *(DWORD *)(v16 + 4) = v15;
          }
          else
          {
            v17 = (DWORD *)v10[1];
            if ( v10 == (DWORD *)*v17 )
            {
              *v17 = v15;
            }
            else
            {
              v17[2] = v15;
            }
          }
          *v15 = v10;
          v10[1] = v15;
        }
        *(DWORD *)(v10[1] + 308) = 1;
        *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
        v18 = *(DWORD **)(v10[1] + 4);
        v19 = (DWORD *)*v18;
        *v18 = *(DWORD *)(*v18 + 8);
        v20 = v19[2];
        if ( v20 != DAT_055c9b98 )
        {
          *(DWORD *)(v20 + 4) = v18;
        }
        v19[1] = v18[1];
        v21 = _this[1];
        if ( v18 == *(DWORD **)(v21 + 4) )
        {
          *(DWORD *)(v21 + 4) = v19;
          v19[2] = v18;
        }
        else
        {
          v22 = (DWORD *)v18[1];
          if ( v18 == (DWORD *)v22[2] )
          {
            v22[2] = v19;
          }
          else
          {
            *v22 = v19;
          }
          v19[2] = v18;
        }
LABEL_51:
        v18[1] = v19;
        continue;
      }
      v11[77] = 1;
      *(DWORD *)(v14 + 308) = 1;
      *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
      v10 = *(DWORD **)(v10[1] + 4);
    }
    else
    {
      if ( v13[77] )
      {
        if ( v10 == (DWORD *)*v11 )
        {
          v10 = (DWORD *)v10[1];
          v23 = (DWORD *)*v11;
          *v10 = v23[2];
          v24 = v23[2];
          if ( v24 != DAT_055c9b98 )
          {
            *(DWORD *)(v24 + 4) = v10;
          }
          v23[1] = v10[1];
          v25 = _this[1];
          if ( v10 == *(DWORD **)(v25 + 4) )
          {
            *(DWORD *)(v25 + 4) = v23;
          }
          else
          {
            v26 = (DWORD *)v10[1];
            if ( v10 == (DWORD *)v26[2] )
            {
              v26[2] = v23;
            }
            else
            {
              *v26 = v23;
            }
          }
          v23[2] = v10;
          v10[1] = v23;
        }
        *(DWORD *)(v10[1] + 308) = 1;
        *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
        v18 = *(DWORD **)(v10[1] + 4);
        v19 = (DWORD *)v18[2];
        v18[2] = *v19;
        if ( *v19 != DAT_055c9b98 )
        {
          *(DWORD *)(*v19 + 4) = v18;
        }
        v19[1] = v18[1];
        v27 = _this[1];
        if ( v18 == *(DWORD **)(v27 + 4) )
        {
          *(DWORD *)(v27 + 4) = v19;
        }
        else
        {
          v28 = (DWORD *)v18[1];
          if ( v18 == (DWORD *)*v28 )
          {
            *v28 = v19;
          }
          else
          {
            v28[2] = v19;
          }
        }
        *v19 = v18;
        goto LABEL_51;
      }
      v11[77] = 1;
      v13[77] = 1;
      *(DWORD *)(*(DWORD *)(v10[1] + 4) + 308) = 0;
      v10 = *(DWORD **)(v10[1] + 4);
    }
  }
  *(DWORD *)(*(DWORD *)(_this[1] + 4) + 308) = 1;
  result = a2;
  *a2 = v6;
  return result;
}
#endif
