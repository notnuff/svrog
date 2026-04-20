#pragma once

#include <presentation/i_render_target.h>
#include <core/context/ctx.h>

namespace nuff::renderer {

class SwapchainRenderTarget : public IRenderTarget {
public:
    explicit SwapchainRenderTarget(CoreCtx* ctx, uint32_t maxFramesInFlight = 2);
    ~SwapchainRenderTarget() override = default;

    void recreateResources();

    FrameResult beginFrame() override;
    FrameResult endFrame() override;

    const vk::raii::CommandBuffer& commandBuffer() const override;
    const vk::raii::ImageView& imageView() const override;
    vk::Image image() const override;
    vk::Extent2D extent() const override;
    vk::Format format() const override;
    vk::ImageLayout finalLayout() const override;

private:
    void createResources();

    CoreCtx* m_ctx;
    uint32_t m_maxFramesInFlight;
    uint32_t m_currentFrame = 0;
    uint32_t m_imageIndex = 0;

    vk::raii::CommandPool m_commandPool{nullptr};
    vk::raii::CommandBuffers m_commandBuffers{nullptr};
    std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_inFlightFences;
};

} // namespace nuff::renderer

