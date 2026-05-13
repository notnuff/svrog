#include "glfw_platform_config.h"
#include "glfw_initializer.h"

#include "presentation/swapchain/swapchain_builder.h"
#include "presentation/swapchain/swapchain_render_target.h"

#include <stdexcept>
#include <vector>

namespace nuff::runtime {

GlfwPlatformConfig::GlfwPlatformConfig(GLFWwindow* window)
    : m_window(window) {}

void GlfwPlatformConfig::setShaderPath(std::string vertexPath,
                                       std::string fragmentPath) {
    m_vertexShaderPath = std::move(vertexPath);
    m_fragmentShaderPath = std::move(fragmentPath);
}

std::unique_ptr<renderer::CoreInitializer> GlfwPlatformConfig::makeInitializer() {
    auto initializer = std::make_unique<GlfwInitializer>();

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    initializer->setGlfwExtensions(extensions);

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    initializer->setExtent(static_cast<uint32_t>(width),
                           static_cast<uint32_t>(height));

    GLFWwindow* window = m_window;
    initializer->setSurfaceCreator([window](VkInstance instance) {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
        return surface;
    });

    initializer->setVertexShaderPath(m_vertexShaderPath)
                .setFragmentShaderPath(m_fragmentShaderPath);

    return initializer;
}

std::unique_ptr<renderer::IRenderTarget>
GlfwPlatformConfig::makeRenderTarget(renderer::CoreCtx& ctx) {
    return std::make_unique<renderer::SwapchainRenderTarget>(&ctx);
}

void GlfwPlatformConfig::onRecreate(renderer::CoreCtx& ctx,
                                    renderer::IRenderTarget& target) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    ctx.device.waitIdle();

    auto& swapchain = ctx.component<renderer::SwapchainCtxComponent>();
    swapchain.swapchainImageViews.clear();
    swapchain.swapchainImages.clear();

    renderer::SwapchainBuilder swapchainBuilder;
    swapchainBuilder.setExtent(static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height));
    swapchainBuilder.build(ctx);

    static_cast<renderer::SwapchainRenderTarget&>(target).recreateResources();
}

} // namespace nuff::runtime
