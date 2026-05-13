#pragma once

#include "../events/event_bus.h"
#include "../scene/scene.h"

#include "core/context/ctx.h"
#include "core/memory/memory_manager.h"
#include "core/renderer/renderer.h"
#include "presentation/i_render_target.h"
#include "primitives/mesh.h"
#include "primitives/texture_image.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace nuff::engine {

struct ActiveSceneChangedEvent { Scene* previous; Scene* current; };

class Engine {
public:
    using RecreateCallback = std::function<void()>;

    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void setRenderContext(renderer::CoreCtx* ctx, renderer::IRenderTarget* target);
    void setRecreateCallback(RecreateCallback callback);
    void notifyFramebufferResized();
    void onTargetResized(uint32_t width, uint32_t height);

    void initialize();
    void loadDefaultScene(const std::string& modelPath);
    void shutdown();

    void tick(float dt);

    Scene* createScene(std::string name);
    void   destroyScene(Scene* scene);

    void   setActiveScene(Scene* scene);
    Scene* activeScene() const;

    const std::vector<std::unique_ptr<Scene>>& scenes() const;

    void setSimulationPaused(bool paused);
    bool isSimulationPaused() const;
    void stepOneFrame();

    EventBus& globalEvents();

    uint64_t frameIndex() const;
    float    totalTime() const;
    float    deltaTime() const;

private:
    void uploadModelToScene(Scene& scene, const std::string& modelPath);
    void buildMaterialDescriptorPool(uint32_t materialCount);

    bool m_initialized = false;
    std::vector<std::unique_ptr<Scene>> m_scenes;
    Scene*   m_activeScene = nullptr;
    EventBus m_events;

    bool     m_paused = false;
    bool     m_stepRequested = false;

    uint64_t m_frameIndex = 0;
    float    m_totalTime = 0.0f;
    float    m_deltaTime = 0.0f;

    renderer::CoreCtx*        m_ctx = nullptr;
    renderer::IRenderTarget*  m_renderTarget = nullptr;
    renderer::Renderer        m_renderer;
    std::unique_ptr<renderer::MemoryManager> m_memoryManager;

    std::vector<std::unique_ptr<renderer::Mesh>>         m_meshes;
    std::vector<std::unique_ptr<renderer::TextureImage>> m_textures;
    vk::raii::DescriptorPool                             m_materialPool{nullptr};
    std::vector<vk::DescriptorSet>                       m_materialSets;
};

} // namespace nuff::engine
