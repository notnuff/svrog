#pragma once

#include "common/vk_common.h"
#include "primitives/vertex.h"
#include "core/memory/memory_manager.h"

#include <vector>
#include <cstring>

namespace nuff::renderer {

class Mesh {
public:
    Mesh() = default;

    void uploadVertices(MemoryManager& memoryManager, const std::vector<Vertex>& vertices) {
        vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();
        m_vertexCount = static_cast<uint32_t>(vertices.size());

        auto staging = memoryManager.createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = staging.memory.mapMemory(0, bufferSize);
        std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        staging.memory.unmapMemory();

        auto vertexBuf = memoryManager.createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_vertexBuffer = std::move(vertexBuf.buffer);
        m_vertexBufferMemory = std::move(vertexBuf.memory);

        memoryManager.copyBuffer(*staging.buffer, *m_vertexBuffer, bufferSize);
    }

    void uploadIndices(MemoryManager& memoryManager, const std::vector<uint32_t>& indices) {
        vk::DeviceSize bufferSize = sizeof(uint32_t) * indices.size();
        m_indexCount = static_cast<uint32_t>(indices.size());

        auto staging = memoryManager.createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = staging.memory.mapMemory(0, bufferSize);
        std::memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        staging.memory.unmapMemory();

        auto indexBuf = memoryManager.createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

        m_indexBuffer = std::move(indexBuf.buffer);
        m_indexBufferMemory = std::move(indexBuf.memory);

        memoryManager.copyBuffer(*staging.buffer, *m_indexBuffer, bufferSize);
    }

    void bind(const vk::raii::CommandBuffer& cmd) const {
        vk::Buffer vertexBuffers[] = {*m_vertexBuffer};
        vk::DeviceSize offsets[] = {0};
        cmd.bindVertexBuffers(0, vertexBuffers, offsets);
        cmd.bindIndexBuffer(*m_indexBuffer, 0, vk::IndexType::eUint32);
    }

    void draw(const vk::raii::CommandBuffer& cmd) const {
        cmd.drawIndexed(m_indexCount, 1, 0, 0, 0);
    }

    void cleanup() {
        m_vertexBuffer = nullptr;
        m_vertexBufferMemory = nullptr;
        m_vertexCount = 0;
        m_indexBuffer = nullptr;
        m_indexBufferMemory = nullptr;
        m_indexCount = 0;
    }

    [[nodiscard]] uint32_t vertexCount() const { return m_vertexCount; }
    [[nodiscard]] uint32_t indexCount() const { return m_indexCount; }

private:
    vk::raii::Buffer m_vertexBuffer{nullptr};
    vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};
    uint32_t m_vertexCount = 0;

    vk::raii::Buffer m_indexBuffer{nullptr};
    vk::raii::DeviceMemory m_indexBufferMemory{nullptr};
    uint32_t m_indexCount = 0;
};

} // namespace nuff::renderer
