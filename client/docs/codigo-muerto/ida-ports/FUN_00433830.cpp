// 0x00433830 FUN_00433830 — nunca activado: IDA_PORT_00433830 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00433830 (IDA-only, gated) ──
#if defined(IDA_PORT_00433830)
void __cdecl FUN_00433830(BYTE *ReceiveBuffer)
{
  BYTE *v1; // esi
  int v2; // edi
  DWORD *p_Key; // esi

  if ( EquipmentItem )
  {
    if ( !g_bPacketAfter_EquipmentItem )
    {
      g_bPacketAfter_EquipmentItem = 1;
      *(DWORD *)g_byPacketAfter_EquipmentItem = *(DWORD *)ReceiveBuffer;
    }
  }
  else
  {
    v1 = ReceiveBuffer;
    if ( !ReceiveBuffer[3] )
    {
      UIChatLogWindow_AddText(DAT_05826d54, GlobalText[494], 2);
      DAT_07eaa0e8 = 0;
      v2 = 0;
      p_Key = &Inventory[0].Key;
      do
      {
        if ( *((WORD *)p_Key - 28) != 0xFFFF && *p_Key )
        {
          FUN_004cc530(v2, (int)Inventory);
        }
        p_Key += 17;
        ++v2;
      }
      while ( (int)p_Key < (int)&Inventory[32].Key );
      v1 = ReceiveBuffer;
    }
    if ( v1[3] == 2 )
    {
      UIChatLogWindow_AddText(DAT_05826d58, GlobalText[495], 2);
    }
    if ( v1[3] == 3 )
    {
      UIChatLogWindow_AddText(DAT_05826d5c, GlobalText[496], 2);
      SetErrorMessage(0);
    }
    InventoryOpened = 0;
    CloseInventoryRelatedWindows();
    DAT_07e91388 = 0;
    if ( ErrorMessage == 116 )
    {
      SetErrorMessage(0);
      ClearInput(0);
      InputTextMax[0] = 42;
      InputNumber = 2;
      GoldInputEnable = 0;
      InputEnable = 0;
    }
  }
}
#endif
