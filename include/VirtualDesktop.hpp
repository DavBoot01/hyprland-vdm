#pragma once

/**
 * @file VirtualDesktop.hpp
 * @brief Virtual desktop domain model.
 */

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace VDM {

struct VDesktopID {
    int id;
    std::string name;
};

class CVirtualDesktop {

public:
    /**
     * @brief Creates a virtual desktop abstraction.
     * @param id Virtual desktop id (1-based suggested).
     * @param name Optional display name.
     */
    CVirtualDesktop(int id, std::optional<std::string_view> name = std::nullopt);
    ~CVirtualDesktop();

    using WorkspaceId = int64_t;
    using MonitorId = int64_t;

    // Getters
    /** @brief Desktop name (stable owned string). */
    const std::string &getName() const { return m_name; }

    /** @brief Desktop id. */
    int getID() const { return m_id; }

    // Setters
    /** @brief Sets the desktop name. */
    void setName(std::string_view name) { m_name = std::string(name); }

    // State
    /** @brief Marks this virtual desktop as active/inactive (model state). */
    void setActive(bool active) { m_isActive = active; }

    /** @brief Whether the desktop is marked active (model state). */
    bool isActive() const { return m_isActive; }

    /**
     * @brief Associates a workspace to a monitor for this virtual desktop.
     * @param monitorId Monitor id.
     * @param workspaceId Workspace id.
     */
    void bindWorkspace(MonitorId monitorId, WorkspaceId workspaceId);

    /**
     * @brief Gets the workspace id associated to a monitor.
     * @return Workspace id if bound.
     */
    std::optional<WorkspaceId> getWorkspaceForMonitor(MonitorId monitorId) const;

    /** @brief Returns all bindings monitor->workspace for this desktop. */
    const std::unordered_map<MonitorId, WorkspaceId> &getBindings() const {
        return m_workspaceByMonitor;
    }

    /**
     * @brief Returns a string representation.
     * @param json If true returns JSON, otherwise a human-readable list format.
     */
    std::string toString(bool json = false, std::optional<bool> asNode = std::nullopt) const;

    /**
     * @brief Returns a detailed string representation.
     * @param json If true returns JSON (including bindings), otherwise list format.
     */
    std::string toStringDetailed(bool json = false,
                                 std::optional<bool> asNode = std::nullopt) const;

private:
    int m_id;
    std::string m_name;
    std::unordered_map<MonitorId, WorkspaceId> m_workspaceByMonitor;
    bool m_isActive = false;

    /**
     * @brief Returns a list representation.
     * @param detailed If true, includes workspace bindings.
     */
    std::string toList(bool detailed = false) const;

    /**
     * @brief JSON representation of this virtual desktop.
     *
     * @param detailed If true, includes workspace bindings.
     * @param asNode If true, formats as a JSON node (without enclosing braces).
     */
    std::string toJson(bool detailed = true, bool asNode = false) const;

}; // class CVirtualDesktop

} // namespace VDM