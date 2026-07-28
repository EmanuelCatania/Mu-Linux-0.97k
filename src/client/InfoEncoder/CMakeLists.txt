add_executable(InfoEncoder
  CCRC32.cpp
  CustomBow.cpp
  CustomGlow.cpp
  CustomItem.cpp
  CustomMonster.cpp
  CustomWing.cpp
  InfoEncoder.cpp
  InfoEncoder.rc
  ItemOption.cpp
  ItemPosition.cpp
  MapFog.cpp
  MapManager.cpp
  ReadScript.cpp
  stdafx.cpp)

mu_configure_client_output(InfoEncoder)
set_target_properties(InfoEncoder PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

target_compile_definitions(InfoEncoder PRIVATE
  WIN32
  _MBCS
  _CONSOLE
  "$<$<CONFIG:Debug>:_DEBUG>"
  "$<$<CONFIG:Release>:NDEBUG>")

target_compile_options(InfoEncoder PRIVATE
  /W3
  "$<$<CONFIG:Release>:/Gy>"
  "$<$<CONFIG:Release>:/Oi>")

target_include_directories(InfoEncoder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(InfoEncoder PRIVATE user32)
target_link_options(InfoEncoder PRIVATE
  /MANIFEST:NO
  /SUBSYSTEM:CONSOLE
  "$<$<CONFIG:Release>:/OPT:REF>"
  "$<$<CONFIG:Release>:/OPT:ICF>"
  "$<$<CONFIG:Debug>:/MAP:$<TARGET_FILE_DIR:InfoEncoder>/InfoEncoder.map>")

target_precompile_headers(InfoEncoder PRIVATE stdafx.h)

set_property(TARGET InfoEncoder PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
