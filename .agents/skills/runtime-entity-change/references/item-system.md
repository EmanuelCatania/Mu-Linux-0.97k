# Item-system checklist

Use for items, equipment, jewels, wings, bows, glows, and item-bound effects.

## Check

- Confirm the complete group/index namespace, reserved values, persisted
  representation, and a neighboring item of the same category.
- Trace authoritative stats and restrictions plus encoder/client names, models,
  textures, icons, tooltips, inventory/equip behavior, effects, and bounds.
- Verify serialization widths before introducing an ID, option, state, or flag
  outside existing masks or fields.
- For transformations such as jewels, define valid targets, consumption,
  success/failure, rollback, duplicate-request protection, persistence, and
  server-side enforcement.
- Record model, texture, icon, effect, sound, and other asset provenance; verify
  exact loader paths and case.
- Load
  [`economy-and-progression.md`](economy-and-progression.md)
  only when acquisition, sale, drop, reward, or mix behavior changes.

## Validate

Test creation/acquisition, display, use or equip, inventory boundaries,
trade/warehouse restrictions, reconnect and persistence, invalid targets or
options, missing assets, and one neighboring legacy item.
