#include "vk_visual_app.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <QLoggingCategory>
#include <QFileInfo>

#include "renderer/vk_ctx/vk_initializer.h"
#include "renderer/vk_ctx/builders/swapchain_builder.h"
#include "renderer/vk_ctx/builders/framebuffer_builder.h"
#include "renderer/vk_ctx/builders/sync_builder.h"
#include "utils/file_utils.h"
#include "renderer/shaders/shader_utils.h"
#include "renderer/vk_ctx/utils/vk_image_utils.h"

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

void VkVisualTestApp::framebufferResizeCallback(GLFWwindow* window, int /*width*/, int /*height*/) {
    auto* app = reinterpret_cast<VkVisualTestApp*>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}

void VkVisualTestApp::initVulkan() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    VkInitializer initializer;

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);

    std::string shaderPath = "shaders/triangle_shader.spv";
    qCInfo(L::vkVisualApp) << "Loading shader from:" << shaderPath.c_str();

    m_ctx = initializer
        .setAppName("VkVisualMain")
        .setEngineName("svrog")
        .addInstanceExtensions(extensions)
        .setSurfaceCreator([this](VkInstance instance) {
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface");
            }
            return surface;
        })
        .setExtent(width, height)
        .setVertexShaderPath(shaderPath)
        .setFragmentShaderPath(shaderPath)
        .initialize();
}

void VkVisualTestApp::recordCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    auto preRenderBarrier = utils::createImageTransitionInfo(
        m_ctx->swapchainImages[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {}, // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
    );
    commandBuffer.pipelineBarrier2(preRenderBarrier.dependencyInfo);

    vk::ClearValue clearColor{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.9f}}};

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_ctx->swapchainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = m_ctx->swapchainExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_ctx->graphicsPipeline);

    commandBuffer.setViewport(0,
                              vk::Viewport{
                                  .width = static_cast<float>(m_ctx->swapchainExtent.width),
                                  .height = static_cast<float>(m_ctx->swapchainExtent.height),
                                  .maxDepth = 1.0f
                              });
    commandBuffer.setScissor(0,
                             vk::Rect2D{
                                 .extent = m_ctx->swapchainExtent
                             });

    // TODO: try instanced rendering some time
    commandBuffer.draw(3, 1, 0, 0); // Draw triangle (3 vertices)
    commandBuffer.endRendering();


    auto postRenderBarrier = utils::createImageTransitionInfo(
        m_ctx->swapchainImages[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
        {}, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe // dstStage
    );
    commandBuffer.pipelineBarrier2(postRenderBarrier.dependencyInfo);

    commandBuffer.end();
}

void VkVisualTestApp::drawFrame() {
    // Wait for previous frame
    auto waitResult = m_ctx->device.waitForFences(
        {*m_ctx->inFlightFences[m_currentFrame]}, vk::True, UINT64_MAX);
    (void)waitResult;

    // Acquire next image
    auto [acquireResult, imageIndex] = m_ctx->swapchain.acquireNextImage(
        UINT64_MAX, *m_ctx->imageAvailableSemaphores[m_currentFrame], nullptr);

    if (acquireResult == ::vk::Result::eErrorOutOfDateKHR) {
        recreateSwapchain();
        return;
    }
    if (acquireResult != ::vk::Result::eSuccess && acquireResult != ::vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    m_ctx->device.resetFences({*m_ctx->inFlightFences[m_currentFrame]});

    m_ctx->commandBuffers[m_currentFrame].reset();
    recordCommandBuffer(m_ctx->commandBuffers[m_currentFrame], imageIndex);

    // Submit
    vk::Semaphore waitSemaphores[] = {*m_ctx->imageAvailableSemaphores[m_currentFrame]};
    vk::PipelineStageFlags waitStages[] = {::vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {*m_ctx->renderFinishedSemaphores[imageIndex]};
    vk::CommandBuffer cmdBuf = *m_ctx->commandBuffers[m_currentFrame];

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    m_ctx->graphicsQueue.submit(submitInfo, *m_ctx->inFlightFences[m_currentFrame]);

    // Present
    vk::SwapchainKHR swapchains[] = {*m_ctx->swapchain};
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex
    };

    auto presentResult = m_ctx->presentQueue.presentKHR(presentInfo);

    if (presentResult == ::vk::Result::eErrorOutOfDateKHR
        || presentResult == ::vk::Result::eSuboptimalKHR
        || m_framebufferResized) {
        m_framebufferResized = false;
        recreateSwapchain();
    }

    m_currentFrame = (m_currentFrame + 1) % VkCtx::MAX_FRAMES_IN_FLIGHT;
}

void VkVisualTestApp::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    m_ctx->device.waitIdle();

    m_ctx->framebuffers.clear();
    m_ctx->swapchainImageViews.clear();
    m_ctx->swapchainImages.clear();

    SwapchainBuilder swapchainBuilder;
    swapchainBuilder.setExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    swapchainBuilder.build(*m_ctx);

    FramebufferBuilder framebufferBuilder;
    framebufferBuilder.build(*m_ctx);

    SyncBuilder syncBuilder;
    syncBuilder.build(*m_ctx);

    qCInfo(L::vkVisualApp) << "Swapchain recreated (" << width << "x" << height << ")";
}

void VkVisualTestApp::mainLoop() {
    qCInfo(L::vkVisualApp) << "Entering main loop. Close window to exit.";
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        drawFrame();
    }
    m_ctx->device.waitIdle();
}

void VkVisualTestApp::cleanup() {
    // RAII handles Vulkan cleanup automatically
    m_ctx.reset();

    glfwDestroyWindow(m_window);
    glfwTerminate();
    qCInfo(L::vkVisualApp) << "Cleanup complete";
}

} // namespace nuff::renderer

