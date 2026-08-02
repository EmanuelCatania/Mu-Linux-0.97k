add_library(Main SHARED
  Buff.cpp
  BuffDisplay.cpp
  Camera3D.cpp
  CCRC32.Cpp
  ChaosMix.cpp
  Console.cpp
  Controller.cpp
  CustomBow.cpp
  CustomGlow.cpp
  CustomItem.cpp
  CustomMonster.cpp
  CustomWing.cpp
  EventTimer.cpp
  Font.cpp
  GoldenArcher.cpp
  GuildLogo.cpp
  HackCheck.cpp
  HealthBar.cpp
  HWID.cpp
  Interface.cpp
  Item.cpp
  ItemLink.cpp
  ItemLinkChat.cpp
  ItemLinkTooltip.cpp
  ItemManager.cpp
  ItemOption.cpp
  ItemPosition.cpp
  ItemStack.cpp
  ItemValue.cpp
  Language.cpp
  LoadModels.cpp
  Main.cpp
  Main.rc
  MapFog.cpp
  MapManager.cpp
  MathLib.cpp
  MiniMap.cpp
  MoveList.cpp
	Notification.cpp
  MP3.cpp
  OpenglUtil.cpp
  OptionsMenu.cpp
  PacketManager.cpp
  Patchs.cpp
  PingSystem.cpp
  PrintPlayer.cpp
  Protect.cpp
  Protocol.cpp
  Reconnect.cpp
  RightClickMove.cpp
  ServerList.cpp
  Sound.cpp
  stb.cpp
  stdafx.cpp
  Util.cpp
  WeaponView.cpp
  Window.cpp
  Input.cpp)

mu_configure_client_output(Main)
set_target_properties(Main PROPERTIES
  DEFINE_SYMBOL ""
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

target_compile_definitions(Main PRIVATE
  WIN32
  _MBCS
  GLEW_STATIC
  _WINDOWS
  _USRDLL
  MAIN_EXPORTS
  "$<$<CONFIG:Debug>:_DEBUG>"
  "$<$<CONFIG:Release>:NDEBUG>")

target_compile_options(Main PRIVATE
  /W3
  "$<$<CONFIG:Release>:/Gy>"
  "$<$<CONFIG:Release>:/Oi>")

target_include_directories(Main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Main PRIVATE user32 shell32 advapi32 gdi32 ole32)
target_link_options(Main PRIVATE
  /NODEFAULTLIB:libcmt.lib
  /MANIFEST:NO
  /SUBSYSTEM:WINDOWS
  "$<$<CONFIG:Release>:/OPT:REF>"
  "$<$<CONFIG:Release>:/OPT:ICF>"
  "$<$<CONFIG:Debug>:/MAP:$<TARGET_FILE_DIR:Main>/Main.map>")

target_precompile_headers(Main PRIVATE stdafx.h)
set_source_files_properties(CCRC32.Cpp PROPERTIES LANGUAGE CXX)
set_source_files_properties(stb.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

set_property(TARGET Main PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
