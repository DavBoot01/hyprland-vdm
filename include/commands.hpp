#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprutils/string/VarList.hpp>
#include <array>

#include "HyprlandUtils.hpp"

namespace VDM::Commands {

    const std::string CMD_DISPATCH_VDINFO_STR    = "vdinfo";
    const std::string CMD_DISPATCH_VDMINFO_STR = "vdminfo";
    const std::string CMD_DISPATCH_VDLIST_STR   = "vdlist";
    const std::string CMD_DISPATCH_VDLAYOUT_STR = "vdlayout";
    const std::string CMD_DISPATCH_PRINTMONITORINFO_STR = "printmonitorinfo";

    std::string handleDbgPluginInfo(eHyprCtlOutputFormat format, std::string args);
    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args);
    std::string handleVirtualDesktopLayout(eHyprCtlOutputFormat format, std::string args);
    std::string handleVirtualDesktopInfo(eHyprCtlOutputFormat format, std::string args);
    std::string handleMonitorInfo(eHyprCtlOutputFormat format, std::string args);

    // Static array used as the command source (definitions)
    inline static const std::array<SHyprCtlCommand, 5> PLUGIN_COMMANDS = {{
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
        },
        {
            .name = CMD_DISPATCH_VDLAYOUT_STR,
            .exact = false,
            .fn = [](eHyprCtlOutputFormat f, std::string a){
                return handleVirtualDesktopLayout(f, a);
            }
        },
        {
            .name = CMD_DISPATCH_VDINFO_STR,
            .exact = false,
            .fn = [](eHyprCtlOutputFormat f, std::string a){
                return handleVirtualDesktopInfo(f, a);
            }
        },
        {
            .name = CMD_DISPATCH_PRINTMONITORINFO_STR,  
            .exact = false, 
            .fn = [](eHyprCtlOutputFormat f, std::string a){
                return handleMonitorInfo(f, a);
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
        HANDLE handle,
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

} // namespace VDM::Commands
