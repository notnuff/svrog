#include "renderer.h"

#include "primitives/push_constants.h"
#include "primitives/uniform_buffer_object.h"
#include "utils/image_utils.h"

#include <cstring>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace nuff::renderer {

void Renderer::setContext(CoreCtx* ctx) {
    m_ctx = ctx;
    m_memoryManager = std::make_unique<MemoryManager>(*ctx);
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

void Renderer::uploadVertices(const std::vector<Vertex>& vertices) {
    vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();
    m_vertexCount = static_cast<uint32_t>(vertices.size());

    auto staging = m_memoryManager->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging.memory.mapMemory(0, bufferSize);
    std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    staging.memory.unmapMemory();

    auto vertexBuf = m_memoryManager->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    m_vertexBuffer = std::move(vertexBuf.buffer);
    m_vertexBufferMemory = std::move(vertexBuf.memory);

    m_memoryManager->copyBuffer(*staging.buffer, *m_vertexBuffer, bufferSize);
}

void Renderer::uploadIndices(const std::vector<uint32_t>& indices) {
    vk::DeviceSize bufferSize = sizeof(uint32_t) * indices.size();
    m_indexCount = static_cast<uint32_t>(indices.size());

    auto staging = m_memoryManager->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging.memory.mapMemory(0, bufferSize);
    std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    staging.memory.unmapMemory();

    auto indexBuf = m_memoryManager->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    m_indexBuffer = std::move(indexBuf.buffer);
    m_indexBufferMemory = std::move(indexBuf.memory);

    m_memoryManager->copyBuffer(*staging.buffer, *m_indexBuffer, bufferSize);
}

void Renderer::updateUniformBuffer() {
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_startTime).count();

    auto extent = m_renderTarget->extent();
    float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1; // flip Y for Vulkan

    std::memcpy(m_renderTarget->currentUniformBufferMapping(), &ubo, sizeof(ubo));
}

void Renderer::initialize() {
    uploadVertices({
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}},
    });

    uploadIndices({
        0, 1, 2,
        2, 3, 0
    });

    auto& pipeline = m_ctx->extension<PipelineCtxMixin>();
    m_renderTarget->initFrameResources(pipeline.descriptorSetLayout, sizeof(UniformBufferObject));
}

void Renderer::cleanup() {
    m_ctx->device.waitIdle();
    m_renderTarget->cleanupFrameResources();
    m_vertexBuffer = nullptr;
    m_vertexBufferMemory = nullptr;
    m_vertexCount = 0;
    m_indexBuffer = nullptr;
    m_indexBufferMemory = nullptr;
    m_indexCount = 0;
    m_memoryManager.reset();
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

    updateUniformBuffer();
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           *pipeline.pipelineLayout, 0,
                           m_renderTarget->currentDescriptorSet(), nullptr);

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_startTime).count();
    TimePushConstantData pushData{.time = elapsed};
    cmd.pushConstants<TimePushConstantData>(*pipeline.pipelineLayout,
                                            vk::ShaderStageFlagBits::eFragment,
                                            0, pushData);

    vk::Buffer vertexBuffers[] = {*m_vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    cmd.bindVertexBuffers(0, vertexBuffers, offsets);
    cmd.bindIndexBuffer(*m_indexBuffer, 0, vk::IndexType::eUint32);

    cmd.drawIndexed(m_indexCount, 1, 0, 0, 0);
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
