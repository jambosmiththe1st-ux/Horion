# Horion — `.xp`-only build for Minecraft Bedrock v1.26.13

This fork is an aggressively stripped-down build of [Horion](https://github.com/horionclient/Horion).

> **Only one command is implemented: `.xp <amount>[L]`.**
> Every module (KillAura, Fly, ESP, …), every GUI (ClickGui, TabGui),
> the scripting engine, the friend-list, the pathfinder, and every other
> command were deleted. The remaining surface area is the smallest thing
> that can install a chat hook, locate the local player, and call
> `Player::addExperience` / `Player::addLevels` via its vtable.

Upstream Horion was last meaningfully updated in December 2021 (~MC Bedrock
1.18). Updating it for **Minecraft Bedrock Edition v1.26.13** (April 2026)
requires re-deriving a small set of version-specific byte patterns and
offsets from the new `Minecraft.Windows.exe`. Every such site in this fork
is clearly marked with a `TODO(v26.13)` comment.

---

## What was removed

```
Horion/Module/          - every cheat module
Horion/Menu/            - ClickGui, TabGui
Horion/Scripting/       - Chakra-based JS scripting
Horion/Config/          - config persistence
Horion/FriendList/      - friend list
Horion/path/            - pathfinder
Horion/DrawUtils, GuiUtils, ImmediateGui, ...
Horion/Command/Commands/*   - all commands except XpCommand
SDK/                    - all SDK except the bits used by XpCommand
Utils/                  - Target, SkinUtil, HMath, Json, ChakraHelper, ...
```

## What is left

```
Horion/Loader.cpp                        - DllMain, start thread, uninject hotkey
Horion/Command/CommandMgr.{h,cpp}        - dispatches ".xp" only
Horion/Command/Commands/ICommand.{h,cpp} - minimal command base
Horion/Command/Commands/XpCommand.{h,cpp}- the only command

Memory/GameData.{h,cpp}                  - ClientInstance sig scan
Memory/Hooks.{h,cpp}                     - chat-send detour only
Memory/SlimMem.{h,cpp}                   - pointer-chain helper (unchanged)
Memory/MinHook.h                         - MinHook header (unchanged)

SDK/CClientInstance.h                    - minimal; one offset
SDK/CEntity.h                            - minimal; two vtable slots
SDK/TextHolder.{h,cpp}                   - MC's SSO string (unchanged)

Utils/Logger.{h,cpp}, TextFormat.{h,cpp} - logging + chat color escapes
Utils/Utils.{h,cpp}                      - FindSignature helper + bits
Utils/xorstr.h                           - XorString macro (unchanged)
```

---

## v1.26.13 TODO checklist

There are **four** signature / offset sites that MUST be verified against
the `Minecraft.Windows.exe` binary shipped with MC Bedrock v1.26.13 before
the DLL will function. Each is marked in source with a loud
`TODO(v26.13)` block that includes step-by-step instructions for
re-deriving it. Ordered by importance:

| # | File | What it finds | Status |
|---|------|---------------|--------|
| 1 | `Memory/GameData.cpp`     → `HORION_V26_13_SIG_CLIENT_INSTANCE` | Signature for the static `C_ClientInstance**` pointer inside `Minecraft.Windows.exe`. Without this, `getLocalPlayer()` always returns `nullptr`. | **TODO** |
| 2 | `Memory/Hooks.cpp`        → `HORION_V26_13_SIG_CHAT_SEND`       | Signature for `ClientInstanceScreenModel::sendChatMessage` (or the modern equivalent). Without this, chat-typed `.xp` commands aren't intercepted. | **TODO** |
| 3 | `SDK/CEntity.h`           → `HORION_V26_13_VTIDX_ADD_EXPERIENCE` / `_ADD_LEVELS` | Vtable slot indices for `Player::addExperience(int)` / `Player::addLevels(int)`. Wrong indices will call the wrong virtual and most likely crash MC. | **TODO** |
| 4 | `SDK/CClientInstance.h`   → `HORION_V26_13_OFFSET_CLIENT_INSTANCE_TO_LOCAL_PLAYER` | Byte offset from `ClientInstance*` to its `LocalPlayer*` field. | **TODO** |

### Recommended workflow

1. Install MC Bedrock Edition v1.26.13 from the Microsoft Store (or extract
   `Minecraft.Windows.exe` from the UWP appx package).
2. Load it in IDA Pro, Ghidra, or Binary Ninja.
3. For each of the four TODOs above, follow the per-file instructions in
   the `TODO(v26.13)` comment block:
   - Locate the target function / data / vtable entry.
   - Confirm the layout hasn't changed semantically.
   - Paste a fresh pattern, vtable index, or offset.
4. Rebuild the DLL with MSBuild (`Horion.vcxproj`, x64 configuration) or
   MSVC CMake (`cmake -G "Visual Studio 17 2022" -A x64 .`).
5. Inject the resulting `Horion.dll` into `Minecraft.Windows.exe` using
   any UWP-capable DLL injector (the original Horion injector, Extreme
   Injector v3 with UWP patch, Process Hacker's DLL inject action, etc.).
6. In-game, open chat and type `.xp 100` (add 100 XP) or `.xp 5L` (add 5
   levels). Unhook cleanly with **Ctrl+L**.

If `FindSignature` fails at runtime, Horion writes a clear error to its
log file (`%APPDATA%\Packages\<MinecraftUWP>\RoamingState\logs.txt`)
indicating which signature needs updating.

---

## Building

### MSBuild

- Open `Horion.sln` in Visual Studio 2019+ with the Desktop C++ workload.
- Select `Release | x64` and build.
- MinHook (`minhook/libMinHook.x64.lib`) is already vendored.

### CMake

```
cmake -G "Visual Studio 17 2022" -A x64 -B build
cmake --build build --config Release
```

### Build environment note

The build is **Windows-only** — it depends on the Win32/UWP SDK and MSVC.
This repo cannot be built on Linux or macOS.

---

## Original Horion license

This work is licensed under the Creative Commons Attribution-NonCommercial
4.0 International License. To view a copy of this license, visit
<http://creativecommons.org/licenses/by-nc/4.0/> or send a letter to
Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
