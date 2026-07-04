#include "android/InputBridge.h"

#include <atomic>

#include "pl/c/PreloaderInput.h"

#include "ui/Overlay.h"
#include "util/Logger.h"

namespace toast_message::input {

namespace {

// Android MotionEvent action codes.
constexpr int kActionDown = 0;
constexpr int kActionUp = 1;
constexpr int kActionMove = 2;
constexpr int kActionPointerDown = 5;
constexpr int kActionPointerUp = 6;

std::atomic<float> gX{0.0f};
std::atomic<float> gY{0.0f};
std::atomic<bool> gDown{false};
bool gRegistered = false;

bool onTouch(int action, int /*pointerId*/, float x, float y) {
    gX.store(x);
    gY.store(y);

    switch (action) {
    case kActionDown:
    case kActionPointerDown:
        gDown.store(true);
        break;
    case kActionUp:
    case kActionPointerUp:
        gDown.store(false);
        break;
    case kActionMove:
    default:
        break;
    }

    // Consume the event only while the overlay is open, so game camera / touch
    // controls are unaffected when it is hidden.
    return ui::isVisible();
}

} // namespace

bool init() {
    PreloaderInput_Interface *api = GetPreloaderInput();
    if (!api || !api->RegisterTouchCallback) {
        log::warn("Preloader input interface unavailable; virtual keyboard touch "
                  "input will not work.");
        return false;
    }
    api->RegisterTouchCallback(&onTouch);
    gRegistered = true;
    log::info("Registered preloader touch callback");
    return true;
}

void shutdown() {
    // The preloader input interface exposes registration but not
    // de-registration; leave the callback installed and simply stop consuming
    // events (onTouch already gates on overlay visibility).
    gDown.store(false);
    gRegistered = false;
}

void latestPointer(float &x, float &y, bool &down) {
    x = gX.load();
    y = gY.load();
    down = gDown.load();
}

} // namespace toast_message::input
