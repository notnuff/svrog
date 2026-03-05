#include "visual_debug/vk_visual_app.h"

#include <QLoggingCategory>

#include <cstdlib>
#include <stdexcept>

namespace L {
Q_LOGGING_CATEGORY(vkVisualMain, "nuff.renderer.vk.visual_main")
}

int main() {
    nuff::renderer::VkVisualTestApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        qCCritical(L::vkVisualMain) << "Error:" << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

