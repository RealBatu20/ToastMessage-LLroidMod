#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace toast_message {

// Mirror of Minecraft Bedrock's ToastMessageType enum as documented for the
// LeviLamina 1.26.x class reference. The integer values are part of the game's
// ABI: they are used directly when constructing a ToastMessage, so they must
// match the engine. Reference: LeviLamina ToastMessage class definition.
enum class ToastMessageType : int {
    Unknown                     = 0,
    Achievement                 = 1,
    Invite                      = 2,
    RawInvite                   = 3,
    FocusLost                   = 4,
    ImportResourcePack          = 5,
    ImportExportWorld           = 6,
    GlobalResourceCrashRecovery = 7,
    Message                     = 8,
    PlayerMessaging             = 9,
    DialogMessage               = 10,
    ChatScreenNotification      = 11,
    Debug                       = 12,
    RecipeUnlocked              = 13,
    Snackbar                    = 14,
    ProgressPopup               = 15,
    PartyInvite                 = 16,
    PartyTravelOptOut           = 17,
    PartyTravelOptIn            = 18,
    ReturnToGame                = 19,
};

struct ToastTypeInfo {
    ToastMessageType type;
    std::string_view id;    // stable identifier stored in config
    std::string_view label; // human readable name shown in the dropdown
};

// Ordered list used to populate the ImGui dropdown and to persist the selected
// type by stable string id (so config survives enum reordering).
inline constexpr std::array<ToastTypeInfo, 20> kToastTypes = {{
    {ToastMessageType::Unknown, "unknown", "Unknown"},
    {ToastMessageType::Achievement, "achievement", "Achievement"},
    {ToastMessageType::Invite, "invite", "Invite"},
    {ToastMessageType::RawInvite, "raw_invite", "Raw Invite"},
    {ToastMessageType::FocusLost, "focus_lost", "Focus Lost"},
    {ToastMessageType::ImportResourcePack, "import_resource_pack", "Import Resource Pack"},
    {ToastMessageType::ImportExportWorld, "import_export_world", "Import/Export World"},
    {ToastMessageType::GlobalResourceCrashRecovery, "crash_recovery", "Crash Recovery"},
    {ToastMessageType::Message, "message", "Message"},
    {ToastMessageType::PlayerMessaging, "player_messaging", "Player Messaging"},
    {ToastMessageType::DialogMessage, "dialog_message", "Dialog Message"},
    {ToastMessageType::ChatScreenNotification, "chat_notification", "Chat Notification"},
    {ToastMessageType::Debug, "debug", "Debug"},
    {ToastMessageType::RecipeUnlocked, "recipe_unlocked", "Recipe Unlocked"},
    {ToastMessageType::Snackbar, "snackbar", "Snackbar"},
    {ToastMessageType::ProgressPopup, "progress_popup", "Progress Popup"},
    {ToastMessageType::PartyInvite, "party_invite", "Party Invite"},
    {ToastMessageType::PartyTravelOptOut, "party_travel_opt_out", "Party Travel Opt-Out"},
    {ToastMessageType::PartyTravelOptIn, "party_travel_opt_in", "Party Travel Opt-In"},
    {ToastMessageType::ReturnToGame, "return_to_game", "Return To Game"},
}};

inline const ToastTypeInfo &toastTypeInfoByIndex(std::size_t index) {
    if (index >= kToastTypes.size())
        index = 0;
    return kToastTypes[index];
}

inline std::size_t toastTypeIndexById(std::string_view id) {
    for (std::size_t i = 0; i < kToastTypes.size(); ++i) {
        if (kToastTypes[i].id == id)
            return i;
    }
    return 0;
}

// A "player messaging" style toast additionally carries a message id, silent
// flag, link and button action. These are only meaningful for the extended
// constructor, so the UI reveals them only when that type is chosen.
inline constexpr bool typeUsesButtonAction(ToastMessageType type) {
    return type == ToastMessageType::PlayerMessaging || type == ToastMessageType::Invite ||
           type == ToastMessageType::PartyInvite;
}

} // namespace toast_message
