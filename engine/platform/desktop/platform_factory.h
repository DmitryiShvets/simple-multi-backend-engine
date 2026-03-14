#pragma once

#include "desktop_platform.h"
#include <memory>
#include <string>
#include <vector>

namespace ssme {

/**
 * @brief Factory for creating Platform instances.
 */
class PlatformFactory {
public:
    /**
     * @brief Create platform with single window.
     * @param appName Application name (also window title).
     * @param width Window width.
     * @param height Window height.
     * @return Platform instance.
     */
    static std::unique_ptr<DesktopPlatform> create(
        const std::string& appName,
        int width,
        int height
    ) {
        auto platform = std::make_unique<DesktopPlatform>();
        if (!platform->initialize(appName, width, height)) {
            return nullptr;
        }
        return platform;
    }

    /**
     * @brief Create platform with multiple windows.
     * @param windows Vector of WindowConfig.
     * @return Platform instance.
     */
    static std::unique_ptr<DesktopPlatform> create(
        const std::vector<WindowConfig>& windows
    ) {
        if (windows.empty()) {
            return nullptr;
        }

        auto platform = std::make_unique<DesktopPlatform>();

        // First window via initialize()
        if (!platform->initialize(
                windows[0].title,
                windows[0].width,
                windows[0].height)) {
            return nullptr;
        }

        // Rest via addWindow()
        for (size_t i = 1; i < windows.size(); ++i) {
            platform->addWindow(
                windows[i].title,
                windows[i].width,
                windows[i].height
            );
        }

        return platform;
    }
};

} // namespace ssme
