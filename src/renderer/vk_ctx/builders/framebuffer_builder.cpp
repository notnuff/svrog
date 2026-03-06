#include "vk_ctx/builders/framebuffer_builder.h"

namespace nuff::renderer {

void FramebufferBuilder::build(VkCtx& ctx) {
    ctx.framebuffers.resize(ctx.swapchainImageViews.size());

    for (size_t i = 0; i < ctx.swapchainImageViews.size(); i++) {
        vk::ImageView attachments[] = {ctx.swapchainImageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{
            {},
            ctx.renderPass,
            1, attachments,
            ctx.swapchainExtent.width,
            ctx.swapchainExtent.height,
            1
        };

        ctx.framebuffers[i] = ctx.device.createFramebuffer(framebufferInfo);
    }

    qCInfo(logger()) << ctx.framebuffers.size() << "framebuffers created";
}

} // namespace nuff::renderer

