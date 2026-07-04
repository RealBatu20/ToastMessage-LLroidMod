#pragma once

#include <atomic>
#include <string>

#include "toast/ToastTypes.h"

namespace toast_message {

// Parameters for one toast push request, gathered from the UI / config.
struct ToastRequest {
    ToastMessageType type = ToastMessageType::Message;
    std::string title;
    std::string subtitle;

    // Only used when the selected type routes through the extended
    // (player-messaging) constructor.
    std::string messageId;
    bool isSilent = false;
    std::string link;
    int buttonActionType = 0; // PlayerMessaging::ButtonActionType::None
};

// Bridges our mod to the game's toast system.
//
// Strategy (FACT for the API shape, INFERENCE for exact struct size):
//   * We resolve the ToastMessage constructors/destructor and
//     ToastManager::pushToast by symbol name.
//   * We hook pushToast so the first time the game itself shows any toast we
//     capture the live ToastManager `this` pointer. This avoids guessing a
//     brittle instance path through the client singletons.
//   * To show our own toast we construct a ToastMessage in a local buffer with
//     the resolved constructor and hand it to the captured manager via the
//     original pushToast.
class ToastBridge {
  public:
    static ToastBridge &instance();

    // Resolve symbols and install the capture hook. Safe to call once during
    // enable(). Returns false if the required symbols are unavailable.
    bool init();

    // Remove the capture hook. Called from disable()/unload().
    void shutdown();

    [[nodiscard]] bool isReady() const { return mReady.load(); }

    // True once a live ToastManager pointer has been captured and a toast can
    // actually be pushed.
    [[nodiscard]] bool hasManager() const { return mManager.load() != nullptr; }

    // Construct and push a toast. Returns false if not ready or no manager has
    // been captured yet.
    bool push(const ToastRequest &request);

    // Records the live ToastManager `this` captured from the game's own toast
    // activity. Called from the pushToast detour; the first non-null capture
    // wins.
    void captureFromGame(void *self);

  private:
    ToastBridge() = default;

    std::atomic<bool> mReady{false};
    std::atomic<void *> mManager{nullptr};
};

} // namespace toast_message
