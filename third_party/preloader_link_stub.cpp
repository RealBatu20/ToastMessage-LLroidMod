// Link-time-only stub for the LeviLauncher preloader runtime's core GlossHook
// API.
//
// GlossInit/GlossOpen/GlossSymbol/GlossHook/GlossHookDisable are foundational
// to every LeviLauncher build and are implemented by the launcher's
// libpreloader.so, which is already loaded (with soname "libpreloader.so")
// in the host process before any mod .so is dlopen'd. Android's dynamic
// linker only resolves a dlopen'd library's undefined symbols against
// libraries listed in its DT_NEEDED table (unlike glibc, it does not fall
// back to an implicit flat/global symbol search), so simply leaving these
// symbols undefined and stripping -Wl,--no-undefined does NOT work at
// runtime: it satisfies the static linker but produces a mod .so with no
// DT_NEEDED entry for libpreloader.so, which fails at dlopen with
// "cannot locate symbol" (java.lang.UnsatisfiedLinkError).
//
// This translation unit is built into a small SHARED library whose
// OUTPUT_NAME/soname is set to "preloader" (i.e. libpreloader.so), and the
// mod links against it PRIVATE for symbol resolution only. This stub .so is
// never packaged into the .levipack; it exists purely so the static linker
// records a DT_NEEDED="libpreloader.so" entry with correctly-typed symbols
// in the mod's shared object. At runtime, Android's dynamic linker resolves
// that DT_NEEDED entry against the already-loaded, identically-named
// libpreloader.so supplied by the launcher (matched by soname, not by
// filesystem path), so calls are dispatched to the real implementation, and
// the bodies defined here are never executed.
//
// GetPreloaderModMenu and GetPreloaderInput are deliberately NOT declared
// here. Those belong to separate, newer preloader subsystems (mod-menu UI,
// touch/key input bridging) that are not guaranteed to be present in every
// installed preloader-android build - hard-linking either one crashed
// dlopen on a user's device that lacked the mod-menu API (confirmed via
// logcat: "cannot locate symbol GetPreloaderModMenu"). They are resolved at
// runtime via dlsym instead; see util/PreloaderOptional.h.
#include "pl/Gloss.h"

extern "C" {

GLOSS_API void GlossInit(bool /*is_init_linker*/) {}

GLOSS_API GHandle GlossOpen(const char* /*lib_name*/) { return nullptr; }

GLOSS_API uintptr_t GlossSymbol(GHandle /*handle*/, const char* /*symbol*/, size_t* /*sym_size*/) {
    return 0;
}

GLOSS_API GHook GlossHook(void* /*sym_addr*/, void* /*new_func*/, void** /*old_func*/) {
    return nullptr;
}

GLOSS_API void GlossHookDisable(GHook /*hook*/) {}

} // extern "C"
