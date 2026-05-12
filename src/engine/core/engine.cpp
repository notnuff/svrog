#include "engine.h"

#include <algorithm>
#include <utility>

namespace nuff::engine {

Engine::Engine() = default;
Engine::~Engine() { shutdown(); }

void Engine::initialize() {
    if (m_initialized) return;
    m_initialized = true;
}

void Engine::shutdown() {
    if (!m_initialized) return;
    m_activeScene = nullptr;
    m_scenes.clear();
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
