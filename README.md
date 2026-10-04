# HighlightObjects (patched)

Source of [HighlightObjects](https://www.nexusmods.com/skyrimspecialedition/mods/183648)
1.8 by godfreysnow (GPL-3.0 with modding exception, see LICENSE and
EXCEPTIONS.md), rebuilt with xmake + CommonLibSSE-NG and extended with:

- `[Filter] ignoreEmptyContainers` - skip containers that have already been
  emptied. Containers that were never opened are still highlighted, since
  their leveled loot is not rolled until then.

`src/SKSEMenuFramework.h` comes from
[SKSE-Menu-Framework-3-Example](https://github.com/QTR-Modding/SKSE-Menu-Framework-3-Example).

## Build

Windows + MSVC + xmake:

```
git clone --recursive <this repo>
xmake config --mode=releasedbg
xmake build
```

CI builds on every push and uploads the dll as an artifact.

The plugin still needs `HighlightObjects.esp` from the original Nexus
release, plus SKSE Menu Framework, and on SE/AE a plugin that keeps
EditorIDs loaded (e.g. powerofthree's Tweaks).
