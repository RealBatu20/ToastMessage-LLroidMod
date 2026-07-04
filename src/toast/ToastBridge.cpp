#include "toast/ToastBridge.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>

#include "pl/Gloss.h"
#include "signatures/Symbols.h"
#include "util/Logger.h"

namespace toast_message {

namespace {

// ---- Resolved game function pointers -----------------------------------

// ToastManager::pushToast(ToastMessage&&). In the Itanium ABI a non-trivial
// rvalue-reference parameter is passed as a pointer to the object.
using PushToastFn = void (*)(void *self, void *toast);

// ToastMessage(ToastMessageType, std::string const&, std::string const&)
using Ctor3Fn = void (*)(void *self, int type, const std::string *title,
                         const std::string *subtitle);

// ToastMessage(ToastMessageType, std::string const&, std::string const&,
//              std::string const&, bool, std::string const&,
//              PlayerMessaging::ButtonActionType)
using Ctor7Fn = void (*)(void *self, int type, const std::string *title,
                         const std::string *subtitle, const std::string *messageId,
                         bool isSilent, const std::string *link, int buttonAction);

// ToastMessage::~ToastMessage()
using DtorFn = void (*)(void *self);

PushToastFn gOriginalPushToast = nullptr;
PushToastFn gPushToast = nullptr;
Ctor3Fn gCtor3 = nullptr;
Ctor7Fn gCtor7 = nullptr;
DtorFn gDtor = nullptr;
GHook gPushHook = nullptr;

// The real ToastMessage is large (title/subtitle strings, a Json::Value
// property bag, several glm::vec2 positions, icon vector, control ids, ...).
// We do not know its exact size at compile time, so we over-allocate a
// generously sized, suitably aligned buffer and construct in place. The
// constructor only ever writes within the true object size, so the extra tail
// bytes are harmless. 4 KiB is far larger than any plausible layout of the
// documented fields.
constexpr std::size_t kToastBufferSize = 4096;

// Detour: capture the live ToastManager `this` pointer from the game's own
// toast activity, then forward to the original implementation unchanged.
void pushToastDetour(void *self, void *toast) {
    ToastBridge::instance().captureFromGame(self);
    if (gOriginalPushToast)
        gOriginalPushToast(self, toast);
}

} // namespace

ToastBridge &ToastBridge::instance() {
    static ToastBridge bridge;
    return bridge;
}

bool ToastBridge::init(const ModConfig &config) {
    if (mReady.load())
        return true;

    GHandle handle = GlossOpen(symbols::kMinecraftModule);
    if (!handle) {
        log::error("Could not open {} for symbol resolution", symbols::kMinecraftModule);
        return false;
    }

    // A non-empty config override replaces the built-in default, so a wrong
    // or Minecraft-version-specific mangled name can be corrected via
    // config.json without waiting for a new mod build.
    const char *pushToastSymbol =
        config.pushToastSymbol.empty() ? symbols::kPushToast : config.pushToastSymbol.c_str();
    const char *ctor3Symbol =
        config.toastCtor3Symbol.empty() ? symbols::kToastCtor3 : config.toastCtor3Symbol.c_str();
    const char *ctor7Symbol =
        config.toastCtor7Symbol.empty() ? symbols::kToastCtor7 : config.toastCtor7Symbol.c_str();
    const char *dtorSymbol =
        config.toastDtorSymbol.empty() ? symbols::kToastDtor : config.toastDtorSymbol.c_str();

    gPushToast = reinterpret_cast<PushToastFn>(GlossSymbol(handle, pushToastSymbol, nullptr));
    gCtor3 = reinterpret_cast<Ctor3Fn>(GlossSymbol(handle, ctor3Symbol, nullptr));
    gCtor7 = reinterpret_cast<Ctor7Fn>(GlossSymbol(handle, ctor7Symbol, nullptr));
    gDtor = reinterpret_cast<DtorFn>(GlossSymbol(handle, dtorSymbol, nullptr));

    if (!gPushToast || !gCtor3 || !gDtor) {
        log::error("Missing required ToastMessage symbols (push={}, ctor3={}, dtor={}). This "
                   "Minecraft build's real mangled names differ from the mod's built-in "
                   "defaults (which are unverified against your exact build). Extract the real "
                   "symbols from libminecraftpe.so with IDA Pro/Ghidra and set "
                   "pushToastSymbol/toastCtor3Symbol/toastDtorSymbol in config.json.",
                   reinterpret_cast<void *>(gPushToast), reinterpret_cast<void *>(gCtor3),
                   reinterpret_cast<void *>(gDtor));
        return false;
    }
    if (!gCtor7)
        log::warn("Extended ToastMessage constructor not found; player-messaging "
                  "style toasts will fall back to the basic constructor.");

    // Install the capture hook on pushToast.
    gPushHook = GlossHook(reinterpret_cast<void *>(gPushToast),
                          reinterpret_cast<void *>(&pushToastDetour),
                          reinterpret_cast<void **>(&gOriginalPushToast));
    if (!gPushHook) {
        log::warn("Failed to hook ToastManager::pushToast; toasts can only be "
                  "sent after the game pushes one on its own is unavailable.");
    }

    mReady.store(true);
    log::info("ToastBridge initialised (push={}, ctor3={}, ctor7={}, dtor={})",
              reinterpret_cast<void *>(gPushToast), reinterpret_cast<void *>(gCtor3),
              reinterpret_cast<void *>(gCtor7), reinterpret_cast<void *>(gDtor));
    return true;
}

void ToastBridge::shutdown() {
    if (gPushHook) {
        GlossHookDisable(gPushHook);
        gPushHook = nullptr;
    }
    mReady.store(false);
}

void ToastBridge::captureFromGame(void *self) {
    void *expected = nullptr;
    if (mManager.compare_exchange_strong(expected, self))
        log::info("Captured live ToastManager instance at {}", self);
}

bool ToastBridge::push(const ToastRequest &request) {
    if (!mReady.load()) {
        log::warn("push() called before ToastBridge is ready");
        return false;
    }
    void *manager = mManager.load();
    if (!manager) {
        log::warn("No ToastManager captured yet; join a world / trigger any game "
                  "toast once so the manager pointer can be captured.");
        return false;
    }

    // Over-aligned storage for the ToastMessage temporary.
    alignas(std::max_align_t) unsigned char buffer[kToastBufferSize];
    std::memset(buffer, 0, sizeof(buffer));
    void *toast = buffer;

    const std::string title = request.title;
    const std::string subtitle = request.subtitle;

    if (typeUsesButtonAction(request.type) && gCtor7) {
        const std::string messageId = request.messageId;
        const std::string link = request.link;
        gCtor7(toast, static_cast<int>(request.type), &title, &subtitle, &messageId,
               request.isSilent, &link, request.buttonActionType);
    } else {
        gCtor3(toast, static_cast<int>(request.type), &title, &subtitle);
    }

    // Hand the toast to the game. pushToast moves out of it; our buffer is left
    // in a valid moved-from state that we still destruct below.
    gPushToast(manager, toast);
    gDtor(toast);

    log::info("Pushed toast: type={} title=\"{}\" subtitle=\"{}\"",
              static_cast<int>(request.type), title, subtitle);
    return true;
}

} // namespace toast_message
