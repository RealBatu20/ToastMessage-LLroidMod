# Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| CMake: `LEVI_PRELOADER_ROOT must point to a preloader-android checkout` | Offline / fetch failed | Clone `preloader-android` and pass `-DLEVI_PRELOADER_ROOT=<path>` (or `-PreloaderRoot`). |
| CMake: `LEVI_PACKAGE_CONFIG_DIR must contain config.json` | Config not generated before the Android build | Run the host `levi_generate_config` target first (the scripts do this automatically). |
| CMake: `Android NDK not found` | `ANDROID_HOME`/`ANDROID_NDK_HOME` unset | Set one; NDK `28.2.13676358` (or `r28`) is preferred. |
| Link error: `cannot find -lGLESv3` / `-lEGL` | Wrong platform level | Use `-DANDROID_PLATFORM=android-28` or newer. |
| Log: `Missing required ToastMessage symbols` | Build stripped or renamed the symbols | Regenerate per `SIGNATURE_REPORT.md` (IDA/Ghidra), or switch to a byte-pattern signature. |
| `Send Toast` does nothing, log says *No ToastManager captured yet* | No vanilla toast has fired | Join a world / trigger any game toast once so the capture hook can grab the manager. |
| Overlay never appears | `eglSwapBuffers` hook failed or non-GLES3 device | Check `logcat` for `Render hook installed`; confirm GLES3 support. |
| Virtual keyboard keys do nothing | Wrong edit target selected, or field hidden for the current type | Pick the correct target in the on-screen-keyboard combo; Message Id/Link only exist for player-messaging types. |
| Game touch controls stop working | Overlay left open (it consumes touches while visible) | Close the overlay with the mod-menu button. |
| Toast text is cut off / huge | Extreme scale values | Adjust **UI Scale** / **Font Scale** sliders (range 0.75–4.0). |

## Build tips

- Clean rebuild: `./scripts/clean.sh` then rebuild.
- Offline reproducible build: local `preloader-android` + a warm CMake
  `FetchContent` cache (or vendor `imgui`/deps and point `FetchContent` at
  local mirrors).
- 16 KB page size: the link flags already set `-Wl,-z,max-page-size=16384` for
  newer Android devices.
