#pragma once

/**
 * @file VirtualDesktopManager.hpp
 * @brief Orchestrator singleton for the VDM plugin.
 */

#include <cstddef>
#include <string>
#include <vector>

#include "HyprlandUtils.hpp"
#include "Layout.hpp"

namespace VDM {

/**
 * @brief Main plugin orchestrator.
 *
 * Responsibilities:
 * - Owns the current layout model (CLayout).
 * - Translates high-level actions (activate virtual desktop N) into Hyprland operations
 *   via the CHyprlandUtils façade.
 * - Keeps the model in sync with Hyprland state when requested.
 */
class CVirtualDesktopManager {
public:
    /** @brief Singleton access. */
    static CVirtualDesktopManager& getInstance();

    /**
     * @brief Initializes the manager.
     * @param vdeskCount Number of virtual desktops. If 0, a default is used.
     * @param workspaceStride Stride used to map (desktop, monitorIndex) -> workspaceId.
     */
    void initialize(int vdeskCount = 0, int workspaceStride = 100);

    /** @brief Whether initialize() has been called. */
    bool isInitialized() const { return m_initialized; }

    /** @brief Resets internal state. */
    void shutdown();

    /** @brief Returns the current layout model. */
    const CLayout& getLayout() const { return m_layout; }

    /** @brief Returns the number of managed virtual desktops. */
    int getVirtualDesktopCount() const { return m_layout.getVirtualDesktopCount(); }

    /**
     * @brief Updates the number of managed virtual desktops.
     *
     * This rebuilds the layout list and re-syncs bindings from Hyprland.
     */
    void setVirtualDesktopCount(int count);

    /**
     * @brief Rebuilds bindings in the layout from the current Hyprland state.
     */
    bool syncFromHyprland();

    /**
     * @brief Activates a virtual desktop (best-effort).
     *
     * On multi-monitor setups, maps the virtual desktop to a per-monitor workspace id
     * using the configured stride and switches each monitor to its target workspace.
     */
    bool activateVirtualDesktop(int desktopId);

    /**
     * @brief Debug representation of the current manager state.
     */
    std::string toString(bool detailed = false) const;

private:
    CVirtualDesktopManager() = default;
    ~CVirtualDesktopManager() = default;

    CVirtualDesktopManager(const CVirtualDesktopManager&) = delete;
    CVirtualDesktopManager& operator=(const CVirtualDesktopManager&) = delete;
    CVirtualDesktopManager(CVirtualDesktopManager&&) = delete;
    CVirtualDesktopManager& operator=(CVirtualDesktopManager&&) = delete;

    bool m_initialized = false;
    int m_workspaceStride = 100;
    CHyprlandUtils* m_hypr = nullptr;
    CLayout m_layout;

    std::vector<CHyprlandUtils::MonitorInfo> getSortedMonitors() const;
    CHyprlandUtils::WorkspaceId computeWorkspaceIdFor(int desktopId, size_t monitorIndex) const;
};

} // namespace VDM