#include "VirtualDesktop.hpp"

#include "JsonUtils.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>
using json = nlohmann::json;

namespace VDM {

CVirtualDesktop::CVirtualDesktop(int id, std::optional<std::string_view> name)
    : m_id(id),
      m_name(name.has_value() ? std::string{name.value()} : ("VDesk " + std::to_string(id))) {}

CVirtualDesktop::~CVirtualDesktop() = default;

void CVirtualDesktop::bindWorkspace(MonitorId monitorId, WorkspaceId workspaceId) {
    m_workspaceByMonitor[monitorId] = workspaceId;
}

std::optional<CVirtualDesktop::WorkspaceId>
CVirtualDesktop::getWorkspaceForMonitor(MonitorId monitorId) const {
    if (const auto it = m_workspaceByMonitor.find(monitorId); it != m_workspaceByMonitor.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string CVirtualDesktop::toString(bool json, std::optional<bool> asNode) const {
    if (json)
        return toJson(false, asNode.value_or(false));
    return toList(false);
}

std::string CVirtualDesktop::toStringDetailed(bool json, std::optional<bool> asNode) const {
    if (json)
        return toJson(true, asNode.value_or(false));
    return toList(true);
}

std::string CVirtualDesktop::toList(bool detailed) const {
    std::ostringstream ss;
    ss << "Virtual Desktop ID " << m_id << " (" << m_name
       << ") - Active: " << (m_isActive ? "Yes" : "No");

    if (detailed) {
        std::vector<std::pair<MonitorId, WorkspaceId>> bindings;
        bindings.reserve(m_workspaceByMonitor.size());
        for (const auto &kv : m_workspaceByMonitor)
            bindings.push_back(kv);

        std::sort(bindings.begin(), bindings.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });

        for (const auto &[monitorId, workspaceId] : bindings) {
            ss << "\n  Monitor ID " << monitorId << " -> Workspace ID " << workspaceId;
        }
    }

    return ss.str();
}

std::string CVirtualDesktop::toJson(bool detailed, bool asNode) const {
    json j;
    j["id"] = m_id;
    j["name"] = m_name;
    j["active"] = m_isActive;

    if (detailed) {
        std::vector<json> bindings;
        std::vector<std::pair<MonitorId, WorkspaceId>> sortedBindings(m_workspaceByMonitor.begin(),
                                                                      m_workspaceByMonitor.end());
        std::sort(sortedBindings.begin(), sortedBindings.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });
        for (const auto &[monitorId, workspaceId] : sortedBindings) {
            bindings.push_back({{"monitorId", monitorId}, {"workspaceId", workspaceId}});
        }
        j["bindings"] = bindings;
    }

    if (asNode)
        return j.dump();
    else
        return j.dump(4); // pretty print with 4 spaces
}

} // namespace VDM
