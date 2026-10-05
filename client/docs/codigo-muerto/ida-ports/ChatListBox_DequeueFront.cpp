// 0x0040C580 ChatListBox_DequeueFront — nunca activado: IDA_PORT_0040C580 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── ChatListBox_DequeueFront (IDA-only, gated) ──
#if defined(IDA_PORT_0040C580)
int __cdecl ChatListBox_DequeueFront(int _this)
{
  int result; // eax
  DWORD *v3; // eax
  DWORD **v4; // [esp-4h] [ebp-8h]

  result = *(DWORD *)(_this + 12);
  if ( result )
  {
    v3 = **(DWORD ***)(_this + 8);
    *(DWORD *)(_this + 16) = v3[2];
    *(DWORD *)(_this + 20) = v3[3];
    *(DWORD *)(_this + 24) = v3[4];
    v4 = **(DWORD ****)(_this + 8);
    *v4[1] = *v4;
    (*v4)[1] = v4[1];
    delete__(v4);
    result = *(DWORD *)(_this + 12) - 1;
    *(DWORD *)(_this + 12) = result;
  }
  return result;
}
#endif
