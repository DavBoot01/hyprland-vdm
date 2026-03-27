#pragma once

/**
 * @file Layout.hpp
 * @brief Virtual desktop layout policy/model.
 */

#include "VirtualDesktop.hpp"

#include <vector>

namespace VDM {

class CLayout {

public:
    /**
     * @brief Creates a layout with an optional initial desktop count.
     * @param vdeskCounter Number of virtual desktops to create.
     */
    CLayout(int vdeskCounter = 0);
    ~CLayout();

    /**
     * @brief Sets the number of virtual desktops in the layout.
     *
     * This recreates the internal list.
     */
    void setVirtualDesktopCount(int count);

    /** @brief Returns the number of virtual desktops in the layout. */
    int getVirtualDesktopCount() const { return m_vdeskCounter; }

    /** @brief Returns an immutable list of virtual desktops. */
    const std::vector<CVirtualDesktop> &getVirtualDesktops() const { return m_virtualDesktops; }

    /** @brief Returns a mutable list of virtual desktops. */
    std::vector<CVirtualDesktop> &getVirtualDesktops() { return m_virtualDesktops; }

    /**
     * @brief Returns a pointer to the desktop with the given id.
     * @return Pointer or nullptr if not found.
     */
    CVirtualDesktop *getVirtualDesktopById(int id);

    /**
     * @brief Returns a pointer to the desktop with the given id.
     * @return Pointer or nullptr if not found.
     */
    const CVirtualDesktop *getVirtualDesktopById(int id) const;

    /**
     * @brief Marks a desktop as active and all others inactive.
     */
    void setActiveVirtualDesktop(int id);

    /**
     * @brief Returns a string representation.
     * @param json If true returns JSON, otherwise a human-readable list format.
     */
    std::string toString(bool json = false) const;

    /**
     * @brief Returns a detailed string representation.
     * @param json If true returns JSON (including bindings), otherwise list format.
     */
    std::string toStringDetailed(bool json = false) const;

private:
    int m_vdeskCounter = 0;
    std::vector<CVirtualDesktop> m_virtualDesktops;

    /**
     * @brief Returns a list representation.
     * @param detailed If true, includes per-desktop bindings.
     */
    std::string toList(bool detailed = false) const;

    /**
     * @brief JSON representation of this layout.
     *
     * @param detailed If true, includes per-desktop bindings.
     */
    std::string toJson(bool detailed = true) const;
}; // class CLayout

} // namespace VDM