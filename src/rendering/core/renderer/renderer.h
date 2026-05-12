#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "core/context/ctx.h"
#include "core/memory/memory_manager.h"
#include "presentation/i_render_target.h"
#include "primitives/mesh.h"
#include "primitives/texture_image.h"

namespace nuff::renderer {

struct RenderObject {
    Mesh         mesh;
    glm::mat4    transform{1.0f};
    TextureImage texture;
    vk::DescriptorSet materialSet{};
};

class Renderer {
public:
    using RecreateCallback = std::function<void()>;

    void setContext(CoreCtx* ctx);
    void setRenderTarget(IRenderTarget* renderTarget);
    void setRecreateCallback(RecreateCallback callback);
    void setModelPath(const std::string& path);

    void notifyFramebufferResized();

    void initialize();
    void cleanup();

    void drawFrame();

private:
    void recordRendering();
    void updateUniformBuffer();
    void loadFallbackGeometry();
    void loadModel(const std::string& path);
    void buildMaterialDescriptors();

    CoreCtx* m_ctx = nullptr;
    IRenderTarget* m_renderTarget = nullptr;
    RecreateCallback m_recreateCallback;
    std::unique_ptr<MemoryManager> m_memoryManager;
    bool m_framebufferResized = false;

    vk::raii::DescriptorPool m_materialPool{nullptr};
    std::vector<RenderObject> m_objects;
    std::string m_modelPath;

    glm::vec3 m_sceneCentroid{0.0f};
    float     m_sceneRadius = 0.0f;

    std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
};

} // namespace nuff::renderer

