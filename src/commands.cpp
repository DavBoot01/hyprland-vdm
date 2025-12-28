#include "globals.hpp"
#include "commands.hpp"
#include <string>
#include <format>

namespace VDM::Commands {

    Hyprutils::Memory::CSharedPointer<SHyprCtlCommand> registerHyprCtlCommand(
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
        return ptr;
    }

    std::string handleDbgPluginInfo(eHyprCtlOutputFormat format, std::string args) {
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            return std::format(R"({{"status": "ok", "plugin": "{}", "version": "{}", "author": "{}"}})", 
                                      PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR);
        }
        return std::format("{} v{} by {}\n", PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR);
    }

    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args) {
        // TODO: Implement virtual desktop management
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            return R"({"status": "ok", "message": "VDM command not yet implemented"})";
        }
         return "VDM: Not yet implemented\n";
    }

    void registerAll(HANDLE handle) {
        for (const auto& cmd : PLUGIN_COMMANDS) {
            // Register the command and store the returned shared pointer (SP)
            // This pointer is required for proper unregistration later
            auto pCmd = registerHyprCtlCommand(cmd.name.c_str(), cmd.fn, cmd.exact);
            m_vRegisteredCommands.push_back(pCmd);

            // registerHyprCtlCommand(cmd.name.c_str(), cmd.fn, cmd.exact);
        }

        HyprlandAPI::addNotification(PHANDLE, "[VDM] Commands registered", 
                                     CHyprColor(0.2, 0.8, 0.2, 1.0), 2000);
    }

    void unregisterAll(HANDLE handle) {
        for (auto& pCmd : m_vRegisteredCommands) {
            // Unregister using the stored SharedPointer as required by the v0.52.2 API
            // This ensures the socket is cleaned up and prevents memory leaks or crashes
            HyprlandAPI::unregisterHyprCtlCommand(PHANDLE, pCmd);
        }
        
        HyprlandAPI::addNotification(PHANDLE, "[VDM] Commands unregistered", 
                                     CHyprColor(0.8, 0.2, 0.2, 1.0), 2000);
    }

} // namespace VDM::Commands