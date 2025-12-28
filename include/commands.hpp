#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <array>

namespace VDM::Commands {

    const std::string CMD_DISPATCH_VDMINFO_STR = "vdminfo";
    const std::string CMD_DISPATCH_VDLIST_STR   = "vdlist2";

    std::string handleDbgPluginInfo(eHyprCtlOutputFormat format, std::string args);
    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args);

    // Static array used as the command source (definitions)
    inline static const std::array<SHyprCtlCommand, 2> PLUGIN_COMMANDS = {{
        {
            .name = CMD_DISPATCH_VDMINFO_STR, 
            .exact = true, 
            .fn = [](eHyprCtlOutputFormat f, std::string a){ 
                return handleDbgPluginInfo(f, a); 
            }
        },
        {
            .name = CMD_DISPATCH_VDLIST_STR,  
            .exact = true, 
            .fn = [](eHyprCtlOutputFormat f, std::string a){ 
                return handleVirtualDesktopList(f, a); 
            }
        }
    }};

    // Vector to store the shared pointers returned by Hyprland upon registration
    // SP is an alias for Hyprutils::Memory::CSharedPointer
    inline std::vector<Hyprutils::Memory::CSharedPointer<SHyprCtlCommand>> m_vRegisteredCommands;

     /**
     * Helper function to register a Hyprland hyprctl command
     * @param name Command name as shown in hyprctl
     * @param fn Handler function with signature: std::string(eHyprCtlOutputFormat, std::string)
     * @param exact Whether the command name must match exactly (true) or can have arguments (false)
     */
    inline Hyprutils::Memory::CSharedPointer<SHyprCtlCommand> registerHyprCtlCommand(
        const char* name,
        std::function<std::string(eHyprCtlOutputFormat, std::string)> fn,
        bool exact = false);

    /**
     * Register all hyprctl commands for the VDM plugin
     * @param handle Plugin handle from PLUGIN_INIT
     */
    void registerAll(HANDLE handle);
    /*
     * Unregister all hyprctl commands for the VDM plugin
     * @param handle Plugin handle from PLUGIN_INIT
    */
    void unregisterAll(HANDLE handle);

    /**
     * Command handlers
     */
    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args);
}
