/**
 * @brief New application entry point using ssme engine
 *
 * This is the new application that uses the ssme engine
 * with proper abstraction layers.
 */

#include "platform/desktop/platform_factory.h"
#include "utils/logger.h"

#include <memory>
#include <string>
#include <cstdlib>
#include <iostream>

int main() {
    std::cout << "=== SSME Engine Test ===" << std::endl;

    try {
        // Create platform with multiple windows
        auto platform = ssme::PlatformFactory::create({
            {"OpenGL Window", 800, 600},
            {"Vulkan Window", 800, 600}
        });

        if (!platform) {
            std::cerr << "Failed to create platform!" << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "DesktopPlatform created successfully!" << std::endl;
        std::cout << "Window count: " << platform->getWindowCount() << std::endl;

        // Test window size
        int width = 0, height = 0;
        platform->getWindowSize(0, &width, &height);
        std::cout << "Window 0 size: " << width << "x" << height << std::endl;

        // Test update loop
        std::cout << "Running update loop (press Ctrl+C to exit)..." << std::endl;

        int frame_count = 0;
        while (platform->allAlive() && frame_count < 100) {
            platform->updateAllWindows();
            frame_count++;

            if (frame_count % 30 == 0) {
                std::cout << "Frame " << frame_count << std::endl;
            }
        }

        std::cout << "Test completed successfully!" << std::endl;

        // Cleanup
        platform->cleanup();

        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
