#include "mod/MyMod.h"

#include <filesystem>

#include "pl/Gloss.h"
#include "pl/cpp/Mod.hpp"
#include "pl/cpp/ModMenu.hpp"

#include "android/InputBridge.h"
#include "toast/ToastBridge.h"
#include "ui/ImGuiRenderer.h"
#include "ui/Overlay.h"

namespace toast_message {

namespace {

// Mod-menu button callback: flip overlay visibility on click.
void onMenuButton(const char * /*buttonId*/, PLModMenu_ButtonEvent event, float /*value*/) {
    if (event == PL_BUTTON_EVENT_CLICK)
        ui::toggle();
}

} // namespace

MyMod &MyMod::getInstance() {
    static MyMod instance;
    return instance;
}

pl::mod::NativeMod &MyMod::getSelf() const { return *pl::mod::NativeMod::current(); }

bool MyMod::load() {
    auto &self = getSelf();
    self.getLogger().debug("Loading...");

    std::error_code ec;
    std::filesystem::create_directories(self.getDataDir(), ec);
    std::filesystem::create_directories(self.getConfigDir(), ec);

    configFile = std::make_unique<pl::config::ConfigFile<ModConfig>>();
    if (!configFile->load()) {
        self.getLogger().warn("Failed to load typed config; using defaults");
    }

    self.getLogger().info("Loaded {} from {}", self.getName(), self.getModDir().string());
    return true;
}

bool MyMod::enable() {
    auto &self = getSelf();
    self.getLogger().debug("Enabling...");

    if (!configFile) {
        self.getLogger().error("Config not initialised");
        return false;
    }
    if (!configFile->value().enabled) {
        self.getLogger().info("Mod disabled by config");
        return true;
    }

    // GlossInit is normally auto-run, but pre-initialise to be safe before we
    // install hooks from a lifecycle callback.
    GlossInit(true);

    ui::setConfig(&configFile->value());

    if (!ToastBridge::instance().init())
        self.getLogger().warn("ToastBridge init incomplete; see log above");

    input::init();
    renderReady = ui::initRenderer();
    if (!renderReady)
        self.getLogger().warn("Renderer not installed; ImGui overlay unavailable");

    // Register a mod-menu toggle button.
    const bool buttonRegistered = pl::modmenu::ButtonBuilder("toast_message_toggle", "Toast Message")
                                      .behavior(PL_BUTTON_CLICK)
                                      .onEvent(&onMenuButton)
                                      .registerButton();
    if (!buttonRegistered)
        self.getLogger().warn("Failed to register mod-menu toggle button");

    self.getLogger().info("Toast Message enabled");
    return true;
}

bool MyMod::disable() {
    auto &self = getSelf();
    self.getLogger().debug("Disabling...");

    ui::setVisible(false);
    ui::shutdownRenderer();
    input::shutdown();
    ToastBridge::instance().shutdown();

    if (configFile && !configFile->save())
        self.getLogger().warn("Failed to persist config on disable");

    renderReady = false;
    return true;
}

bool MyMod::unload() {
    getSelf().getLogger().debug("Unloading...");
    ui::setConfig(nullptr);
    configFile.reset();
    return true;
}

} // namespace toast_message
