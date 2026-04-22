#pragma once

#include "core/context/ctx.h"
#include "core/memory/memory_manager.h"

#include <vector>

namespace nuff::renderer {

class FrameResources {
public:
    void init(CoreCtx& ctx,
              const vk::raii::DescriptorSetLayout& layout,
              vk::DeviceSize uboSize,
              uint32_t frameCount);

    void cleanup(CoreCtx& ctx);

    [[nodiscard]] vk::DescriptorSet descriptorSet(uint32_t frameIndex) const;
    [[nodiscard]] void* uniformBufferMapping(uint32_t frameIndex) const;

private:
    struct PerFrame {
        vk::raii::Buffer uniformBuffer{nullptr};
        vk::raii::DeviceMemory uniformBufferMemory{nullptr};
        void* mappedMemory = nullptr;
    };

    vk::raii::DescriptorPool m_descriptorPool{nullptr};
    std::vector<vk::raii::DescriptorSet> m_descriptorSets;
    std::vector<PerFrame> m_frames;
    vk::DeviceSize m_uboSize = 0;
};

} // namespace nuff::renderer
