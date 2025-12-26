#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>

namespace VDM::Commands {

    const std::string CMD_DISPATCH_VDLIST_STR   = "vdlist";

     /**
     * Helper function to register a Hyprland hyprctl command
     * @param name Command name as shown in hyprctl
     * @param fn Handler function with signature: std::string(eHyprCtlOutputFormat, std::string)
     * @param exact Whether the command name must match exactly (true) or can have arguments (false)
     */
    inline void registerHyprCtlCommand(
        const char* name,
        std::function<std::string(eHyprCtlOutputFormat, std::string)> fn,
        bool exact = false);

    /**
     * Register all hyprctl commands for the VDM plugin
     * @param handle Plugin handle from PLUGIN_INIT
     */
    void registerAll(HANDLE handle);

    /**
     * Command handlers
     */
    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args);
}
