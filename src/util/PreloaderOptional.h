#pragma once

// Best-effort dynamic resolution of optional preloader runtime symbols.
//
// The GlossHook core symbols (GlossInit/Open/Symbol/Hook/HookDisable) are hard
// -linked via third_party/preloader_link_stub.cpp: they are foundational to
// every LeviLauncher build and always exported. GetPreloaderModMenu and
// GetPreloaderInput, however, belong to separate, newer subsystems
// (mod-menu UI, touch/key input bridging) that are not guaranteed to be
// present in every installed preloader-android build. Hard-linking a symbol
// that a given user's launcher build does not export makes Android's dynamic
// linker fail the ENTIRE mod at dlopen with "cannot locate symbol" -
// confirmed on-device via logcat for GetPreloaderModMenu after GlossInit was
// fixed in v0.1.1. A runtime guard inside the function body cannot help,
// because the failure happens during relocation processing before any of our
// code runs.
//
// Resolve such symbols with dlsym against the already-resident
// libpreloader.so instead of calling them directly, so a missing symbol
// degrades the one dependent feature (log a warning, skip it) rather than
// crashing the whole mod's load.
namespace toast_message::preloader_optional {

// Looks up `symbolName` in the launcher's already-loaded libpreloader.so.
// Returns nullptr (and logs a warning) if libpreloader.so is not resident in
// this process, or if it does not export the requested symbol. Never loads a
// new copy of libpreloader.so from disk.
void *resolveSymbol(const char *symbolName);

} // namespace toast_message::preloader_optional
