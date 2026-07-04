# Conversion / Implementation Report

## 1. Project classification

This is **not** a Java-mod conversion — it is a **new native Levi Launchroid
mod** built to a spec: hook `ToastManager` to push a `ToastMessage`, with an
in-game ImGui editor (using the requested Flix01 virtual-keyboard layout), big
UI with live scale, and a `ToastMessageType` dropdown that changes the
interface. Target: Minecraft Bedrock `1.26.30+`, `arm64-v8a`.

## 2. Inputs used

| Source | Role |
|--------|------|
| LeviLamina 1.26.x `ToastMessage` reference (provided) | Class layout, constructors, `ToastMessageType` enum, `IToastManager::pushToast`. |
| [Flix01 ImGui virtual keyboard gist](https://gist.github.com/Flix01/78182e9c9e0f9dfad79619e56530568a) | On-screen keyboard widget (vendored, patched for ImGui 1.91.6). |
| [QYCottage LeviLauncher Android mod template](https://github.com/QYCottage/levilauncher-android-mod-template) | Scaffold: CMake, `manifest.json.in`, lifecycle, typed config, packaging. |
| [preloader-android](https://github.com/LiteLDev/preloader-android) | Mod API: `NativeMod`, `Config`, `Hook`, `Signature`, `ModMenu`, `PreloaderInput`, `Gloss`. |
| [Dear ImGui v1.91.6](https://github.com/ocornut/imgui) | UI + GLES3 backend. |

## 3. Bedrock system mapping

| Requested behavior | Native implementation |
|--------------------|-----------------------|
| Push a toast | Resolve `ToastMessage` ctor + `ToastManager::pushToast` by symbol; capture manager via hook; construct & push. |
| Edit message in-game | ImGui overlay with `InputText` + Flix01 virtual keyboard. |
| Change `ToastMessageType` | Dropdown over all 20 enum values (`src/toast/ToastTypes.h`); extended fields shown only for player-messaging types. |
| Big UI + live scale | `ImGuiStyle::ScaleAllSizes(uiScale)` (rebased each change) + `io.FontGlobalScale = fontScale`, driven by sliders. |
| Persist settings | preloader typed `ConfigFile<ModConfig>` → `config/config.json` (+ schema). |
| Toggle overlay | LeviLauncher mod-menu button via `pl::modmenu::ButtonBuilder`. |

## 4. Evidence classification

- **FACT:** template layout & APIs; preloader Hook/Signature/Config/ModMenu/
  Input APIs (read from headers); ImGui 1.91.6 API (compiled against);
  `ToastMessage` mangled names (compiler-derived); config generation (built &
  run on host — see below).
- **INFERENCE:** 4 KiB is a safe over-allocation for `ToastMessage`; the toast
  visuals for each type match the game's own styling.
- **HYPOTHESIS (device-verify):** the exact `ToastMessage` size; that the target
  build exports the symbols; touch/orientation behavior across devices; that the
  `eglSwapBuffers` hook order composes correctly with Minecraft's own UI.

## 5. What was actually verified in this environment

- ✅ Host config generator **compiles and runs**, producing valid
  `config.json` + `config.schema.json`.
- ✅ Vendored virtual keyboard **compiles** against ImGui v1.91.6 (after a
  documented 2-spot compat patch: `imgui_internal.h` for `IM_STATIC_ASSERT`,
  math operators, non-const `ImFont*`, and a `>=` label-count assert).
- ✅ Every mod translation unit **compiles to an object** against the real
  preloader + ImGui headers (Android platform headers stubbed on host).
- ⛔ **Not built here:** no Android NDK is available in this environment, so
  `libtoast_message.so` and the `.levipack` were **not** produced. Build them
  with the commands in `README.md` / `TESTING_GUIDE.md`.

## 6. Template integration

- **Kept:** `manifest.json.in`, `scripts/package.ps1`, host config-generator
  path, `levi_package` target, lifecycle shape (`load/enable/disable/unload`).
- **Modified:** mod metadata (`toast-message` / `toast_message` / `1.26.*`);
  `MyMod`/`Config` renamed to the `toast_message` namespace and expanded.
- **Expanded:** added `toast/`, `ui/`, `android/`, `signatures/`, `util/`
  modules, ImGui + GLES3 linkage, EGL render hook, extra scripts (`build.sh`,
  `clean.sh`) and the GitHub Actions workflow.

## 7. Feature parity

Full parity with the request. The one operational caveat (documented in-UI and
in the README): a toast can only be sent after the game pushes one first, since
the manager pointer is captured rather than guessed — a deliberate robustness
trade-off over a brittle hardcoded instance path.
