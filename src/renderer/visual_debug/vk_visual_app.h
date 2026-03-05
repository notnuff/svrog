#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include "vk_ctx.h"

#include <memory>

namespace svrog::renderer {

/**
 * VkVisualTestApp - Standalone Vulkan test application using GLFW.
 * 
 * Provides a simple environment for testing Vulkan rendering
 * without Qt dependencies.
 */
class VkVisualTestApp {
public:
    VkVisualTestApp() = default;
    ~VkVisualTestApp() = default;

    // Non-copyable, non-movable
    VkVisualTestApp(const VkVisualTestApp&) = delete;
    VkVisualTestApp& operator=(const VkVisualTestApp&) = delete;
    VkVisualTestApp(VkVisualTestApp&&) = delete;
    VkVisualTestApp& operator=(VkVisualTestApp&&) = delete;

    /**
     * Run the application main loop.
     * Initializes window, Vulkan, runs render loop, then cleans up.
     */
    void run();

private:
    static constexpr uint32_t DEFAULT_WIDTH = 800;
    static constexpr uint32_t DEFAULT_HEIGHT = 600;

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<VkCtx> ctx_;
    uint32_t currentFrame_ = 0;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void recordCommandBuffer(::vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
};

} // namespace svrog::renderer

