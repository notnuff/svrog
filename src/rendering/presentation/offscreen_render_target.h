#pragma once

#include "i_render_target.h"
#include "frame_resources.h"
#include "core/context/ctx.h"

namespace nuff::renderer {

class OffscreenRenderTarget : public IRenderTarget {
public:
    OffscreenRenderTarget(CoreCtx* ctx, uint32_t width, uint32_t height,
                          vk::Format format = vk::Format::eB8G8R8A8Unorm,
                          bool exportable = false);
    ~OffscreenRenderTarget() override = default;

    void resize(uint32_t width, uint32_t height);

    FrameResult beginFrame() override;
    FrameResult endFrame() override;

    const vk::raii::CommandBuffer& commandBuffer() const override;
    const vk::raii::ImageView& imageView() const override;
    vk::Image image() const override;
    vk::Extent2D extent() const override;
    vk::Format format() const override;
    vk::ImageLayout finalLayout() const override;

    uint32_t currentFrameIndex() const override;
    uint32_t framesInFlight() const override;

    void initFrameResources(const vk::raii::DescriptorSetLayout& layout,
                             vk::DeviceSize uboSize) override;
    void cleanupFrameResources() override;
    vk::DescriptorSet currentDescriptorSet() const override;
    void* currentUniformBufferMapping() const override;

    VkImage nativeImage() const { return *m_image; }
    bool isExportable() const { return m_exportable; }

    int getMemoryFd() const;
    VkDeviceSize memorySize() const { return m_memorySize; }

private:
    void createResources();

    CoreCtx* m_ctx;
    uint32_t m_width;
    uint32_t m_height;
    vk::Format m_format;
    bool m_exportable;

    vk::raii::Image m_image{nullptr};
    vk::raii::DeviceMemory m_imageMemory{nullptr};
    vk::raii::ImageView m_imageView{nullptr};
    VkDeviceSize m_memorySize = 0;

    vk::raii::CommandPool m_commandPool{nullptr};
    vk::raii::CommandBuffers m_commandBuffers{nullptr};
    vk::raii::Fence m_fence{nullptr};

    FrameResources m_frameResources;
};

} // namespace nuff::renderer

