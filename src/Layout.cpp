#include "Layout.hpp"

#include "JsonUtils.hpp"


#include <algorithm>
#include <sstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace VDM {

    CLayout::CLayout(int vdeskCounter) : m_vdeskCounter(0) {
        setVirtualDesktopCount(vdeskCounter);
    }

    CLayout::~CLayout() = default;

    void CLayout::setVirtualDesktopCount(int count) {
        m_vdeskCounter = std::max(0, count);

        m_virtualDesktops.clear();
        m_virtualDesktops.reserve(static_cast<size_t>(m_vdeskCounter));
        for (int i = 1; i <= m_vdeskCounter; ++i) {
            m_virtualDesktops.emplace_back(i);
        }
    }

    CVirtualDesktop* CLayout::getVirtualDesktopById(int id) {
        for (auto& vd : m_virtualDesktops) {
            if (vd.getID() == id)
                return &vd;
        }
        return nullptr;
    }

    const CVirtualDesktop* CLayout::getVirtualDesktopById(int id) const {
        for (const auto& vd : m_virtualDesktops) {
            if (vd.getID() == id)
                return &vd;
        }
        return nullptr;
    }

    void CLayout::setActiveVirtualDesktop(int id) {
        for (auto& vd : m_virtualDesktops) {
            vd.setActive(vd.getID() == id);
        }
    }

    std::string CLayout::toString(bool json) const {
        if (json)
            return toJson(false);
        return toList(false);
    }

    std::string CLayout::toStringDetailed(bool json) const {
        if (json)
            return toJson(true);
        return toList(true);
    }

    std::string CLayout::toList(bool detailed) const {
        std::ostringstream ss;
        ss << "Layout:\n";
        ss << "  Virtual Desktop Count: " << m_vdeskCounter << "\n";

        if (detailed) {
            for (const auto& vd : m_virtualDesktops) {
                ss << "\n" << vd.toStringDetailed(false) << "\n";
            }
        } else {
            for (const auto& vd : m_virtualDesktops) {
                ss << "\n" << vd.toString(false) << "\n";
            }
        }
        

        return ss.str();
    }

    std::string CLayout::toJson(bool detailed) const {
        json j;
        j["vdeskCounter"] = m_vdeskCounter;
        std::vector<json> vdesks;
        for (const auto& vd : m_virtualDesktops) {
            if (detailed)
                vdesks.push_back(json::parse(vd.toStringDetailed(true, true)));
            else
                vdesks.push_back(json::parse(vd.toString(false, true)));
        }
        j["virtualDesktops"] = vdesks;
        return j.dump(4);
    }

} // namespace VDM