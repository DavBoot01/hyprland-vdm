#include "VirtualDesktopManager.hpp"

namespace VDM {

CVirtualDesktopManager& CVirtualDesktopManager::instance() {
    static CVirtualDesktopManager s_instance;
    return s_instance;
}

CVirtualDesktopManager::CVirtualDesktopManager() = default;
CVirtualDesktopManager::~CVirtualDesktopManager() = default;

} // namespace VDM