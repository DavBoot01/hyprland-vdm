#include "globals.hpp"
#include "commands.hpp"
#include "VirtualDesktopManager.hpp"


#include <sstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

//#include "JsonUtils.hpp"
#include <string>


#include "LoggerFacade.hpp"

using namespace Hyprutils::String;

namespace VDM::Commands {

    

    std::string handleDbgPluginInfo(eHyprCtlOutputFormat format, std::string args) {
        (void)args;
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            nlohmann::json j;
            j["status"] = "ok";
            j["plugin"] = PLUGIN_NAME;
            j["version"] = PLUGIN_VERSION;
            j["author"] = PLUGIN_AUTHOR;
            return j.dump(4);
        }
        std::stringstream ss;
        ss << PLUGIN_NAME << " v" << PLUGIN_VERSION << " by " << PLUGIN_AUTHOR << "\n";
        return ss.str();
    }

    std::string handleVirtualDesktopList(eHyprCtlOutputFormat format, std::string args) {
        (void)args;

        auto& mgr = VDM::CVirtualDesktopManager::getInstance();
        const std::string state = mgr.toString(true);

        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            nlohmann::json j;
            j["status"] = "ok";
            j["vdeskCount"] = mgr.getVirtualDesktopCount();
            j["layout"] = nlohmann::json::parse(mgr.getLayout().toStringDetailed(true));
            return j.dump(4);
        }

        return state + "\n";
    }

    std::string handleVirtualDesktopInfo(eHyprCtlOutputFormat format, std::string args) {
        (void)args;

        auto& mgr = VDM::CVirtualDesktopManager::getInstance();
        const int vdeskCount = mgr.getVirtualDesktopCount();

        CVarList vars(args, 0, 's');
        int vId = -1;
        if (vars.size() > 1) {
            try {
                vId = std::stoi(vars[1]);
            } catch (const std::exception&) {
                // Not a number, vId remains -1
            }
        }

        if (vId < 0 || vId > vdeskCount) {
            if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
                nlohmann::json j;
                j["status"] = "error";
                std::stringstream ss;
                ss << "Invalid or missing virtual desktop id: " << vId;
                j["message"] = ss.str();
                return j.dump(4);
            }
            std::stringstream ss;
            ss << "Error: Invalid or missing virtual desktop id: " << vId << "\n";
            return ss.str();
        }

        const auto* vd = mgr.getLayout().getVirtualDesktopById(vId);
        if (!vd) {
            if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
                nlohmann::json j;
                j["status"] = "error";
                j["message"] = "Virtual desktop not found";
                return j.dump(4);
            }
            return std::string("Error: Virtual desktop not found\n");
        }

        const std::string state = vd->toStringDetailed(format == eHyprCtlOutputFormat::FORMAT_JSON, true);
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            nlohmann::json j;
            j["status"] = "ok";
            j["vdeskId"] = vId;
            j["desktop"] = nlohmann::json::parse(state);
            return j.dump(4);
        }

        return state + "\n";
    }

    std::string handleVirtualDesktopLayout(eHyprCtlOutputFormat format, std::string args) {
        auto& mgr = VDM::CVirtualDesktopManager::getInstance();
        const bool wantJson = format == eHyprCtlOutputFormat::FORMAT_JSON;

        CVarList vars(args, 0, 's');
        bool all = false;
        for (const auto& v : vars) {
            if (v == "a" || v == "all") {
                all = true;
            }
        }

        const std::string layout = all ? mgr.getLayout().toStringDetailed(wantJson) : mgr.getLayout().toString(wantJson);

        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            nlohmann::json j;
            j["status"] = "ok";
            j["detailed"] = all;
            j["layout"] = nlohmann::json::parse(layout);
            return j.dump(4);
        }

        return layout + "\n";
    }

    std::string handleMonitorInfo(eHyprCtlOutputFormat format, const std::string args) {
        CVarList vars(args, 0, 's');
        std::string_view monitorId = "0";
        // if (vars.size() > 1) {
        //     try {
        //         monitorId = vars[1];
        //     } catch (const std::exception&) {
        //         // Not a number, monitorId remains -1
        //     }
        // }

        auto& hypr = CHyprlandUtils::get();
        if (hypr.isInitialized() == false) {
            return "Hyprland utils not initialized\n";
        }

        auto monitorOpt = hypr.getMonitorInfo(monitorId);
        if (!monitorOpt) {
            if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
                nlohmann::json j;
                j["status"] = "error";
                std::stringstream ss;
                ss << "Monitor " << monitorId << " not found";
                j["message"] = ss.str();
                return j.dump(4);
            }
            std::stringstream ss;
            ss << "Monitor " << monitorId << " not found\n";
            return ss.str();
        }

        auto monitor = *monitorOpt;
        if (format == eHyprCtlOutputFormat::FORMAT_JSON) {
            nlohmann::json j;
            j["status"] = "ok";
            j["id"] = monitor.id;
            j["name"] = monitor.name;
            j["description"] = monitor.description;
            j["width"] = monitor.width;
            j["height"] = monitor.height;
            j["refreshRate"] = monitor.refreshRate;
            j["x"] = monitor.x;
            j["y"] = monitor.y;
            j["activeWorkspaceID"] = monitor.activeWorkspaceID;
            j["activeWorkspaceName"] = monitor.activeWorkspaceName;
            j["workspaces"] = monitor.workspaces;
            return j.dump(4);
        } else {
            std::stringstream ss;
            ss << "Monitor ID: " << monitor.id << "\n";
            ss << "Name: " << monitor.name << "\n";
            ss << "Description: " << monitor.description << "\n";
            ss << "Resolution: " << monitor.width << "x" << monitor.height << "\n";
            ss << "Refresh Rate: " << monitor.refreshRate << " Hz\n";
            ss << "Position: (" << monitor.x << ", " << monitor.y << ")\n";
            ss << "Active Workspace ID: " << monitor.activeWorkspaceID << "\n";
            ss << "Active Workspace Name: " << monitor.activeWorkspaceName << "\n";
            ss << "Workspaces: ";
            for (size_t i = 0; i < monitor.workspaces.size(); ++i) {
                ss << monitor.workspaces[i];
                if (i < monitor.workspaces.size() - 1)
                    ss << ", ";
            }
            ss << "\n";
            return ss.str();
        }
    }

    Hyprutils::Memory::CSharedPointer<SHyprCtlCommand> registerHyprCtlCommand(
        HANDLE handle,
        const char* name,
        std::function<std::string(eHyprCtlOutputFormat, std::string)> fn,
        bool exact)
    {
        (void)handle;

        SHyprCtlCommand cmd;
        cmd.name = name;
        cmd.fn = fn;
        cmd.exact = exact;
        
        auto ptr = HyprlandAPI::registerHyprCtlCommand(handle, cmd);
        if (!ptr) {
            std::stringstream ss;
            ss << "Failed to register hyprctl command: " << name;
            HyprlandAPI::addNotification(handle, 
                ss.str(),
                CHyprColor(0.8, 0.2, 0.2, 1.0), 5000);
        }
        return ptr;
    }

    void registerAll(HANDLE handle) {
        for (const auto& cmd : PLUGIN_COMMANDS) {
            // Register the command and store the returned shared pointer (SP)
            // This pointer is required for proper unregistration later
            auto pCmd = registerHyprCtlCommand(handle, cmd.name.c_str(), cmd.fn, cmd.exact);
            if (pCmd)
                m_vRegisteredCommands.push_back(pCmd);

            // registerHyprCtlCommand(cmd.name.c_str(), cmd.fn, cmd.exact);
        }

        HyprlandAPI::addNotification(handle, "[VDM] Commands registered", 
                                     CHyprColor(0.2, 0.8, 0.2, 1.0), 2000);
    }

    void unregisterAll(HANDLE handle) {
        for (auto& pCmd : m_vRegisteredCommands) {
            // Unregister using the stored SharedPointer as required by the v0.52.2 API
            // This ensures the socket is cleaned up and prevents memory leaks or crashes
            HyprlandAPI::unregisterHyprCtlCommand(handle, pCmd);
        }

        m_vRegisteredCommands.clear();
        
        HyprlandAPI::addNotification(handle, "[VDM] Commands unregistered", 
                                     CHyprColor(0.8, 0.2, 0.2, 1.0), 2000);
    }

} // namespace VDM::Commands