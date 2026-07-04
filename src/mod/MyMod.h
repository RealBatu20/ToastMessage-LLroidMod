#pragma once

#include <memory>

#include "pl/cpp/Config.hpp"

#include "mod/Config.h"

namespace pl::mod {
class NativeMod;
}

namespace toast_message {

class MyMod {
  public:
    static MyMod &getInstance();

    [[nodiscard]] pl::mod::NativeMod &getSelf() const;

    bool load();
    bool enable();
    bool disable();
    bool unload();

  private:
    // Owns the live config; the overlay edits value() in place and disable()
    // persists it.
    std::unique_ptr<pl::config::ConfigFile<ModConfig>> configFile;
    bool renderReady = false;
};

} // namespace toast_message
