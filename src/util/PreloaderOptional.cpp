#include "util/PreloaderOptional.h"

#include <dlfcn.h>

#include "util/Logger.h"

namespace toast_message::preloader_optional {

void *resolveSymbol(const char *symbolName) {
    // RTLD_NOLOAD: only fetch a handle to libpreloader.so if it is already
    // loaded in this process; never load a fresh copy from disk. The
    // launcher is expected to have it resident before dlopen'ing any mod.
    void *handle = dlopen("libpreloader.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        log::warn("libpreloader.so is not resident; optional symbol '{}' unavailable", symbolName);
        return nullptr;
    }

    void *symbol = dlsym(handle, symbolName);
    if (!symbol)
        log::warn("libpreloader.so does not export optional symbol '{}' on this LeviLauncher build",
                  symbolName);

    // RTLD_NOLOAD only bumped the refcount of the already-resident library;
    // release it here. The symbol address stays valid regardless, since the
    // launcher keeps its own reference to libpreloader.so for the process
    // lifetime - this dlclose can't unload it out from under us.
    dlclose(handle);
    return symbol;
}

} // namespace toast_message::preloader_optional
