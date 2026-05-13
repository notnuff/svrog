#include "engine.h"

#include "../components/camera_component.h"
#include "../components/mesh_component.h"
#include "../components/transform_component.h"
#include "../entities/entity.h"
#include "../systems/render_system.h"

#include "assets/gltf_loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <utility>

namespace nuff::engine {

Engine::Engine() = default;
Engine::~Engine() { shutdown(); }

void Engine::setRenderContext(renderer::CoreCtx* ctx, renderer::IRenderTarget* target) {
    m_ctx = ctx;
    m_renderTarget = target;
    m_renderer.setContext(ctx);
    m_renderer.setRenderTarget(target);
    if (ctx) m_memoryManager = std::make_unique<renderer::MemoryManager>(*ctx);
}

void Engine::setRecreateCallback(RecreateCallback callback) {
    m_renderer.setRecreateCallback(std::move(callback));
}

void Engine::notifyFramebufferResized() {
    m_renderer.notifyFramebufferResized();
}

void Engine::onTargetResized(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    if (m_activeScene) {
        if (auto* cam = m_activeScene->activeCamera()) {
            if (auto* cc = cam->getComponent<CameraComponent>()) {
                cc->setAspectRatio(aspect);
            }
        }
    }
}

void Engine::initialize() {
    if (m_initialized) return;
    if (m_ctx && m_renderTarget) m_renderer.initialize();
    m_initialized = true;
}

void Engine::shutdown() {
    if (!m_initialized) return;
    if (m_ctx) m_ctx->device.waitIdle();
    m_renderer.cleanup();
    m_activeScene = nullptr;
    m_scenes.clear();
    m_materialSets.clear();
    m_materialPool = nullptr;
    m_textures.clear();
    m_meshes.clear();
    m_memoryManager.reset();
    m_initialized = false;
}

void Engine::tick(float dt) {
    m_deltaTime = dt;
    m_totalTime += dt;
    ++m_frameIndex;

    if (m_activeScene && (!m_paused || m_stepRequested)) {
        m_activeScene->update(dt);
        m_stepRequested = false;
    }

    if (!m_ctx || !m_renderTarget) return;
    const auto frame = m_activeScene
        ? RenderSystem::buildFrameView(*m_activeScene, m_totalTime)
        : renderer::FrameView{};
    m_renderer.drawFrame(frame);
}

void Engine::buildMaterialDescriptorPool(uint32_t materialCount) {
    if (materialCount == 0) return;
    auto& pipeline = m_ctx->component<renderer::PipelineCtxComponent>();

    vk::DescriptorPoolSize poolSize{
        .type = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = materialCount
    };
    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = materialCount,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };
    m_materialPool = vk::raii::DescriptorPool(m_ctx->device, poolInfo);

    std::vector<vk::DescriptorSetLayout> layouts(materialCount, *pipeline.materialSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = *m_materialPool,
        .descriptorSetCount = materialCount,
        .pSetLayouts = layouts.data()
    };
    auto sets = m_ctx->device.allocateDescriptorSets(allocInfo);
    m_materialSets.resize(materialCount);
    for (uint32_t i = 0; i < materialCount; ++i) {
        m_materialSets[i] = *sets[i];
        sets[i].release();
    }
}

void Engine::uploadModelToScene(Scene& scene, const std::string& modelPath) {
    renderer::GltfLoader loader;
    auto loaded = loader.load(modelPath);
    if (loaded.nodes.empty()) return;

    const auto nodeCount = static_cast<uint32_t>(loaded.nodes.size());
    m_meshes.reserve(m_meshes.size() + nodeCount);
    m_textures.reserve(m_textures.size() + nodeCount);
    buildMaterialDescriptorPool(nodeCount);

    Entity* modelRoot = scene.createEntity("Model");
    modelRoot->addComponent<TransformComponent>();

    for (uint32_t i = 0; i < nodeCount; ++i) {
        const auto& node = loaded.nodes[i];

        auto mesh = std::make_unique<renderer::Mesh>();
        mesh->upload(*m_memoryManager, node.mesh);

        auto texture = std::make_unique<renderer::TextureImage>();
        texture->loadFromPixels(*m_ctx, *m_memoryManager,
            node.material.pixels.data(), node.material.width, node.material.height);

        auto info = texture->descriptorInfo();
        vk::WriteDescriptorSet write{
            .dstSet = m_materialSets[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &info
        };
        m_ctx->device.updateDescriptorSets(write, nullptr);

        Entity* nodeEntity = scene.createEntity("Node_" + std::to_string(i), modelRoot);
        auto* xf = nodeEntity->addComponent<TransformComponent>();
        glm::vec3 scale, translation, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(node.transform, scale, rotation, translation, skew, perspective);
        xf->setPosition(translation);
        xf->setRotation(rotation);
        xf->setScale(scale);
        nodeEntity->addComponent<MeshComponent>(mesh.get(), m_materialSets[i]);

        m_meshes.push_back(std::move(mesh));
        m_textures.push_back(std::move(texture));
    }
}

void Engine::loadDefaultScene(const std::string& modelPath) {
    Scene* scene = createScene("Main");

    const auto extent = m_renderTarget ? m_renderTarget->extent() : vk::Extent2D{1, 1};
    const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    Entity* cameraEntity = scene->createEntity("Camera");
    auto* xf = cameraEntity->addComponent<TransformComponent>();
    const glm::vec3 eye{3.0f, 3.0f, 3.0f};
    xf->setPosition(eye);
    xf->setRotation(glm::quatLookAt(glm::normalize(-eye), glm::vec3(0.0f, 0.0f, 1.0f)));
    cameraEntity->addComponent<CameraComponent>(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    scene->setActiveCamera(cameraEntity->id());

    if (!modelPath.empty() && m_memoryManager) {
        uploadModelToScene(*scene, modelPath);
    }
}

Scene* Engine::createScene(std::string name) {
    auto scene = std::make_unique<Scene>(std::move(name));
    Scene* raw = scene.get();
    m_scenes.push_back(std::move(scene));
    if (!m_activeScene) setActiveScene(raw);
    return raw;
}

void Engine::destroyScene(Scene* scene) {
    if (!scene) return;
    if (scene == m_activeScene) setActiveScene(nullptr);
    const auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
        [scene](const std::unique_ptr<Scene>& s){ return s.get() == scene; });
    if (it != m_scenes.end()) m_scenes.erase(it);
}

void Engine::setActiveScene(Scene* scene) {
    if (scene == m_activeScene) return;
    Scene* prev = m_activeScene;
    m_activeScene = scene;
    m_events.publish(ActiveSceneChangedEvent{prev, scene});
}

Scene* Engine::activeScene() const { return m_activeScene; }
const std::vector<std::unique_ptr<Scene>>& Engine::scenes() const { return m_scenes; }

void Engine::setSimulationPaused(bool paused) { m_paused = paused; }
bool Engine::isSimulationPaused() const { return m_paused; }
void Engine::stepOneFrame() { m_stepRequested = true; }

EventBus& Engine::globalEvents() { return m_events; }

uint64_t Engine::frameIndex() const { return m_frameIndex; }
float    Engine::totalTime() const  { return m_totalTime; }
float    Engine::deltaTime() const  { return m_deltaTime; }

} // namespace nuff::engine
