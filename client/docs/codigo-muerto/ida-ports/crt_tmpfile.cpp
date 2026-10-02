// 0x00543D81 crt_tmpfile — nunca activado: IDA_PORT_00543D81 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── crt_tmpfile (IDA-only, gated) ──
#if defined(IDA_PORT_00543D81)
FILE *__cdecl crt_tmpfile()
{
  FILE *v0; // esi
  int v1; // ebp
  char *v2; // eax
  int v3; // eax

  _lock(3);
  if ( FileName )
  {
    if ( genfname((unsigned char *)&FileName) )
    {
      goto LABEL_13;
    }
  }
  else
  {
    init_namebuf(1);
  }
  v0 = (FILE *)_getstream();
  if ( v0 )
  {
    v1 = _sopen(&FileName, 34114, 64, 384);
    if ( v1 != -1 )
    {
      goto LABEL_10;
    }
    do
    {
      if ( *_errno() != 17 )
      {
        break;
      }
      if ( genfname((unsigned char *)&FileName) )
      {
        break;
      }
      v1 = _sopen(&FileName, 34114, 64, 384);
    }
    while ( v1 == -1 );
    if ( v1 != -1 )
    {
LABEL_10:
      v2 = _strdup(&FileName);
      v0->_tmpfname = v2;
      if ( v2 )
      {
        v0->_cnt = 0;
        v0->_ptr = 0;
        v0->_base = 0;
        v3 = DAT_083bbe30;
        (BYTE)(v3) = DAT_083bbe30 | 0x80;
        v0->_flag = v3;
        v0->_file = v1;
        _unlock_file(v0);
        goto LABEL_15;
      }
      _close(v1);
    }
    _unlock_file(v0);
  }
LABEL_13:
  v0 = 0;
LABEL_15:
  _unlock(3);
  return v0;
}
#endif
