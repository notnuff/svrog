#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include "core/context/ctx.h"
#include "core/memory/memory_manager.h"
#include "primitives/vertex.h"
#include "presentation/i_render_target.h"

namespace nuff::renderer {

class Renderer {
public:
    using RecreateCallback = std::function<void()>;

    void setContext(CoreCtx* ctx);
    void setRenderTarget(IRenderTarget* renderTarget);
    void setRecreateCallback(RecreateCallback callback);

    void notifyFramebufferResized();

    void initialize();
    void cleanup();

    void drawFrame();

private:
    void recordRendering();
    void uploadVertices(const std::vector<Vertex>& vertices);
    void uploadIndices(const std::vector<uint32_t>& indices);
    void updateUniformBuffer();

    CoreCtx* m_ctx = nullptr;
    IRenderTarget* m_renderTarget = nullptr;
    RecreateCallback m_recreateCallback;
    std::unique_ptr<MemoryManager> m_memoryManager;
    bool m_framebufferResized = false;

    vk::raii::Buffer m_vertexBuffer{nullptr};
    vk::raii::DeviceMemory m_vertexBufferMemory{nullptr};
    uint32_t m_vertexCount = 0;

    vk::raii::Buffer m_indexBuffer{nullptr};
    vk::raii::DeviceMemory m_indexBufferMemory{nullptr};
    uint32_t m_indexCount = 0;

    std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
};

} // namespace nuff::renderer

