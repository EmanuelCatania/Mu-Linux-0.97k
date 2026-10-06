# CMakeLists.txt — build del cliente Mu Online 0.97k.
#
# Replica mu97k.vcxproj (Win32, toolset v143). Los flags se fijan a mano en vez
# de usar los defaults de CMake para que la linea de comandos de cl/link sea la
# misma que la del .vcxproj. tools/check_cmake_sources.py verifica en CI que la
# lista de fuentes coincide con los ClCompile/ClInclude del .vcxproj.
#
# Uso:
#   cmake -S . -B build -A Win32
#   cmake --build build --config Release
#
# Salida: bin/Client/main.exe + bin/Client/main.map (igual que el .vcxproj).

cmake_minimum_required(VERSION 3.25)

# CMP0091: el runtime de MSVC se elige con MSVC_RUNTIME_LIBRARY.
# CMP0141: el formato de debug info se elige con MSVC_DEBUG_INFORMATION_FORMAT.
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0141 NEW)

project(mu97k LANGUAGES C CXX RC)

if(NOT MSVC)
    message(FATAL_ERROR "mu97k sólo compila con MSVC (Visual Studio 2022, toolset v143).")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "mu97k es Win32 (x86). Configurar con: cmake -A Win32")
endif()

# Sólo existen Debug y Release, como en el .sln.
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

# Se vacían los flags por defecto de CMake (/RTC1, /Ob0, /Ob2, /DNDEBUG, /W1...,
# y -DWIN32/-D_DEBUG de rc.exe) para que sólo queden los que pone cada target,
# igual que en el .vcxproj.
foreach(lang C CXX RC)
    set(CMAKE_${lang}_FLAGS "")
    set(CMAKE_${lang}_FLAGS_DEBUG "")
    set(CMAKE_${lang}_FLAGS_RELEASE "")
endforeach()
foreach(kind EXE STATIC SHARED MODULE)
    set(CMAKE_${kind}_LINKER_FLAGS "")
    set(CMAKE_${kind}_LINKER_FLAGS_DEBUG "")
    set(CMAKE_${kind}_LINKER_FLAGS_RELEASE "")
endforeach()

# Opciones de compilación comunes a mu97k y libjpeg, equivalentes a los defaults
# de v143 que el .vcxproj no pisa (msbuild las pasa explícitas a cl):
#   /EHsc  ExceptionHandling=Sync          /FC  UseFullPaths (afecta a __FILE__)
#   /Gd /Gm- /GS /Zc:forScope /Zc:inline /Zc:wchar_t   defaults de v143
#   /JMC   SupportJustMyCode (Debug)       /Od  Optimization=Disabled (Debug)
#   /O2 /Oi /Gy /GL  MaxSpeed + IntrinsicFunctions + FunctionLevelLinking +
#                    WholeProgramOptimization (Release)
set(MU_COMMON_COMPILE_OPTIONS
    /EHsc
    /FC
    /Gd
    /Gm-
    /GS
    /Zc:forScope
    /Zc:inline
    /Zc:wchar_t
    $<$<CONFIG:Debug>:/JMC>
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Release>:/Oi>
    $<$<CONFIG:Release>:/Gy>
    $<$<CONFIG:Release>:/GL>
)

# UseDebugLibraries=true en Debug -> /MDd; Release -> /MD.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
# Debug -> /ZI (EditAndContinue, default de v143 en Debug); Release -> /Zi.
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<IF:$<CONFIG:Debug>,EditAndContinue,ProgramDatabase>")

add_subdirectory(lib/libjpeg)

# ── Fuentes ─────────────────────────────────────────────────────────────
# Copiadas de los <ClCompile> del .vcxproj, en el mismo orden. NO usar glob:
# la lista explícita es la que se compara contra el .vcxproj
# (tools/check_cmake_sources.py).
set(MU_SOURCES
    src/stdafx.cpp
    src/WinMain.cpp
    src/Local/Text_Data.cpp
    src/Scene/Scene_Dispatch.cpp
    src/Scene/Scene_Intro.cpp
    src/Scene/Scene_Login.cpp
    src/Scene/Scene_Loading.cpp
    src/Scene/Scene_CharSelect.cpp
    src/Game/Game_MainLoop.cpp
    src/Game/Player_InputTick.cpp
    src/Render/Render_Frame.cpp
    src/Net/Net_Process.cpp
    src/Config/Config_Load.cpp
    src/Core/Window.cpp
    src/Core/Font.cpp
    src/Sound/SoundManager.cpp
    src/Game/MapManager.cpp
    src/Core/WndProc.cpp
    src/Net/Network.cpp
    src/Input/Input_WndProc.cpp
    src/Config/UserSettings.cpp
    src/Sound/Sound_Queue.cpp
    src/Monster/Monster_Data.cpp
    src/Terrain/Terrain_Utils.cpp
    src/Math/Math_3D.cpp
    src/Render/Particle.cpp
    src/Render/Particle_Move.cpp
    src/Render/BMD_Anim.cpp
    src/Render/BMD_DrawMesh.cpp
    src/Render/BMD_Collision.cpp
    src/Render/MoveEffect.cpp
    src/Render/MoveEffect_Helpers.cpp
    src/Render/MoveJoint.cpp
    src/Render/BMD_SetupRender.cpp
    src/Game/PathFinder.cpp
    src/Entity/Entity_Reset.cpp
    src/Entity/Entity_HeadMotion.cpp
    src/Render/GL_State.cpp
    src/Render/Effect_Create.cpp
    src/Render/Effect_Combat.cpp
    src/Render/Joint_Create.cpp
    src/Render/Joint_Render.cpp
    src/Render/Sprite.cpp
    src/Render/Camera.cpp
    src/Render/Camera_Movement.cpp
    src/Render/Entity_Render.cpp
    src/Net/Crypto.cpp
    src/Game/Timer.cpp
    src/Sound/Music.cpp
    src/Terrain/Terrain_Tile.cpp
    src/Render/GL_2D.cpp
    src/Render/Effect_Tick.cpp
    src/Terrain/Terrain_Light.cpp
    src/Terrain/Terrain_RenderFace.cpp
    src/Terrain/Terrain_RenderBlocks.cpp
    src/Sound/Sound_DS3D.cpp
    src/Net/Net_Connect.cpp
    src/Net/Net_PacketSession.cpp
    src/Net/MuEmu.cpp
    src/Net/PacketFrame.cpp
    src/Net/Net_Events.cpp
    src/Net/HWID.cpp
    src/Entity/Entity_Init.cpp
    src/Entity/Entity_MoveClient.cpp
    src/Entity/Entity_Spawn.cpp
    src/Render/Scene_CharPreview.cpp
    src/Scene/Scene_Resources.cpp
    src/Scene/Scene_MapTick.cpp
    src/Scene/Scene_ServerSelect_Input.cpp
    src/UI/Chat.cpp
    src/UI/Chat_Bubbles.cpp
    src/UI/Chat_CommandParser.cpp
    src/UI/ChatListBox.cpp
    src/UI/CharMenu_Build.cpp
    src/Render/HUD_Pass1.cpp
    src/Render/HUD_Pass2.cpp
    src/Render/HUD_Pass3.cpp
    src/Render/HUD_Pass4.cpp
    src/Render/HUD_Pass5.cpp
    src/Render/HUD_Pass6.cpp
    src/UI/UI_StatsPanel.cpp
    src/Util/Misc.cpp
    src/Core/Legacy_Runtime.cpp
    src/Entity/Entity_Lookup.cpp
    src/Net/Net_Disconnect.cpp
    src/Render/Render_MatrixStack.cpp
    src/Scene/Scene_ObjectTick.cpp
    src/Render/Particle_Legacy.cpp
    src/Render/Camera_Login.cpp
    src/UI/Chat_Log.cpp
    src/Terrain/Terrain_RayCollision.cpp
    src/Debug/MiniDump.cpp
    src/Combat/Combat_AttackEffect.cpp
    src/Core/Runtime_Small.cpp
    src/Core/Runtime_Medium.cpp
    src/UI/Quest_Legacy.cpp
    src/Core/System_Legacy.cpp
    src/GameGuard/GameGuard_Packet.cpp
    src/Render/SMD_Legacy.cpp
    src/UI/UI_LegacyWidgets.cpp
    src/Net/Net_LegacyRuntime.cpp
    src/Scene/Scene_CharSelect_Nav.cpp
    src/Item/Item_LegacyHelpers.cpp
    src/UI/UI_DialogInterface.cpp
    src/Item/Item_Durability.cpp
    src/Item/Item_ServerValue.cpp
    src/Render/BMD_LegacyDraw.cpp
    src/Config/Config_TokenParser.cpp
    src/Entity/Entity_BoneEffects.cpp
    src/Net/Pipe_Legacy.cpp
    src/Core/Runtime_Linker.cpp
    src/GameGuard/GameGuard_LegacyHealth.cpp
    src/Item/Item_LegacyLinker.cpp
    src/Math/Math_LegacyTransforms.cpp
    src/Render/Render_LegacyLinker.cpp
    src/Scene/Scene_LegacyCleanup.cpp
    src/Scene/Scene_ObjectLegacy.cpp
    src/Terrain/Terrain_LegacyRender.cpp
    src/UI/UI_LegacyText.cpp
    src/Render/BMD_FilterData.cpp
    src/Item/Quest_UIState.cpp
    src/Physics/Cloth_Simulation.cpp
    src/Render/BMD_LegacyEffects.cpp
    src/Scene/Scene_LegacyHelpers.cpp
    src/Path/Path_Legacy.cpp
    src/Core/Runtime_Externs.cpp
    src/UI/UI_LegacyExterns.cpp
    src/Entity/Entity_ActionLegacy.cpp
    src/Render/Render_LegacyRotatedRect.cpp
    src/UI/UI_GuildLegacy.cpp
    src/Core/String_ResourceLegacy.cpp
    src/Entity/Entity_LegacyEffects.cpp
    src/Combat/Combat_LegacyAnimation.cpp
    src/Render/Render_LegacyTransforms.cpp
    src/Render/Render_LegacyBillboards.cpp
    src/UI/UI_LegacyWidgetSystem.cpp
    src/Render/Render_LegacyTrails.cpp
    src/UI/UI_LegacyGameHelpers.cpp
    src/Net/Net_LegacyAuth.cpp
    src/Terrain/Terrain_LegacyLoad.cpp
    src/Path/Path_LegacyReset.cpp
    src/Render/Font_LegacyFactory.cpp
    src/Render/SMD_Parser.cpp
    src/Render/Entity_AnimationLegacy.cpp
    src/Combat/Combat_LegacyAttackEffects.cpp
    src/Entity/Entity_LegacyPoolCleanup.cpp
    src/Item/Item_LegacyGridInsert.cpp
    src/Item/Item_TypeLegacy.cpp
    src/Entity/Entity_LegacyTeleport.cpp
    src/Combat/Skills_WarriorLegacy.cpp
    src/Render/Render_LegacyDamageNumbers.cpp
    src/Render/GL_LegacyState.cpp
    src/Math/Math_LegacyAngle.cpp
    src/Render/BMD_LegacyLoading.cpp
    src/Input/Cursor_Render.cpp
    src/Input/Input_Timing.cpp
    src/Util/Process_Exit.cpp
    src/Net/Net_Bux.cpp
    src/Scene/Scene_EntityLifecycle.cpp
    src/Scene/Scene_OpenWorld.cpp
    src/Scene/Scene_AssetLoad.cpp
    src/Render/Camera_Unproject.cpp
    src/Net/Net_PipeQuery.cpp
    src/Net/Protocol/Protocol_Check.cpp
    src/GameGuard/GameGuard_InitTrampoline.cpp
    src/Render/Render_WorldHelpers.cpp
    src/Render/BMD_Load.cpp
    src/globals.cpp
    src/Model/Model_Items.cpp
    src/Model/Model_Monsters.cpp
    src/Model/Model_Players.cpp
    src/Model/Model_Effects.cpp
    src/Model/Model_Misc.cpp
    src/Model/Model_SkillEffects.cpp
    src/Model/Model_Gates.cpp
    src/Combat/Combat.cpp
    src/Combat/Combat_AttackStage.cpp
    src/Combat/Combat_Projectiles.cpp
    src/Combat/Combat_Targeting.cpp
    src/Combat/Skills.cpp
    src/Combat/Skills_WizardElf.cpp
    src/Game/Game_CharSelectTick.cpp
    src/Game/Game_EnterWorldTick.cpp
    src/Game/Game_SceneUpdate.cpp
    src/Game/Movement.cpp
    src/Input/Input.cpp
    src/Input/Mouse_Hover.cpp
    src/Item/Item_Equip.cpp
    src/Item/Item_Move.cpp
    src/Item/Item_Display.cpp
    src/Item/Item_SpecialOption.cpp
    src/Item/Item_Data.cpp
    src/Item/Item_Inventory.cpp
    src/Item/Item_EquipmentQueries.cpp
    src/Item/Item_EquipmentAutoSwap.cpp
    src/Item/Item_InventoryGrid.cpp
    src/Item/Item_InventoryRender.cpp
    src/Item/Item_ChaosMix.cpp
    src/Item/Item_ClickHandler.cpp
    src/Item/Skill_Data.cpp
    src/Item/Gate_Data.cpp
    src/Item/Filter_Data.cpp
    src/Item/Dialog_Data.cpp
    src/Item/Quest_Data.cpp
    src/Item/NPC_Data.cpp
    src/Item/NPC_ModelLoad.cpp
    src/Render/Font_Layout.cpp
    src/Monster/Monster.cpp
    src/Net/SecondPassword.cpp
    src/Party/Party.cpp
    src/Game/Party_NameMatch.cpp
    src/Render/Ambient_Particles.cpp
    src/Render/Entity_DrawByType.cpp
    src/Render/Entity_DrawAt.cpp
    src/Render/Entity_DrawSetup.cpp
    src/Render/Entity_PrepareRender.cpp
    src/Render/Entity_Render_3D.cpp
    src/Render/Entity_RenderAll_3D.cpp
    src/Render/Entity_UpdateRender.cpp
    src/Render/Render_PlayerEquipment.cpp
    src/Render/HUD_Render.cpp
    src/Render/ItemDrop_Render.cpp
    src/Render/ItemDrop_Render2.cpp
    src/Render/Particle_Draw.cpp
    src/Render/Particle_Render.cpp
    src/Render/Particle_Spawn.cpp
    src/Render/Player_Render.cpp
    src/Render/RenderLinkObject.cpp
    src/Render/SkillEffect_Render.cpp
    src/Render/Sprite_Draw.cpp
    src/Render/Render_SpriteHelpers.cpp
    src/Render/Terrain_Render.cpp
    src/Render/Terrain_Water.cpp
    src/Render/Texture/Texture.cpp
    src/Render/Trail_Render.cpp
    src/Render/Weather.cpp
    src/Render/Weather_Particles.cpp
    src/Scene/Scene_Login_ServerSelect.cpp
    src/Sound/Sound.cpp
    src/Terrain/Terrain.cpp
    src/Trade/Trade.cpp
    src/UI/UI_InGameMenu.cpp
    src/UI/UI_IME.cpp
    src/UI/UI_TextBitmap.cpp
    src/UI/UI_Tooltip.cpp
    src/UI/Chat_Gestures.cpp
    src/UI/GoldenArcher.cpp
    src/UI/Chat_InputTick.cpp
    src/UI/Chat_Send.cpp
    src/UI/UI_SkillHotkeys.cpp
    src/UI/UI_GuildMark.cpp
    src/UI/RenderItemInfo.cpp
    src/Scene/Scene_Objects.cpp
    src/Scene/Scene_ObjectInteraction.cpp
    src/Scene/Scene_ObjectUpdate.cpp
    src/Render/BMD_SkinUpdate.cpp
)

# <ClInclude> del .vcxproj: sólo para que aparezcan en el IDE.
set(MU_HEADERS
    src/stdafx.h
    src/functions.h
    src/globals.h
    src/structs.h
    src/ghidra_compat.h
    src/types.h
    src/Config/Config.h
    src/Config/ServerConfig.h
    src/Config/UserSettings.h
    src/Core/Window.h
    src/Core/Font.h
    src/Sound/SoundManager.h
    src/Game/MapManager.h
    src/Net/Network.h
    src/Combat/Combat.h
    src/Combat/Skills.h
    src/Game/Game_CharSelectTick.h
    src/Game/Game_EnterWorldTick.h
    src/Game/Game_MainLoop.h
    src/Game/Game_SceneUpdate.h
    src/Game/Movement.h
    src/Input/Input.h
    src/Item/Item_Equip.h
    src/Monster/Monster.h
    src/Net/Net.h
    src/Net/HWID.h
    src/Net/PacketFrame.h
    src/Net/SecondPassword.h
    src/Party/Party.h
    src/Render/Camera.h
    src/Render/Entity_DrawAt.h
    src/Render/Entity_PrepareRender.h
    src/Render/Entity_Render.h
    src/Render/Entity_Render_3D.h
    src/Render/Entity_RenderAll_3D.h
    src/Render/HUD_Render.h
    src/Render/ItemDrop_Render.h
    src/Render/Particle_Draw.h
    src/Render/Particle_Render.h
    src/Render/Player_Render.h
    src/Render/Render.h
    src/Render/SkillEffect_Render.h
    src/Render/Sprite_Draw.h
    src/Render/Terrain_Render.h
    src/Render/Texture/Texture.h
    src/Scene/Scene.h
    src/Scene/Scene_CharSelect.h
    src/Scene/Scene_Dispatch.h
    src/Scene/Scene_Intro.h
    src/Scene/Scene_Loading.h
    src/Scene/Scene_Login.h
    src/Scene/Scene_Login_ServerSelect.h
    src/Sound/Sound.h
    src/Terrain/Terrain.h
    src/Trade/Trade.h
    src/UI/UI.h
    src/Game/GameEnums.h
    src/Game/GameStructs.h
    src/Debug/MiniDump.h
    src/resource.h
    src/Net/Protocol/ProtocolBase.h
    src/Net/Protocol/GameServerProtocol.h
    src/Net/Protocol/ConnectServerProtocol.h
)

set(MU_RESOURCES
    src/resource.rc
)

# El .rc va en una librería OBJECT aparte: el generador de Visual Studio copia
# los defines de C++ al ResourceCompile del mismo target, y el .vcxproj no le
# pasa ningún define a rc.exe. El .res resultante se enlaza igual en main.exe.
add_library(mu97k_res OBJECT ${MU_RESOURCES})
set_target_properties(mu97k_res PROPERTIES FOLDER lib)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${MU_RESOURCES})

add_executable(mu97k WIN32 ${MU_SOURCES} ${MU_HEADERS} $<TARGET_OBJECTS:mu97k_res>)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src"
    FILES ${MU_SOURCES} ${MU_HEADERS})

set_target_properties(mu97k PROPERTIES
    OUTPUT_NAME main
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    # Igual que el .vcxproj: se enlaza directo sobre bin/Client/, donde están
    # Data/ y Config.ini.
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/bin/Client"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/Client"
    PDB_OUTPUT_DIRECTORY_DEBUG       "${CMAKE_CURRENT_SOURCE_DIR}/bin/Client"
    PDB_OUTPUT_DIRECTORY_RELEASE     "${CMAKE_CURRENT_SOURCE_DIR}/bin/Client"
    VS_DEBUGGER_WORKING_DIRECTORY    "${CMAKE_CURRENT_SOURCE_DIR}/bin/Client"
)

target_include_directories(mu97k PRIVATE
    src
    lib/libjpeg/jpeg-6b
    lib/miniaudio
)

# CharacterSet=MultiByte -> _MBCS. NO se define UNICODE/_UNICODE.
# Van como opciones /D y no con target_compile_definitions porque el
# generador de Visual Studio ignora $<COMPILE_LANGUAGE> en las definiciones y
# se las pasaría también a rc.exe; el .vcxproj no le pasa defines al .rc.
set(MU_DEFINES
    /DWIN32
    $<$<CONFIG:Debug>:/D_DEBUG>
    $<$<CONFIG:Release>:/DNDEBUG>
    /D_WINDOWS
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /D_WINSOCK_DEPRECATED_NO_WARNINGS
    /D_MBCS
)
target_compile_options(mu97k PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${MU_DEFINES}>"
)

# BasicRuntimeChecks=Default y SDLCheck=false en Debug: sin /RTC, con /sdl-.
target_compile_options(mu97k PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:/W3>"
    "$<$<COMPILE_LANGUAGE:CXX>:${MU_COMMON_COMPILE_OPTIONS}>"
    "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>>:/sdl->"
)

# PCH: stdafx.cpp lo crea (/Yc), el resto lo usa (/Yu). No se usa
# target_precompile_headers porque ése fuerza un /FI propio; el .vcxproj
# depende de que cada .cpp haga #include "stdafx.h".
set(MU_PCH_USERS ${MU_SOURCES})
list(REMOVE_ITEM MU_PCH_USERS src/stdafx.cpp)
set_source_files_properties(${MU_PCH_USERS} PROPERTIES
    COMPILE_OPTIONS "/Yustdafx.h")
set_source_files_properties(src/stdafx.cpp PROPERTIES
    COMPILE_OPTIONS "/Ycstdafx.h")

target_link_libraries(mu97k PRIVATE
    jpeg
    opengl32
    glu32
    winmm
    ws2_32
    version
    dxguid
    dsound
    imm32
)

# Linker:
#   Debug:   /INCREMENTAL, /DYNAMICBASE:NO + /FIXED (sin ASLR, para leer DAT_ en
#            sus VAs originales), /MAP + /MAPINFO:EXPORTS.
#   Release: /INCREMENTAL:NO, /OPT:REF, /OPT:ICF, /LTCG. El .vcxproj NO genera
#            .map en Release; acá sí se pide (/MAP) para poder comparar símbolos.
#            /MAP no cambia el contenido del exe.
target_link_options(mu97k PRIVATE
    /DEBUG
    /MAP:$<TARGET_FILE_DIR:mu97k>/main.map
    $<$<CONFIG:Debug>:/INCREMENTAL>
    $<$<CONFIG:Debug>:/DYNAMICBASE:NO>
    $<$<CONFIG:Debug>:/FIXED>
    $<$<CONFIG:Debug>:/MAPINFO:EXPORTS>
    $<$<CONFIG:Release>:/INCREMENTAL:NO>
    $<$<CONFIG:Release>:/OPT:REF>
    $<$<CONFIG:Release>:/OPT:ICF>
    $<$<CONFIG:Release>:/LTCG>
)

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT mu97k)
