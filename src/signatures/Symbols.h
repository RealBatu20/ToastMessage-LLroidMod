#pragma once

#include <cstdint>

namespace toast_message::symbols {

// Itanium C++ ABI mangled names for the Bedrock ToastMessage / ToastManager
// members we interact with. These were derived by compiling the exact
// signatures published in the LeviLamina 1.26.x ToastMessage reference against
// an NDK libc++ toolchain (the same std::__ndk1::string ABI the game uses).
//
// They are resolved at runtime with GlossSymbol(); if a future Minecraft build
// renames or inlines them, resolution fails gracefully and the mod logs the
// missing symbol instead of crashing. To regenerate for a new version, open
// libminecraftpe.so in IDA Pro / Ghidra, locate the ToastMessage constructor
// and ToastManager::pushToast, and copy the exported mangled name from the
// symbol table (Exports view). Demangle with `c++filt` to confirm the
// signature matches.

// ToastManager::pushToast(ToastMessage&&)
inline constexpr const char *kPushToast =
    "_ZN12ToastManager9pushToastEO12ToastMessage";

// ToastMessage::ToastMessage(ToastMessageType, std::string const&, std::string const&)
inline constexpr const char *kToastCtor3 =
    "_ZN12ToastMessageC1E16ToastMessageTypeRKNSt6__ndk112basic_stringIcNS1_"
    "11char_traitsIcEENS1_9allocatorIcEEEES9_";

// ToastMessage::ToastMessage(ToastMessageType, std::string const&, std::string const&,
//                            std::string const&, bool, std::string const&,
//                            PlayerMessaging::ButtonActionType)
inline constexpr const char *kToastCtor7 =
    "_ZN12ToastMessageC1E16ToastMessageTypeRKNSt6__ndk112basic_stringIcNS1_"
    "11char_traitsIcEENS1_9allocatorIcEEEES9_S9_bS9_N15PlayerMessaging16Button"
    "ActionTypeE";

// ToastMessage::~ToastMessage()
inline constexpr const char *kToastDtor = "_ZN12ToastMessageD1Ev";

inline constexpr const char *kMinecraftModule = "libminecraftpe.so";

} // namespace toast_message::symbols
