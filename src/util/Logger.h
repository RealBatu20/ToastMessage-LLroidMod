#pragma once

#include <string_view>
#include <utility>

#include "pl/cpp/mod/NativeMod.hpp"

// Thin forwarding wrapper around the current NativeMod's logger so feature code
// can log without threading the mod instance everywhere. All calls are no-ops
// (rather than crashes) if invoked before the mod is loaded or after unload.
namespace toast_message::log {

template <typename... Args> void info(std::string_view fmt, Args &&...args) {
    if (auto self = pl::mod::NativeMod::current())
        self->getLogger().info(fmt, std::forward<Args>(args)...);
}

template <typename... Args> void warn(std::string_view fmt, Args &&...args) {
    if (auto self = pl::mod::NativeMod::current())
        self->getLogger().warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args> void error(std::string_view fmt, Args &&...args) {
    if (auto self = pl::mod::NativeMod::current())
        self->getLogger().error(fmt, std::forward<Args>(args)...);
}

template <typename... Args> void debug(std::string_view fmt, Args &&...args) {
    if (auto self = pl::mod::NativeMod::current())
        self->getLogger().debug(fmt, std::forward<Args>(args)...);
}

} // namespace toast_message::log
