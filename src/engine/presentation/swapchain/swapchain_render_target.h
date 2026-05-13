#pragma once

#include <presentation/i_render_target.h>
#include <presentation/frame_resources.h>
#include <context/ctx.h>

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
    const vk::raii::ImageView& depthImageView() const override;
    vk::Image depthImage() const override;
    vk::Format depthFormat() const override;
    vk::ImageLayout finalLayout() const override;

    uint32_t currentFrameIndex() const override;
    uint32_t framesInFlight() const override;

    void initFrameResources(const vk::raii::DescriptorSetLayout& layout,
                             vk::DeviceSize uboSize) override;
    void cleanupFrameResources() override;
    vk::DescriptorSet currentDescriptorSet() const override;
    void* currentUniformBufferMapping() const override;

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

    vk::Format m_depthFormat = vk::Format::eD32Sfloat;
    vk::raii::Image m_depthImage{nullptr};
    vk::raii::DeviceMemory m_depthImageMemory{nullptr};
    vk::raii::ImageView m_depthImageView{nullptr};

    FrameResources m_frameResources;
};

} // namespace nuff::renderer

