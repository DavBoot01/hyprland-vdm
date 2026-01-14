#pragma once

/**
 * @file HyprlandUtils.hpp
 * @brief Hyprland façade used by the plugin.
 *
 * This header is intentionally small and stable.
 * All direct interactions with Hyprland internals (e.g. compositor globals,
 * internal structs/classes) should live in the corresponding implementation
 * file, so the rest of the plugin stays insulated from Hyprland internal API
 * changes.
 */

#include <hyprland/src/plugins/PluginAPI.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace VDM {

/**
 * @brief Hyprland façade.
 *
 * Goal: keep all direct interactions with Hyprland internals (e.g. compositor
 * globals, internal types) inside the .cpp.
 *
 * The rest of the plugin should only depend on this header and on stable
 * PluginAPI surface.
 *
 * @note This type is a process-wide singleton accessed via get().
 * @note Intended usage is from Hyprland's main thread.
 */
class CHyprlandUtils final {
public:
    using WorkspaceId = int64_t;
    using MonitorId = int64_t;

    /**
     * @brief Workspace snapshot information.
     */
    struct WorkspaceInfo {
        WorkspaceId id;
        std::string name;
        std::string monitorName;
        MonitorId monitorID;
        int32_t windowCount;
        bool isActive;
        bool hasFullscreen;
    };

    /**
     * @brief Monitor snapshot information.
     */
    struct MonitorInfo {
        MonitorId id;
        std::string name;
        std::string description;
        int32_t width;
        int32_t height;
        float refreshRate;
        int32_t x;
        int32_t y;
        WorkspaceId activeWorkspaceID;
        std::string activeWorkspaceName;
        std::vector<WorkspaceId> workspaces;
    };

    /**
     * @brief Layout information.
     */
    struct LayoutInfo {
        std::string name;
        std::string description;
    };

    /**
     * @brief Notification severity level.
     */
    enum class NotificationLevel {
        Info,
        Warn,
        Error,
    };

    /**
     * @brief Returns the singleton instance.
     */
    static CHyprlandUtils& get();

    /**
     * @brief Initializes the façade with the plugin handle.
     * @param handle Handle provided by Hyprland in PLUGIN_INIT.
     */
    void initialize(HANDLE handle);

    /**
     * @brief Whether initialize() has been called with a valid handle.
     */
    bool isInitialized() const;

    // Notifications
    // - Prefix and level colors are configurable to keep this façade generic.
    // - If a level color is not configured, notify(level, ...) will be a no-op.
    /**
     * @brief Sets the notification prefix/tag.
     *
     * Example: "[VDM]".
     *
     * @note The prefix is prepended to notifications emitted via notify(). If the
     * prefix is empty, the message is emitted as-is.
     */
    void setNotificationPrefix(std::string_view prefix);

    /**
     * @brief Returns the current notification prefix/tag.
     */
    std::string_view getNotificationPrefix() const;

    /**
     * @brief Configures the colors used by notify(NotificationLevel, ...).
     *
     * @note If a level color is not configured (std::nullopt), notifications for
     * that level are suppressed (no-op).
     */
    void setNotificationColors(std::optional<CHyprColor> info,
                               std::optional<CHyprColor> warn,
                               std::optional<CHyprColor> error);

    /**
     * @brief Emits a notification with an explicit color.
     * @param message Notification message.
     * @param color Notification color.
     * @param durationMs Duration in milliseconds.
     */
    void notify(std::string_view message, const CHyprColor& color, int durationMs = 3000) const;

    /**
     * @brief Emits a notification with a configured per-level color.
     *
     * If the color for the requested level is not configured, this is a no-op.
     *
     * @param level Notification level.
     * @param message Notification message.
     * @param durationMs Duration in milliseconds.
     */
    void notify(NotificationLevel level, std::string_view message, int durationMs = 3000) const;

    // Workspace operations
    /**
     * @brief Creates a workspace.
     * @param id Optional explicit workspace id. If not provided, the next available id is used.
     * @param name Optional name. If empty, a default name is used.
     * @return The created workspace id on success, or -1 on failure.
     */
    WorkspaceId createWorkspace(std::optional<WorkspaceId> id = std::nullopt, std::string_view name = "");

    /**
     * @brief Deletes a workspace.
     *
     * @note Deletion may be refused if the workspace contains windows or is active.
     */
    bool deleteWorkspace(WorkspaceId id);

    /**
     * @brief Switches to a workspace. If it doesn't exist, it may be created.
     */
    bool switchToWorkspace(WorkspaceId id);

    /**
     * @brief Moves a workspace to a monitor.
     * @param workspaceID Workspace id.
     * @param monitorSelector Monitor selector: either monitor name or numeric id string.
     */
    bool moveWorkspaceToMonitor(WorkspaceId workspaceID, std::string_view monitorSelector);

    /**
     * @brief Renames a workspace.
     */
    bool renameWorkspace(WorkspaceId id, std::string_view newName);

    // Query operations
    /**
     * @brief Returns information about all workspaces.
     */
    std::vector<WorkspaceInfo> getAllWorkspaces() const;

    /**
     * @brief Returns information about a workspace.
     */
    std::optional<WorkspaceInfo> getWorkspaceInfo(WorkspaceId id) const;

    /**
     * @brief Returns the active workspace id for the current/active monitor.
     * @return Active workspace id or -1 if not available.
     */
    WorkspaceId getActiveWorkspaceID() const;

    /**
     * @brief Lists workspaces on a monitor.
     * @param monitorSelector Monitor selector: either monitor name or numeric id string.
     */
    std::vector<WorkspaceId> getWorkspacesOnMonitor(std::string_view monitorSelector) const;

    // Monitor operations
    /**
     * @brief Returns information about all monitors.
     */
    std::vector<MonitorInfo> getAllMonitors() const;

    /**
     * @brief Returns information about one monitor.
     * @param monitorSelector Monitor selector: either monitor name or numeric id string.
     */
    std::optional<MonitorInfo> getMonitorInfo(std::string_view monitorSelector) const;

    /**
     * @brief Returns the active monitor id.
     */
    std::optional<MonitorId> getActiveMonitorID() const;

    /**
     * @brief Returns the number of monitors.
     */
    size_t getMonitorCount() const;

    // Layout operations
    /**
     * @brief Returns current layout name.
     */
    std::string getCurrentLayout() const;

    /**
     * @brief Returns a list of available layouts.
     */
    std::vector<std::string> getAvailableLayouts() const;

    /**
     * @brief Returns information about the current layout.
     */
    LayoutInfo getLayoutInfo() const;

    // Utility
    /**
     * @brief Checks whether a workspace exists.
     */
    bool workspaceExists(WorkspaceId id) const;

    /**
     * @brief Returns the next available workspace id.
     */
    WorkspaceId getNextAvailableWorkspaceID() const;

    /**
     * @brief Returns total number of windows across all workspaces.
     */
    size_t getTotalWindowCount() const;

private:
    CHyprlandUtils() = default;

    HANDLE m_hHandle = nullptr;

    std::string m_notificationPrefix;
    std::optional<CHyprColor> m_infoColor;
    std::optional<CHyprColor> m_warnColor;
    std::optional<CHyprColor> m_errorColor;

    // Prevent copying
    CHyprlandUtils(const CHyprlandUtils&) = delete;
    CHyprlandUtils& operator=(const CHyprlandUtils&) = delete;
};

} // namespace VDM
