#pragma once

#include <chrono>
#include <memory>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "core/context/ctx.h"
#include "engine/core/engine.h"
#include "presentation/swapchain_render_target.h"

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

    GLFWwindow* m_window = nullptr;

    std::unique_ptr<CoreCtx> m_ctx;
    std::unique_ptr<SwapchainRenderTarget> m_renderTarget;

    engine::Engine m_engine;
    std::chrono::steady_clock::time_point m_lastFrameTime;
    bool m_hasLastFrameTime = false;

    void initWindow();
    void initRenderer();
    void mainLoop();
    void cleanup();
    void recreateSwapchain();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

} // namespace nuff::renderer
