// 0x004E13A0 RenderObjectScreen — nunca activado: IDA_PORT_004E13A0 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── RenderObjectScreen (IDA-only, gated) ──
#if defined(IDA_PORT_004E13A0)
void __cdecl RenderObjectScreen(int Type, int ItemLevel, int Option1, float Target[3], int Select, bool PickUp)
{
  int Level; // edi
  short v9; // ax
  DWORD v10; // ecx
  int v11; // [esp+30h] [ebp-3BCh]
  float Position[3]; // [esp+34h] [ebp-3B8h] BYREF
  float Direction[3]; // [esp+40h] [ebp-3ACh] BYREF
  float Light[3]; // [esp+4Ch] [ebp-3A0h] BYREF
  DWORD o; // [esp+58h] [ebp-394h] BYREF
  int v16; // [esp+64h] [ebp-388h]
  float v17; // [esp+68h] [ebp-384h]
  float v18; // [esp+6Ch] [ebp-380h]
  float v19; // [esp+70h] [ebp-37Ch]
  char v20; // [esp+134h] [ebp-2B8h]
  float v21; // [esp+1C0h] [ebp-22Ch]
  char v22; // [esp+214h] [ebp-1D8h]

  Direction[0] = *Target - MousePosition[0];
  Level = (ItemLevel >> 3) & 0xF;
  Direction[1] = Target[1] - MousePosition[1];
  Direction[2] = Target[2] - MousePosition[2];
  if ( PickUp )
  {
    VectorMA(MousePosition, 0.07, Direction, Position);
  }
  else
  {
    VectorMA(MousePosition, 0.1, Direction, Position);
  }
  if ( Type == 535 || Type == 543 )
  {
    Angle[0] = 0.0;
    DAT_07ea9530 = 1132920832;
    goto LABEL_50;
  }
  if ( Type == 545 )
  {
    Angle[0] = 0.0;
    DAT_07ea9530 = 1119092736;
LABEL_50:
    DAT_07ea9534 = 1097859072;
    goto LABEL_51;
  }
  if ( Type >= 536 && Type < 560 )
  {
    Angle[0] = 90.0;
    DAT_07ea9530 = 1127481344;
    DAT_07ea9534 = 1101004800;
    goto LABEL_51;
  }
  if ( Type == 506 )
  {
    Angle[0] = 180.0;
    DAT_07ea9530 = 1132920832;
    DAT_07ea9534 = 1101004800;
    goto LABEL_51;
  }
  if ( Type >= 400 )
  {
    if ( Type < 592 )
    {
      Angle[0] = 180.0;
      DAT_07ea9530 = 1132920832;
      if ( *((BYTE *)&ItemAttribute[Type - 399] - 34) )
      {
        DAT_07ea9534 = 1103626240;
        goto LABEL_51;
      }
      goto LABEL_50;
    }
    if ( Type < 624 )
    {
      Angle[0] = 270.0;
      DAT_07ea9530 = 1132920832;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    }
  }
  switch ( Type )
  {
    case 819:
      Angle[0] = -90.0;
      DAT_07ea9530 = -1028390912;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    case 832:
    case 833:
      Angle[0] = 270.0;
      goto LABEL_48;
    case 834:
      Angle[0] = 290.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    case 958:
      Angle[0] = -90.0;
      DAT_07ea9530 = -1046478848;
      DAT_07ea9534 = -1046478848;
      goto LABEL_51;
  }
  if ( Type >= 828 && Type < 848 && Type != 830 && Type != 831 )
  {
    Angle[0] = 360.0;
    DAT_07ea9530 = 0;
    DAT_07ea9534 = 0;
    goto LABEL_51;
  }
  if ( Type != 860 )
  {
    switch ( Type )
    {
      case 952:
        Angle[0] = 270.0;
        DAT_07ea9530 = 0;
        DAT_07ea9534 = 0;
        goto LABEL_51;
      case 953:
        goto LABEL_42;
      case 954:
        Angle[0] = 270.0;
        DAT_07ea9530 = 0;
        DAT_07ea9534 = 0;
        goto LABEL_51;
    }
    Angle[0] = 270.0;
    if ( Type == 868 )
    {
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      goto LABEL_51;
    }
LABEL_48:
    DAT_07ea9530 = -1054867456;
    DAT_07ea9534 = 0;
    goto LABEL_51;
  }
  switch ( Level )
  {
    case 0:
      Angle[0] = 180.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      break;
    case 1:
LABEL_42:
      Angle[0] = 270.0;
      DAT_07ea9530 = 1119092736;
      DAT_07ea9534 = 0;
      break;
    case 2:
      Angle[0] = 90.0;
      DAT_07ea9530 = 0;
      DAT_07ea9534 = 0;
      break;
  }
LABEL_51:
  if ( Select == 1 )
  {
    *(float *)&DAT_07ea9530 = WorldTime * 0.44999999;
  }
  v9 = Type;
  ObjectSelect_Type = Type;
  if ( (short)Type < 624 || (short)Type >= 784 )
  {
    if ( (WORD)Type == 860 )
    {
      if ( Level )
      {
        if ( Level == 2 )
        {
          v9 = 948;
          Type = 948;
          ObjectSelect_Type = 948;
        }
      }
      else
      {
        v9 = 947;
        Type = 947;
        ObjectSelect_Type = 947;
      }
    }
  }
  else
  {
    v9 = 390;
    ObjectSelect_Type = 390;
  }
  ObjectSelect_AnimationFrame = 0;
  ObjectSelect_PriorAnimationFrame = 0;
  ObjectSelect_PriorAction = 0;
  v10 = Models + 188 * v9;
  *(BYTE *)(v10 + 160) = 0;
  if ( Type >= 624 )
  {
    if ( Type < 656 )
    {
      *(DWORD *)(v10 + 132) = -1021313024;
      goto LABEL_74;
    }
    if ( Type < 688 )
    {
      *(DWORD *)(v10 + 132) = -1027080192;
      goto LABEL_74;
    }
  }
  if ( Type < 720 || Type >= 752 )
  {
    if ( Type < 688 || Type >= 720 )
    {
      *(DWORD *)(v10 + 132) = 0;
    }
    else
    {
      *(DWORD *)(v10 + 132) = -1035468800;
    }
  }
  else
  {
    *(DWORD *)(v10 + 132) = -1031012352;
  }
LABEL_74:
  if ( Type >= 624 && Type < 784 )
  {
    if ( Type >= 656 )
    {
      if ( Type >= 688 )
      {
        if ( Type < 720 || Type >= 752 )
        {
          if ( Type >= 720 )
          {
            v11 = 995211031;
          }
          else
          {
            v11 = 995640528;
          }
        }
        else
        {
          v11 = 997788012;
        }
      }
      else
      {
        v11 = 998217508;
      }
    }
    else
    {
      v11 = 998217508;
    }
    goto LABEL_142;
  }
  if ( Type == 790 )
  {
    v11 = 985963430;
    goto LABEL_142;
  }
  if ( Type >= 784 && Type < 816 )
  {
    v11 = 990057071;
    goto LABEL_142;
  }
  switch ( Type )
  {
    case 869:
      v11 = 990057071;
      goto LABEL_142;
    case 958:
      v11 = 985963430;
      goto LABEL_142;
    case 832:
      v11 = 990057071;
      goto LABEL_142;
    case 833:
      v11 = 988540410;
      goto LABEL_142;
    case 834:
      v11 = 988540410;
      goto LABEL_142;
    case 419:
      if ( ItemLevel >= 0 )
      {
        v11 = 992204554;
        goto LABEL_142;
      }
LABEL_104:
      v11 = 981668463;
      ItemLevel = 0;
      goto LABEL_142;
    case 570:
      if ( ItemLevel >= 0 )
      {
        v11 = 989399404;
        goto LABEL_142;
      }
      goto LABEL_104;
    case 546:
      if ( ItemLevel < 0 )
      {
        v11 = 985963430;
        ItemLevel = 0;
        goto LABEL_142;
      }
      break;
    default:
      if ( Type >= 870 )
      {
        if ( Type < 873 )
        {
          v11 = 992204554;
          goto LABEL_142;
        }
        if ( Type < 875 )
        {
          v11 = 993493044;
          goto LABEL_142;
        }
      }
      if ( Type == 830 || Type == 831 )
      {
        v11 = 994352038;
        goto LABEL_142;
      }
      if ( Type >= 848 && Type < 880 )
      {
        v11 = 996499522;
        goto LABEL_142;
      }
      if ( Type >= 496 && Type < 528 )
      {
        v11 = 988540410;
        goto LABEL_142;
      }
      if ( Type >= 560 && Type < 592 )
      {
        v11 = 990916064;
        goto LABEL_142;
      }
      switch ( Type )
      {
        case 543:
          v11 = 982527456;
          goto LABEL_142;
        case 535:
          v11 = 983386450;
          goto LABEL_142;
        case 953:
          v11 = 998217508;
          goto LABEL_142;
        case 955:
          v11 = 985963430;
          goto LABEL_142;
        case 956:
          v11 = 989399404;
          goto LABEL_142;
        case 957:
          v11 = 981668463;
          goto LABEL_142;
      }
      break;
  }
  v11 = 0x3B23D70A;
LABEL_142:
  BMD_Animation(v10, (float (*)[3][4])BoneMatrix, 0.0, 0.0, 0, Angle, ObjectSelect_HeadAngle, 0, 0);
  (WORD)((o) >> 16) = Type;
  ItemObjectAttribute((DWORD)&o);
  v17 = Position[0];
  v16 = v11;
  v18 = Position[1];
  v19 = Position[2];
  v20 = 0;
  v22 = 2;
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  RenderPartObject((DWORD)&o, Type, 0, Light, v21, ItemLevel, Option1, 1, 1, 1, 0, 2);
}
#endif
