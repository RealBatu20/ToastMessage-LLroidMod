#pragma once

#include <cstdint>

namespace toast_message::symbols {

// Itanium C++ ABI mangled names for the Bedrock ToastMessage / ToastManager
// members we interact with.
//
// STATUS: HYPOTHESIS, not yet confirmed correct for any specific shipped
// Minecraft build. These were derived by compiling the exact signatures
// published in the LeviLamina 1.26.x ToastMessage reference against an NDK
// libc++ toolchain (the same std::__ndk1::string ABI the game uses) - this
// reproduces the correct Itanium mangling IF the class layout/namespace the
// reference documents is exactly what a given libminecraftpe.so build
// contains, but that has not been verified against a real game binary (no
// binary or disassembler access was available while writing this). On-device
// logs have shown these exact names unresolved on at least one real 1.26.30+
// build ("Missing required ToastMessage symbols"), meaning either this
// specific Minecraft build mangles/names them differently, or (more likely
// for a release/stripped binary) these symbols are not exported at all and
// require signature/byte-pattern scanning (pl_resolve_signature(), declared in
// pl/c/Signature.h - also available as the C++ wrapper
// pl::signature::resolveSignature() in pl/cpp/Signature.hpp) instead of a
// symbol-table lookup - that also requires disassembler access
// to a real binary to derive and verify, which has not been done here either.
//
// They are resolved at runtime with GlossSymbol(); if unresolved, resolution
// fails gracefully and the mod logs the missing symbol instead of crashing.
// ToastBridge also accepts per-symbol overrides from config.json
// (ModConfig::pushToastSymbol / toastCtor3Symbol / toastCtor7Symbol /
// toastDtorSymbol - see mod/Config.h) so a corrected value can be supplied
// without a new mod build once verified. To regenerate/verify for a given
// Minecraft build, open its libminecraftpe.so in IDA Pro / Ghidra, locate the
// ToastMessage constructor and ToastManager::pushToast, and copy the exported
// mangled name from the symbol table (Exports view) - demangle with
// `c++filt` to confirm the signature matches. If the symbols are not present
// in the Exports view at all, symbol-name resolution cannot work regardless
// of the name used; a signature/AOB scan against the real binary is required
// instead.

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
