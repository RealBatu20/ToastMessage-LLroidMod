#pragma once

#include <string>

#include "pl/cpp/Config.hpp"

namespace toast_message {

// Persisted configuration. Field names map 1:1 to config.json keys via
// boost::pfr reflection in the preloader Config system.
struct ModConfig {
    int version = 1;
    bool enabled = true;

    // Toast content.
    std::string toastType = "achievement"; // stable id from ToastTypes.h
    std::string title = "Toast Message";
    std::string subtitle = "Hello from LeviLaunchroid";

    // Extended (player-messaging) fields.
    std::string messageId = "toast_msg";
    bool silent = false;
    std::string link;
    int buttonActionType = 0;

    // UI appearance. uiScale scales the whole ImGui interface; fontScale scales
    // just the text. Both are applied live from the overlay sliders.
    float uiScale = 2.0f;
    float fontScale = 1.5f;
};

nlohmann::json makeDefaultConfigJson();
nlohmann::json makeConfigSchemaJson();

} // namespace toast_message

template <> struct pl::config::Schema<toast_message::ModConfig> {
    static constexpr std::string_view title = "Toast Message Config";
    static constexpr std::string_view description =
        "Push custom Bedrock toast notifications, editable in-game via ImGui.";

    static constexpr FieldSchema field(std::string_view name) {
        if (name == "version")
            return {.title = "Version", .readOnly = true};
        if (name == "enabled")
            return {.title = "Enabled", .description = "Master on/off switch for the mod."};
        if (name == "toastType")
            return {.title = "Toast Type", .description = "Stable id of the ToastMessageType."};
        if (name == "title")
            return {.title = "Title", .description = "Toast title line."};
        if (name == "subtitle")
            return {.title = "Subtitle", .description = "Toast subtitle line."};
        if (name == "messageId")
            return {.title = "Message Id",
                    .description = "Message id for player-messaging style toasts."};
        if (name == "silent")
            return {.title = "Silent", .description = "Suppress the toast sound."};
        if (name == "link")
            return {.title = "Link", .description = "Optional link for actionable toasts."};
        if (name == "buttonActionType")
            return {.title = "Button Action", .description = "ButtonActionType integer value."};
        if (name == "uiScale")
            return {.title = "UI Scale", .description = "Live ImGui interface scale."};
        if (name == "fontScale")
            return {.title = "Font Scale", .description = "Live ImGui font scale."};
        return {};
    }
};
