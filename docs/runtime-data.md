# Runtime, encoder, and legacy data

Use `.agents/skills/runtime-entity-change/SKILL.md` for changes to items, monsters, maps, effects, shops, drops, mixes, or other cross-component entities.

## Tracked runtime and external runtime

- `runtime/` contains versioned templates and data.
- `C:\Dev\runtime\mu-097k` is the local executable client copy.
- Use the workflow to initialize and deploy; do not use the tracked template as a normal build directory.
- `-ForceRuntime` can overwrite local changes and requires an explicit decision.

## Encoder flow

```text
runtime/encoder/*.ini|*.txt -> InfoEncoder.exe -> ClientInfo.bmd -> Main.dll
```

Rules:

- do not edit `ClientInfo.bmd` manually;
- identical inputs must produce identical bytes;
- encoder and DLL structures must keep identical layouts;
- new fields require a deterministic default and compatibility strategy;
- preserve order when it determines indices or offsets;
- validate ranges and references before generating the binary.

## Legacy formats

Before editing INI, DAT, TXT, or SQL:

1. locate the parser;
2. confirm encoding, comments, delimiters, and terminators;
3. confirm whether order is significant;
4. confirm ranges, sentinels, and duplicate handling;
5. identify whether the file is copied, mounted, encoded, or read directly.

Do not convert format, encoding, or line endings as incidental cleanup.

## Entities and assets

Items, monsters, maps, wings, bows, glows, and effects may require changes in the client, encoder, GameServer, runtime, and web panel. Verify IDs, models, textures, names, stats, and web representation.

New assets and binaries require origin, authorization, and hashes. Do not import material from another game or distribution without verifiable permission.

## Duplicated contracts

Repeated serials, versions, ports, flags, and layouts should have one canonical source and automated validation whenever possible.
