# HighlightObjects - Ignore Empty Containers

> **What this fork adds:**
>
> - **Ignore empty containers** - chests, barrels, sacks etc. that you have
>   already emptied are no longer highlighted, neither under the crosshair nor
>   by the area-scan hotkey. Containers you have never opened are still
>   highlighted, because their leveled loot is not rolled until then.
> - **Hotkey toggles highlight** - press the scan hotkey once and everything in
>   the scan radius stays highlighted until you press it again. The highlight
>   follows you as you move and drops objects as you loot them.
>
> Both are off by default - turn them on in the SKSE Menu Framework page (F1)
> or in `HighlightObjects.ini`:
>
> ```ini
> [Filter]
> ignoreEmptyContainers = true
>
> [Radius Highlight]
> scanToggle = true
> ```

This is the source of [HighlightObjects](https://www.nexusmods.com/skyrimspecialedition/mods/183648)
1.8 by godfreysnow (GPL-3.0 with modding exception, see LICENSE and
EXCEPTIONS.md), with the build system recreated (xmake + CommonLibSSE-NG)
and the option above added. All credit for the mod itself goes to the
original author.

## Other changes vs. upstream 1.8

- builds against CommonLibSSE-NG v10.1.0 for all runtimes (SE/AE/VR), so the
  crosshair target is read via `CrosshairPickData::GetActiveTarget()`
- builds against the SKSE Menu Framework 3 header (`src/SKSEMenuFramework.h`,
  MIT, from [SKSE-Menu-Framework-3-Example](https://github.com/QTR-Modding/SKSE-Menu-Framework-3-Example),
  license in `licenses/`)

## Install

1. Install the original HighlightObjects from Nexus (it provides the esp and ini)
   and its requirements.
2. Grab `HighlightObjects.dll` from the latest CI run artifact and install it as a
   mod that overrides the original dll.

On SE/AE you also need a plugin that keeps EditorIDs loaded, e.g.
[powerofthree's Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/51073) -
without it the plugin logs `No TESEffectShader forms found` and does nothing.

Tested on Skyrim AE 1.7.104 with SKSE 2.3.1 and SKSE Menu Framework 3.18.

## Build

Windows + MSVC + xmake:

```
git clone --recursive <this repo>
xmake config --mode=releasedbg
xmake build
```

CI builds on every push and uploads the dll + pdb as an artifact.
