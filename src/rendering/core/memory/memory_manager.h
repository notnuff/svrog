#pragma once

#include "core/context/ctx.h"

namespace nuff::renderer {

struct AllocatedBuffer {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

class MemoryManager {
public:
    explicit MemoryManager(CoreCtx& ctx);

    AllocatedBuffer createBuffer(vk::DeviceSize size,
                                 vk::BufferUsageFlags usage,
                                 vk::MemoryPropertyFlags memoryProperties) const;

    void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size) const;

private:
    uint32_t findMemoryType(uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties) const;

    CoreCtx& m_ctx;
};

} // namespace nuff::renderer
