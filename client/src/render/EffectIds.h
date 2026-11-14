#pragma once

// Ids de tipo de los efectos, partículas, joints y sprites.
//
// Los ids >= 1000 son slots de textura (BITMAP_) y los < 1000 son slots de
// modelo (MODEL_). Donde el mismo archivo existe en MU 5.2 se usa el nombre
// de 5.2, con su misma forma BASE + n (por ejemplo BITMAP_SPARK + 1). El
// resto lleva el nombre del archivo que carga ese slot.
//
// Los MODEL_ de items (400 en adelante) están en game/item/ItemDefines.h.

// Modelos de skill
constexpr int MODEL_ICE             = 190;
constexpr int MODEL_FIRE            = 191;
constexpr int MODEL_POISON          = 192;
constexpr int MODEL_TAMA            = 193;
constexpr int MODEL_ICE_SMALL       = 199;
constexpr int MODEL_CIRCLE          = 200;
constexpr int MODEL_CIRCLE_LIGHT    = 201;
constexpr int MODEL_MAGIC2          = 203;
constexpr int MODEL_STORM           = 204;
constexpr int MODEL_LASER           = 205;
constexpr int MODEL_SAW             = 209;
constexpr int MODEL_ARROW           = 216;
constexpr int MODEL_ARROW_STEEL     = 217;
constexpr int MODEL_ARROW_THUNDER   = 218;
constexpr int MODEL_ARROW_LASER     = 219;
constexpr int MODEL_ARROW_V         = 220;
constexpr int MODEL_ARROW_SAW       = 221;
constexpr int MODEL_ARROW_NATURE    = 222;
constexpr int MODEL_ARROW_BOMB      = 223;
constexpr int MODEL_ARROW_WING      = 224;
constexpr int MODEL_MAGIC_CIRCLE1   = 233;
constexpr int MODEL_BOSS_HEAD       = 234;
constexpr int MODEL_SKILL_BLAST     = 240;
constexpr int MODEL_SKILL_INFERNO   = 241;
constexpr int MODEL_ARROW_DOUBLE    = 242;
constexpr int MODEL_EARTHQUAKE      = 245;
constexpr int MODEL_EARTHQUAKE2     = 246;
constexpr int MODEL_EARTHQUAKE3     = 247;
constexpr int MODEL_EARTHQUAKE4     = 248;
constexpr int MODEL_EARTHQUAKE5     = 249;
constexpr int MODEL_EARTHQUAKE7     = 251;
constexpr int MODEL_EARTHQUAKE8     = 252;
constexpr int MODEL_WAVE            = 253;
constexpr int MODEL_TAIL            = 254;
constexpr int MODEL_PIERCING        = 255;
constexpr int MODEL_MAGIC_CAPSULE2  = 259;
constexpr int MODEL_STONECOFFIN     = 260;
constexpr int MODEL_STONECOFFIN2    = 261;
constexpr int MODEL_GATE            = 262;
constexpr int MODEL_GATE2           = 263;
constexpr int MODEL_SPEARSKILL      = 266;

// Texturas de efecto
constexpr int BITMAP_LIGHT          = 1150;
constexpr int BITMAP_MOTION_BLUR    = 1166;
constexpr int BITMAP_SPARK          = 1175;
constexpr int BITMAP_ENERGY         = 1180;
constexpr int BITMAP_LIGHTNING      = 1190;
constexpr int BITMAP_FIRE           = 1195;
constexpr int BITMAP_FLAME          = 1200;
constexpr int BITMAP_BLOOD          = 1205;
constexpr int BITMAP_EXPLOTION      = 1215;
constexpr int BITMAP_SMOKE          = 1220;
constexpr int BITMAP_SHINY          = 1230;
constexpr int BITMAP_BUBBLE         = 1241;
constexpr int BITMAP_FLOWER01       = 1242;
constexpr int BITMAP_FIRECRACKER    = 1247;
constexpr int BITMAP_FLARE          = 1249;
constexpr int BITMAP_JOINT_SPIRIT   = 1253;
constexpr int BITMAP_JOINT_THUNDER  = 1254;
constexpr int BITMAP_JOINT_SPARK    = 1257;
constexpr int BITMAP_JOINT_ENERGY   = 1258;
constexpr int BITMAP_JOINT_HEALING  = 1259;
constexpr int BITMAP_JOINT_LASER    = 1260;
constexpr int BITMAP_MOTION_BLUR_R2 = 1262;
constexpr int BITMAP_MAGIC          = 1263;
constexpr int BITMAP_CLOUD          = 1268;
constexpr int BITMAP_SKULL          = 1274;
constexpr int BITMAP_FLARE_BLUE     = 1277;
constexpr int BITMAP_ADV_SMOKE      = 1381;
constexpr int BITMAP_POUNDING_BALL  = 1383;
