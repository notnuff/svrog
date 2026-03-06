#include "vk_ctx/builders/render_pass_builder.h"

namespace nuff::renderer {

void RenderPassBuilder::build(VkCtx& ctx) {
    vk::AttachmentDescription colorAttachment{
        {},
        ctx.swapchainImageFormat,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    };

    vk::AttachmentReference colorAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorAttachmentRef
    };

    vk::SubpassDependency dependency{
        VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite
    };

    vk::RenderPassCreateInfo renderPassInfo{
        {},
        1, &colorAttachment,
        1, &subpass,
        1, &dependency
    };

    ctx.renderPass = ctx.device.createRenderPass(renderPassInfo);
    qCInfo(logger()) << "Render pass created";
}

} // namespace nuff::renderer

