#include "VirtualDesktop.hpp"

#include <sstream>

namespace VDM {

    CVirtualDesktop::CVirtualDesktop(const int id, std::optional<std::string_view> name)
        : m_id(id), m_name(name.has_value() ? std::string{name.value()} : "VDesk " + std::to_string(id)) {
            
    }

    CVirtualDesktop::~CVirtualDesktop() = default;

    const std::string CVirtualDesktop::toString() const {
        std::ostringstream ss;
        ss << "VirtualDesktop{id=" << m_id << ", name='" << m_name << "', active=" << std::boolalpha << m_isActive << '}';
        return ss.str();
    }

    const std::string CVirtualDesktop::toStringDetailed() const {
        std::ostringstream ss;
        ss << "VirtualDesktop{id=" << m_id << ", name='" << m_name << "', active=" << std::boolalpha << m_isActive
        << ", workspaces=[";

        for (size_t i = 0; i < m_workspaceIds.size(); ++i) {
            ss << m_workspaceIds[i];
            if (i + 1 < m_workspaceIds.size())
                ss << ", ";
        }

        ss << "]}";
        return ss.str();
    }

} // namespace VDM
