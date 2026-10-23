# CMakeLists.txt — build del cliente Mu Online 0.97k.
#
# Replica mu97k.vcxproj (Win32, toolset v143). Los flags se fijan a mano en vez
# de usar los defaults de CMake para que la linea de comandos de cl/link sea la
# misma que la del .vcxproj. .github/scripts/check_cmake_sources.py verifica en CI que la
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

project(mu97k LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MuPlatform)
if(MU_PLATFORM_ONLY)
    if(NOT MU_PLATFORM_BACKEND STREQUAL "SDL3")
        message(FATAL_ERROR "MU_PLATFORM_ONLY requiere SDL3")
    endif()
    include(MuSDLDependencies)
    add_subdirectory(src/Platform/SDL3)
    return()
endif()
include(MuCompilerOptions)
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    include(MuSDLDependencies)
    add_subdirectory(src/Platform/SDL3)
else()
    set(MU_MINIAUDIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/miniaudio")
endif()


# ── Fuentes ─────────────────────────────────────────────────────────────
# Copiadas de los <ClCompile> del .vcxproj, en el mismo orden. NO usar glob:
# la lista explícita es la que se compara contra el .vcxproj
# (.github/scripts/check_cmake_sources.py).
set(MU_SOURCES
    src/stdafx.cpp
    src/Platform/ClientEntry.cpp
    src/Platform/GLFunctions.cpp
    src/Local/Text_Data.cpp
    src/Scene/Scene_Dispatch.cpp
    src/Scene/Scene_Intro.cpp
    src/Scene/Scene_Login.cpp
    src/Scene/Scene_Loading.cpp
    src/Scene/Scene_CharSelect.cpp
    src/Render/GlyphAtlas.cpp
    src/Render/ResourceTables.cpp
    src/Render/Backend/GL/Skinning.cpp
    src/Render/Backend/GL/MeshDraw.cpp
    src/Render/TerrainDraw.cpp
    src/Render/Backend/GL/TerrainRenderer.cpp
    src/Render/Backend/GL/Materials.cpp
    src/Render/Backend/GL/FragmentPipeline.cpp
    src/Render/RenderProfile.cpp
    src/Render/Backend/GL/ProfileQueries.cpp
    src/Render/Backend/GL/BufferedDraw.cpp
    src/Render/WorldEffects.cpp
    src/Render/MaterialVisuals.cpp
    src/Render/LooseWingVisuals.cpp
    src/Render/EquipmentVisuals.cpp
    src/Game/RenderDerivedState.cpp
    src/Game/RenderSnapshots.cpp
    src/Game/RenderInterpolation.cpp
    src/Game/RenderInterpolationMath.cpp
    src/Game/RenderFxSnapshots.cpp
    src/Game/SimulationPreparation.cpp
    src/Game/Game_MainLoop.cpp
    src/Game/HeroVitals.cpp
    src/Game/Player_InputTick.cpp
    src/Render/Render_Frame.cpp
    src/Net/Net_Process.cpp
    src/Config/Config_Load.cpp
    src/UI/UIState.cpp
    src/Local/ClientText.cpp
    src/Core/Window.cpp
    src/Platform/Win32/Win32NativeBridge.cpp
    src/Platform/SDL3/SDLWindow.cpp
    src/Platform/SDL3/SDLInput.cpp
    src/Platform/SDL3/SDLGLContext.cpp
    src/Platform/SDL3/SDLTimer.cpp
    src/Platform/SDL3/SDLFileSystem.cpp
    src/Platform/SDL3/SDLNativeBridge.cpp
    src/Platform/SDL3/SDLAudio.cpp
    src/Platform/SDL3/SDLSocket.cpp
    src/Platform/SDL3/SDLFont.cpp
    src/Platform/Win32/Win32Audio.cpp
    src/Platform/Win32/Win32Timer.cpp
    src/Platform/Win32/Win32Input.cpp
    src/Platform/Win32/Win32Window.cpp
    src/Platform/Win32/Win32FileSystem.cpp
    src/Platform/Win32/Win32Socket.cpp
    src/Platform/Win32/Win32GLContext.cpp
    src/Platform/FontFrontend.cpp
    src/Sound/SoundManager.cpp
    src/Game/MapManager.cpp
    src/Platform/Win32/ClientWndProc.cpp
    src/Net/Network.cpp
    src/Net/Reconnect.cpp
    src/Net/Ping.cpp
    src/Net/ServerCharacterStats.cpp
    src/Item/ChaosMixRates.cpp
    src/UI/HealthBar.cpp
    src/Net/Recv/Recv_Account.cpp
    src/Net/Recv/Recv_Character.cpp
    src/Net/Recv/Recv_Chat.cpp
    src/Net/Recv/Recv_Combat.cpp
    src/Net/Recv/Recv_Event.cpp
    src/Net/Recv/Recv_Guild.cpp
    src/Net/Recv/Recv_Item.cpp
    src/Net/Recv/Recv_Party.cpp
    src/Net/Recv/Recv_Quest.cpp
    src/Net/Recv/Recv_Trade.cpp
    src/Net/Recv/Recv_Viewport.cpp
    src/Net/Recv/Recv_World.cpp
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
    src/Game/FrameLimiter.cpp
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
    src/Platform/Win32/MiniDump.cpp
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
    src/Entity/CharacterEffects.cpp
    src/Entity/CharacterCloth.cpp
    src/Entity/ClothPresentation.cpp
    src/Entity/ObjectPose.cpp
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
    src/Item/RightClickMove.cpp
    src/Item/ContentCatalog.cpp
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
    src/Render/Texture/ImageCodec.cpp
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
    src/UI/EventTimer.cpp
    src/UI/MiniMap.cpp
    src/Render/Camera3D.cpp
    src/UI/OptionsMenu.cpp
    src/UI/MoveList.cpp
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
    src/Platform/IClientSettings.h
    src/Platform/ClientSettings.h
    src/Platform/IDiagnostics.h
    src/Platform/Diagnostics.h
    src/Platform/Win32/Win32Diagnostics.h
    src/Platform/Posix/PosixDiagnostics.h
    src/Platform/IIdentity.h
    src/Platform/Identity.h
    src/Platform/Win32/Win32Identity.h
    src/Platform/Posix/PosixIdentity.h
    src/Platform/LocalTime.h
    src/Platform/Text.h
    src/Platform/GuardedOperation.h
    src/Scene/ObjectRegistry.h
    src/Platform/PlatformHeaders.h
    src/Platform/DebugHeaders.h
    src/Platform/Posix/WinTypes.h
    src/stdafx.h
    src/functions.h
    src/globals.h
    src/structs.h
    src/ghidra_compat.h
    src/types.h
    src/Config/Config.h
    src/Config/ServerConfig.h
    src/Config/UserSettings.h
    src/UI/UIState.h
    src/UI/ChatTypes.h
    src/Entity/EntityView.h
    src/Render/WorldEffects.h
    src/Render/MaterialVisuals.h
    src/Render/LooseWingVisuals.h
    src/Render/EquipmentVisuals.h
    src/Game/FramePointerSet.h
    src/Game/RenderDerivedState.h
    src/Game/RenderSnapshots.h
    src/Game/RenderInterpolation.h
    src/Game/RenderInterpolationMath.h
    src/Game/RenderFxSnapshots.h
    src/Game/SimulationPreparation.h
    src/Entity/CharacterEffects.h
    src/Entity/CharacterCloth.h
    src/Entity/ClothPresentation.h
    src/Entity/ObjectPose.h
    src/Entity/ObjectBounds.h
    src/Entity/EntityTypes.h
    src/Model/ModelTypes.h
    src/Entity/CharacterAttributeView.h
    src/Local/ClientText.h
    src/Item/RightClickMove.h
    src/Item/ContentCatalog.h
    src/Item/ItemDefines.h
    src/Item/Item_ServerValue.h
    src/UI/GoldenArcher.h
    src/Core/Window.h
    src/Platform/CompilerCompat.h
    src/Platform/NativeBridge.h
    src/Platform/SDL3/SDLPlatform.h
    src/Platform/IAudio.h
    src/Platform/ITimer.h
    src/Platform/IInput.h
    src/Platform/IWindow.h
    src/Platform/Win32/Win32Window.h
    src/Platform/IFileSystem.h
    src/Platform/ISocket.h
    src/Platform/SDL3/SDLFont.h
    src/Render/GLCallBarrier.h
    src/Render/GlyphAtlas.h
    src/Render/ResourceTables.h
    src/Render/Skinning.h
    src/Render/MeshDraw.h
    src/Render/TerrainDraw.h
    src/Render/BMDDevice.h
    src/Render/Materials.h
    src/Render/FragmentStage.h
    src/Render/Backend/GL/FragmentPipeline.h
    src/Render/Backend/GL/TerrainRenderer.h
    src/Render/RenderProfile.h
    src/Render/ProfileStats.h
    src/Render/Backend/GL/ProfileUploads.h
    src/Render/Backend/GL/ProfileQueries.h
    src/Render/BufferedDraw.h
    src/Render/TransformMath.h
    src/Render/TransformWorkers.h
    src/Platform/IGLContext.h
    src/Platform/GLFunctions.h
    src/Platform/GL/glad.h
    src/Platform/Win32/Win32GLContext.h
    src/Core/Font.h
    src/UI/EventTimer.h
    src/UI/MiniMap.h
    src/Render/Camera3D.h
    src/UI/OptionsMenu.h
    src/UI/MoveList.h
    src/Sound/SoundManager.h
    src/Game/MapManager.h
    src/Game/HeroVitals.h
    src/Net/Network.h
    src/Net/Reconnect.h
    src/Net/Ping.h
    src/Net/ServerCharacterStats.h
    src/Item/ChaosMixRates.h
    src/UI/HealthBar.h
    src/Net/Recv/NetRecv.h
    src/Combat/Combat.h
    src/Combat/Skills.h
    src/Game/Game_CharSelectTick.h
    src/Game/Game_EnterWorldTick.h
    src/Game/Game_MainLoop.h
    src/Game/FrameLimiter.h
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
    src/Render/JointTypes.h
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
    src/Render/Texture/ImageCodec.h
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
if(WIN32)
add_library(mu97k_res OBJECT ${MU_RESOURCES})
set_target_properties(mu97k_res PROPERTIES FOLDER lib)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${MU_RESOURCES})

endif()

# Las listas anteriores son el inventario común. Cada build enlaza un solo backend.
list(FILTER MU_SOURCES EXCLUDE REGEX "src/Platform/SDL3/")
if(NOT WIN32)
    list(FILTER MU_SOURCES EXCLUDE REGEX "src/Platform/Win32/")
endif()
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    list(FILTER MU_SOURCES EXCLUDE REGEX "src/Platform/Win32/Win32(Window|Input|GLContext|Timer|FileSystem|NativeBridge|Audio|Socket)\\.cpp$")
endif()
if(WIN32)
    add_executable(mu97k WIN32 ${MU_SOURCES} ${MU_HEADERS} $<TARGET_OBJECTS:mu97k_res>)
else()
    add_executable(mu97k ${MU_SOURCES} ${MU_HEADERS})
endif()

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
    ${MU_MINIAUDIO_INCLUDE_DIR}
)

if(MSVC)
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
list(REMOVE_ITEM MU_PCH_USERS src/Render/Backend/GL/FragmentPipeline.cpp src/Render/TerrainDraw.cpp src/Render/Backend/GL/TerrainRenderer.cpp src/Render/RenderProfile.cpp src/Render/Backend/GL/ProfileQueries.cpp src/stdafx.cpp src/Render/Texture/ImageCodec.cpp src/Platform/GLFunctions.cpp src/Render/Backend/GL/BufferedDraw.cpp src/Render/GlyphAtlas.cpp src/Render/Backend/GL/MeshDraw.cpp src/Render/Backend/GL/Skinning.cpp src/Render/ResourceTables.cpp)
set_source_files_properties(${MU_PCH_USERS} PROPERTIES
    COMPILE_OPTIONS "/Yustdafx.h")
# La sonda gráfica compila el backend real sin depender del PCH del ejecutable.
set_source_files_properties(src/Render/Backend/GL/Materials.cpp PROPERTIES
    COMPILE_OPTIONS "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,mu97k>:/Yustdafx.h>")
set_source_files_properties(src/stdafx.cpp PROPERTIES
    COMPILE_OPTIONS "/Ycstdafx.h")

else()
    target_compile_options(mu97k PRIVATE ${MU_COMMON_COMPILE_OPTIONS})
    target_compile_definitions(mu97k PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
    # Mantener los artefactos Linux separados del cliente Windows de prueba.
    set_target_properties(mu97k PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif()

if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    target_link_libraries(mu97k PRIVATE mu_platform_sdl)
    target_compile_definitions(mu97k PRIVATE MU_MINIAUDIO_LINKED MU_SDL_FONTS)
endif()

if(WIN32)
target_link_libraries(mu97k PRIVATE
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

else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(mu97k PRIVATE OpenGL::GL OpenGL::GLU)
    target_link_options(mu97k PRIVATE "-Wl,-Map,$<TARGET_FILE_DIR:mu97k>/main.map")
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT mu97k)

# DESVIACION: prueba de crecimiento real sin contexto GL, en ambos sistemas.
find_package(Threads REQUIRED)
target_link_libraries(mu97k PRIVATE Threads::Threads)
enable_testing()
add_executable(mu_resource_tables_test EXCLUDE_FROM_ALL
    tests/ResourceTables.cpp src/Render/ResourceTables.cpp)
target_include_directories(mu_resource_tables_test PRIVATE src)
target_compile_features(mu_resource_tables_test PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(mu_resource_tables_test PRIVATE /EHsc)
endif()
add_test(NAME resource_tables COMMAND mu_resource_tables_test)

add_executable(mu_frame_pointer_set_test EXCLUDE_FROM_ALL tests/FramePointerSet.cpp)
target_include_directories(mu_frame_pointer_set_test PRIVATE src)
target_compile_features(mu_frame_pointer_set_test PRIVATE cxx_std_17)
add_test(NAME frame_pointer_set COMMAND mu_frame_pointer_set_test)

add_executable(mu_transform_workers_test EXCLUDE_FROM_ALL tests/TransformWorkers.cpp)
target_include_directories(mu_transform_workers_test PRIVATE src)
target_compile_features(mu_transform_workers_test PRIVATE cxx_std_17)
target_link_libraries(mu_transform_workers_test PRIVATE Threads::Threads)
if(MSVC)
    target_compile_options(mu_transform_workers_test PRIVATE /EHsc /fp:precise)
endif()
add_test(NAME transform_workers COMMAND mu_transform_workers_test)

# DESVIACION: R1 valida percentiles y queries asincronas sin driver/contexto.
add_executable(mu_render_profile_test EXCLUDE_FROM_ALL
    tests/RenderProfile.cpp src/Render/Backend/GL/ProfileQueries.cpp)
target_include_directories(mu_render_profile_test PRIVATE src)
target_compile_features(mu_render_profile_test PRIVATE cxx_std_17)
add_test(NAME render_profile COMMAND mu_render_profile_test)

add_executable(mu_render_profile_flow_test EXCLUDE_FROM_ALL
    tests/RenderProfileFlow.cpp src/Render/RenderProfile.cpp)
target_include_directories(mu_render_profile_flow_test PRIVATE src)
target_compile_features(mu_render_profile_flow_test PRIVATE cxx_std_17)
add_test(NAME render_profile_flow COMMAND mu_render_profile_flow_test)

# DESVIACION R3a: contrato de captura/fallback sin necesitar un driver GL.
add_executable(mu_terrain_draw_flow_test EXCLUDE_FROM_ALL tests/TerrainDrawFlow.cpp src/Render/TerrainDraw.cpp)
target_include_directories(mu_terrain_draw_flow_test PRIVATE src)
target_compile_features(mu_terrain_draw_flow_test PRIVATE cxx_std_17)
add_test(NAME terrain_draw_flow COMMAND mu_terrain_draw_flow_test)

# Verificación gráfica SDL3: obligatoria con Mesa en Linux, omisible si el
# runner Windows no dispone de un contexto 3.3 (salida77, no un falso pase).
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    add_executable(mu_bmd_device_test EXCLUDE_FROM_ALL
        tests/BMDDevice.cpp
        src/Platform/GLFunctions.cpp src/Render/ResourceTables.cpp
        src/Render/RenderProfile.cpp src/Render/Backend/GL/ProfileQueries.cpp
        src/Render/Backend/GL/Materials.cpp src/Render/Backend/GL/FragmentPipeline.cpp
        src/Render/Backend/GL/BufferedDraw.cpp src/Render/Backend/GL/MeshDraw.cpp
        src/Render/Backend/GL/Skinning.cpp)
    target_include_directories(mu_bmd_device_test PRIVATE src ${MU_MINIAUDIO_INCLUDE_DIR})
    target_compile_features(mu_bmd_device_test PRIVATE cxx_std_17)
    target_link_libraries(mu_bmd_device_test PRIVATE SDL3::SDL3-static)
    if(WIN32)
        target_link_libraries(mu_bmd_device_test PRIVATE opengl32 glu32)
    else()
        target_link_libraries(mu_bmd_device_test PRIVATE GL GLU)
    endif()
    if(MSVC)
        target_compile_options(mu_bmd_device_test PRIVATE /EHsc /fp:precise /UNDEBUG)
    else()
        target_compile_options(mu_bmd_device_test PRIVATE -UNDEBUG $<$<CONFIG:Release>:-g>)
    endif()
    add_test(NAME bmd_device COMMAND mu_bmd_device_test)
    set_tests_properties(bmd_device PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120)
endif()
