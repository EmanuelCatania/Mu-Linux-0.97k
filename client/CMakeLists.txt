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
    add_subdirectory(src/platform/SDL3)
    return()
endif()
include(MuCompilerOptions)
include(MuWebGPU)
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    include(MuSDLDependencies)
    add_subdirectory(src/platform/SDL3)
else()
    set(MU_MINIAUDIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/miniaudio")
endif()


# ── Fuentes ─────────────────────────────────────────────────────────────
# Copiadas de los <ClCompile> del .vcxproj, en el mismo orden. NO usar glob:
# la lista explícita es la que se compara contra el .vcxproj
# (.github/scripts/check_cmake_sources.py).
set(MU_SOURCES
    src/stdafx.cpp
    src/platform/ClientEntry.cpp
    src/platform/ContentLayers.cpp
    src/platform/GLFunctions.cpp
    src/assets/text/Text_Data.cpp
    src/scenes/Scene_Dispatch.cpp
    src/scenes/Scene_Intro.cpp
    src/scenes/Scene_Login.cpp
    src/scenes/Scene_Loading.cpp
    src/scenes/Scene_CharSelect.cpp
    src/render/GlyphAtlas.cpp
    src/render/BufferedDraw.cpp
    src/render/Skinning.cpp
    src/render/MeshTopology.cpp
    src/render/Gfx.cpp
    src/rhi/GL/GfxDevice.cpp
    src/rhi/Dispatch.cpp
    src/render/ResourceTables.cpp
    src/rhi/GL/Skinning.cpp
    src/rhi/GL/MeshDraw.cpp
    src/rhi/GL/ShadowMap.cpp
    src/render/ShadowLight.cpp
    src/render/TerrainDraw.cpp
    src/rhi/GL/TerrainRenderer.cpp
    src/render/Materials.cpp
    src/rhi/GL/FragmentPipeline.cpp
    src/render/RenderProfile.cpp
    src/render/RenderState.cpp
    src/rhi/GL/ProfileQueries.cpp
    src/rhi/GL/BufferedDraw.cpp
    src/render/WorldEffects.cpp
    src/render/MaterialVisuals.cpp
    src/render/LooseWingVisuals.cpp
    src/render/EquipmentVisuals.cpp
    src/game/core/RenderDerivedState.cpp
    src/game/core/RenderSnapshots.cpp
    src/game/core/RenderInterpolation.cpp
    src/game/core/RenderInterpolationMath.cpp
    src/game/core/RenderFxSnapshots.cpp
    src/game/core/SimulationPreparation.cpp
    src/game/core/Game_MainLoop.cpp
    src/game/core/HeroVitals.cpp
    src/game/core/Player_InputTick.cpp
    src/render/Render_Frame.cpp
    src/net/Net_Process.cpp
    src/app/config/Config_Load.cpp
    src/ui/UIState.cpp
    src/assets/text/ClientText.cpp
    src/app/core/Window.cpp
    src/platform/Win32/Win32NativeBridge.cpp
    src/platform/SDL3/SDLWindow.cpp
    src/platform/SDL3/SDLInput.cpp
    src/platform/SDL3/SDLGLContext.cpp
    src/platform/SDL3/SDLTimer.cpp
    src/platform/SDL3/SDLFileSystem.cpp
    src/platform/SDL3/SDLNativeBridge.cpp
    src/platform/SDL3/SDLAudio.cpp
    src/platform/SDL3/SDLSocket.cpp
    src/platform/SDL3/SDLFont.cpp
    src/render/text/TextShaper.cpp
    src/platform/Win32/Win32Audio.cpp
    src/platform/Win32/Win32Timer.cpp
    src/platform/Win32/Win32Input.cpp
    src/platform/Win32/Win32Window.cpp
    src/platform/Win32/Win32FileSystem.cpp
    src/platform/Win32/Win32Socket.cpp
    src/platform/Win32/Win32GLContext.cpp
    src/platform/FontFrontend.cpp
    src/audio/SoundManager.cpp
    src/game/core/MapManager.cpp
    src/platform/Win32/ClientWndProc.cpp
    src/net/Network.cpp
    src/net/NetChannel.cpp
    src/net/SecureChannel.cpp
    src/net/ServerTrust.cpp
    src/net/ServerTrustCore.cpp
    src/net/crypto/monocypher.c
    src/net/Reconnect.cpp
    src/net/Ping.cpp
    src/net/ServerCharacterStats.cpp
    src/game/item/ChaosMixRates.cpp
    src/ui/HealthBar.cpp
    src/net/Recv/Recv_Account.cpp
    src/net/Recv/Recv_Character.cpp
    src/net/Recv/Recv_Chat.cpp
    src/net/Recv/Recv_Combat.cpp
    src/net/Recv/Recv_Event.cpp
    src/net/Recv/Recv_Guild.cpp
    src/net/Recv/Recv_Item.cpp
    src/net/Recv/Recv_Party.cpp
    src/net/Recv/Recv_Quest.cpp
    src/net/Recv/Recv_Trade.cpp
    src/net/Recv/Recv_Viewport.cpp
    src/net/Recv/Recv_World.cpp
    src/platform/input/Input_WndProc.cpp
    src/app/config/UserSettings.cpp
    src/audio/Sound_Queue.cpp
    src/assets/data/Monster_Data.cpp
    src/game/terrain/Terrain_Utils.cpp
    src/render/Terrain_RenderTerrain.cpp
    src/game/math/Math_3D.cpp
    src/render/Particle.cpp
    src/render/Particle_Move.cpp
    src/render/BMD_Anim.cpp
    src/render/BMD_DrawMesh.cpp
    src/render/BMD_Collision.cpp
    src/render/MoveEffect.cpp
    src/render/MoveEffect_Helpers.cpp
    src/render/MoveJoint.cpp
    src/render/BMD_SetupRender.cpp
    src/game/core/PathFinder.cpp
    src/game/entity/Entity_Reset.cpp
    src/game/entity/Entity_HeadMotion.cpp
    src/render/GL_State.cpp
    src/render/Effect_Create.cpp
    src/render/Effect_Combat.cpp
    src/render/Joint_Create.cpp
    src/render/Joint_Render.cpp
    src/render/Sprite.cpp
    src/render/Camera.cpp
    src/render/Camera_Movement.cpp
    src/render/Entity_Render.cpp
    src/net/Crypto.cpp
    src/game/core/Timer.cpp
    src/game/core/FrameLimiter.cpp
    src/audio/Music.cpp
    src/audio/EmoteSound.cpp
    src/game/terrain/Terrain_Tile.cpp
    src/render/GL_2D.cpp
    src/render/Effect_Tick.cpp
    src/game/terrain/Terrain_Light.cpp
    src/render/Terrain_RenderFace.cpp
    src/render/Terrain_RenderBlocks.cpp
    src/audio/Sound_DS3D.cpp
    src/net/Net_Connect.cpp
    src/net/Net_PacketSession.cpp
    src/net/MuEmu.cpp
    src/net/PacketFrame.cpp
    src/net/Net_Events.cpp
    src/net/HWID.cpp
    src/game/entity/Entity_Init.cpp
    src/game/entity/Entity_MoveClient.cpp
    src/game/entity/Character_Motion.cpp
    src/game/entity/Entity_Spawn.cpp
    src/render/Scene_CharPreview.cpp
    src/scenes/Scene_Resources.cpp
    src/scenes/Scene_MapTick.cpp
    src/scenes/Scene_ServerSelect_Input.cpp
    src/ui/Chat.cpp
    src/ui/Chat_Bubbles.cpp
    src/ui/Chat_CommandParser.cpp
    src/ui/ChatListBox.cpp
    src/ui/CharMenu_Build.cpp
    src/render/HUD_Pass1.cpp
    src/render/HUD_Pass2.cpp
    src/render/HUD_Pass3.cpp
    src/render/HUD_Pass4.cpp
    src/render/HUD_Pass5.cpp
    src/render/HUD_Pass6.cpp
    src/ui/UI_StatsPanel.cpp
    src/app/util/Misc.cpp
    src/app/core/Legacy_Runtime.cpp
    src/game/entity/EntityName.cpp
    src/game/entity/Entity_Lookup.cpp
    src/net/Net_Disconnect.cpp
    src/render/Render_MatrixStack.cpp
    src/scenes/Scene_ObjectTick.cpp
    src/render/Particle_Legacy.cpp
    src/render/Camera_Login.cpp
    src/ui/Chat_Log.cpp
    src/ui/EmojiPicker.cpp
    src/ui/EmoteMenu.cpp
    src/game/terrain/Terrain_RayCollision.cpp
    src/platform/Win32/MiniDump.cpp
    src/game/combat/Combat_AttackEffect.cpp
    src/app/core/Runtime_Small.cpp
    src/app/core/Runtime_Medium.cpp
    src/ui/Quest_Legacy.cpp
    src/app/core/System_Legacy.cpp
    src/GameGuard/GameGuard_Packet.cpp
    src/render/SMD_Legacy.cpp
    src/ui/UI_LegacyWidgets.cpp
    src/net/Net_LegacyRuntime.cpp
    src/scenes/Scene_CharSelect_Nav.cpp
    src/game/item/Item_LegacyHelpers.cpp
    src/ui/UI_DialogInterface.cpp
    src/game/item/Item_Durability.cpp
    src/game/item/Item_ServerValue.cpp
    src/render/BMD_LegacyDraw.cpp
    src/app/config/Config_TokenParser.cpp
    src/game/entity/Entity_BoneEffects.cpp
    src/app/core/Runtime_Linker.cpp
    src/GameGuard/GameGuard_LegacyHealth.cpp
    src/game/item/Item_LegacyLinker.cpp
    src/game/math/Math_LegacyTransforms.cpp
    src/render/Render_LegacyLinker.cpp
    src/scenes/Scene_LegacyCleanup.cpp
    src/scenes/Scene_ObjectLegacy.cpp
    src/render/Terrain_LegacyRender.cpp
    src/ui/UI_LegacyText.cpp
    src/render/BMD_FilterData.cpp
    src/ui/Quest_UIState.cpp
    src/game/physics/Cloth_Simulation.cpp
    src/render/BMD_LegacyEffects.cpp
    src/scenes/Scene_LegacyHelpers.cpp
    src/game/path/Path_Legacy.cpp
    src/app/core/Runtime_Externs.cpp
    src/ui/UI_LegacyExterns.cpp
    src/game/entity/Entity_ActionLegacy.cpp
    src/render/Render_LegacyRotatedRect.cpp
    src/ui/UI_GuildLegacy.cpp
    src/app/core/String_ResourceLegacy.cpp
    src/game/entity/Entity_LegacyEffects.cpp
    src/game/combat/Combat_LegacyAnimation.cpp
    src/render/Render_LegacyTransforms.cpp
    src/render/Render_LegacyBillboards.cpp
    src/ui/UI_LegacyWidgetSystem.cpp
    src/render/Render_LegacyTrails.cpp
    src/ui/UI_LegacyGameHelpers.cpp
    src/net/Net_LegacyAuth.cpp
    src/game/terrain/Terrain_LegacyLoad.cpp
    src/game/path/Path_LegacyReset.cpp
    src/render/Font_LegacyFactory.cpp
    src/render/SMD_Parser.cpp
    src/game/entity/CharacterEffects.cpp
    src/game/entity/CharacterCloth.cpp
    src/game/entity/ClothPresentation.cpp
    src/game/entity/ObjectPose.cpp
    src/render/Entity_AnimationLegacy.cpp
    src/game/combat/Combat_LegacyAttackEffects.cpp
    src/game/entity/Entity_LegacyPoolCleanup.cpp
    src/game/item/Item_LegacyGridInsert.cpp
    src/game/item/Item_TypeLegacy.cpp
    src/game/entity/Entity_LegacyTeleport.cpp
    src/game/combat/Skills_WarriorLegacy.cpp
    src/render/Render_LegacyDamageNumbers.cpp
    src/render/GL_LegacyState.cpp
    src/game/math/Math_LegacyAngle.cpp
    src/render/BMD_LegacyLoading.cpp
    src/platform/input/Cursor_Render.cpp
    src/platform/input/Input_Timing.cpp
    src/app/util/Process_Exit.cpp
    src/net/Net_Bux.cpp
    src/scenes/Scene_EntityLifecycle.cpp
    src/scenes/Scene_OpenWorld.cpp
    src/scenes/Scene_AssetLoad.cpp
    src/render/Camera_Unproject.cpp
    src/net/Protocol/Protocol_Check.cpp
    src/GameGuard/GameGuard_InitTrampoline.cpp
    src/render/Render_WorldHelpers.cpp
    src/render/BMD_Load.cpp
    src/globals.cpp
    src/assets/model/Model_Items.cpp
    src/assets/model/Model_CustomAnim.cpp
    src/assets/model/Model_Monsters.cpp
    src/assets/model/Model_Players.cpp
    src/assets/model/Model_Effects.cpp
    src/assets/model/Model_Misc.cpp
    src/assets/model/Model_SkillEffects.cpp
    src/assets/model/Model_Gates.cpp
    src/game/combat/Combat.cpp
    src/game/combat/Combat_AttackStage.cpp
    src/game/combat/Combat_Projectiles.cpp
    src/game/combat/Combat_Targeting.cpp
    src/game/combat/Skills.cpp
    src/game/combat/Skills_WizardElf.cpp
    src/game/core/Game_CharSelectTick.cpp
    src/game/core/Game_EnterWorldTick.cpp
    src/game/core/Game_SceneUpdate.cpp
    src/game/core/Movement.cpp
    src/game/core/CheckGate.cpp
    src/platform/input/Input.cpp
    src/platform/input/Mouse_Hover.cpp
    src/game/item/Item_Equip.cpp
    src/game/item/Item_EquipSlots.cpp
    src/game/item/Item_Move.cpp
    src/ui/inventory/Item_Display.cpp
    src/game/item/Item_SpecialOption.cpp
    src/assets/data/Item_Data.cpp
    src/game/item/Item_Inventory.cpp
    src/game/item/Item_EquipmentQueries.cpp
    src/game/item/Item_EquipmentAutoSwap.cpp
    src/game/item/Item_InventoryGrid.cpp
    src/ui/inventory/Item_InventoryRender.cpp
    src/game/item/Item_ChaosMix.cpp
    src/ui/inventory/Item_ClickHandler.cpp
    src/game/item/RightClickMove.cpp
    src/assets/data/ContentCatalog.cpp
    src/assets/data/Skill_Data.cpp
    src/assets/data/Gate_Data.cpp
    src/assets/data/Filter_Data.cpp
    src/assets/data/Dialog_Data.cpp
    src/assets/data/Quest_Data.cpp
    src/assets/data/NPC_Data.cpp
    src/assets/model/NPC_ModelLoad.cpp
    src/render/Font_Layout.cpp
    src/game/monster/Monster.cpp
    src/net/SecondPassword.cpp
    src/game/party/Party.cpp
    src/game/core/Party_NameMatch.cpp
    src/render/Ambient_Particles.cpp
    src/render/Entity_DrawByType.cpp
    src/render/Entity_DrawAt.cpp
    src/render/Entity_DrawSetup.cpp
    src/render/Entity_PrepareRender.cpp
    src/render/Entity_Render_3D.cpp
    src/render/Entity_RenderAll_3D.cpp
    src/render/Entity_UpdateRender.cpp
    src/render/Render_PlayerEquipment.cpp
    src/render/HUD_Render.cpp
    src/render/ItemDrop_Render.cpp
    src/render/ItemDrop_Render2.cpp
    src/render/Particle_Draw.cpp
    src/render/Particle_Render.cpp
    src/render/Particle_Spawn.cpp
    src/render/Player_Render.cpp
    src/render/RenderLinkObject.cpp
    src/render/SkillEffect_Render.cpp
    src/render/Sprite_Draw.cpp
    src/render/Render_SpriteHelpers.cpp
    src/render/Terrain_Render.cpp
    src/render/Terrain_Water.cpp
    src/render/Texture/ImageCodec.cpp
    src/render/text/TextCore.cpp
    src/render/text/TextAtlas.cpp
    src/render/text/UniBreak.c
    src/render/text/Utf8Proc.c
    src/render/Texture/Texture.cpp
    src/render/Trail_Render.cpp
    src/render/Weather.cpp
    src/render/Weather_Particles.cpp
    src/scenes/Scene_Login_ServerSelect.cpp
    src/audio/Sound.cpp
    src/game/terrain/Terrain.cpp
    src/game/trade/Trade.cpp
    src/ui/UI_InGameMenu.cpp
    src/ui/UI_PanelInput.cpp
    src/ui/UI_IME.cpp
    src/ui/UI_TextBitmap.cpp
    src/ui/UI_Tooltip.cpp
    src/ui/Chat_Gestures.cpp
    src/ui/GoldenArcher.cpp
    src/ui/EventTimer.cpp
    src/ui/BuffBar.cpp
    src/ui/ChatHistory.cpp
    src/app/core/DataPrefetch.cpp
    src/ui/MiniMap.cpp
    src/render/Camera3D.cpp
    src/ui/OptionsMenu.cpp
    src/ui/MoveList.cpp
    src/ui/Chat_InputTick.cpp
    src/ui/Chat_Send.cpp
    src/ui/UI_SkillHotkeys.cpp
    src/ui/UI_GuildMark.cpp
    src/ui/RenderItemInfo.cpp
    src/scenes/Scene_Objects.cpp
    src/scenes/Scene_ObjectInteraction.cpp
    src/scenes/Scene_ObjectUpdate.cpp
    src/render/BMD_SkinUpdate.cpp
    src/app/debug/Benchmark.cpp
)

# <ClInclude> del .vcxproj: sólo para que aparezcan en el IDE.
set(MU_HEADERS
    src/platform/IClientSettings.h
    src/platform/ClientSettings.h
    src/platform/IDiagnostics.h
    src/platform/Diagnostics.h
    src/platform/Win32/Win32Diagnostics.h
    src/platform/Posix/PosixDiagnostics.h
    src/platform/IIdentity.h
    src/platform/Identity.h
    src/platform/Win32/Win32Identity.h
    src/platform/Posix/PosixIdentity.h
    src/platform/LocalTime.h
    src/platform/Text.h
    src/platform/GuardedOperation.h
    src/scenes/ObjectRegistry.h
    src/scenes/SceneObjectCell.h
    src/platform/PlatformHeaders.h
    src/platform/DebugHeaders.h
    src/platform/Posix/WinTypes.h
    src/stdafx.h
    src/functions.h
    src/globals.h
    src/structs.h
    src/ghidra_compat.h
    src/types.h
    src/app/config/Config.h
    src/app/config/ServerConfig.h
    src/app/config/UserSettings.h
    src/ui/UIState.h
    src/ui/ChatTypes.h
    src/game/entity/EntityView.h
    src/game/entity/EntityName.h
    src/render/WorldEffects.h
    src/render/MaterialVisuals.h
    src/render/LooseWingVisuals.h
    src/render/EquipmentVisuals.h
    src/game/core/FramePointerSet.h
    src/game/core/RenderDerivedState.h
    src/game/core/RenderSnapshots.h
    src/game/core/RenderInterpolation.h
    src/game/core/RenderInterpolationMath.h
    src/game/core/RenderFxSnapshots.h
    src/game/core/SimulationPreparation.h
    src/game/entity/CharacterEffects.h
    src/game/entity/CharacterCloth.h
    src/game/entity/ClothPresentation.h
    src/game/physics/ClothTypes.h
    src/game/entity/ObjectPose.h
    src/game/entity/ObjectBounds.h
    src/game/entity/EntityTypes.h
    src/game/core/PathContext.h
    src/assets/model/ModelTypes.h
    src/assets/model/SmdImport.h
    src/app/core/ResourceLimits.h
    src/assets/model/BMDFileTypes.h
    src/game/entity/CharacterAttributeView.h
    src/assets/text/ClientText.h
    src/game/item/RightClickMove.h
    src/assets/data/ContentCatalog.h
    src/game/item/ItemDefines.h
    src/game/item/Item_ServerValue.h
    src/ui/GoldenArcher.h
    src/app/core/Window.h
    src/platform/CompilerCompat.h
    src/platform/NativeBridge.h
    src/platform/SDL3/SDLPlatform.h
    src/platform/IAudio.h
    src/platform/ITimer.h
    src/platform/IInput.h
    src/platform/IWindow.h
    src/platform/Win32/Win32Window.h
    src/platform/IFileSystem.h
    src/platform/ContentLayers.h
    src/platform/UserPath.h
    src/platform/ISocket.h
    src/platform/SDL3/SDLFont.h
    src/render/GlyphAtlas.h
    src/render/Gfx.h
    src/render/ResourceTables.h
    src/render/Skinning.h
    src/render/MeshDraw.h
    src/render/TerrainDraw.h
    src/render/BMDDevice.h
    src/render/Materials.h
    src/render/RenderBackend.h
    src/rhi/Api.h
    src/rhi/GfxDevice.h
    src/render/MeshTopology.h
    src/rhi/GL/FragmentPipeline.h
    src/rhi/GL/TerrainRenderer.h
    src/render/RenderProfile.h
    src/render/RenderState.h
    src/render/ProfileStats.h
    src/rhi/GL/ProfileUploads.h
    src/rhi/GL/ProfileQueries.h
    src/render/BufferedDraw.h
    src/render/TransformMath.h
    src/render/TransformWorkers.h
    src/platform/IGLContext.h
    src/platform/GLFunctions.h
    src/platform/GL/glad.h
    src/platform/Win32/Win32GLContext.h
    src/app/core/Font.h
    src/ui/PanelAnchor.h
    src/ui/EventTimer.h
    src/ui/BuffBar.h
    src/ui/ChatHistory.h
    src/app/core/DataPrefetch.h
    src/ui/MiniMap.h
    src/render/Camera3D.h
    src/ui/OptionsMenu.h
    src/ui/MoveList.h
    src/audio/SoundManager.h
    src/game/core/MapManager.h
    src/game/core/HeroVitals.h
    src/net/Network.h
    src/net/NetChannel.h
    src/net/SecureChannel.h
    src/net/crypto/SecureRandom.h
    src/net/ServerTrust.h
    src/net/crypto/monocypher.h
    src/net/Reconnect.h
    src/net/Ping.h
    src/net/ServerCharacterStats.h
    src/game/item/ChaosMixRates.h
    src/ui/HealthBar.h
    src/net/Recv/NetRecv.h
    src/game/combat/Combat.h
    src/game/combat/Skills.h
    src/game/core/Game_CharSelectTick.h
    src/game/core/Game_EnterWorldTick.h
    src/game/core/Game_MainLoop.h
    src/game/core/FrameLimiter.h
    src/game/core/Game_SceneUpdate.h
    src/game/core/Movement.h
    src/platform/input/Input.h
    src/game/item/Item_Equip.h
    src/game/monster/Monster.h
    src/net/Net.h
    src/net/HWID.h
    src/net/PacketFrame.h
    src/net/SecondPassword.h
    src/game/party/Party.h
    src/render/Camera.h
    src/render/Entity_DrawAt.h
    src/render/Entity_PrepareRender.h
    src/render/Entity_Render.h
    src/render/DamagePointTypes.h
    src/game/item/ItemTypes.h
    src/render/JointTypes.h
    src/render/Entity_Render_3D.h
    src/render/Entity_RenderAll_3D.h
    src/render/HUD_Render.h
    src/render/ItemDrop_Render.h
    src/render/Particle_Draw.h
    src/render/Particle_Render.h
    src/render/Player_Render.h
    src/render/Render.h
    src/render/SkillEffect_Render.h
    src/render/Sprite_Draw.h
    src/render/Terrain_Render.h
    src/render/Texture/ImageCodec.h
    src/render/text/TextCore.h
    src/net/NetText.h
    src/render/text/TextShaper.h
    src/render/text/TextAtlas.h
    src/render/Texture/Texture.h
    src/scenes/Scene.h
    src/scenes/Scene_CharSelect.h
    src/scenes/Scene_Dispatch.h
    src/scenes/Scene_Intro.h
    src/scenes/Scene_Loading.h
    src/scenes/Scene_Login.h
    src/scenes/Scene_Login_ServerSelect.h
    src/audio/Sound.h
    src/game/terrain/Terrain.h
    src/game/trade/Trade.h
    src/ui/UI.h
    src/game/core/GameEnums.h
    src/app/debug/MiniDump.h
    src/app/debug/Benchmark.h
    src/resource.h
    src/net/Protocol/ProtocolBase.h
    src/net/Protocol/GameServerProtocol.h
    src/net/Protocol/ConnectServerProtocol.h
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
list(FILTER MU_SOURCES EXCLUDE REGEX "src/platform/SDL3/")
# El shaping (HarfBuzz/SheenBidi) vive en la biblioteca de plataforma SDL3.
list(REMOVE_ITEM MU_SOURCES src/render/text/TextShaper.cpp)
if(NOT WIN32)
    list(FILTER MU_SOURCES EXCLUDE REGEX "src/platform/Win32/")
endif()
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    list(FILTER MU_SOURCES EXCLUDE REGEX "src/platform/Win32/Win32(Window|Input|GLContext|Timer|FileSystem|NativeBridge|Audio|Socket)\\.cpp$")
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

# Modo benchmark (src/app/debug/Benchmark.h): siempre en Debug; en Release sólo si
# se configura con -DMU_BENCHMARK=ON.  Las Releases publicadas no lo traen.
option(MU_BENCHMARK "Incluir el modo benchmark tambien en Release" OFF)

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

# Modo benchmark: ver la opcion MU_BENCHMARK arriba.
target_compile_options(mu97k PRIVATE
    "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<OR:$<BOOL:${MU_BENCHMARK}>,$<CONFIG:Debug>>>:/DMU_BENCHMARK>")

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
list(REMOVE_ITEM MU_PCH_USERS src/net/SecureChannel.cpp src/net/ServerTrustCore.cpp src/net/crypto/monocypher.c src/rhi/GL/FragmentPipeline.cpp src/render/Gfx.cpp src/rhi/GL/GfxDevice.cpp src/rhi/Dispatch.cpp src/render/RenderState.cpp src/render/TerrainDraw.cpp src/rhi/GL/TerrainRenderer.cpp src/render/RenderProfile.cpp src/rhi/GL/ProfileQueries.cpp src/stdafx.cpp src/render/Texture/ImageCodec.cpp src/render/text/TextCore.cpp src/render/text/Utf8Proc.c src/render/text/UniBreak.c src/platform/GLFunctions.cpp src/rhi/GL/BufferedDraw.cpp src/render/GlyphAtlas.cpp src/render/BufferedDraw.cpp src/render/Skinning.cpp src/render/MeshTopology.cpp src/rhi/GL/MeshDraw.cpp src/rhi/GL/Skinning.cpp src/render/ResourceTables.cpp src/rhi/GL/ShadowMap.cpp src/render/ShadowLight.cpp src/platform/ContentLayers.cpp)
set_source_files_properties(${MU_PCH_USERS} PROPERTIES
    COMPILE_OPTIONS "/Yustdafx.h")
# La sonda gráfica compila el backend real sin depender del PCH del ejecutable.
set_source_files_properties(src/render/Materials.cpp PROPERTIES
    COMPILE_OPTIONS "$<$<STREQUAL:$<TARGET_PROPERTY:NAME>,mu97k>:/Yustdafx.h>")
set_source_files_properties(src/stdafx.cpp PROPERTIES
    COMPILE_OPTIONS "/Ycstdafx.h")

else()
    target_compile_options(mu97k PRIVATE ${MU_COMMON_COMPILE_OPTIONS})
    target_compile_definitions(mu97k PRIVATE $<$<CONFIG:Debug>:_DEBUG>
        $<$<OR:$<BOOL:${MU_BENCHMARK}>,$<CONFIG:Debug>>:MU_BENCHMARK>)
    if(MINGW)
        # Los mismos defines que MSVC. Con -std=c++17 (sin extensiones) GCC
        # define _WIN32 pero no WIN32.
        target_compile_definitions(mu97k PRIVATE WIN32 _WINDOWS _MBCS
            _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    endif()
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
    winmm
    ws2_32
    bcrypt
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
if(MINGW)
    # Todo estático (libstdc++, libgcc y winpthreads): el exe no pide las DLL
    # del toolchain. El .map sirve para resolver stacks de crash.
    target_link_options(mu97k PRIVATE -static "-Wl,-Map,$<TARGET_FILE_DIR:mu97k>/main.map")
    # GCC ignora los #pragma comment(lib): la que MSVC toma de ahí va acá.
    target_link_libraries(mu97k PRIVATE rpcrt4)
else()
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
endif()

else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(mu97k PRIVATE OpenGL::GL)
    target_link_options(mu97k PRIVATE "-Wl,-Map,$<TARGET_FILE_DIR:mu97k>/main.map")
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT mu97k)

# DESVIACION: prueba de crecimiento real sin contexto GL, en ambos sistemas.
find_package(Threads REQUIRED)
target_link_libraries(mu97k PRIVATE Threads::Threads)
if(MU_RENDER_WEBGPU)
    target_sources(mu97k PRIVATE
        src/rhi/WebGPU/Device.cpp src/rhi/WebGPU/Renderer.cpp
        src/rhi/WebGPU/MeshDraw.cpp
        src/rhi/WebGPU/Stubs.cpp src/rhi/WebGPU/Device.h
        src/rhi/WebGPU/Renderer.h)
    target_link_libraries(mu97k PRIVATE dawn::webgpu_dawn)
    target_compile_definitions(mu97k PRIVATE MU_RENDER_WEBGPU)
endif()
enable_testing()
add_executable(mu_resource_tables_test EXCLUDE_FROM_ALL
    tests/ResourceTables.cpp src/render/ResourceTables.cpp)
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

# Fase 5, C3: firma del manifiesto y confianza en la clave del server.
add_executable(mu_server_trust_test EXCLUDE_FROM_ALL tests/ServerTrust.cpp src/net/ServerTrustCore.cpp src/net/crypto/monocypher.c)
target_include_directories(mu_server_trust_test PRIVATE src)
target_compile_features(mu_server_trust_test PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(mu_server_trust_test PRIVATE /utf-8)
endif()
add_test(NAME server_trust COMMAND mu_server_trust_test)

# Fase 5, C4: canal cifrado entre el cliente y el GameServer.
add_executable(mu_secure_channel_test EXCLUDE_FROM_ALL tests/SecureChannel.cpp src/net/SecureChannel.cpp src/net/crypto/monocypher.c)
target_include_directories(mu_secure_channel_test PRIVATE src)
target_compile_features(mu_secure_channel_test PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(mu_secure_channel_test PRIVATE /utf-8)
endif()
add_test(NAME secure_channel COMMAND mu_secure_channel_test)

# Listado de carpetas por las capas de datos (Fase 5, E2).
add_executable(mu_content_layers_list_test EXCLUDE_FROM_ALL tests/ContentLayersList.cpp src/platform/ContentLayers.cpp)
target_include_directories(mu_content_layers_list_test PRIVATE src)
target_compile_features(mu_content_layers_list_test PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(mu_content_layers_list_test PRIVATE /utf-8 /EHsc)
endif()
add_test(NAME content_layers_list COMMAND mu_content_layers_list_test)

add_executable(mu_text_core_test EXCLUDE_FROM_ALL tests/TextCore.cpp src/render/text/TextCore.cpp src/render/text/Utf8Proc.c src/render/text/UniBreak.c)
target_include_directories(mu_text_core_test PRIVATE src)
target_compile_features(mu_text_core_test PRIVATE cxx_std_17)
if(MSVC)
    # Sin /utf-8 MSVC codifica dos veces los \u de los literales u8.
    target_compile_options(mu_text_core_test PRIVATE /utf-8)
endif()
add_test(NAME text_core COMMAND mu_text_core_test)

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
    tests/RenderProfile.cpp src/rhi/GL/ProfileQueries.cpp)
target_include_directories(mu_render_profile_test PRIVATE src)
target_compile_features(mu_render_profile_test PRIVATE cxx_std_17)
add_test(NAME render_profile COMMAND mu_render_profile_test)

add_executable(mu_render_profile_flow_test EXCLUDE_FROM_ALL
    tests/RenderProfileFlow.cpp src/render/RenderProfile.cpp)
target_include_directories(mu_render_profile_flow_test PRIVATE src)
target_compile_features(mu_render_profile_flow_test PRIVATE cxx_std_17)
add_test(NAME render_profile_flow COMMAND mu_render_profile_flow_test)

# DESVIACION R3a: contrato de captura/fallback sin necesitar un driver GL.
add_executable(mu_terrain_draw_flow_test EXCLUDE_FROM_ALL tests/TerrainDrawFlow.cpp src/render/TerrainDraw.cpp)
target_include_directories(mu_terrain_draw_flow_test PRIVATE src)
target_compile_features(mu_terrain_draw_flow_test PRIVATE cxx_std_17)
add_test(NAME terrain_draw_flow COMMAND mu_terrain_draw_flow_test)

# Verificación gráfica SDL3: obligatoria con Mesa en Linux, omisible si el
# runner Windows no dispone de un contexto 3.3 (salida77, no un falso pase).
if(MU_PLATFORM_BACKEND STREQUAL "SDL3")
    add_executable(mu_bmd_device_test EXCLUDE_FROM_ALL
        tests/BMDDevice.cpp
        src/platform/GLFunctions.cpp src/render/ResourceTables.cpp
        src/render/RenderProfile.cpp src/rhi/GL/ProfileQueries.cpp
        src/render/Materials.cpp src/rhi/GL/FragmentPipeline.cpp src/render/GlyphAtlas.cpp src/render/BufferedDraw.cpp src/render/Skinning.cpp src/render/MeshTopology.cpp
        src/rhi/GL/BufferedDraw.cpp src/rhi/GL/MeshDraw.cpp
        src/rhi/GL/ShadowMap.cpp src/render/ShadowLight.cpp
        src/rhi/GL/Skinning.cpp src/render/Gfx.cpp src/rhi/GL/GfxDevice.cpp
        src/rhi/Dispatch.cpp src/render/RenderState.cpp)
    target_include_directories(mu_bmd_device_test PRIVATE src ${MU_MINIAUDIO_INCLUDE_DIR})
    target_compile_features(mu_bmd_device_test PRIVATE cxx_std_17)
    target_link_libraries(mu_bmd_device_test PRIVATE SDL3::SDL3-static)
    if(WIN32)
        target_link_libraries(mu_bmd_device_test PRIVATE opengl32)
    else()
        target_link_libraries(mu_bmd_device_test PRIVATE GL)
    endif()
    if(MSVC)
        target_compile_options(mu_bmd_device_test PRIVATE /EHsc /fp:precise /UNDEBUG)
    else()
        target_compile_options(mu_bmd_device_test PRIVATE -UNDEBUG $<$<CONFIG:Release>:-g>)
    endif()
    add_test(NAME bmd_device COMMAND mu_bmd_device_test)

    # Texto Unicode: el layout nuevo tiene que dar el mismo 1252 que antes.
    add_executable(mu_text_shaper_test EXCLUDE_FROM_ALL
        tests/TextShaper.cpp src/render/text/TextCore.cpp src/render/text/Utf8Proc.c src/render/text/UniBreak.c
        src/render/Texture/ImageCodec.cpp)
    target_include_directories(mu_text_shaper_test PRIVATE src)
    target_compile_features(mu_text_shaper_test PRIVATE cxx_std_17)
    target_compile_definitions(mu_text_shaper_test PRIVATE
        MU_TEST_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/bin/Client/System/Font")
    target_link_libraries(mu_text_shaper_test PRIVATE mu_platform_sdl)
    if(MSVC)
        target_compile_options(mu_text_shaper_test PRIVATE /utf-8 /EHsc)
    endif()
    add_test(NAME text_shaper COMMAND mu_text_shaper_test)
    set_tests_properties(bmd_device PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120)
endif()

add_executable(mu_panel_anchor_test EXCLUDE_FROM_ALL tests/PanelAnchor.cpp)
target_include_directories(mu_panel_anchor_test PRIVATE src)
target_compile_features(mu_panel_anchor_test PRIVATE cxx_std_17)
add_test(NAME panel_anchor COMMAND mu_panel_anchor_test)
