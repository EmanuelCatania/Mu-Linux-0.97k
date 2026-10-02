// 0x00409B60 scalar_deleting_destructor_locale — nunca activado: IDA_PORT_00409B60 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── scalar_deleting_destructor_locale (IDA-only, gated) ──
#if defined(IDA_PORT_00409B60)
std::locale::_Locimp *__cdecl std::locale::_Locimp::`scalar deleting destructor'(
        std::locale::_Locimp *_this,
        unsigned int a2)
{
  std::locale::_Locimp::~_Locimp(_this);
  if ( (a2 & 1) != 0 )
  {
    delete__(_this);
  }
  return _this;
}
#endif
