#include <hyprland/src/plugins/PluginAPI.hpp>

#include "globals.hpp"
#include "HyprlandUtils.hpp"
#include "LoggerFacade.hpp"
#include "commands.hpp"
#include "VirtualDesktopManager.hpp"

// Plugin initialization

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    // Initialize Hyprland façade (generic utilities + notifications)
    auto& hypr = VDM::CHyprlandUtils::get();
    hypr.initialize(handle);
    hypr.setNotificationPrefix("[VDM]");
    hypr.setNotificationColors(
        CHyprColor(0.2, 0.8, 0.2, 1.0),
        CHyprColor(0.8, 0.5, 0.2, 1.0),
        CHyprColor(0.8, 0.2, 0.2, 1.0));

    AppLog::initLogging(
        false,                   // toStdout
        false,                   // colored
        "/tmp/hyprland-vdm.log", // filePath
        "VDM"                    // tag
    );
    AppLog::setMinLevel(AppLog::LogLevel::Trace);


    VDM::Commands::registerAll(handle);

    // Initialize the plugin orchestrator (layout + actions)
    VDM::CVirtualDesktopManager::getInstance().initialize();
    hypr.notify(VDM::CHyprlandUtils::NotificationLevel::Info, "Plugin loaded");
    return {VDM::PLUGIN_NAME, VDM::PLUGIN_DESCRIPTION, VDM::PLUGIN_AUTHOR, VDM::PLUGIN_VERSION};
}

APICALL EXPORT void PLUGIN_EXIT() {
    VDM::Commands::unregisterAll(PHANDLE);

    VDM::CVirtualDesktopManager::getInstance().shutdown();

    auto& hypr = VDM::CHyprlandUtils::get();
    hypr.notify(VDM::CHyprlandUtils::NotificationLevel::Error, "Plugin unloaded");
}
