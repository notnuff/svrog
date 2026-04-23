#pragma once

#include "common/vk_common.h"
#include "core/memory/memory_manager.h"

#include <string>

namespace nuff::renderer {

class TextureImage {
public:
    TextureImage() = default;

    void loadFromFile(CoreCtx& ctx, MemoryManager& memoryManager, const std::string& filePath);

    void cleanup();

    [[nodiscard]] vk::DescriptorImageInfo descriptorInfo() const;

private:
    vk::raii::Image m_image{nullptr};
    vk::raii::DeviceMemory m_imageMemory{nullptr};
    vk::raii::ImageView m_imageView{nullptr};
    vk::raii::Sampler m_sampler{nullptr};
};

} // namespace nuff::renderer
