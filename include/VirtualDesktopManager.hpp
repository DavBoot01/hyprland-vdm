#pragma once

namespace VDM {

    class CVirtualDesktopManager {
    public:
        static CVirtualDesktopManager& getInstance();

    private:
        CVirtualDesktopManager();
        ~CVirtualDesktopManager();

        CVirtualDesktopManager(const CVirtualDesktopManager&) = delete;
        CVirtualDesktopManager& operator=(const CVirtualDesktopManager&) = delete;
        CVirtualDesktopManager(CVirtualDesktopManager&&) = delete;
        CVirtualDesktopManager& operator=(CVirtualDesktopManager&&) = delete;
    }; // class CVirtualDesktopManager

} // namespace VDM