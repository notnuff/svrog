#include "presentation/frame_resources.h"

namespace nuff::renderer {

void FrameResources::init(CoreCtx& ctx,
                           const vk::raii::DescriptorSetLayout& layout,
                           vk::DeviceSize uboSize,
                           uint32_t frameCount) {
    m_uboSize = uboSize;

    MemoryManager memMgr(ctx);

    m_frames.resize(frameCount);
    for (uint32_t i = 0; i < frameCount; i++) {
        auto buf = memMgr.createBuffer(
            uboSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        m_frames[i].uniformBuffer = std::move(buf.buffer);
        m_frames[i].uniformBufferMemory = std::move(buf.memory);
        m_frames[i].mappedMemory = m_frames[i].uniformBufferMemory.mapMemory(0, uboSize);
    }

    vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = frameCount
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = frameCount,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    m_descriptorPool = vk::raii::DescriptorPool(ctx.device, poolInfo);

    std::vector<vk::DescriptorSetLayout> layouts(frameCount, *layout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = *m_descriptorPool,
        .descriptorSetCount = frameCount,
        .pSetLayouts = layouts.data()
    };

    auto sets = ctx.device.allocateDescriptorSets(allocInfo);
    m_descriptorSets.clear();
    m_descriptorSets.reserve(frameCount);
    for (auto& s : sets) {
        m_descriptorSets.push_back(std::move(s));
    }

    for (uint32_t i = 0; i < frameCount; i++) {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *m_frames[i].uniformBuffer,
            .offset = 0,
            .range = uboSize
        };

        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = *m_descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo
        };
        ctx.device.updateDescriptorSets(descriptorWrite, nullptr);
    }
}

void FrameResources::cleanup(CoreCtx& ctx) {
    (void)ctx;
    for (auto& ds : m_descriptorSets) {
        ds.release();
    }
    m_descriptorSets.clear();
    m_descriptorPool = nullptr;
    for (auto& frame : m_frames) {
        if (frame.mappedMemory) {
            frame.uniformBufferMemory.unmapMemory();
            frame.mappedMemory = nullptr;
        }
        frame.uniformBuffer = nullptr;
        frame.uniformBufferMemory = nullptr;
    }
    m_frames.clear();
}

vk::DescriptorSet FrameResources::descriptorSet(uint32_t frameIndex) const {
    return *m_descriptorSets[frameIndex];
}

void* FrameResources::uniformBufferMapping(uint32_t frameIndex) const {
    return m_frames[frameIndex].mappedMemory;
}

} // namespace nuff::renderer
