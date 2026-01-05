#pragma once

#include <hyprland/src/desktop/Workspace.hpp>

#include <string>
#include <vector>
#include <optional>
#include <string_view>

namespace VDM {

    class CVirtualDesktop {

    public:
        CVirtualDesktop(const int id, std::optional<std::string_view> name = std::nullopt);
        ~CVirtualDesktop();

        // Getters
        const std::string_view& getName() const { return m_name; }
        const int getID() const { return m_id; }

        // Setters
        void setName(const std::string_view name) { m_name = name; }

        // State
        void setActive(const bool active) { m_isActive = active; }
        const bool isActive() const { return m_isActive; }

        const std::string toString() const;
        const std::string toStringDetailed() const;

    private:
        int m_id;
        std::string_view m_name;
        std::vector<WORKSPACEID> m_workspaceIds;
        bool m_isActive = false;

    }; // class CVirtualDesktop

} // namespace VDM