#include "core/memory/memory_manager.h"

#include <stdexcept>

namespace nuff::renderer {

MemoryManager::MemoryManager(CoreCtx& ctx)
    : m_ctx(ctx) {}

AllocatedBuffer MemoryManager::createBuffer(vk::DeviceSize size,
                                             vk::BufferUsageFlags usage,
                                             vk::MemoryPropertyFlags memoryProperties) const {
    AllocatedBuffer result;

    vk::BufferCreateInfo bufferInfo{
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive
    };
    result.buffer = vk::raii::Buffer(m_ctx.device, bufferInfo);

    auto memReqs = result.buffer.getMemoryRequirements();
    uint32_t memType = findMemoryType(memReqs.memoryTypeBits, memoryProperties);

    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memType
    };
    result.memory = vk::raii::DeviceMemory(m_ctx.device, allocInfo);
    result.buffer.bindMemory(*result.memory, 0);

    return result;
}

void MemoryManager::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const {
    auto& graphics = m_ctx.extension<GraphicsCtxMixin>();

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = graphics.queueFamilyIndices.graphicsFamily.value()
    };
    vk::raii::CommandPool cmdPool(m_ctx.device, poolInfo);

    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = *cmdPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffers cmdBuffers(m_ctx.device, cmdAllocInfo);
    auto& cmd = cmdBuffers[0];

    cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmd.copyBuffer(src, dst, vk::BufferCopy{.size = size});
    cmd.end();

    vk::CommandBuffer rawCmd = *cmd;
    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &rawCmd
    };
    graphics.graphicsQueue.submit(submitInfo);
    graphics.graphicsQueue.waitIdle();
}

uint32_t MemoryManager::findMemoryType(uint32_t typeFilter,
                                        vk::MemoryPropertyFlags properties) const {
    auto memProps = m_ctx.physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

} // namespace nuff::renderer
