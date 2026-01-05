#pragma once

#include <vector>

#include "VirtualDesktop.hpp"

namespace VDM {

    class CLayout {
    public:
        CLayout();
        ~CLayout();

    private:
        int m_vdeskCounter = 0;
        std::vector<CVirtualDesktop> m_virtualDesktops;

    }; // class CLayout

} // namespace VDM