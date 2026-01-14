#include "HyprlandUtils.hpp"

/**
 * @file HyprlandUtils.cpp
 * @brief Implementation of the Hyprland façade.
 *
 * This file may include Hyprland internal headers and interact with internal
 * globals/types. Keep such usage confined here to reduce coupling in the rest
 * of the plugin.
 */

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Workspace.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/LayoutManager.hpp>

#include <algorithm>
#include <charconv>
#include <format>
#include <string>

namespace VDM {

namespace {

/**
 * @brief Builds the final notification message by applying an optional prefix.
 *
 * If the prefix ends with whitespace, it is concatenated as-is; otherwise a
 * single space is inserted between prefix and message.
 */

std::string withOptionalPrefix(std::string_view prefix, std::string_view message) {
    if (prefix.empty())
        return std::string(message);

    // If the prefix already ends with whitespace, keep it as-is.
    if (!prefix.empty()) {
        const char last = prefix.back();
        if (last == ' ' || last == '\t')
            return std::string(prefix) + std::string(message);
    }

    return std::string(prefix) + " " + std::string(message);
}

/**
 * @brief Attempts to parse a numeric monitor id from a selector.
 * @param selector String containing a numeric id.
 * @return Parsed id or std::nullopt.
 */
std::optional<CHyprlandUtils::MonitorId> tryParseMonitorId(std::string_view selector) {
    if (selector.empty())
        return std::nullopt;

    // from_chars does not accept leading whitespace; keep behavior strict.
    CHyprlandUtils::MonitorId value = 0;
    const auto* begin = selector.data();
    const auto* end = selector.data() + selector.size();

    const auto [ptr, ec] = std::from_chars(begin, end, value);
    if (ec != std::errc() || ptr != end)
        return std::nullopt;

    return value;
}

/**
 * @brief Resolves a monitor by selector.
 *
 * Selector can be either a monitor name or a numeric id.
 */
Hyprutils::Memory::CSharedPointer<CMonitor> getMonitorBySelector(std::string_view selector) {
    if (!g_pCompositor)
        return {};

    // Try by name first.
    if (auto monByName = g_pCompositor->getMonitorFromName(std::string(selector)); monByName)
        return monByName;

    // Then try as numeric id.
    if (const auto id = tryParseMonitorId(selector); id.has_value())
        return g_pCompositor->getMonitorFromID(*id);

    return {};
}

} // namespace

CHyprlandUtils& CHyprlandUtils::get() {
    static CHyprlandUtils instance;
    return instance;
}

void CHyprlandUtils::initialize(HANDLE handle) {
    m_hHandle = handle;
}

bool CHyprlandUtils::isInitialized() const {
    return m_hHandle != nullptr;
}

void CHyprlandUtils::setNotificationPrefix(std::string_view prefix) {
    m_notificationPrefix = std::string(prefix);
}

std::string_view CHyprlandUtils::getNotificationPrefix() const {
    return m_notificationPrefix;
}

void CHyprlandUtils::setNotificationColors(std::optional<CHyprColor> info,
                                           std::optional<CHyprColor> warn,
                                           std::optional<CHyprColor> error) {
    m_infoColor = info;
    m_warnColor = warn;
    m_errorColor = error;
}

void CHyprlandUtils::notify(std::string_view message, const CHyprColor& color, int durationMs) const {
    if (!m_hHandle)
        return;

    HyprlandAPI::addNotification(m_hHandle, withOptionalPrefix(m_notificationPrefix, message), color, durationMs);
}

void CHyprlandUtils::notify(NotificationLevel level, std::string_view message, int durationMs) const {
    if (!m_hHandle)
        return;

    const std::optional<CHyprColor>* color = nullptr;
    switch (level) {
        case NotificationLevel::Info:
            color = &m_infoColor;
            break;
        case NotificationLevel::Warn:
            color = &m_warnColor;
            break;
        case NotificationLevel::Error:
            color = &m_errorColor;
            break;
    }

    if (!color || !color->has_value())
        return;

    HyprlandAPI::addNotification(m_hHandle,
                                 withOptionalPrefix(m_notificationPrefix, message),
                                 **color,
                                 durationMs);
}

CHyprlandUtils::WorkspaceId CHyprlandUtils::createWorkspace(std::optional<WorkspaceId> id, std::string_view name) {
    if (!g_pCompositor || !m_hHandle)
        return -1;

    const WorkspaceId workspaceID = id.value_or(getNextAvailableWorkspaceID());
    if (workspaceExists(workspaceID)) {
        notify(NotificationLevel::Warn, std::format("Workspace {} already exists", workspaceID));
        return -1;
    }

    auto activeMonitor = g_pCompositor->getMonitorFromCursor();
    if (!activeMonitor) {
        // Fallback: first real monitor
        for (const auto& mon : g_pCompositor->m_realMonitors) {
            if (mon) {
                activeMonitor = mon;
                break;
            }
        }
    }

    if (!activeMonitor) {
        notify(NotificationLevel::Error, "No active monitor found");
        return -1;
    }

    auto pMonitor = g_pCompositor->getMonitorFromID(activeMonitor->m_id);
    const std::string wsName = name.empty() ? std::to_string(workspaceID) : std::string(name);

    auto workspace = CWorkspace::create(workspaceID, pMonitor, wsName);
    if (!workspace) {
        notify(NotificationLevel::Error, std::format("Failed to create workspace {}", workspaceID));
        return -1;
    }

    notify(NotificationLevel::Info, std::format("Created workspace {} ({})", workspaceID, wsName));
    return workspaceID;
}

bool CHyprlandUtils::deleteWorkspace(WorkspaceId id) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = g_pCompositor->getWorkspaceByID(id);
    if (!workspace) {
        notify(NotificationLevel::Warn, std::format("Workspace {} not found", id));
        return false;
    }

    const int windowCount = workspace->getWindows();
    if (windowCount > 0) {
        notify(NotificationLevel::Warn, std::format("Cannot delete workspace {} - contains {} windows", id, windowCount));
        return false;
    }

    for (const auto& monitor : g_pCompositor->m_realMonitors) {
        if (!monitor)
            continue;

        auto activeWS = monitor->m_activeWorkspace;
        if (activeWS && activeWS->m_id == id) {
            notify(NotificationLevel::Warn, std::format("Cannot delete active workspace {}", id));
            return false;
        }
    }

    // Hyprland handles actual cleanup when the workspace is no longer referenced.
    workspace.reset();

    notify(NotificationLevel::Info, std::format("Workspace {} marked for deletion", id));
    return true;
}

bool CHyprlandUtils::switchToWorkspace(WorkspaceId id) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = g_pCompositor->getWorkspaceByID(id);
    if (!workspace) {
        const auto newID = createWorkspace(id);
        if (newID == -1)
            return false;

        workspace = g_pCompositor->getWorkspaceByID(newID);
        if (!workspace)
            return false;
    }

    if (auto monitor = g_pCompositor->getMonitorFromCursor(); monitor) {
        monitor->changeWorkspace(id);
        return true;
    }

    return false;
}

bool CHyprlandUtils::moveWorkspaceToMonitor(WorkspaceId workspaceID, std::string_view monitorSelector) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = g_pCompositor->getWorkspaceByID(workspaceID);
    if (!workspace) {
        notify(NotificationLevel::Warn, std::format("Workspace {} not found", workspaceID));
        return false;
    }

    auto monitor = getMonitorBySelector(monitorSelector);
    if (!monitor) {
        notify(NotificationLevel::Warn, std::format("Monitor {} not found", monitorSelector));
        return false;
    }

    auto pMonitor = g_pCompositor->getMonitorFromID(monitor->m_id);
    g_pCompositor->moveWorkspaceToMonitor(workspace, pMonitor);

    notify(NotificationLevel::Info, std::format("Moved workspace {} to monitor {}", workspaceID, monitor->m_name));
    return true;
}

bool CHyprlandUtils::renameWorkspace(WorkspaceId id, std::string_view newName) {
    if (!g_pCompositor || !m_hHandle)
        return false;

    auto workspace = g_pCompositor->getWorkspaceByID(id);
    if (!workspace) {
        notify(NotificationLevel::Warn, std::format("Workspace {} not found", id));
        return false;
    }

    workspace->m_name = std::string(newName);

    notify(NotificationLevel::Info, std::format("Renamed workspace {} to '{}'", id, newName));
    return true;
}

std::vector<CHyprlandUtils::WorkspaceInfo> CHyprlandUtils::getAllWorkspaces() const {
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

        auto monitor = g_pCompositor->getMonitorFromID(workspace->monitorID());
        info.monitorName = monitor ? monitor->m_name : "unknown";

        info.windowCount = workspace->getWindows();

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
        workspaces.push_back(std::move(info));
    }

    return workspaces;
}

std::optional<CHyprlandUtils::WorkspaceInfo> CHyprlandUtils::getWorkspaceInfo(WorkspaceId id) const {
    if (!g_pCompositor)
        return std::nullopt;

    auto workspace = g_pCompositor->getWorkspaceByID(id);
    if (!workspace)
        return std::nullopt;

    WorkspaceInfo info;
    info.id = workspace->m_id;
    info.name = workspace->m_name;
    info.monitorID = workspace->monitorID();

    auto monitor = g_pCompositor->getMonitorFromID(workspace->monitorID());
    info.monitorName = monitor ? monitor->m_name : "unknown";

    info.windowCount = workspace->getWindows();

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

CHyprlandUtils::WorkspaceId CHyprlandUtils::getActiveWorkspaceID() const {
    if (!g_pCompositor)
        return -1;

    auto mon = g_pCompositor->getMonitorFromCursor();
    if (!mon) {
        for (const auto& m : g_pCompositor->m_realMonitors) {
            if (m) {
                mon = m;
                break;
            }
        }
    }

    if (!mon)
        return -1;

    auto workspace = mon->m_activeWorkspace;
    if (!workspace)
        return -1;

    return workspace->m_id;
}

std::vector<CHyprlandUtils::WorkspaceId> CHyprlandUtils::getWorkspacesOnMonitor(std::string_view monitorSelector) const {
    std::vector<WorkspaceId> workspaceIDs;

    auto monitor = getMonitorBySelector(monitorSelector);
    if (!monitor || !g_pCompositor)
        return workspaceIDs;

    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace && workspace->monitorID() == monitor->m_id) {
            workspaceIDs.push_back(workspace->m_id);
        }
    }

    return workspaceIDs;
}

std::vector<CHyprlandUtils::MonitorInfo> CHyprlandUtils::getAllMonitors() const {
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

        for (auto& workspace : g_pCompositor->getWorkspaces()) {
            if (workspace && workspace->monitorID() == monitor->m_id) {
                info.workspaces.push_back(workspace->m_id);
            }
        }

        monitors.push_back(std::move(info));
    }

    return monitors;
}

std::optional<CHyprlandUtils::MonitorInfo> CHyprlandUtils::getMonitorInfo(std::string_view monitorSelector) const {
    auto monitor = getMonitorBySelector(monitorSelector);
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

    if (g_pCompositor) {
        for (auto& workspace : g_pCompositor->getWorkspaces()) {
            if (workspace && workspace->monitorID() == monitor->m_id) {
                info.workspaces.push_back(workspace->m_id);
            }
        }
    }

    return info;
}

std::optional<CHyprlandUtils::MonitorId> CHyprlandUtils::getActiveMonitorID() const {
    if (!g_pCompositor)
        return std::nullopt;

    if (auto monitor = g_pCompositor->getMonitorFromCursor(); monitor)
        return monitor->m_id;

    for (const auto& mon : g_pCompositor->m_realMonitors) {
        if (mon)
            return mon->m_id;
    }

    return std::nullopt;
}

size_t CHyprlandUtils::getMonitorCount() const {
    if (!g_pCompositor)
        return 0;

    return g_pCompositor->m_realMonitors.size();
}

std::string CHyprlandUtils::getCurrentLayout() const {
    if (!g_pLayoutManager)
        return "unknown";

    auto layout = g_pLayoutManager->getCurrentLayout();
    return layout ? layout->getLayoutName() : "unknown";
}

std::vector<std::string> CHyprlandUtils::getAvailableLayouts() const {
    std::vector<std::string> layouts;

    if (!g_pLayoutManager)
        return layouts;

    auto currentLayout = g_pLayoutManager->getCurrentLayout();
    if (currentLayout) {
        layouts.push_back(currentLayout->getLayoutName());
    }

    if (std::find(layouts.begin(), layouts.end(), "dwindle") == layouts.end())
        layouts.push_back("dwindle");
    if (std::find(layouts.begin(), layouts.end(), "master") == layouts.end())
        layouts.push_back("master");

    return layouts;
}

CHyprlandUtils::LayoutInfo CHyprlandUtils::getLayoutInfo() const {
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

bool CHyprlandUtils::workspaceExists(WorkspaceId id) const {
    if (!g_pCompositor)
        return false;

    return g_pCompositor->getWorkspaceByID(id) != nullptr;
}

CHyprlandUtils::WorkspaceId CHyprlandUtils::getNextAvailableWorkspaceID() const {
    if (!g_pCompositor)
        return 1;

    WorkspaceId maxID = 0;
    for (auto& workspace : g_pCompositor->getWorkspaces()) {
        if (workspace && workspace->m_id > maxID) {
            maxID = workspace->m_id;
        }
    }

    return maxID + 1;
}

size_t CHyprlandUtils::getTotalWindowCount() const {
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
