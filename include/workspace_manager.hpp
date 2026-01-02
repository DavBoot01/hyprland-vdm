#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Workspace.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace VDM {

/**
 * @brief Workspace information structure
 */
struct WorkspaceInfo {
    WORKSPACEID id;
    std::string name;
    std::string monitorName;
    MONITORID monitorID;
    int32_t windowCount;
    bool isActive;
    bool hasFullscreen;
};

/**
 * @brief Monitor information structure
 */
struct MonitorInfo {
    MONITORID id;
    std::string name;
    std::string description;
    int32_t width;
    int32_t height;
    float refreshRate;
    int32_t x;
    int32_t y;
    WORKSPACEID activeWorkspaceID;
    std::string activeWorkspaceName;
    std::vector<WORKSPACEID> workspaces;
};

/**
 * @brief Layout information structure
 */
struct LayoutInfo {
    std::string name;
    std::string description;
};

/**
 * @brief Singleton class to manage Hyprland workspaces, monitors, and layouts
 * 
 * This class provides a centralized interface for workspace management operations
 * including creation, deletion, workspace-to-monitor assignments, and querying
 * information about the current Hyprland configuration.
 */
class CWorkspaceManager {
private:
    // Singleton instance
    static CWorkspaceManager* m_pInstance;
    
    // Plugin handle for Hyprland API calls
    HANDLE m_hHandle;

    // Private constructor for singleton
    CWorkspaceManager();
    
    // Prevent copying
    CWorkspaceManager(const CWorkspaceManager&) = delete;
    CWorkspaceManager& operator=(const CWorkspaceManager&) = delete;

    /**
     * @brief Helper to get workspace by ID
     */
    PHLWORKSPACE getWorkspaceByID(WORKSPACEID id);

    /**
     * @brief Helper to get monitor by ID
     */
    CMonitor* getMonitorByID(MONITORID id);

    /**
     * @brief Helper to get monitor by name
     */
    CMonitor* getMonitorByName(const std::string& name);

public:
    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static CWorkspaceManager* getInstance();

    /**
     * @brief Destroy the singleton instance
     */
    static void destroy();

    /**
     * @brief Initialize the workspace manager with plugin handle
     * @param handle Plugin handle from PLUGIN_INIT
     */
    void initialize(HANDLE handle);

    // Workspace management operations
    
    /**
     * @brief Create a new workspace
     * @param id Workspace ID (optional, auto-generated if not provided)
     * @param name Workspace name (optional)
     * @return Workspace ID if successful, -1 on failure
     */
    WORKSPACEID createWorkspace(std::optional<WORKSPACEID> id = std::nullopt, 
                           const std::string& name = "");

    /**
     * @brief Delete a workspace by ID
     * @param id Workspace ID
     * @return true if successful, false otherwise
     */
    bool deleteWorkspace(WORKSPACEID id);

    /**
     * @brief Switch to a workspace
     * @param id Workspace ID
     * @return true if successful, false otherwise
     */
    bool switchToWorkspace(WORKSPACEID id);

    /**
     * @brief Move workspace to a specific monitor
     * @param workspaceID Workspace ID
     * @param monitorID Monitor ID or monitor name
     * @return true if successful, false otherwise
     */
    bool moveWorkspaceToMonitor(WORKSPACEID workspaceID, const std::string& monitorID);

    /**
     * @brief Rename a workspace
     * @param id Workspace ID
     * @param newName New name for the workspace
     * @return true if successful, false otherwise
     */
    bool renameWorkspace(WORKSPACEID id, const std::string& newName);

    // Query operations

    /**
     * @brief Get information about all workspaces
     * @return Vector of WorkspaceInfo structures
     */
    std::vector<WorkspaceInfo> getAllWorkspaces();

    /**
     * @brief Get information about a specific workspace
     * @param id Workspace ID
     * @return Optional WorkspaceInfo, nullopt if workspace not found
     */
    std::optional<WorkspaceInfo> getWorkspaceInfo(WORKSPACEID id);

    /**
     * @brief Get active workspace ID
     * @return Active workspace ID, or -1 if none
     */
    WORKSPACEID getActiveWorkspaceID();

    /**
     * @brief Get workspaces on a specific monitor
     * @param monitorID Monitor ID or name
     * @return Vector of workspace IDs
     */
    std::vector<WORKSPACEID> getWorkspacesOnMonitor(const std::string& monitorID);

    // Monitor operations

    /**
     * @brief Get information about all monitors
     * @return Vector of MonitorInfo structures
     */
    std::vector<MonitorInfo> getAllMonitors();

    /**
     * @brief Get information about a specific monitor
     * @param id Monitor ID or name
     * @return Optional MonitorInfo, nullopt if monitor not found
     */
    std::optional<MonitorInfo> getMonitorInfo(const std::string& id);

    /**
     * @brief Get the active monitor
     * @return Pointer to active monitor, nullptr if none
     */
    CMonitor* getActiveMonitor();

    /**
     * @brief Get monitor count
     * @return Number of monitors
     */
    size_t getMonitorCount();

    // Layout operations

    /**
     * @brief Get current layout name
     * @return Current layout name
     */
    std::string getCurrentLayout();

    /**
     * @brief Get all available layouts
     * @return Vector of layout names
     */
    std::vector<std::string> getAvailableLayouts();

    /**
     * @brief Get layout information
     * @return LayoutInfo structure
     */
    LayoutInfo getLayoutInfo();

    // Utility functions

    /**
     * @brief Check if a workspace exists
     * @param id Workspace ID
     * @return true if workspace exists, false otherwise
     */
    bool workspaceExists(WORKSPACEID id);

    /**
     * @brief Get next available workspace ID
     * @return Next available workspace ID
     */
    WORKSPACEID getNextAvailableWorkspaceID();

    /**
     * @brief Get total window count across all workspaces
     * @return Total number of windows
     */
    size_t getTotalWindowCount();
};

} // namespace VDM
