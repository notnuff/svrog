#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/context/ctx.h"
#include "core/memory/memory_manager.h"
#include "primitives/mesh.h"
#include "primitives/texture_image.h"
#include "presentation/i_render_target.h"

namespace nuff::renderer {

class Renderer {
public:
    using RecreateCallback = std::function<void()>;

    void setContext(CoreCtx* ctx);
    void setRenderTarget(IRenderTarget* renderTarget);
    void setRecreateCallback(RecreateCallback callback);
    void setTexturePath(const std::string& path);

    void notifyFramebufferResized();

    void initialize();
    void cleanup();

    void drawFrame();

private:
    void recordRendering();
    void updateUniformBuffer();

    CoreCtx* m_ctx = nullptr;
    IRenderTarget* m_renderTarget = nullptr;
    RecreateCallback m_recreateCallback;
    std::unique_ptr<MemoryManager> m_memoryManager;
    bool m_framebufferResized = false;

    Mesh m_mesh;
    TextureImage m_texture;
    std::string m_texturePath;

    std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
};

} // namespace nuff::renderer

