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

SingleTimeCommandContext MemoryManager::beginSingleTimeCommands() const {
    auto& graphics = m_ctx.extension<GraphicsCtxMixin>();

    SingleTimeCommandContext cmdCtx;

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = graphics.queueFamilyIndices.graphicsFamily.value()
    };
    cmdCtx.commandPool = vk::raii::CommandPool(m_ctx.device, poolInfo);

    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = *cmdCtx.commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    cmdCtx.commandBuffers = vk::raii::CommandBuffers(m_ctx.device, cmdAllocInfo);

    cmdCtx.cmd().begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    return cmdCtx;
}

void MemoryManager::endSingleTimeCommands(SingleTimeCommandContext& cmdCtx) const {
    auto& graphics = m_ctx.extension<GraphicsCtxMixin>();

    cmdCtx.cmd().end();

    vk::CommandBuffer rawCmd = *cmdCtx.cmd();
    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &rawCmd
    };
    graphics.graphicsQueue.submit(submitInfo);
    graphics.graphicsQueue.waitIdle();
}

void MemoryManager::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const {
    auto cmdCtx = beginSingleTimeCommands();
    cmdCtx.cmd().copyBuffer(src, dst, vk::BufferCopy{.size = size});
    endSingleTimeCommands(cmdCtx);
}

AllocatedImage MemoryManager::createImage(uint32_t width, uint32_t height,
                                           vk::Format format,
                                           vk::ImageTiling tiling,
                                           vk::ImageUsageFlags usage,
                                           vk::MemoryPropertyFlags memoryProperties) const {
    AllocatedImage result;

    vk::ImageCreateInfo imageInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };
    result.image = vk::raii::Image(m_ctx.device, imageInfo);

    auto memReqs = result.image.getMemoryRequirements();
    uint32_t memType = findMemoryType(memReqs.memoryTypeBits, memoryProperties);

    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memType
    };
    result.memory = vk::raii::DeviceMemory(m_ctx.device, allocInfo);
    result.image.bindMemory(*result.memory, 0);

    return result;
}

void MemoryManager::transitionImageLayout(vk::Image image,
                                           vk::ImageLayout oldLayout,
                                           vk::ImageLayout newLayout) const {
    auto cmdCtx = beginSingleTimeCommands();

    vk::AccessFlags2 srcAccess{};
    vk::AccessFlags2 dstAccess{};
    vk::PipelineStageFlags2 srcStage{};
    vk::PipelineStageFlags2 dstStage{};

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        dstAccess = vk::AccessFlagBits2::eTransferWrite;
        srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits2::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        srcAccess = vk::AccessFlagBits2::eTransferWrite;
        dstAccess = vk::AccessFlagBits2::eShaderRead;
        srcStage = vk::PipelineStageFlagBits2::eTransfer;
        dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    vk::ImageMemoryBarrier2 barrier{
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vk::DependencyInfo depInfo{
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    cmdCtx.cmd().pipelineBarrier2(depInfo);

    endSingleTimeCommands(cmdCtx);
}

void MemoryManager::copyBufferToImage(vk::Buffer buffer, vk::Image image,
                                       uint32_t width, uint32_t height) const {
    auto cmdCtx = beginSingleTimeCommands();

    vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1}
    };
    cmdCtx.cmd().copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);

    endSingleTimeCommands(cmdCtx);
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
