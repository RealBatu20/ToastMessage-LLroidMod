# Signature / Symbol Report

The mod resolves everything **by symbol name at runtime** (via `GlossSymbol`),
not by hardcoded offset, so it survives address changes between Minecraft
builds. Symbols are only re-derived if Mojang renames the functions.

## Derived mangled names (`src/signatures/Symbols.h`)

These Itanium C++ ABI names were produced by compiling the exact signatures from
the LeviLamina 1.26.x `ToastMessage` reference against an NDK-compatible libc++
(`std::__ndk1::string`) toolchain — the same string ABI `libminecraftpe.so`
uses. This is why the strings contain `NSt6__ndk1...`.

| Purpose | Demangled | Mangled |
|---------|-----------|---------|
| Push a toast | `ToastManager::pushToast(ToastMessage&&)` | `_ZN12ToastManager9pushToastEO12ToastMessage` |
| 3-arg ctor | `ToastMessage::ToastMessage(ToastMessageType, std::string const&, std::string const&)` | `_ZN12ToastMessageC1E16ToastMessageTypeRKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_` |
| 7-arg ctor | `ToastMessage::ToastMessage(ToastMessageType, std::string const&, std::string const&, std::string const&, bool, std::string const&, PlayerMessaging::ButtonActionType)` | `_ZN12ToastMessageC1E16...S9_S9_bS9_N15PlayerMessaging16ButtonActionTypeE` |
| Destructor | `ToastMessage::~ToastMessage()` | `_ZN12ToastMessageD1Ev` |

Classification:
- **FACT:** the mangling is a deterministic function of the published
  signatures (verified with `g++` + `c++filt`).
- **HYPOTHESIS (needs the target binary):** that `libminecraftpe.so` for a
  given build actually **exports** these symbols. Release builds are often
  stripped of many exports. If `GlossSymbol` returns 0, the log prints exactly
  which symbol was missing and the mod disables sending rather than crashing.

## Regenerating for a new / stripped build

1. Open `libminecraftpe.so` (arm64-v8a) in IDA Pro (with the IDA Pro MCP) or
   Ghidra.
2. In the Exports/Names view, search `ToastManager` / `ToastMessage`.
   - If present: copy the mangled name into `Symbols.h` and confirm with
     `c++filt`.
   - If stripped: locate `ToastManager::pushToast` by xrefs from achievement /
     invite code, derive a byte-pattern signature, and swap the resolver to
     `pl_resolve_signature(SIG, "libminecraftpe.so")` (see `pl/c/Signature.h`).
     Keep the signature isolated in `Symbols.h` and comment the regeneration
     steps inline.
3. Verify the constructor's argument order matches the version's reference; a
   changed enum or an added parameter changes the mangling.

## Non-Minecraft symbols

- `eglSwapBuffers`, `eglQuerySurface` — resolved from `libEGL.so`. These are
  **stable public Android platform symbols** (EGL 1.x ABI); no per-build
  regeneration is ever required. **FACT.**
