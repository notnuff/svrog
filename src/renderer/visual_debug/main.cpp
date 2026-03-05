#include "visual_debug/vk_visual_app.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    nuff::renderer::VkVisualTestApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

