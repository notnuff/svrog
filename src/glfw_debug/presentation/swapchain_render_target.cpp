#include "swapchain_render_target.h"

#include <stdexcept>

namespace nuff::renderer {

SwapchainRenderTarget::SwapchainRenderTarget(CoreCtx* ctx, uint32_t maxFramesInFlight)
    : m_ctx(ctx), m_maxFramesInFlight(maxFramesInFlight)
{
    createResources();
}

void SwapchainRenderTarget::createResources() {
    auto& graphics = m_ctx->extension<GraphicsCtxMixin>();
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();

    m_commandBuffers = nullptr;
    m_commandPool = nullptr;

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphics.queueFamilyIndices.graphicsFamily.value()
    };
    m_commandPool = vk::raii::CommandPool(m_ctx->device, poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = *m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = m_maxFramesInFlight
    };
    m_commandBuffers = vk::raii::CommandBuffers(m_ctx->device, allocInfo);

    m_imageAvailableSemaphores.clear();
    m_renderFinishedSemaphores.clear();
    m_inFlightFences.clear();

    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

    for (uint32_t i = 0; i < m_maxFramesInFlight; i++) {
        m_imageAvailableSemaphores.emplace_back(m_ctx->device, semaphoreInfo);
        m_inFlightFences.emplace_back(m_ctx->device, fenceInfo);
    }
    for (size_t i = 0; i < swapchain.swapchainImages.size(); i++) {
        m_renderFinishedSemaphores.emplace_back(m_ctx->device, semaphoreInfo);
    }
}

void SwapchainRenderTarget::recreateResources() {
    m_currentFrame = 0;
    createResources();
}

IRenderTarget::FrameResult SwapchainRenderTarget::beginFrame() {
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();

    auto waitResult = m_ctx->device.waitForFences(
        {*m_inFlightFences[m_currentFrame]}, vk::True, UINT64_MAX);
    (void)waitResult;

    auto [acquireResult, imageIndex] = swapchain.swapchain.acquireNextImage(
        UINT64_MAX, *m_imageAvailableSemaphores[m_currentFrame], nullptr);

    if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
        return FrameResult::Recreate;
    }
    if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    m_imageIndex = imageIndex;

    m_ctx->device.resetFences({*m_inFlightFences[m_currentFrame]});
    m_commandBuffers[m_currentFrame].reset();

    return FrameResult::Success;
}

IRenderTarget::FrameResult SwapchainRenderTarget::endFrame() {
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();
    auto& graphics = m_ctx->extension<GraphicsCtxMixin>();
    auto& present = m_ctx->extension<PresentQueueMixin>();

    vk::Semaphore waitSemaphores[] = {*m_imageAvailableSemaphores[m_currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signalSemaphores[] = {*m_renderFinishedSemaphores[m_imageIndex]};
    vk::CommandBuffer cmdBuf = *m_commandBuffers[m_currentFrame];

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    graphics.graphicsQueue.submit(submitInfo, *m_inFlightFences[m_currentFrame]);

    vk::SwapchainKHR swapchains[] = {*swapchain.swapchain};
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &m_imageIndex
    };

    auto presentResult = present.presentQueue.presentKHR(presentInfo);

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;

    if (presentResult == vk::Result::eErrorOutOfDateKHR
        || presentResult == vk::Result::eSuboptimalKHR) {
        return FrameResult::Recreate;
    }

    return FrameResult::Success;
}

const vk::raii::CommandBuffer& SwapchainRenderTarget::commandBuffer() const {
    return m_commandBuffers[m_currentFrame];
}

const vk::raii::ImageView& SwapchainRenderTarget::imageView() const {
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();
    return swapchain.swapchainImageViews[m_imageIndex];
}

vk::Image SwapchainRenderTarget::image() const {
    auto& swapchain = m_ctx->extension<SwapchainCtxMixin>();
    return swapchain.swapchainImages[m_imageIndex];
}

vk::Extent2D SwapchainRenderTarget::extent() const {
    return m_ctx->extension<SwapchainCtxMixin>().swapchainExtent;
}

vk::Format SwapchainRenderTarget::format() const {
    return m_ctx->extension<SwapchainCtxMixin>().swapchainImageFormat;
}

vk::ImageLayout SwapchainRenderTarget::finalLayout() const {
    return vk::ImageLayout::ePresentSrcKHR;
}

uint32_t SwapchainRenderTarget::currentFrameIndex() const {
    return m_currentFrame;
}

uint32_t SwapchainRenderTarget::framesInFlight() const {
    return m_maxFramesInFlight;
}

void SwapchainRenderTarget::initFrameResources(const vk::raii::DescriptorSetLayout& layout,
                                                 vk::DeviceSize uboSize) {
    m_frameResources.init(*m_ctx, layout, uboSize, m_maxFramesInFlight);
}

void SwapchainRenderTarget::cleanupFrameResources() {
    m_frameResources.cleanup(*m_ctx);
}

vk::DescriptorSet SwapchainRenderTarget::currentDescriptorSet() const {
    return m_frameResources.descriptorSet(m_currentFrame);
}

void* SwapchainRenderTarget::currentUniformBufferMapping() const {
    return m_frameResources.uniformBufferMapping(m_currentFrame);
}

} // namespace nuff::renderer

