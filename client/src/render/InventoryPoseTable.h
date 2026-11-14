#pragma once

// Pose de los items 3D en las grillas (Content/item/inventory_pose.toml).

enum class InventoryAngleMode : unsigned char { Fixed, ItemAttribute, Level };

struct InventoryPose {
    float Angle[3] = { 270.0f, -10.0f, 0.0f };
    float BodyHeight = 0.0f;
    float Scale = 0.0025f;
    float NegativeLevelScale = 0.0025f;
    InventoryAngleMode AngleMode = InventoryAngleMode::Fixed;
};

// Pose del modelo; los modelos sin regla devuelven la de por defecto.
const InventoryPose& InventoryPose_Get(int model);
