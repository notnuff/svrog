#include "primitives/texture_image.h"

#include <QImage>
#include <cstring>
#include <stdexcept>

namespace nuff::renderer {

void TextureImage::loadFromFile(CoreCtx& ctx, MemoryManager& memoryManager,
                                 const std::string& filePath) {
    QImage image(QString::fromStdString(filePath));
    if (image.isNull()) {
        throw std::runtime_error("Failed to load texture image: " + filePath);
    }

    image = image.convertToFormat(QImage::Format_RGBA8888);

    auto texWidth = static_cast<uint32_t>(image.width());
    auto texHeight = static_cast<uint32_t>(image.height());
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    auto staging = memoryManager.createBuffer(
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = staging.memory.mapMemory(0, imageSize);
    std::memcpy(data, image.constBits(), static_cast<size_t>(imageSize));
    staging.memory.unmapMemory();

    auto allocatedImage = memoryManager.createImage(
        texWidth, texHeight,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    m_image = std::move(allocatedImage.image);
    m_imageMemory = std::move(allocatedImage.memory);

    memoryManager.transitionImageLayout(*m_image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    memoryManager.copyBufferToImage(*staging.buffer, *m_image, texWidth, texHeight);

    memoryManager.transitionImageLayout(*m_image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    // Create image view
    vk::ImageViewCreateInfo viewInfo{
        .image = *m_image,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    m_imageView = vk::raii::ImageView(ctx.device, viewInfo);

    // Create sampler
    auto physProps = ctx.physicalDevice.getProperties();

    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::False,
        .maxAnisotropy = 1.0f,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = vk::False
    };
    m_sampler = vk::raii::Sampler(ctx.device, samplerInfo);
}

void TextureImage::cleanup() {
    m_sampler = nullptr;
    m_imageView = nullptr;
    m_image = nullptr;
    m_imageMemory = nullptr;
}

vk::DescriptorImageInfo TextureImage::descriptorInfo() const {
    return {
        .sampler = *m_sampler,
        .imageView = *m_imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
}

} // namespace nuff::renderer
