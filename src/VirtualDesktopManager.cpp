#include "VirtualDesktopManager.hpp"

#include "LoggerFacade.hpp"

#include <algorithm>
#include <format>

namespace VDM {

CVirtualDesktopManager& CVirtualDesktopManager::getInstance() {
    static CVirtualDesktopManager s_instance;
    return s_instance;
}

void CVirtualDesktopManager::initialize(int vdeskCount, int workspaceStride) {
    m_hypr = &CHyprlandUtils::get();

    const int defaultCount = 5;
    const int finalCount = vdeskCount > 0 ? vdeskCount : defaultCount;
    m_layout.setVirtualDesktopCount(finalCount);

    // Stride must exceed the desktop count or workspaces on adjacent monitors collide.
    // e.g. stride=1, 2 monitors: Mon[0]/VD2 and Mon[1]/VD1 both map to WS2.
    const int minStride = finalCount + 1;
    if (workspaceStride > 0 && workspaceStride < minStride) {
        AppLog::logWarn(std::format(
            "initialize: workspaceStride {} is too small for {} desktops (min {}); clamping",
            workspaceStride, finalCount, minStride));
    }
    m_workspaceStride = std::max(minStride, std::max(1, workspaceStride));

    m_initialized = m_hypr->isInitialized();
    syncFromHyprland();
}

void CVirtualDesktopManager::shutdown() {
    m_initialized = false;
    m_layout.setVirtualDesktopCount(0);
    m_hypr = nullptr;
}

void CVirtualDesktopManager::setVirtualDesktopCount(int count) {
    const int clamped = std::max(0, count);
    if (clamped >= m_workspaceStride) {
        AppLog::logWarn(std::format(
            "setVirtualDesktopCount: count {} >= stride {}; workspace IDs will collide across monitors",
            clamped, m_workspaceStride));
    }
    m_layout.setVirtualDesktopCount(clamped);
    syncFromHyprland();
}

std::vector<CHyprlandUtils::MonitorInfo> CVirtualDesktopManager::getSortedMonitors() const {
    if (!m_hypr)
        return {};

    auto monitors = m_hypr->getAllMonitors();
    std::sort(monitors.begin(), monitors.end(), [](const auto& a, const auto& b) {
        return a.name < b.name;
    });
    return monitors;
}

CHyprlandUtils::WorkspaceId CVirtualDesktopManager::computeWorkspaceIdFor(int desktopId, size_t monitorIndex) const {
    // Simple deterministic mapping: desktopId + monitorIndex*stride.
    // Example with stride=100:
    // - Monitor#0: VD 1.. => WS 1..
    // - Monitor#1: VD 1.. => WS 101..
    return static_cast<CHyprlandUtils::WorkspaceId>(desktopId) +
           static_cast<CHyprlandUtils::WorkspaceId>(monitorIndex) *
               static_cast<CHyprlandUtils::WorkspaceId>(m_workspaceStride);
}

bool CVirtualDesktopManager::syncFromHyprland() {
    if (!m_hypr)
        return false;

    const auto monitors = getSortedMonitors();
    if (monitors.empty()) {
        AppLog::logWarn("syncFromHyprland: no monitors available, skipping");
        return false;
    }

    AppLog::logInfo(std::format("syncFromHyprland: {} monitor(s), {} virtual desktop(s)",
                                monitors.size(), m_layout.getVirtualDesktopCount()));

    // Populate bindings for all desktops based on our deterministic mapping.
    // Also ensure that the target workspaces exist in Hyprland and live on the
    // correct monitor. Notifications are suppressed to avoid startup spam.
    for (auto& vd : m_layout.getVirtualDesktops()) {
        for (size_t i = 0; i < monitors.size(); ++i) {
            const auto wsId = computeWorkspaceIdFor(vd.getID(), i);
            vd.bindWorkspace(monitors[i].id, wsId);

            if (!m_hypr->workspaceExists(wsId)) {
                // Create upfront so switching desktops is instantaneous.
                // Name is best-effort and only meant for debug friendliness.
                // silent=true: suppress per-workspace notifications during batch init.
                const auto created = m_hypr->createWorkspace(
                    wsId, std::format("vd{}:{}", vd.getID(), monitors[i].name), /*silent=*/true);

                if (created == -1) {
                    AppLog::logWarn(std::format("syncFromHyprland: failed to create WS {} (VD {} on {})",
                                                wsId, vd.getID(), monitors[i].name));
                    continue;
                }

                AppLog::logTrace(std::format("syncFromHyprland: created WS {} for VD {} on {}",
                                             wsId, vd.getID(), monitors[i].name));

                // A freshly created workspace lands on the cursor/first monitor.
                // Move it to the monitor it actually belongs to.
                if (!m_hypr->moveWorkspaceToMonitor(wsId, monitors[i].name, /*silent=*/true)) {
                    AppLog::logWarn(std::format("syncFromHyprland: failed to place WS {} on {}",
                                                wsId, monitors[i].name));
                }
            }

            // Make sure VDM-managed workspaces are permanent (best-effort).
            (void)m_hypr->setWorkspacePersistent(wsId, true);
        }
    }

    // Mark active desktop (best-effort) from the active monitor + active workspace.
    const auto activeMonId = m_hypr->getActiveMonitorID();
    const auto activeWsId  = m_hypr->getActiveWorkspaceID();
    if (!activeMonId.has_value() || activeWsId < 0)
        return true;

    const auto it = std::find_if(monitors.begin(), monitors.end(),
                                 [&](const auto& m) { return m.id == *activeMonId; });
    if (it == monitors.end())
        return true;

    const auto activeIndex = static_cast<CHyprlandUtils::WorkspaceId>(
        std::distance(monitors.begin(), it));
    const CHyprlandUtils::WorkspaceId base     = activeIndex * m_workspaceStride;
    const CHyprlandUtils::WorkspaceId desktopId = activeWsId - base;

    if (desktopId >= 1 && desktopId <= static_cast<CHyprlandUtils::WorkspaceId>(m_layout.getVirtualDesktopCount())) {
        m_layout.setActiveVirtualDesktop(static_cast<int>(desktopId));
        AppLog::logTrace(std::format("syncFromHyprland: active desktop detected as {}", desktopId));
    }

    return true;
}

bool CVirtualDesktopManager::activateVirtualDesktop(int desktopId) {
    if (!m_hypr)
        return false;

    if (desktopId < 1 || desktopId > m_layout.getVirtualDesktopCount()) {
        m_hypr->notify(CHyprlandUtils::NotificationLevel::Warn,
                       std::format("Invalid virtual desktop id {}", desktopId));
        return false;
    }

    const auto monitors = getSortedMonitors();
    if (monitors.empty())
        return false;

    bool ok = true;
    for (size_t i = 0; i < monitors.size(); ++i) {
        const auto wsId = computeWorkspaceIdFor(desktopId, i);
        if (!m_hypr->workspaceExists(wsId)) {
            // Optional name to make debugging easier.
            (void)m_hypr->createWorkspace(wsId, std::format("vd{}:{}", desktopId, monitors[i].name));
        }

        (void)m_hypr->setWorkspacePersistent(wsId, true);

        ok = m_hypr->moveWorkspaceToMonitor(wsId, monitors[i].name) && ok;
        ok = m_hypr->switchToWorkspaceOnMonitor(wsId, monitors[i].name) && ok;
    }

    m_layout.setActiveVirtualDesktop(desktopId);
    return ok;
}

std::string CVirtualDesktopManager::toString(bool detailed) const {
    return detailed ? m_layout.toStringDetailed(false) : m_layout.toString(false);
}

} // namespace VDM