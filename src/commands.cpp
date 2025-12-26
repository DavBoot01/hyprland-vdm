#include "globals.hpp"
#include "commands.hpp"
#include <string>
#include <format>

namespace VDM::Commands {

    void registerHyprCtlCommand(
        const char* name,
        std::function<std::string(eHyprCtlOutputFormat, std::string)> fn,
        bool exact)
    {
        SHyprCtlCommand cmd;
        cmd.name = name;
        cmd.fn = fn;
        cmd.exact = exact;
        
        auto ptr = HyprlandAPI::registerHyprCtlCommand(PHANDLE, cmd);
        if (!ptr) {
            HyprlandAPI::addNotification(PHANDLE, 
                std::format("Failed to register hyprctl command: {}", name),
                CHyprColor(0.8, 0.2, 0.2, 1.0), 5000);
        }
    }

    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args) {
        // TODO: Implement virtual desktop management
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            return R"({"status": "ok", "message": "VDM command not yet implemented"})";
        }
         return "VDM: Not yet implemented\n";
    }

    void registerAll(HANDLE handle) {
        // Register virtual desktop list command
        registerHyprCtlCommand(CMD_DISPATCH_VDLIST_STR.c_str(), handleVirtualDesktopList, true);
        
        HyprlandAPI::addNotification(PHANDLE, "[VDM] Commands registered", 
                                     CHyprColor(0.2, 0.8, 0.2, 1.0), 2000);
    }

} // namespace VDM::Commands