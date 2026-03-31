#include "vk_visual_app.h"
#include "renderer/vk_ctx/vk_initializer.h"
#include "utils/file_utils.h"
#include "renderer/shaders/shader_utils.h"

#include <QLoggingCategory>
#include <QFileInfo>

#include <stdexcept>
#include <string>
#include <vector>

namespace L {
Q_LOGGING_CATEGORY(vkVisualApp, "nuff.renderer.vk.visual_app")
}

namespace nuff::renderer {

void VkVisualTestApp::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VkVisualTestApp::initWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, 
                                "VkVisualMain - Triangle", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
}

void VkVisualTestApp::initVulkan() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    VkInitializer initializer;

    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);

    std::string shaderPath = "shaders/triangle_shader.spv";
    qCInfo(L::vkVisualApp) << "Loading shader from:" << shaderPath.c_str();

    ctx_ = initializer
        .setAppName("VkVisualMain")
        .setEngineName("svrog")
        .addInstanceExtensions(extensions)
        .setSurfaceCreator([this](VkInstance instance) {
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance, window_, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface");
            }
            return surface;
        })
        .setExtent(width, height)
        .setVertexShaderPath(shaderPath)
        .setFragmentShaderPath(shaderPath)
        .initialize();
}

void VkVisualTestApp::recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    vk::ClearValue clearColor{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}};

    vk::RenderPassBeginInfo renderPassInfo{
        *ctx_->renderPass,
        *ctx_->framebuffers[imageIndex],
        {{0, 0}, ctx_->swapchainExtent},
        1, &clearColor
    };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *ctx_->graphicsPipeline);
    commandBuffer.draw(3, 1, 0, 0);  // Draw triangle (3 vertices)
    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void VkVisualTestApp::drawFrame() {
    // Wait for previous frame
    auto waitResult = ctx_->device.waitForFences(
        {*ctx_->inFlightFences[currentFrame_]}, vk::True, UINT64_MAX);
    (void)waitResult;

    // Acquire next image
    auto [acquireResult, imageIndex] = ctx_->swapchain.acquireNextImage(
        UINT64_MAX, *ctx_->imageAvailableSemaphores[currentFrame_], nullptr);

    if (acquireResult != ::vk::Result::eSuccess && acquireResult != ::vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    ctx_->device.resetFences({*ctx_->inFlightFences[currentFrame_]});

    ctx_->commandBuffers[currentFrame_].reset();
    recordCommandBuffer(ctx_->commandBuffers[currentFrame_], imageIndex);

    // Submit
    ::vk::Semaphore waitSemaphores[] = {*ctx_->imageAvailableSemaphores[currentFrame_]};
    ::vk::PipelineStageFlags waitStages[] = {::vk::PipelineStageFlagBits::eColorAttachmentOutput};
    ::vk::Semaphore signalSemaphores[] = {*ctx_->renderFinishedSemaphores[currentFrame_]};
    ::vk::CommandBuffer cmdBuf = *ctx_->commandBuffers[currentFrame_];

    ::vk::SubmitInfo submitInfo{
        1, waitSemaphores, waitStages,
        1, &cmdBuf,
        1, signalSemaphores
    };

    ctx_->graphicsQueue.submit(submitInfo, *ctx_->inFlightFences[currentFrame_]);

    // Present
    ::vk::SwapchainKHR swapchains[] = {*ctx_->swapchain};
    ::vk::PresentInfoKHR presentInfo{
        1, signalSemaphores,
        1, swapchains,
        &imageIndex
    };

    auto presentResult = ctx_->presentQueue.presentKHR(presentInfo);
    (void)presentResult;

    currentFrame_ = (currentFrame_ + 1) % VkCtx::MAX_FRAMES_IN_FLIGHT;
}

void VkVisualTestApp::mainLoop() {
    qCInfo(L::vkVisualApp) << "Entering main loop. Close window to exit.";
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        drawFrame();
    }
    ctx_->device.waitIdle();
}

void VkVisualTestApp::cleanup() {
    // RAII handles Vulkan cleanup automatically
    ctx_.reset();

    glfwDestroyWindow(window_);
    glfwTerminate();
    qCInfo(L::vkVisualApp) << "Cleanup complete";
}

} // namespace nuff::renderer

