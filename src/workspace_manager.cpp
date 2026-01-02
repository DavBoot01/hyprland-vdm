#include "workspace_manager.hpp"
#include "globals.hpp"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/LayoutManager.hpp>
#include <algorithm>
#include <format>

namespace VDM {

// Initialize static member
CWorkspaceManager* CWorkspaceManager::m_pInstance = nullptr;

CWorkspaceManager::CWorkspaceManager() : m_hHandle(nullptr) {}

CWorkspaceManager* CWorkspaceManager::getInstance() {
    if (!m_pInstance) {
        m_pInstance = new CWorkspaceManager();
    }
    return m_pInstance;
}

void CWorkspaceManager::destroy() {
    if (m_pInstance) {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

void CWorkspaceManager::initialize(HANDLE handle) {
    m_hHandle = handle;
}

PHLWORKSPACE CWorkspaceManager::getWorkspaceByID(WORKSPACEID id) {
    if (!g_pCompositor)
        return nullptr;
    
    return g_pCompositor->getWorkspaceByID(id);
}

CMonitor* CWorkspaceManager::getMonitorByID(MONITORID id) {
    if (!g_pCompositor)
        return nullptr;
    
    auto monitor = g_pCompositor->getMonitorFromID(id);
    return monitor.get();
}

CMonitor* CWorkspaceManager::getMonitorByName(const std::string& name) {
    if (!g_pCompositor)
        return nullptr;
    
    auto monitor = g_pCompositor->getMonitorFromName(name);
    return monitor.get();
}

// Workspace management operations

WORKSPACEID CWorkspaceManager::createWorkspace(std::optional<WORKSPACEID> id, const std::string& name) {
    if (!g_pCompositor || !m_hHandle)
        return -1;

    WORKSPACEID workspaceID = id.value_or(getNextAvailableWorkspaceID());
    
    // Check if workspace already exists
    if (workspaceExists(workspaceID)) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Workspace {} already exists", workspaceID),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return -1;
    }

    // Create workspace using Hyprland's compositor
    auto monitor = getActiveMonitor();
    if (!monitor) {
        HyprlandAPI::addNotification(m_hHandle,
            "[VDM] No active monitor found",
            CHyprColor(0.8, 0.2, 0.2, 1.0), 3000);
        return -1;
    }

    auto pMonitor = g_pCompositor->getMonitorFromID(monitor->m_id);
    auto workspace = CWorkspace::create(workspaceID, pMonitor, name.empty() ? std::to_string(workspaceID) : name);
    if (!workspace) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Failed to create workspace {}", workspaceID),
            CHyprColor(0.8, 0.2, 0.2, 1.0), 3000);
        return -1;
    }

    HyprlandAPI::addNotification(m_hHandle,
        std::format("[VDM] Created workspace {} ({})", workspaceID, 
                   name.empty() ? std::to_string(workspaceID) : name),
        CHyprColor(0.2, 0.8, 0.2, 1.0), 3000);

    return workspaceID;
}

bool CWorkspaceManager::deleteWorkspace(WORKSPACEID id) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = getWorkspaceByID(id);
    if (!workspace) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Workspace {} not found", id),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return false;
    }

    // Don't delete if it has windows
    int windowCount = workspace->getWindows();
    if (windowCount > 0) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Cannot delete workspace {} - contains {} windows", 
                       id, windowCount),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return false;
    }

    // Don't delete if it's the active workspace on any monitor
    for (const auto& monitor : g_pCompositor->m_realMonitors) {
        if (!monitor)
            continue;
        auto activeWS = monitor->m_activeWorkspace;
        if (activeWS && activeWS->m_id == id) {
            HyprlandAPI::addNotification(m_hHandle,
                std::format("[VDM] Cannot delete active workspace {}", id),
                CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
            return false;
        }
    }

    // We can't directly erase from m_workspaces since it's private
    // Instead, Hyprland will cleanup when workspace is no longer referenced
    workspace.reset();
    
    HyprlandAPI::addNotification(m_hHandle,
        std::format("[VDM] Workspace {} marked for deletion", id),
        CHyprColor(0.2, 0.8, 0.2, 1.0), 3000);

    return true;
}

bool CWorkspaceManager::switchToWorkspace(WORKSPACEID id) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = getWorkspaceByID(id);
    if (!workspace) {
        // Workspace doesn't exist, create it first
        WORKSPACEID newID = createWorkspace(id);
        if (newID == -1)
            return false;
        workspace = getWorkspaceByID(newID);
        if (!workspace)
            return false;
    }

    // Get current monitor and switch workspace
    auto monitor = g_pCompositor->getMonitorFromCursor();
    if (monitor) {
        monitor->changeWorkspace(id);
        return true;
    }

    return false;
}

bool CWorkspaceManager::moveWorkspaceToMonitor(WORKSPACEID workspaceID, const std::string& monitorID) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = getWorkspaceByID(workspaceID);
    if (!workspace) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Workspace {} not found", workspaceID),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return false;
    }

    // Try to get monitor by name first, then by ID
    CMonitor* monitor = getMonitorByName(monitorID);
    if (!monitor) {
        try {
            MONITORID monID = std::stoll(monitorID);
            monitor = getMonitorByID(monID);
        } catch (...) {
            // Invalid monitor ID
        }
    }

    if (!monitor) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Monitor {} not found", monitorID),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return false;
    }

    // Move workspace to monitor
    auto pMonitor = g_pCompositor->getMonitorFromID(monitor->m_id);
    g_pCompositor->moveWorkspaceToMonitor(workspace, pMonitor);

    HyprlandAPI::addNotification(m_hHandle,
        std::format("[VDM] Moved workspace {} to monitor {}", workspaceID, monitor->m_name),
        CHyprColor(0.2, 0.8, 0.2, 1.0), 3000);

    return true;
}

bool CWorkspaceManager::renameWorkspace(WORKSPACEID id, const std::string& newName) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = getWorkspaceByID(id);
    if (!workspace) {
        HyprlandAPI::addNotification(m_hHandle,
            std::format("[VDM] Workspace {} not found", id),
            CHyprColor(0.8, 0.5, 0.2, 1.0), 3000);
        return false;
    }

    workspace->m_name = newName;

    HyprlandAPI::addNotification(m_hHandle,
        std::format("[VDM] Renamed workspace {} to '{}'", id, newName),
        CHyprColor(0.2, 0.8, 0.2, 1.0), 3000);

    return true;
}

// Query operations

std::vector<WorkspaceInfo> CWorkspaceManager::getAllWorkspaces() {
    std::vector<WorkspaceInfo> workspaces;
    
    if (!g_pCompositor)
        return workspaces;

    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (!workspace)
            continue;

        WorkspaceInfo info;
        info.id = workspace->m_id;
        info.name = workspace->m_name;
        info.monitorID = workspace->monitorID();
        
        auto monitor = getMonitorByID(workspace->monitorID());
        info.monitorName = monitor ? monitor->m_name : "unknown";
        
        info.windowCount = workspace->getWindows();
        
        // Check if workspace is active on any monitor
        info.isActive = false;
        for (const auto& mon : g_pCompositor->m_realMonitors) {
            if (!mon)
                continue;
            auto activeWS = mon->m_activeWorkspace;
            if (activeWS && activeWS->m_id == workspace->m_id) {
                info.isActive = true;
                break;
            }
        }
        
        info.hasFullscreen = workspace->m_hasFullscreenWindow;

        workspaces.push_back(info);
    }

    return workspaces;
}

std::optional<WorkspaceInfo> CWorkspaceManager::getWorkspaceInfo(WORKSPACEID id) {
    if (!g_pCompositor)
        return std::nullopt;

    auto workspace = getWorkspaceByID(id);
    if (!workspace)
        return std::nullopt;

    WorkspaceInfo info;
    info.id = workspace->m_id;
    info.name = workspace->m_name;
    info.monitorID = workspace->monitorID();
    
    auto monitor = getMonitorByID(workspace->monitorID());
    info.monitorName = monitor ? monitor->m_name : "unknown";
    
    info.windowCount = workspace->getWindows();
    
    // Check if workspace is active on any monitor
    info.isActive = false;
    for (const auto& mon : g_pCompositor->m_realMonitors) {
        if (!mon)
            continue;
        auto activeWS = mon->m_activeWorkspace;
        if (activeWS && activeWS->m_id == workspace->m_id) {
            info.isActive = true;
            break;
        }
    }
    
    info.hasFullscreen = workspace->m_hasFullscreenWindow;

    return info;
}

WORKSPACEID CWorkspaceManager::getActiveWorkspaceID() {
    if (!g_pCompositor)
        return -1;

    auto* monitor = getActiveMonitor();
    if (!monitor)
        return -1;

    auto workspace = monitor->m_activeWorkspace;
    if (!workspace)
        return -1;

    return workspace->m_id;
}

std::vector<WORKSPACEID> CWorkspaceManager::getWorkspacesOnMonitor(const std::string& monitorID) {
    std::vector<WORKSPACEID> workspaceIDs;
    
    if (!g_pCompositor)
        return workspaceIDs;

    // Try to get monitor by name first, then by ID
    CMonitor* monitor = getMonitorByName(monitorID);
    if (!monitor) {
        try {
            MONITORID monID = std::stoll(monitorID);
            monitor = getMonitorByID(monID);
        } catch (...) {
            return workspaceIDs;
        }
    }

    if (!monitor)
        return workspaceIDs;

    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace && workspace->monitorID() == monitor->m_id) {
            workspaceIDs.push_back(workspace->m_id);
        }
    }

    return workspaceIDs;
}

// Monitor operations

std::vector<MonitorInfo> CWorkspaceManager::getAllMonitors() {
    std::vector<MonitorInfo> monitors;
    
    if (!g_pCompositor)
        return monitors;

    for (const auto& monitor : g_pCompositor->m_realMonitors) {
        if (!monitor)
            continue;

        MonitorInfo info;
        info.id = monitor->m_id;
        info.name = monitor->m_name;
        info.description = monitor->m_description;
        info.width = monitor->m_size.x;
        info.height = monitor->m_size.y;
        info.refreshRate = monitor->m_refreshRate;
        info.x = monitor->m_position.x;
        info.y = monitor->m_position.y;
        
        auto activeWorkspace = monitor->m_activeWorkspace;
        if (activeWorkspace) {
            info.activeWorkspaceID = activeWorkspace->m_id;
            info.activeWorkspaceName = activeWorkspace->m_name;
        } else {
            info.activeWorkspaceID = -1;
            info.activeWorkspaceName = "";
        }

        // Get all workspaces on this monitor
        for (auto& workspace : g_pCompositor->getWorkspaces()) {
            if (workspace && workspace->monitorID() == monitor->m_id) {
                info.workspaces.push_back(workspace->m_id);
            }
        }

        monitors.push_back(info);
    }

    return monitors;
}

std::optional<MonitorInfo> CWorkspaceManager::getMonitorInfo(const std::string& id) {
    if (!g_pCompositor)
        return std::nullopt;

    // Try to get monitor by name first, then by ID
    CMonitor* monitor = getMonitorByName(id);
    if (!monitor) {
        try {
            MONITORID monID = std::stoll(id);
            monitor = getMonitorByID(monID);
        } catch (...) {
            return std::nullopt;
        }
    }

    if (!monitor)
        return std::nullopt;

    MonitorInfo info;
    info.id = monitor->m_id;
    info.name = monitor->m_name;
    info.description = monitor->m_description;
    info.width = monitor->m_size.x;
    info.height = monitor->m_size.y;
    info.refreshRate = monitor->m_refreshRate;
    info.x = monitor->m_position.x;
    info.y = monitor->m_position.y;
    
    auto activeWorkspace = monitor->m_activeWorkspace;
    if (activeWorkspace) {
        info.activeWorkspaceID = activeWorkspace->m_id;
        info.activeWorkspaceName = activeWorkspace->m_name;
    } else {
        info.activeWorkspaceID = -1;
        info.activeWorkspaceName = "";
    }

    // Get all workspaces on this monitor
    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace && workspace->monitorID() == monitor->m_id) {
            info.workspaces.push_back(workspace->m_id);
        }
    }

    return info;
}

CMonitor* CWorkspaceManager::getActiveMonitor() {
    if (!g_pCompositor)
        return nullptr;

    if (auto monitor = g_pCompositor->getMonitorFromCursor(); monitor)
        return monitor.get();

    for (const auto& mon : g_pCompositor->m_realMonitors) {
        if (mon)
            return mon.get();
    }

    return nullptr;
}

size_t CWorkspaceManager::getMonitorCount() {
    if (!g_pCompositor)
        return 0;

    return g_pCompositor->m_realMonitors.size();
}

// Layout operations

std::string CWorkspaceManager::getCurrentLayout() {
    if (!g_pLayoutManager)
        return "unknown";

    auto layout = g_pLayoutManager->getCurrentLayout();
    return layout ? layout->getLayoutName() : "unknown";
}

std::vector<std::string> CWorkspaceManager::getAvailableLayouts() {
    std::vector<std::string> layouts;
    
    if (!g_pLayoutManager)
        return layouts;

    // Since m_layouts is private, we can only get the current layout
    // or return common Hyprland layouts
    auto currentLayout = g_pLayoutManager->getCurrentLayout();
    if (currentLayout) {
        layouts.push_back(currentLayout->getLayoutName());
    }
    
    // Common Hyprland layouts
    if (std::find(layouts.begin(), layouts.end(), "dwindle") == layouts.end())
        layouts.push_back("dwindle");
    if (std::find(layouts.begin(), layouts.end(), "master") == layouts.end())
        layouts.push_back("master");

    return layouts;
}

LayoutInfo CWorkspaceManager::getLayoutInfo() {
    LayoutInfo info;
    
    if (!g_pLayoutManager) {
        info.name = "unknown";
        info.description = "Layout manager not available";
        return info;
    }

    auto layout = g_pLayoutManager->getCurrentLayout();
    if (layout) {
        info.name = layout->getLayoutName();
        info.description = std::format("Current layout: {}", info.name);
    } else {
        info.name = "unknown";
        info.description = "No layout active";
    }

    return info;
}

// Utility functions

bool CWorkspaceManager::workspaceExists(WORKSPACEID id) {
    return getWorkspaceByID(id) != nullptr;
}

WORKSPACEID CWorkspaceManager::getNextAvailableWorkspaceID() {
    if (!g_pCompositor)
        return 1;

    WORKSPACEID maxID = 0;
    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace && workspace->m_id > maxID) {
            maxID = workspace->m_id;
        }
    }

    return maxID + 1;
}

size_t CWorkspaceManager::getTotalWindowCount() {
    if (!g_pCompositor)
        return 0;

    size_t count = 0;
    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace) {
            count += workspace->getWindows();
        }
    }

    return count;
}

} // namespace VDM
