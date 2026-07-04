# Hook Analysis

Two hooks are installed, both via GlossHook (inline hook), both removable.

## 1. `ToastManager::pushToast` — capture hook (`src/toast/ToastBridge.cpp`)

- **Target:** resolved `libminecraftpe.so` symbol (see `SIGNATURE_REPORT.md`).
- **Detour:** `pushToastDetour(void* self, void* toast)`.
- **Purpose:** the first time the game itself pushes any toast, record the live
  `ToastManager* this` (an atomic, first-writer-wins), then forward to the
  original unchanged. This gives us a *verified* manager pointer captured from a
  real call, instead of guessing a fragile path through client singletons.
- **ABI note (FACT):** in the Itanium C++ ABI a non-trivial `ToastMessage&&`
  parameter is passed as a pointer, so the detour signature is
  `(void* self, void* toast)`.
- **Safety:** the detour does no allocation on the hot path beyond one atomic
  compare-exchange and always calls the original; it cannot change game
  behavior. Removed in `disable()`/`unload()` via `GlossHookDisable`.

### Sending a toast

`ToastBridge::push()` constructs a `ToastMessage` in an over-aligned 4 KiB local
buffer using the resolved constructor, hands it to the captured manager through
the **original** `pushToast`, then runs the resolved destructor.

- **INFERENCE:** 4 KiB safely exceeds the true `ToastMessage` size (its
  documented fields — two `std::string`s, a `Json::Value`, several `glm::vec2`,
  an icon vector and control-id strings — are well under 1 KiB). The constructor
  only writes within the real object, so the tail is untouched.
- **HYPOTHESIS (device-verify):** exact struct size and that `pushToast` moves
  (not copies-then-frees) out of the argument. The destructor call after the
  push covers the moved-from object either way.

## 2. `eglSwapBuffers` — render hook (`src/ui/ImGuiRenderer.cpp`)

- **Target:** `libEGL.so!eglSwapBuffers` (stable platform symbol).
- **Detour:** builds one ImGui frame (query surface size → `NewFrame` →
  `Overlay::draw()` → `Render` → `ImGui_ImplOpenGL3_RenderDrawData`) then calls
  the original swap.
- **Why here (FACT):** at swap time a valid GLES context is current and the
  game's frame is complete — the correct, version-independent place to overlay
  UI. This avoids hooking any Minecraft-internal renderer, so no RenderDragon /
  bgfx signature work is needed.
- **Lazy init:** the ImGui context and GLES3 backend are created on the first
  swapped frame, guaranteeing a live GL context.

## Input hook

Touch input is **not** a code hook — it uses the preloader's supported
`PreloaderInput` touch callback (`RegisterTouchCallback`). The callback stores
the latest pointer state in atomics; the render hook feeds it into ImGui IO each
frame (thread-safe, no cross-thread ImGui event-queue mutation). Events are
consumed only while the overlay is visible, so game touch controls are
unaffected when it is closed.
