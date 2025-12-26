#include <hyprland/src/plugins/PluginAPI.hpp>
#include "globals.hpp"
#include "commands.hpp"


// Plugin initialization
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    VDM::Commands::registerAll(handle);
    
    HyprlandAPI::addNotification(PHANDLE, "[VDM] Plugin loaded", CHyprColor(0.2, 0.8, 0.2, 1.0), 3000);
    return {VDM::PLUGIN_NAME, VDM::PLUGIN_DESCRIPTION, VDM::PLUGIN_AUTHOR, VDM::PLUGIN_VERSION};
}

APICALL EXPORT void PLUGIN_EXIT() {
    HyprlandAPI::addNotification(PHANDLE, "[VDM] Plugin unloaded", CHyprColor(0.8, 0.2, 0.2, 1.0), 3000);
}
