#pragma once

#include "core/context/ctx.h"

namespace nuff::renderer {

struct AllocatedBuffer {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

struct AllocatedImage {
    vk::raii::Image image{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

struct SingleTimeCommandContext {
    vk::raii::CommandPool commandPool{nullptr};
    vk::raii::CommandBuffers commandBuffers{nullptr};

    [[nodiscard]] const vk::raii::CommandBuffer& cmd() const { return commandBuffers[0]; }
};

class MemoryManager {
public:
    explicit MemoryManager(CoreCtx& ctx);

    AllocatedBuffer createBuffer(vk::DeviceSize size,
                                 vk::BufferUsageFlags usage,
                                 vk::MemoryPropertyFlags memoryProperties) const;

    AllocatedImage createImage(uint32_t width, uint32_t height,
                               vk::Format format,
                               vk::ImageTiling tiling,
                               vk::ImageUsageFlags usage,
                               vk::MemoryPropertyFlags memoryProperties) const;

    void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const;

    void transitionImageLayout(vk::Image image,
                                vk::ImageLayout oldLayout,
                                vk::ImageLayout newLayout) const;

    void copyBufferToImage(vk::Buffer buffer, vk::Image image,
                            uint32_t width, uint32_t height) const;

    [[nodiscard]] SingleTimeCommandContext beginSingleTimeCommands() const;
    void endSingleTimeCommands(SingleTimeCommandContext& cmdCtx) const;

private:
    uint32_t findMemoryType(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties) const;

    CoreCtx& m_ctx;
};

} // namespace nuff::renderer
