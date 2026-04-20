#include "offscreen_render_target.h"

#include <stdexcept>

namespace nuff::renderer {

OffscreenRenderTarget::OffscreenRenderTarget(CoreCtx* ctx, uint32_t width, uint32_t height,
                                             vk::Format format, bool exportable)
    : m_ctx(ctx), m_width(width), m_height(height), m_format(format), m_exportable(exportable)
{
    createResources();
}

void OffscreenRenderTarget::resize(uint32_t width, uint32_t height) {
    m_ctx->device.waitIdle();
    m_width = width;
    m_height = height;
    createResources();
}

void OffscreenRenderTarget::createResources() {
    auto& graphics = m_ctx->extension<GraphicsCtxMixin>();

    vk::ExternalMemoryImageCreateInfo extMemImageInfo{
        .handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd
    };

    vk::ImageCreateInfo imageInfo{
        .pNext = m_exportable ? &extMemImageInfo : nullptr,
        .imageType = vk::ImageType::e2D,
        .format = m_format,
        .extent = {m_width, m_height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };
    m_image = vk::raii::Image(m_ctx->device, imageInfo);

    auto memReqs = m_image.getMemoryRequirements();
    auto memProps = m_ctx->physicalDevice.getMemoryProperties();

    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            memTypeIndex = i;
            break;
        }
    }
    if (memTypeIndex == UINT32_MAX) {
        throw std::runtime_error("Failed to find suitable memory type for offscreen image");
    }

    vk::MemoryDedicatedAllocateInfo dedicatedAllocInfo{
        .image = *m_image
    };

    vk::ExportMemoryAllocateInfo exportAllocInfo{
        .pNext = &dedicatedAllocInfo,
        .handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd
    };

    vk::MemoryAllocateInfo allocInfo{
        .pNext = m_exportable ? &exportAllocInfo : nullptr,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memTypeIndex
    };
    m_memorySize = memReqs.size;
    m_imageMemory = vk::raii::DeviceMemory(m_ctx->device, allocInfo);
    m_image.bindMemory(*m_imageMemory, 0);

    vk::ImageViewCreateInfo viewInfo{
        .image = *m_image,
        .viewType = vk::ImageViewType::e2D,
        .format = m_format,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    m_imageView = vk::raii::ImageView(m_ctx->device, viewInfo);

    vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = graphics.queueFamilyIndices.graphicsFamily.value()
    };
    m_commandPool = vk::raii::CommandPool(m_ctx->device, poolInfo);

    vk::CommandBufferAllocateInfo cmdAllocInfo{
        .commandPool = *m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    m_commandBuffers = vk::raii::CommandBuffers(m_ctx->device, cmdAllocInfo);

    vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled
    };
    m_fence = vk::raii::Fence(m_ctx->device, fenceInfo);
}

int OffscreenRenderTarget::getMemoryFd() const {
    if (!m_exportable) {
        throw std::runtime_error("OffscreenRenderTarget: not created with exportable flag");
    }

    vk::MemoryGetFdInfoKHR getFdInfo{
        .memory = *m_imageMemory,
        .handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd
    };

    return m_ctx->device.getMemoryFdKHR(getFdInfo);
}

IRenderTarget::FrameResult OffscreenRenderTarget::beginFrame() {
    auto waitResult = m_ctx->device.waitForFences({*m_fence}, vk::True, UINT64_MAX);
    (void)waitResult;
    m_ctx->device.resetFences({*m_fence});
    m_commandBuffers[0].reset();
    return FrameResult::Success;
}

IRenderTarget::FrameResult OffscreenRenderTarget::endFrame() {
    auto& graphics = m_ctx->extension<GraphicsCtxMixin>();

    vk::CommandBuffer cmdBuf = *m_commandBuffers[0];
    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuf
    };
    graphics.graphicsQueue.submit(submitInfo, *m_fence);

    auto waitResult = m_ctx->device.waitForFences({*m_fence}, vk::True, UINT64_MAX);
    (void)waitResult;

    return FrameResult::Success;
}

const vk::raii::CommandBuffer& OffscreenRenderTarget::commandBuffer() const {
    return m_commandBuffers[0];
}

const vk::raii::ImageView& OffscreenRenderTarget::imageView() const {
    return m_imageView;
}

vk::Image OffscreenRenderTarget::image() const {
    return *m_image;
}

vk::Extent2D OffscreenRenderTarget::extent() const {
    return {m_width, m_height};
}

vk::Format OffscreenRenderTarget::format() const {
    return m_format;
}

vk::ImageLayout OffscreenRenderTarget::finalLayout() const {
    return vk::ImageLayout::eShaderReadOnlyOptimal;
}

} // namespace nuff::renderer

