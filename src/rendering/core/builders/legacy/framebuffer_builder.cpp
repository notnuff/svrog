#include "framebuffer_builder.h"

namespace nuff::renderer {

void FramebufferBuilder::build(CoreCtx& ctx) {
    auto& swapchain = ctx.extension<SwapchainCtxMixin>();
    auto& rp = ctx.extension<RenderPassCtxMixin>();

    rp.framebuffers.clear();
    rp.framebuffers.reserve(swapchain.swapchainImageViews.size());

    for (size_t i = 0; i < swapchain.swapchainImageViews.size(); i++) {
        vk::ImageView attachments[] = {*swapchain.swapchainImageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{
            .renderPass = *rp.renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain.swapchainExtent.width,
            .height = swapchain.swapchainExtent.height,
            .layers = 1
        };

        rp.framebuffers.emplace_back(ctx.device, framebufferInfo);
    }

    qCInfo(logger()) << rp.framebuffers.size() << "framebuffers created";
}

} // namespace nuff::renderer

