#include "renderer.h"

#include "assets/gltf_loader.h"
#include "primitives/push_constants.h"
#include "primitives/uniform_buffer_object.h"
#include "utils/image_utils.h"

#include <cstring>
#include <limits>
#include <utility>

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

void Renderer::setModelPath(const std::string& path) {
    m_modelPath = path;
}

void Renderer::notifyFramebufferResized() {
    m_framebufferResized = true;
}

void Renderer::updateUniformBuffer() {
    auto extent = m_renderTarget->extent();
    float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    glm::vec3 target = m_sceneRadius > 0.0f ? m_sceneCentroid : glm::vec3(0.0f);
    glm::vec3 eye    = m_sceneRadius > 0.0f
        ? target + glm::normalize(glm::vec3(1.0f, 1.0f, 0.6f)) * (m_sceneRadius * 2.5f)
        : glm::vec3(2.0f, 2.0f, 2.0f);
    float farPlane   = m_sceneRadius > 0.0f ? m_sceneRadius * 20.0f : 100.0f;

    UniformBufferObject ubo{};
    ubo.view = glm::lookAt(eye, target, glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, farPlane);
    ubo.proj[1][1] *= -1; // flip Y for Vulkan

    std::memcpy(m_renderTarget->currentUniformBufferMapping(), &ubo, sizeof(ubo));
}

void Renderer::loadFallbackGeometry() {
    MeshData data;
    data.vertices = {
        {{-0.6f, -0.6f,  0.3f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.6f, -0.6f,  0.3f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.6f,  0.6f,  0.3f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.6f,  0.6f,  0.3f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

        {{-0.3f, -0.2f, -0.3f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.9f, -0.2f, -0.3f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.9f,  1.0f, -0.3f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.3f,  1.0f, -0.3f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    };
    data.indices = {
        0, 1, 2,  0, 2, 3,
        4, 5, 6,  4, 6, 7,
    };

    const unsigned char white[4] = {255, 255, 255, 255};
    RenderObject obj;
    obj.mesh.upload(*m_memoryManager, data);
    obj.texture.loadFromPixels(*m_ctx, *m_memoryManager, white, 1, 1);
    m_objects.push_back(std::move(obj));
}

void Renderer::loadModel(const std::string& path) {
    GltfLoader loader;
    auto loaded = loader.load(path);
    m_objects.reserve(m_objects.size() + loaded.nodes.size());

    glm::vec3 mn(std::numeric_limits<float>::max());
    glm::vec3 mx(std::numeric_limits<float>::lowest());
    for (auto& n : loaded.nodes) {
        for (const auto& v : n.mesh.vertices) {
            glm::vec3 p = glm::vec3(n.transform * glm::vec4(v.pos, 1.0f));
            mn = glm::min(mn, p);
            mx = glm::max(mx, p);
        }
        RenderObject obj;
        obj.mesh.upload(*m_memoryManager, n.mesh);
        obj.transform = n.transform;
        obj.texture.loadFromPixels(*m_ctx, *m_memoryManager,
                                    n.material.pixels.data(),
                                    n.material.width, n.material.height);
        m_objects.push_back(std::move(obj));
    }

    if (!loaded.nodes.empty()) {
        m_sceneCentroid = (mn + mx) * 0.5f;
        m_sceneRadius   = glm::length(mx - mn) * 0.5f;
    }
}

void Renderer::buildMaterialDescriptors() {
    auto& pipeline = m_ctx->component<PipelineCtxComponent>();
    auto setCount = static_cast<uint32_t>(m_objects.size());

    vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = setCount
    };
    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = setCount,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    m_materialPool = vk::raii::DescriptorPool(m_ctx->device, poolInfo);

    std::vector<vk::DescriptorSetLayout> layouts(setCount, *pipeline.materialSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = *m_materialPool,
        .descriptorSetCount = setCount,
        .pSetLayouts = layouts.data()
    };
    auto sets = m_ctx->device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < m_objects.size(); ++i) {
        auto info = m_objects[i].texture.descriptorInfo();
        vk::WriteDescriptorSet write{
            .dstSet = *sets[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &info
        };
        m_ctx->device.updateDescriptorSets(write, nullptr);
        m_objects[i].materialSet = *sets[i];
        sets[i].release();
    }
}

void Renderer::initialize() {
    if (m_modelPath.empty()) {
        loadFallbackGeometry();
    } else {
        loadModel(m_modelPath);
        if (m_objects.empty()) loadFallbackGeometry();
    }

    buildMaterialDescriptors();

    auto& pipeline = m_ctx->component<PipelineCtxComponent>();
    m_renderTarget->initFrameResources(pipeline.descriptorSetLayout,
                                        sizeof(UniformBufferObject));
}

void Renderer::cleanup() {
    m_ctx->device.waitIdle();
    m_renderTarget->cleanupFrameResources();
    for (auto& obj : m_objects) {
        obj.materialSet = nullptr;
        obj.texture.cleanup();
        obj.mesh.cleanup();
    }
    m_objects.clear();
    m_materialPool = nullptr;
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
        .renderArea = {
            .offset = {0, 0},
            .extent = targetExtent
        },
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

    updateUniformBuffer();
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                           *pipeline.pipelineLayout, 0,
                           m_renderTarget->currentDescriptorSet(), nullptr);

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - m_startTime).count();
    float angle = elapsed * glm::radians(30.0f);
    glm::mat4 spin = m_sceneRadius > 0.0f
        ? glm::translate(glm::mat4(1.0f), m_sceneCentroid)
          * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f))
          * glm::translate(glm::mat4(1.0f), -m_sceneCentroid)
        : glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));

    for (const auto& obj : m_objects) {
        ObjectPushConstantData pc{
            .model = spin * obj.transform,
            .time  = elapsed
        };
        cmd.pushConstants<ObjectPushConstantData>(*pipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, pc);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               *pipeline.pipelineLayout, 1,
                               obj.materialSet, nullptr);
        obj.mesh.bind(cmd);
        obj.mesh.draw(cmd);
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
