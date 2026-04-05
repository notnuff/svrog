#pragma once

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include "renderer/vk_ctx/vk_ctx.h"

#include <memory>

namespace nuff::renderer {

// VkVisualTestApp - Standalone Vulkan test application using GLFW.
// Provides a simple environment for testing Vulkan rendering
// without Qt's Window dependencies.

class VkVisualTestApp {
public:
    VkVisualTestApp() = default;
    ~VkVisualTestApp() = default;

    VkVisualTestApp(const VkVisualTestApp&) = delete;
    VkVisualTestApp& operator=(const VkVisualTestApp&) = delete;
    VkVisualTestApp(VkVisualTestApp&&) = delete;
    VkVisualTestApp& operator=(VkVisualTestApp&&) = delete;


    void run();

private:
    static constexpr uint32_t DEFAULT_WIDTH = 800;
    static constexpr uint32_t DEFAULT_HEIGHT = 600;

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<VkCtx> ctx_;
    uint32_t currentFrame_ = 0;
    bool framebufferResized_ = false;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void recreateSwapchain();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace nuff::renderer

