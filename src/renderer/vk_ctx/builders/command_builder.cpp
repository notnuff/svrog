#include "vk_ctx/builders/command_builder.h"

namespace nuff::renderer {

void CommandBuilder::build(VkCtx& ctx) {
    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        ctx.queueFamilyIndices.graphicsFamily.value()
    };

    ctx.commandPool = ctx.device.createCommandPool(poolInfo);

    ctx.commandBuffers.resize(VkCtx::MAX_FRAMES_IN_FLIGHT);
    vk::CommandBufferAllocateInfo allocInfo{
        ctx.commandPool,
        vk::CommandBufferLevel::ePrimary,
        static_cast<uint32_t>(ctx.commandBuffers.size())
    };

    ctx.commandBuffers = ctx.device.allocateCommandBuffers(allocInfo);

    qCInfo(logger()) << "Command pool and" << ctx.commandBuffers.size() << "command buffers created";
}

} // namespace nuff::renderer

