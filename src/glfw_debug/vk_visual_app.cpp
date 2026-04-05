#include "vk_visual_app.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <QLoggingCategory>
#include <QFileInfo>

#include "renderer/vk_ctx/vk_initializer.h"
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

void VkVisualTestApp::recordCommandBuffer(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex) {
    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    auto preRenderBarrier = utils::createImageTransitionInfo(
        ctx_->swapchainImages[imageIndex],
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
        .imageView = ctx_->swapchainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = ctx_->swapchainExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *ctx_->graphicsPipeline);

    commandBuffer.setViewport(0,
                              vk::Viewport{
                                  .width = static_cast<float>(ctx_->swapchainExtent.width),
                                  .height = static_cast<float>(ctx_->swapchainExtent.height),
                                  .maxDepth = 1.0f
                              });
    commandBuffer.setScissor(0,
                             vk::Rect2D{
                                 .extent = ctx_->swapchainExtent
                             });

    // TODO: try instanced rendering some time
    commandBuffer.draw(3, 1, 0, 0); // Draw triangle (3 vertices)
    commandBuffer.endRendering();


    auto postRenderBarrier = utils::createImageTransitionInfo(
        ctx_->swapchainImages[imageIndex],
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
    ::vk::Semaphore signalSemaphores[] = {*ctx_->renderFinishedSemaphores[imageIndex]};
    ::vk::CommandBuffer cmdBuf = *ctx_->commandBuffers[currentFrame_];

    ::vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    ctx_->graphicsQueue.submit(submitInfo, *ctx_->inFlightFences[currentFrame_]);

    // Present
    ::vk::SwapchainKHR swapchains[] = {*ctx_->swapchain};
    ::vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex
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

