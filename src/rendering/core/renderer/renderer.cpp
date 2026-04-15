#include "renderer.h"

#include "utils/image_utils.h"

namespace nuff::renderer {

void Renderer::setContext(CoreCtx* ctx) {
    m_ctx = ctx;
}

void Renderer::setRenderTarget(std::unique_ptr<IRenderTarget> &&renderTarget) {
    m_renderTarget = std::move(renderTarget);
}

void Renderer::stopAndWait() const {
    m_ctx->device.waitIdle();
}


void Renderer::drawFrame() {
    auto& present = m_ctx->extension<PresentCtxMixin>();
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();
    auto& command = m_ctx->extension<CommandCtxMixin>();
    auto& graphics = m_ctx->extension<GraphicsCtxMixin>();

    // Wait for previous frame
    auto waitResult = m_ctx->device.waitForFences(
        {*present.inFlightFences[m_currentFrame]}, vk::True, UINT64_MAX);
    (void)waitResult;

    // Acquire next image
    auto [acquireResult, imageIndex] = swapchain.swapchain.acquireNextImage(
        UINT64_MAX, *present.imageAvailableSemaphores[m_currentFrame], nullptr);

    if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
        // TODO: signal swapchain recreation needed
        return;
    }
    if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    m_ctx->device.resetFences({*present.inFlightFences[m_currentFrame]});

    command.commandBuffers[m_currentFrame].reset();

    recordRenderingInfo(
        command.commandBuffers[m_currentFrame],
        swapchain.swapchainImageViews[imageIndex],
        swapchain.swapchainExtent,
        swapchain.swapchainImages[imageIndex]);

    // Submit
    vk::Semaphore waitSemaphores[] = {*present.imageAvailableSemaphores[m_currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {*present.renderFinishedSemaphores[imageIndex]};
    vk::CommandBuffer cmdBuf = *command.commandBuffers[m_currentFrame];

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    graphics.graphicsQueue.submit(submitInfo, *present.inFlightFences[m_currentFrame]);

    // Present
    vk::SwapchainKHR swapchains[] = {*swapchain.swapchain};
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex
    };

    auto presentResult = present.presentQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR
        || presentResult == vk::Result::eSuboptimalKHR
        || m_framebufferResized) {
        m_framebufferResized = false;
        // TODO: signal swapchain recreation needed
    }

    m_currentFrame = (m_currentFrame + 1) % present.maxFramesInFlight;
}


void Renderer::recordRenderingInfo(
    const vk::raii::CommandBuffer &commandBuffer,
    const vk::raii::ImageView &targetImageView,
    const vk::Extent2D &extent,
    vk::Image swapchainImage)
{
    auto& pipeline = m_ctx->extension<PipelineCtxMixin>();

    vk::CommandBufferBeginInfo beginInfo{};
    commandBuffer.begin(beginInfo);

    auto preRenderBarrier = utils::createImageTransitionInfo(
        swapchainImage,
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
        .imageView = targetImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = extent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);

    commandBuffer.setViewport(0,
                              vk::Viewport{
                                  .width = static_cast<float>(extent.width),
                                  .height = static_cast<float>(extent.height),
                                  .maxDepth = 1.0f
                              });
    commandBuffer.setScissor(0,
                             vk::Rect2D{
                                 .extent = extent
                             });

    // TODO: try instanced rendering some time
    commandBuffer.draw(3, 1, 0, 0); // Draw triangle (3 vertices)
    commandBuffer.endRendering();

    auto postRenderBarrier = utils::createImageTransitionInfo(
        swapchainImage,
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

} //namespace nuff::renderer
