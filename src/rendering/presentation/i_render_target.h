#pragma once

#include <common/vk_common.h>

namespace nuff::renderer {

class IRenderTarget {
public:
    virtual ~IRenderTarget() = default;

    enum class FrameResult { Success, Recreate, Error };

    // Frame lifecycle
    virtual FrameResult beginFrame() = 0;
    virtual FrameResult endFrame() = 0;

    // Per-frame resources (valid between beginFrame/endFrame)
    virtual const vk::raii::CommandBuffer& commandBuffer() const = 0;
    virtual const vk::raii::ImageView& imageView() const = 0;
    virtual vk::Image image() const = 0;
    virtual vk::Extent2D extent() const = 0;
    virtual vk::Format format() const = 0;

    // The layout the image should be transitioned to after rendering
    virtual vk::ImageLayout finalLayout() const = 0;

    // Frame indexing
    virtual uint32_t currentFrameIndex() const = 0;
    virtual uint32_t framesInFlight() const = 0;

    // Per-frame descriptor resources
    virtual void initFrameResources(const vk::raii::DescriptorSetLayout& layout,
                                     vk::DeviceSize uboSize) = 0;
    virtual void cleanupFrameResources() = 0;
    virtual vk::DescriptorSet currentDescriptorSet() const = 0;
    virtual void* currentUniformBufferMapping() const = 0;
};

} // namespace nuff::renderer

