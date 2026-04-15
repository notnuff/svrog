#include "command_builder.h"

namespace nuff::renderer {

void CommandBuilder::build(CoreCtx& ctx) {
    auto& graphics = ctx.extension<GraphicsCtxMixin>();
    auto& command = ctx.extension<CommandCtxMixin>();
    auto& present = ctx.extension<PresentCtxMixin>();

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphics.queueFamilyIndices.graphicsFamily.value()
    };

    command.commandPool = vk::raii::CommandPool(ctx.device, poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = *command.commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = present.maxFramesInFlight
    };

    command.commandBuffers = vk::raii::CommandBuffers(ctx.device, allocInfo);

    qCInfo(logger()) << "Command pool and" << command.commandBuffers.size() << "command buffers created";
}

} // namespace nuff::renderer

