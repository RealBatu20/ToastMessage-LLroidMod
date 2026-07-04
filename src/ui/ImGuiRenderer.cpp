#include "ui/ImGuiRenderer.h"

#include <EGL/egl.h>
#include <atomic>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include "pl/Gloss.h"

#include "android/InputBridge.h"
#include "ui/Overlay.h"
#include "util/Logger.h"

namespace toast_message::ui {

namespace {

// We render by hooking eglSwapBuffers in libEGL.so. This is a stable, public
// platform symbol (part of the Android EGL ABI), NOT a version-specific
// Minecraft internal, so it needs no per-build signature regeneration. At swap
// time a valid GL context is current and the frame is complete, which is the
// correct place to overlay ImGui.
using EglSwapBuffersFn = EGLBoolean (*)(EGLDisplay dpy, EGLSurface surface);

EglSwapBuffersFn gOriginalSwap = nullptr;
GHook gSwapHook = nullptr;

std::atomic<bool> gInitialised{false};
bool gImguiReady = false;
std::chrono::steady_clock::time_point gLastFrame;

void ensureImGui(EGLDisplay dpy, EGLSurface surface) {
    if (gImguiReady)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // don't write imgui.ini on device
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // GLSL ES 3.0 shaders for the GLES3 backend.
    ImGui_ImplOpenGL3_Init("#version 300 es");

    (void)dpy;
    (void)surface;
    gLastFrame = std::chrono::steady_clock::now();
    gImguiReady = true;
    log::info("ImGui GLES3 backend initialised");
}

EGLBoolean eglSwapBuffersDetour(EGLDisplay dpy, EGLSurface surface) {
    ensureImGui(dpy, surface);

    ImGuiIO &io = ImGui::GetIO();

    // Display size from the current EGL surface.
    EGLint width = 0, height = 0;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);
    if (width > 0 && height > 0)
        io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));

    // Delta time.
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - gLastFrame).count();
    gLastFrame = now;
    io.DeltaTime = dt > 0.0f ? dt : 1.0f / 60.0f;

    // Feed the latest pointer state (set from the input thread) into ImGui.
    float px = 0.0f, py = 0.0f;
    bool down = false;
    input::latestPointer(px, py, down);
    io.AddMousePosEvent(px, py);
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, down);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    draw();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return gOriginalSwap ? gOriginalSwap(dpy, surface) : EGL_TRUE;
}

} // namespace

bool initRenderer() {
    if (gInitialised.load())
        return true;

    GHandle egl = GlossOpen("libEGL.so");
    if (!egl) {
        log::error("Could not open libEGL.so to hook eglSwapBuffers");
        return false;
    }
    auto target = reinterpret_cast<void *>(GlossSymbol(egl, "eglSwapBuffers", nullptr));
    if (!target) {
        log::error("eglSwapBuffers symbol not found in libEGL.so");
        return false;
    }

    gSwapHook = GlossHook(target, reinterpret_cast<void *>(&eglSwapBuffersDetour),
                          reinterpret_cast<void **>(&gOriginalSwap));
    if (!gSwapHook) {
        log::error("Failed to install eglSwapBuffers hook");
        return false;
    }

    gInitialised.store(true);
    log::info("Render hook installed on eglSwapBuffers");
    return true;
}

void shutdownRenderer() {
    if (gSwapHook) {
        GlossHookDisable(gSwapHook);
        gSwapHook = nullptr;
    }
    if (gImguiReady) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        gImguiReady = false;
    }
    gInitialised.store(false);
}

} // namespace toast_message::ui
