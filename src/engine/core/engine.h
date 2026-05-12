#pragma once

#include "../events/event_bus.h"
#include "../scene/scene.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nuff::engine {

struct ActiveSceneChangedEvent { Scene* previous; Scene* current; };

class Engine {
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void initialize();
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
    bool m_initialized = false;
    std::vector<std::unique_ptr<Scene>> m_scenes;
    Scene*   m_activeScene = nullptr;
    EventBus m_events;

    bool     m_paused = false;
    bool     m_stepRequested = false;

    uint64_t m_frameIndex = 0;
    float    m_totalTime = 0.0f;
    float    m_deltaTime = 0.0f;
};

} // namespace nuff::engine
