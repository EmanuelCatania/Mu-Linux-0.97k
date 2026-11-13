"""Carpetas de src/ que cambiaron de nombre (Fase 5, E3).

Los tests que leen el código de una revisión vieja (`git show REV:ruta`) piden
la ruta nueva; si en esa revisión no existe, se prueba la de antes.
"""
import subprocess

# nueva -> vieja, como prefijos de ruta relativos a client/.
MOVED = {
    "src/render/text/": "src/Text/",
    "src/game/terrain/": "src/Terrain/",
    "src/render/Terrain_RenderTerrain.cpp": "src/Terrain/Terrain_RenderTerrain.cpp",
    "src/render/Terrain_RenderFace.cpp": "src/Terrain/Terrain_RenderFace.cpp",
    "src/render/Terrain_RenderBlocks.cpp": "src/Terrain/Terrain_RenderBlocks.cpp",
    "src/render/Terrain_LegacyRender.cpp": "src/Terrain/Terrain_LegacyRender.cpp",
    "src/game/monster/": "src/Monster/",
    "src/game/item/": "src/Item/",
    "src/ui/Quest_UIState.cpp": "src/Item/Quest_UIState.cpp",
    "src/ui/inventory/Item_Display.cpp": "src/Item/Item_Display.cpp",
    "src/ui/inventory/Item_InventoryRender.cpp": "src/Item/Item_InventoryRender.cpp",
    "src/ui/inventory/Item_ClickHandler.cpp": "src/Item/Item_ClickHandler.cpp",
    "src/assets/text/": "src/Local/",
    "src/assets/data/Monster_Data.cpp": "src/Monster/Monster_Data.cpp",
    "src/assets/data/Skill_Data.cpp": "src/Item/Skill_Data.cpp",
    "src/assets/data/Quest_Data.cpp": "src/Item/Quest_Data.cpp",
    "src/assets/data/NPC_Data.cpp": "src/Item/NPC_Data.cpp",
    "src/assets/data/Item_Data.cpp": "src/Item/Item_Data.cpp",
    "src/assets/data/Gate_Data.cpp": "src/Item/Gate_Data.cpp",
    "src/assets/data/Filter_Data.cpp": "src/Item/Filter_Data.cpp",
    "src/assets/data/Dialog_Data.cpp": "src/Item/Dialog_Data.cpp",
    "src/assets/data/ContentCatalog.h": "src/Item/ContentCatalog.h",
    "src/assets/data/ContentCatalog.cpp": "src/Item/ContentCatalog.cpp",
    "src/assets/model/": "src/Model/",
    "src/assets/model/NPC_ModelLoad.cpp": "src/Item/NPC_ModelLoad.cpp",
    "src/app/util/": "src/Util/",
    "src/app/debug/": "src/Debug/",
    "src/app/core/": "src/Core/",
    "src/app/config/": "src/Config/",
    "src/scenes/": "src/Scene/",
    "src/ui/": "src/UI/",
    "src/render/": "src/Render/",
    "src/rhi/": "src/Render/Backend/",
    "src/platform/input/": "src/Input/",
    "src/platform/": "src/Platform/",
    "src/net/": "src/Net/",
    "src/game/core/": "src/Game/",
    "src/game/entity/": "src/Entity/",
    "src/game/combat/": "src/Combat/",
    "src/game/math/": "src/Math/",
    "src/game/trade/": "src/Trade/",
    "src/game/physics/": "src/Physics/",
    "src/game/path/": "src/Path/",
    "src/game/party/": "src/Party/",
    "src/audio/": "src/Sound/",
}


def candidates(path):
    """La ruta pedida y, si estaba en una carpeta movida, la de antes."""
    out = [path]
    for new, old in MOVED.items():
        if path.startswith(new):
            out.append(old + path[len(new):])
    return out


def show(root, revision, path):
    """`git show REV:ruta` probando la ruta vieja si la nueva no existe."""
    error = None
    for candidate in candidates(path):
        try:
            return subprocess.check_output(["git", "show", f"{revision}:{candidate}"], cwd=root,
                                           stderr=subprocess.DEVNULL).decode("utf-8")
        except subprocess.CalledProcessError as e:
            error = e
    raise error
