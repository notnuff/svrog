#include "visual_app.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <QLoggingCategory>

#include "glfw_initializer.h"
#include "presentation/swapchain_builder.h"

namespace L {
Q_LOGGING_CATEGORY(vkVisualApp, "nuff.renderer.glfw_app")
}

namespace nuff::renderer {

void VkVisualTestApp::run() {
    initWindow();
    initRenderer();
    mainLoop();
    cleanup();
}

void VkVisualTestApp::initWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                "VkVisualMain - Triangle", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

void VkVisualTestApp::initRenderer() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    nuff::ui::glfw::GlfwInitializer initializer;

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    std::string shaderPath = "shaders/triangle_shader.spv";
    qCInfo(L::vkVisualApp) << "Loading shader from:" << shaderPath.c_str();

    // Set GLFW extensions and surface creator on the initializer
    initializer.setGlfwExtensions(extensions);
    initializer.setSurfaceCreator([this](VkInstance instance) {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
        return surface;
    });
    initializer.setExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    initializer
        .setVertexShaderPath(shaderPath)
        .setFragmentShaderPath(shaderPath);

    m_ctx = initializer.buildCtx();
    m_renderTarget = std::make_unique<SwapchainRenderTarget>(m_ctx.get());

    m_renderer.setContext(m_ctx.get());
    m_renderer.setRenderTarget(m_renderTarget.get());
    m_renderer.setRecreateCallback([this]() {
        recreateSwapchain();
    });
}

void VkVisualTestApp::framebufferResizeCallback(GLFWwindow* window, int /*width*/, int /*height*/) {
    auto* app = reinterpret_cast<VkVisualTestApp*>(glfwGetWindowUserPointer(window));
    app->m_renderer.notifyFramebufferResized();
}

void VkVisualTestApp::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    m_renderer.stopAndWait();

    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();
    swapchain.swapchainImageViews.clear();
    swapchain.swapchainImages.clear();

    SwapchainBuilder swapchainBuilder;
    swapchainBuilder.setExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    swapchainBuilder.build(*m_ctx);

    m_renderTarget->recreateResources();

    qCInfo(L::vkVisualApp) << "Swapchain recreated (" << width << "x" << height << ")";
}

void VkVisualTestApp::mainLoop() {
    qCInfo(L::vkVisualApp) << "Entering main loop. Close window to exit.";
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        m_renderer.drawFrame();
    }
    m_renderer.stopAndWait();
}

void VkVisualTestApp::cleanup() {
    m_renderTarget.reset();
    m_ctx.reset();

    glfwDestroyWindow(m_window);
    glfwTerminate();
    qCInfo(L::vkVisualApp) << "Cleanup complete";
}

} // namespace nuff::renderer

