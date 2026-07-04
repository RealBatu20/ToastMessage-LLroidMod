# Testing Guide

## Build the artifacts

```bash
ANDROID_HOME=~/Android/Sdk ./scripts/build.sh arm64-v8a
# -> build-arm64-v8a/toast-message-0.1.3-arm64-v8a.levipack
```

Confirm the package contains `libtoast_message.so`, `manifest.json`, and
`config/config.json` + `config/config.schema.json`:

```bash
unzip -l build-arm64-v8a/toast-message-0.1.3-arm64-v8a.levipack
```

## On-device checklist

1. **Load / enable:** import the `.levipack`, enable the mod, check `logcat`
   for `Toast Message enabled` and `Render hook installed on eglSwapBuffers`.
2. **Overlay opens:** tap the **Toast Message** mod-menu button; the ImGui
   window appears.
3. **Live scale:** drag **UI Scale** and **Font Scale** — the whole window and
   the text resize immediately.
4. **Type dropdown:** pick `Player Messaging` / `Invite` / `Party Invite` — the
   **Message Id / Link / Silent / Button Action** fields appear; pick e.g.
   `Achievement` — they disappear.
5. **Virtual keyboard:** set the on-screen keyboard target, tap keys, toggle
   **Shift**; the selected field updates. Test Backspace and Space.
6. **Manager capture:** join a world (or trigger any vanilla toast). The
   "waiting to capture" notice disappears.
7. **Send:** press **Send Toast**; the toast renders with your title/subtitle
   and the chosen type's styling. Verify `logcat` shows `Pushed toast: ...`.
8. **Robustness:** rotate the device and repeat 3–7 (orientation change);
   open/close the overlay several times; verify game touch controls work when
   the overlay is closed.
9. **Persistence:** change fields, close Minecraft, relaunch — values persist
   (written on `disable()`).

## Things to watch (unverified until run on hardware)

- If `Send Toast` logs *"No ToastManager captured yet"* forever, the pushToast
  hook did not capture — check that the symbol resolved (`ToastBridge
  initialised` line) and that a vanilla toast actually fired.
- If nothing renders, confirm the `eglSwapBuffers` hook installed and that the
  device uses GLES3.
- If the missing-symbol error appears at enable, regenerate the mangled names
  per `SIGNATURE_REPORT.md`.
