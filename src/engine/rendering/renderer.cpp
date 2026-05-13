#include "renderer.h"

#include "primitives/push_constants.h"
#include "primitives/uniform_buffer_object.h"
#include "utils/image_utils.h"

#include <cstring>
#include <utility>

namespace nuff::renderer {

void Renderer::setContext(CoreCtx* ctx) {
    m_ctx = ctx;
}

void Renderer::setRenderTarget(IRenderTarget* renderTarget) {
    m_renderTarget = renderTarget;
}

void Renderer::setRecreateCallback(RecreateCallback callback) {
    m_recreateCallback = std::move(callback);
}

void Renderer::notifyFramebufferResized() {
    m_framebufferResized = true;
}

void Renderer::initialize() {
    auto& pipeline = m_ctx->component<PipelineCtxComponent>();
    m_renderTarget->initFrameResources(pipeline.descriptorSetLayout,
                                        sizeof(UniformBufferObject));
}

void Renderer::cleanup() {
    if (!m_ctx) return;
    m_ctx->device.waitIdle();
    if (m_renderTarget) m_renderTarget->cleanupFrameResources();
}

void Renderer::drawFrame(const FrameView& view) {
    auto beginResult = m_renderTarget->beginFrame();
    if (beginResult == IRenderTarget::FrameResult::Recreate) {
        if (m_recreateCallback) m_recreateCallback();
        return;
    }

    recordRendering(view);

    auto endResult = m_renderTarget->endFrame();
    if (endResult == IRenderTarget::FrameResult::Recreate || m_framebufferResized) {
        m_framebufferResized = false;
        if (m_recreateCallback) m_recreateCallback();
    }
}

void Renderer::recordRendering(const FrameView& view) {
    auto& pipeline = m_ctx->component<PipelineCtxComponent>();
    auto& cmd = m_renderTarget->commandBuffer();
    auto targetImage = m_renderTarget->image();
    auto targetExtent = m_renderTarget->extent();

    vk::CommandBufferBeginInfo beginInfo{};
    cmd.begin(beginInfo);

    auto preRenderBarrier = utils::createImageTransitionInfo(
        targetImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );
    cmd.pipelineBarrier2(preRenderBarrier.dependencyInfo);

    vk::ImageMemoryBarrier2 depthBarrier{
        .srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests
            | vk::PipelineStageFlagBits2::eLateFragmentTests,
        .srcAccessMask = {},
        .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests
            | vk::PipelineStageFlagBits2::eLateFragmentTests,
        .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_renderTarget->depthImage(),
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vk::DependencyInfo depthDepInfo{
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &depthBarrier
    };
    cmd.pipelineBarrier2(depthDepInfo);

    vk::ClearValue clearColor{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.9f}}};
    vk::ClearValue clearDepth{vk::ClearDepthStencilValue{.depth = 1.0f, .stencil = 0}};

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_renderTarget->imageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingAttachmentInfo depthAttachmentInfo = {
        .imageView = m_renderTarget->depthImageView(),
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepth
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = {0, 0}, .extent = targetExtent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo
    };

    cmd.beginRendering(renderingInfo);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);

    cmd.setViewport(0, vk::Viewport{
        .width = static_cast<float>(targetExtent.width),
        .height = static_cast<float>(targetExtent.height),
        .maxDepth = 1.0f
    });
    cmd.setScissor(0, vk::Rect2D{.extent = targetExtent});

    UniformBufferObject ubo{};
    ubo.view = view.view;
    ubo.proj = view.proj;
    std::memcpy(m_renderTarget->currentUniformBufferMapping(), &ubo, sizeof(ubo));

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           *pipeline.pipelineLayout, 0,
                           m_renderTarget->currentDescriptorSet(), nullptr);

    for (const auto& item : view.items) {
        if (!item.mesh) continue;
        ObjectPushConstantData pc{ .model = item.model, .time = view.time };
        cmd.pushConstants<ObjectPushConstantData>(*pipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, pc);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               *pipeline.pipelineLayout, 1,
                               item.materialSet, nullptr);
        item.mesh->bind(cmd);
        item.mesh->draw(cmd);
    }

    cmd.endRendering();

    auto postRenderBarrier = utils::createImageTransitionInfo(
        targetImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        m_renderTarget->finalLayout(),
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );
    cmd.pipelineBarrier2(postRenderBarrier.dependencyInfo);

    cmd.end();
}

} // namespace nuff::renderer
