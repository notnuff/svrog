#include "renderer.h"

#include "utils/image_utils.h"

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

void Renderer::stopAndWait() const {
    m_ctx->device.waitIdle();
}

void Renderer::drawFrame() {
    auto beginResult = m_renderTarget->beginFrame();
    if (beginResult == IRenderTarget::FrameResult::Recreate) {
        if (m_recreateCallback) m_recreateCallback();
        return;
    }

    recordRendering();

    auto endResult = m_renderTarget->endFrame();
    if (endResult == IRenderTarget::FrameResult::Recreate || m_framebufferResized) {
        m_framebufferResized = false;
        if (m_recreateCallback) m_recreateCallback();
    }
}

void Renderer::recordRendering() {
    auto& pipeline = m_ctx->extension<PipelineCtxMixin>();
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

    vk::ClearValue clearColor{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.9f}}};

    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_renderTarget->imageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = {
            .offset = {0, 0},
            .extent = targetExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    cmd.beginRendering(renderingInfo);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.graphicsPipeline);

    cmd.setViewport(0, vk::Viewport{
        .width = static_cast<float>(targetExtent.width),
        .height = static_cast<float>(targetExtent.height),
        .maxDepth = 1.0f
    });
    cmd.setScissor(0, vk::Rect2D{.extent = targetExtent});

    cmd.draw(3, 1, 0, 0);
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
