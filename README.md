# Toast Message (Levi Launchroid native mod)

Push custom Minecraft Bedrock **toast** notifications, fully editable in-game
through a large, live-scalable ImGui interface with an on-screen (virtual)
keyboard. The `ToastMessageType` is selectable from a dropdown and the interface
adapts to the chosen type.

- **Target platform:** Minecraft Bedrock Edition on Android
- **Loader:** Levi Launchroid / LeviLauncher native mod loader (`preload-native`)
- **Target Minecraft version:** `1.26.30+` (manifest matches `1.26.*`)
- **Target ABI:** `arm64-v8a` (also builds `armeabi-v7a`)
- **Output:** `libtoast_message.so`, packaged as a `.levipack`

## What it does

- Hooks `ToastManager::pushToast` to capture the live toast manager, then
  constructs a `ToastMessage` with the documented Bedrock constructors and
  pushes it — the same path the game uses for achievements, invites, etc.
- Renders an ImGui overlay (toggled from the LeviLauncher mod menu button
  **"Toast Message"**) containing:
  - a **Toast Type** dropdown covering all 20 `ToastMessageType` values;
  - **Title** / **Subtitle** text fields (plus **Message Id** / **Link** /
    **Silent** / **Button Action** which appear only for player-messaging-style
    types that use the extended constructor);
  - **UI Scale** and **Font Scale** sliders that resize the whole interface and
    the font live;
  - an on-screen QWERTY keyboard (from Flix01's ImGui virtual keyboard) that
    edits the currently selected field — useful on touch devices;
  - a **Send Toast** button.
- Persists everything to `config/config.json` via the preloader typed-config
  system, and generates `config/config.schema.json` for launcher-side editing.

## Requirements

- Android SDK
- Android NDK `28.2.13676358` (or a compatible NDK; `r28`)
- CMake `3.22+`
- Ninja
- PowerShell 7+ (Windows) **or** Bash (Linux/macOS)

The build fetches `preloader-android`, `imgui` (pinned `v1.91.6`),
`nlohmann_json`, `boost::pfr` and `magic_enum` automatically via CMake
`FetchContent`. To build fully offline, provide a local preloader checkout with
`-DLEVI_PRELOADER_ROOT=<path>` (or `LEVI_PRELOADER_ROOT` env var / `-PreloaderRoot`).

## Build & package

### Windows (PowerShell)

```powershell
$env:ANDROID_HOME = "C:/Users/<you>/AppData/Local/Android/Sdk"
./scripts/package.ps1 -Abi arm64-v8a
# or all ABIs:
./scripts/package.ps1 -Abi all
```

### Linux / macOS

```bash
ANDROID_HOME=~/Android/Sdk ./scripts/build.sh arm64-v8a
```

### Manual CMake

```bash
# 1) generate config.json + config.schema.json (host build)
cmake -S . -B build-config -G Ninja
cmake --build build-config --target levi_generate_config

# 2) cross-compile and package
cmake -S . -B build-arm64-v8a -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_HOME/ndk/28.2.13676358/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-28 \
  -DANDROID_STL=c++_shared \
  -DLEVI_PACKAGE_CONFIG_DIR="$PWD/build-config/generated-config"
cmake --build build-arm64-v8a --target levi_package
```

The `.levipack` is written to `build-arm64-v8a/toast-message-0.1.3-arm64-v8a.levipack`
and contains `manifest.json`, `libtoast_message.so`, and the `config/` files.

## Install

Import the generated `.levipack` from the LeviLauncher mod manager, enable
**Toast Message**, launch Minecraft, then tap the **Toast Message** button in the
mod menu to open the overlay.

> Toasts can only be *sent* after the game has pushed at least one toast on its
> own (join a world, unlock something, etc.), because the mod captures the live
> `ToastManager` from the game's own activity rather than guessing its address.

## Configuration (`config/config.json`)

| Key | Meaning |
|-----|---------|
| `enabled` | Master on/off switch. |
| `toastType` | Stable id of the selected `ToastMessageType`. |
| `title` / `subtitle` | Toast text lines. |
| `messageId` / `silent` / `link` / `buttonActionType` | Extended (player-messaging) fields. |
| `uiScale` / `fontScale` | Live ImGui interface / font scale. |

## Safe feature scope & EULA

This mod only displays notification toasts — a cosmetic, client-side UI feature.
It creates no gameplay advantage, does not read hidden game state, and does not
touch anti-cheat, networking exploits, or memory outside the documented toast
API. It is intended for legitimate, Minecraft-EULA-compliant use.

See `CONVERSION_REPORT.md`, `SIGNATURE_REPORT.md`, `HOOK_REPORT.md`,
`TESTING_GUIDE.md` and `TROUBLESHOOTING.md` for engineering detail.
