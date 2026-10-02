// 0x00410E50 FUN_00410e50 — nunca activado: IDA_PORT_00410E50 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00410e50 (IDA-only, gated) ──
#if defined(IDA_PORT_00410E50)
DWORD *__cdecl FUN_00410e50(DWORD *_this, DWORD *a2, DWORD *a3)
{
  DWORD *v3; // edi
  DWORD *v4; // esi
  DWORD *v5; // ebx
  DWORD *v6; // ebp
  DWORD *v7; // eax
  DWORD *i; // ecx
  DWORD *v9; // edx
  DWORD *v10; // ebp
  int v11; // eax
  DWORD *v12; // eax
  int v13; // eax
  DWORD *v14; // ecx
  int v15; // eax
  DWORD *v16; // eax
  DWORD *v17; // eax
  DWORD *v18; // ecx
  DWORD *v19; // eax
  int v20; // ebp
  DWORD *j; // ecx
  DWORD *v22; // eax
  int v23; // ecx
  DWORD *v24; // eax
  int v25; // eax
  DWORD *v26; // ecx
  int v27; // edx
  DWORD *v28; // edx
  DWORD *v29; // eax
  int v30; // ecx
  int v31; // edx
  int v32; // edx
  DWORD *v33; // edx
  int v34; // ecx
  int v35; // edx
  int v36; // edx
  DWORD *v37; // edx
  DWORD *v38; // eax
  DWORD *v39; // ecx
  int v40; // edx
  DWORD *v41; // edx
  DWORD *v42; // ecx
  int v43; // edx
  DWORD *v44; // edx
  int v45; // edx
  int v46; // edx
  DWORD *v47; // edx
  DWORD *v48; // ecx
  DWORD *result; // eax
  DWORD *lpMem; // [esp+14h] [ebp-Ch]
  DWORD *v52; // [esp+18h] [ebp-8h]
  char v53[4]; // [esp+1Ch] [ebp-4h] BYREF

  v3 = a3;
  FUN_004112b0(&a3);
  v4 = (DWORD *)*v3;
  v5 = v3 + 2;
  lpMem = v3;
  v6 = v3 + 2;
  if ( *v3 == DAT_055c9b98 )
  {
    v4 = (DWORD *)*v5;
  }
  else
  {
    v7 = (DWORD *)*v5;
    if ( *v5 != DAT_055c9b98 )
    {
      for ( i = (DWORD *)*v7; i != (DWORD *)DAT_055c9b98; i = (DWORD *)*i )
      {
        v7 = i;
      }
      v4 = (DWORD *)v7[2];
      v6 = v7 + 2;
      lpMem = v7;
    }
  }
  std::_Lockit::_Lockit((std::_Lockit *)v53);
  v9 = lpMem;
  if ( lpMem == v3 )
  {
    v14 = _this;
    v4[1] = lpMem[1];
    v15 = _this[1];
    if ( *(DWORD **)(v15 + 4) == v3 )
    {
      *(DWORD *)(v15 + 4) = v4;
    }
    else
    {
      v16 = (DWORD *)v3[1];
      if ( (DWORD *)*v16 == v3 )
      {
        *v16 = v4;
      }
      else
      {
        v16[2] = v4;
      }
    }
    v17 = (DWORD *)_this[1];
    v52 = v17;
    if ( (DWORD *)*v17 == v3 )
    {
      if ( *v5 == DAT_055c9b98 )
      {
        *v17 = v3[1];
      }
      else
      {
        v18 = v4;
        if ( *v4 != DAT_055c9b98 )
        {
          v19 = (DWORD *)*v4;
          do
          {
            v18 = v19;
            v19 = (DWORD *)*v19;
          }
          while ( v19 != (DWORD *)DAT_055c9b98 );
          v17 = v52;
        }
        *v17 = v18;
        v14 = _this;
      }
    }
    v20 = v14[1];
    if ( *(DWORD **)(v20 + 8) == v3 )
    {
      if ( *v3 == DAT_055c9b98 )
      {
        j = (DWORD *)v3[1];
      }
      else
      {
        v22 = (DWORD *)v4[2];
        for ( j = v4; v22 != (DWORD *)DAT_055c9b98; v22 = (DWORD *)v22[2] )
        {
          j = v22;
        }
      }
      *(DWORD *)(v20 + 8) = j;
    }
    v10 = _this;
  }
  else
  {
    *(DWORD *)(*v3 + 4) = lpMem;
    *lpMem = *v3;
    if ( lpMem == (DWORD *)*v5 )
    {
      v4[1] = lpMem;
    }
    else
    {
      v4[1] = lpMem[1];
      *(DWORD *)lpMem[1] = v4;
      *v6 = *v5;
      *(DWORD *)(*v5 + 4) = lpMem;
    }
    v10 = _this;
    v11 = _this[1];
    if ( *(DWORD **)(v11 + 4) == v3 )
    {
      *(DWORD *)(v11 + 4) = lpMem;
    }
    else
    {
      v12 = (DWORD *)v3[1];
      if ( (DWORD *)*v12 == v3 )
      {
        *v12 = lpMem;
      }
      else
      {
        v12[2] = lpMem;
      }
    }
    lpMem = v3;
    v9[1] = v3[1];
    v13 = v9[77];
    v9[77] = v3[77];
    v3[77] = v13;
    v9 = v3;
  }
  if ( v9[77] == 1 )
  {
    for ( ; v4 != *(DWORD **)(v10[1] + 4); v4 = (DWORD *)v4[1] )
    {
      if ( v4[77] != 1 )
      {
        break;
      }
      v23 = v4[1];
      v24 = *(DWORD **)v23;
      if ( v4 == *(DWORD **)v23 )
      {
        v24 = *(DWORD **)(v23 + 8);
        if ( !v24[77] )
        {
          v24[77] = 1;
          *(DWORD *)(v4[1] + 308) = 0;
          v25 = v4[1];
          v26 = *(DWORD **)(v25 + 8);
          *(DWORD *)(v25 + 8) = *v26;
          if ( *v26 != DAT_055c9b98 )
          {
            *(DWORD *)(*v26 + 4) = v25;
          }
          v26[1] = *(DWORD *)(v25 + 4);
          v27 = v10[1];
          if ( v25 == *(DWORD *)(v27 + 4) )
          {
            *(DWORD *)(v27 + 4) = v26;
          }
          else
          {
            v28 = *(DWORD **)(v25 + 4);
            if ( v25 == *v28 )
            {
              *v28 = v26;
            }
            else
            {
              v28[2] = v26;
            }
          }
          *v26 = v25;
          *(DWORD *)(v25 + 4) = v26;
          v24 = *(DWORD **)(v4[1] + 8);
        }
        if ( *(DWORD *)(*v24 + 308) != 1 || *(DWORD *)(v24[2] + 308) != 1 )
        {
          if ( *(DWORD *)(v24[2] + 308) == 1 )
          {
            *(DWORD *)(*v24 + 308) = 1;
            v34 = *v24;
            v24[77] = 0;
            *v24 = *(DWORD *)(v34 + 8);
            v35 = *(DWORD *)(v34 + 8);
            if ( v35 != DAT_055c9b98 )
            {
              *(DWORD *)(v35 + 4) = v24;
            }
            *(DWORD *)(v34 + 4) = v24[1];
            v36 = v10[1];
            if ( v24 == *(DWORD **)(v36 + 4) )
            {
              *(DWORD *)(v36 + 4) = v34;
            }
            else
            {
              v37 = (DWORD *)v24[1];
              if ( v24 == (DWORD *)v37[2] )
              {
                v37[2] = v34;
              }
              else
              {
                *v37 = v34;
              }
            }
            *(DWORD *)(v34 + 8) = v24;
            v24[1] = v34;
            v24 = *(DWORD **)(v4[1] + 8);
          }
          v24[77] = *(DWORD *)(v4[1] + 308);
          *(DWORD *)(v4[1] + 308) = 1;
          *(DWORD *)(v24[2] + 308) = 1;
          v38 = (DWORD *)v4[1];
          v39 = (DWORD *)v38[2];
          v38[2] = *v39;
          if ( *v39 != DAT_055c9b98 )
          {
            *(DWORD *)(*v39 + 4) = v38;
          }
          v39[1] = v38[1];
          v40 = v10[1];
          if ( v38 == *(DWORD **)(v40 + 4) )
          {
            *(DWORD *)(v40 + 4) = v39;
            *v39 = v38;
          }
          else
          {
            v41 = (DWORD *)v38[1];
            if ( v38 == (DWORD *)*v41 )
            {
              *v41 = v39;
            }
            else
            {
              v41[2] = v39;
            }
            *v39 = v38;
          }
LABEL_100:
          v38[1] = v39;
          break;
        }
      }
      else
      {
        if ( !v24[77] )
        {
          v24[77] = 1;
          *(DWORD *)(v4[1] + 308) = 0;
          v29 = (DWORD *)v4[1];
          v30 = *v29;
          *v29 = *(DWORD *)(*v29 + 8);
          v31 = *(DWORD *)(v30 + 8);
          if ( v31 != DAT_055c9b98 )
          {
            *(DWORD *)(v31 + 4) = v29;
          }
          *(DWORD *)(v30 + 4) = v29[1];
          v32 = v10[1];
          if ( v29 == *(DWORD **)(v32 + 4) )
          {
            *(DWORD *)(v32 + 4) = v30;
          }
          else
          {
            v33 = (DWORD *)v29[1];
            if ( v29 == (DWORD *)v33[2] )
            {
              v33[2] = v30;
            }
            else
            {
              *v33 = v30;
            }
          }
          *(DWORD *)(v30 + 8) = v29;
          v29[1] = v30;
          v24 = *(DWORD **)v4[1];
        }
        if ( *(DWORD *)(v24[2] + 308) != 1 || *(DWORD *)(*v24 + 308) != 1 )
        {
          if ( *(DWORD *)(*v24 + 308) == 1 )
          {
            *(DWORD *)(v24[2] + 308) = 1;
            v42 = (DWORD *)v24[2];
            v24[77] = 0;
            v24[2] = *v42;
            if ( *v42 != DAT_055c9b98 )
            {
              *(DWORD *)(*v42 + 4) = v24;
            }
            v42[1] = v24[1];
            v43 = v10[1];
            if ( v24 == *(DWORD **)(v43 + 4) )
            {
              *(DWORD *)(v43 + 4) = v42;
            }
            else
            {
              v44 = (DWORD *)v24[1];
              if ( v24 == (DWORD *)*v44 )
              {
                *v44 = v42;
              }
              else
              {
                v44[2] = v42;
              }
            }
            *v42 = v24;
            v24[1] = v42;
            v24 = *(DWORD **)v4[1];
          }
          v24[77] = *(DWORD *)(v4[1] + 308);
          *(DWORD *)(v4[1] + 308) = 1;
          *(DWORD *)(*v24 + 308) = 1;
          v38 = (DWORD *)v4[1];
          v39 = (DWORD *)*v38;
          *v38 = *(DWORD *)(*v38 + 8);
          v45 = v39[2];
          if ( v45 != DAT_055c9b98 )
          {
            *(DWORD *)(v45 + 4) = v38;
          }
          v39[1] = v38[1];
          v46 = v10[1];
          if ( v38 == *(DWORD **)(v46 + 4) )
          {
            *(DWORD *)(v46 + 4) = v39;
          }
          else
          {
            v47 = (DWORD *)v38[1];
            if ( v38 == (DWORD *)v47[2] )
            {
              v47[2] = v39;
            }
            else
            {
              *v47 = v39;
            }
          }
          v39[2] = v38;
          goto LABEL_100;
        }
      }
      v24[77] = 0;
    }
    v4[77] = 1;
  }
  std::_Lockit::~_Lockit((std::_Lockit *)v53);
  delete__(lpMem);
  v48 = a3;
  --v10[3];
  result = a2;
  *a2 = v48;
  return result;
}
#endif
