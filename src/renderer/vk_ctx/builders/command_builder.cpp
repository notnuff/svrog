#include "vk_ctx/builders/command_builder.h"

namespace nuff::renderer {

void CommandBuilder::build(VkCtx& ctx) {
    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        ctx.queueFamilyIndices.graphicsFamily.value()
    };

    ctx.commandPool = vk::raii::CommandPool(ctx.device, poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        *ctx.commandPool,
        vk::CommandBufferLevel::ePrimary,
        VkCtx::MAX_FRAMES_IN_FLIGHT
    };

    ctx.commandBuffers = vk::raii::CommandBuffers(ctx.device, allocInfo);

    qCInfo(logger()) << "Command pool and" << ctx.commandBuffers.size() << "command buffers created";
}

} // namespace nuff::renderer

